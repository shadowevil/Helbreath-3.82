// ASIOSocket.cpp: Unified socket implementation using standalone ASIO
//
// Replaces the legacy XSocket (Winsock2/WSAEventSelect) with ASIO.
// Shared between Client and Server.
//
// Supports both polling mode (client) and async mode (server).
//
//////////////////////////////////////////////////////////////////////

#include "ASIOSocket.h"
#include <cstdio>
#include <algorithm>

#ifndef _WIN32
// strncpy_s / _TRUNCATE shim for Linux builds
#ifndef _TRUNCATE
#define _TRUNCATE ((size_t)-1)
inline int strncpy_s(char* dst, size_t dstSize, const char* src, size_t count)
{
    if (!dst || dstSize == 0) return 22;
    if (!src) { dst[0] = '\0'; return 22; }
    if (count == _TRUNCATE || count >= dstSize)
        count = dstSize - 1;
    strncpy(dst, src, count);
    dst[count] = '\0';
    return 0;
}
#endif
#endif

namespace hb::shared::net {

namespace sock = socket;
using asio::ip::tcp;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ASIOSocket::ASIOSocket(asio::io_context& ctx, int iBlockLimit)
	: m_ioContext(ctx)
	, m_socket(ctx)
	, m_acceptor(ctx)
	, m_strand(ctx)
	, m_iBlockLimit(iBlockLimit)
{
}

ASIOSocket::~ASIOSocket()
{
	CancelAsync();
	CloseConnection();
}

bool ASIOSocket::bInitBufferSize(size_t dwBufferSize)
{
	m_rcvBuffer.assign(dwBufferSize + 8, 0);
	m_sndBuffer.assign(dwBufferSize + 8, 0);
	m_dwBufferSize = dwBufferSize;
	// Async read buffer: header (3 bytes) + max body
	m_asyncRcvBuffer.assign(dwBufferSize + 8, 0);
	return true;
}

//////////////////////////////////////////////////////////////////////
// Poll - non-blocking event check (replaces WSAEventSelect + iOnSocketEvent)
// Still used by client (manual poll mode) and for connect completion
//////////////////////////////////////////////////////////////////////

int ASIOSocket::Poll()
{
	if (m_cType == 0) return sock::Event::NotInitialized;

	// Process any pending async operations (connect completion)
	m_ioContext.poll();
	m_ioContext.restart();

	// --- Handle async connect completion ---
	if (m_connectResultReady) {
		m_connectResultReady = false;
		m_connectPending = false;

		if (!m_connectError) {
			m_bIsAvailable = true;
			m_bIsWriteEnabled = true;
			return sock::Event::ConnectionEstablish;
		}
		else {
			m_WSAErr = m_connectError.value();
			// Retry connection (same as legacy behavior)
			if (bConnect(m_pAddr, m_iPortNum) == false) {
				return sock::Event::SocketError;
			}
			return sock::Event::RetryingConnection;
		}
	}

	// Still waiting for connect
	if (m_connectPending) return 0;

	// --- Listen socket: probe for pending accept ---
	if (m_cType == sock::Type::Listen) {
		if (m_pendingAcceptSocket.has_value()) {
			// Already have a pending accept from a previous poll
			return sock::Event::ConnectionEstablish;
		}

		asio::error_code ec;
		tcp::socket peer(m_ioContext);
		m_acceptor.accept(peer, ec);
		if (!ec) {
			m_pendingAcceptSocket.emplace(std::move(peer));
			return sock::Event::ConnectionEstablish;
		}
		// would_block = no pending connections
		return 0;
	}

	// --- Normal socket: check for data and handle writes ---
	if (m_cType != sock::Type::Normal) return sock::Event::SocketMismatch;

	int iResult = 0;

	// Check if socket has been closed by peer
	asio::error_code ec;
	size_t avail = m_socket.available(ec);
	if (ec) {
		if (ec == asio::error::eof || ec == asio::error::connection_reset ||
			ec == asio::error::connection_aborted) {
			m_cType = sock::Type::Shutdown;
			return sock::Event::SocketClosed;
		}
		m_WSAErr = ec.value();
		return sock::Event::SocketError;
	}

	// Read available data.
	// NOTE: If _iOnRead() completes a packet (READCOMPLETE), the data sits
	// in m_rcvBuffer. Callers that also use DrainToQueue() MUST check for
	// READCOMPLETE and queue the packet themselves before calling DrainToQueue(),
	// otherwise DrainToQueue() will overwrite m_rcvBuffer and lose the packet.
	if (avail > 0) {
		int readResult = _iOnRead();
		if (readResult != sock::Event::OnRead) {
			iResult = readResult;
		}
	}

	// Try to drain unsent data queue
	if (!m_unsentQueue.empty() && m_bIsWriteEnabled) {
		int writeResult = _iSendUnsentData();
		if (writeResult != sock::Event::UnsentDataSendComplete && iResult == 0) {
			iResult = writeResult;
		}
	}

	return iResult;
}

//////////////////////////////////////////////////////////////////////
// Connect - async non-blocking connect
//////////////////////////////////////////////////////////////////////

bool ASIOSocket::bConnect(const char* pAddr, int iPort)
{
	if (m_cType == sock::Type::Listen) return false;

	// Close existing socket if open
	asio::error_code ec;
	if (m_socket.is_open()) {
		m_socket.close(ec);
	}

	m_socket = tcp::socket(m_ioContext);
	m_socket.open(tcp::v4(), ec);
	if (ec) {
		printf("[ERROR] ASIOSocket::bConnect - open() failed: %s\n", ec.message().c_str());
		return false;
	}

	// Set non-blocking
	m_socket.non_blocking(true, ec);

	// Set socket options
	uint32_t bufSize = hb::shared::limits::MsgBufferSize * 2;
	m_socket.set_option(asio::socket_base::receive_buffer_size(bufSize), ec);
	m_socket.set_option(asio::socket_base::send_buffer_size(bufSize), ec);
	m_socket.set_option(tcp::no_delay(true), ec);

	// Store connection info for reconnect
	strncpy_s(m_pAddr, sizeof(m_pAddr), (pAddr != nullptr) ? pAddr : "", _TRUNCATE);
	m_iPortNum = iPort;

	// Start async connect
	tcp::endpoint endpoint(asio::ip::make_address(pAddr, ec), static_cast<unsigned short>(iPort));
	if (ec) {
		printf("[ERROR] ASIOSocket::bConnect - invalid address '%s': %s\n", pAddr, ec.message().c_str());
		return false;
	}

	m_connectPending = true;
	m_connectResultReady = false;
	m_connectError.clear();

	m_socket.async_connect(endpoint, [this](const asio::error_code& err) {
		m_connectError = err;
		m_connectResultReady = true;
	});

	m_cType = sock::Type::Normal;
	return true;
}

//////////////////////////////////////////////////////////////////////
// BlockConnect - blocking connect with hostname resolution
//////////////////////////////////////////////////////////////////////

bool ASIOSocket::bBlockConnect(char* pAddr, int iPort)
{
	if (m_cType == sock::Type::Listen) return false;

	asio::error_code ec;
	if (m_socket.is_open()) {
		m_socket.close(ec);
	}

	// Resolve hostname
	tcp::resolver resolver(m_ioContext);
	auto results = resolver.resolve(pAddr, std::to_string(iPort), ec);
	if (ec || results.empty()) {
		return false;
	}

	m_socket = tcp::socket(m_ioContext);

	// Blocking connect to first resolved endpoint
	asio::connect(m_socket, results, ec);
	if (ec) {
		m_WSAErr = ec.value();
		return false;
	}

	// Set socket options
	uint32_t bufSize = hb::shared::limits::MsgBufferSize * 2;
	m_socket.set_option(asio::socket_base::receive_buffer_size(bufSize), ec);
	m_socket.set_option(asio::socket_base::send_buffer_size(bufSize), ec);
	m_socket.set_option(tcp::no_delay(true), ec);

	// Set non-blocking after connect completes
	m_socket.non_blocking(true, ec);

	strncpy_s(m_pAddr, sizeof(m_pAddr), (pAddr != nullptr) ? pAddr : "", _TRUNCATE);
	m_iPortNum = iPort;

	m_cType = sock::Type::Normal;
	m_bIsAvailable = true;
	m_bIsWriteEnabled = true;
	return true;
}

//////////////////////////////////////////////////////////////////////
// Listen - set up listening acceptor
//////////////////////////////////////////////////////////////////////

bool ASIOSocket::bListen(char* pAddr, int iPort)
{
	if (m_cType != 0) return false;

	asio::error_code ec;

	tcp::endpoint endpoint(asio::ip::make_address(pAddr, ec), static_cast<unsigned short>(iPort));
	if (ec) return false;

	m_acceptor.open(tcp::v4(), ec);
	if (ec) return false;

	m_acceptor.set_option(asio::socket_base::reuse_address(true), ec);

	m_acceptor.bind(endpoint, ec);
	if (ec) {
		m_acceptor.close();
		return false;
	}

	m_acceptor.listen(5, ec);
	if (ec) {
		m_acceptor.close();
		return false;
	}

	// Set non-blocking so Poll() can probe for accepts
	m_acceptor.non_blocking(true, ec);

	m_cType = sock::Type::Listen;
	return true;
}

//////////////////////////////////////////////////////////////////////
// Accept - accept pending connection into target socket (polling mode)
//////////////////////////////////////////////////////////////////////

bool ASIOSocket::bAccept(ASIOSocket* pSock)
{
	if (m_cType != sock::Type::Listen) return false;
	if (pSock == nullptr) return false;

	asio::error_code ec;

	// Use pending socket from Poll() if available
	if (m_pendingAcceptSocket.has_value()) {
		if (pSock->m_socket.is_open()) {
			pSock->m_socket.close(ec);
		}
		pSock->m_socket = std::move(m_pendingAcceptSocket.value());
		m_pendingAcceptSocket.reset();
	}
	else {
		// Try direct non-blocking accept
		if (pSock->m_socket.is_open()) {
			pSock->m_socket.close(ec);
		}
		pSock->m_socket = tcp::socket(m_ioContext);
		m_acceptor.accept(pSock->m_socket, ec);
		if (ec) return false;
	}

	// Configure accepted socket
	pSock->m_socket.non_blocking(true, ec);

	uint32_t bufSize = hb::shared::limits::MsgBufferSize * 2;
	pSock->m_socket.set_option(asio::socket_base::receive_buffer_size(bufSize), ec);
	pSock->m_socket.set_option(asio::socket_base::send_buffer_size(bufSize), ec);
	pSock->m_socket.set_option(tcp::no_delay(true), ec);

	pSock->m_cType = sock::Type::Normal;
	pSock->m_bIsWriteEnabled = true;
	return true;
}

//////////////////////////////////////////////////////////////////////
// bAcceptFromSocket - accept a pre-connected socket (async accept path)
//////////////////////////////////////////////////////////////////////

bool ASIOSocket::bAcceptFromSocket(asio::ip::tcp::socket&& peer)
{
	asio::error_code ec;

	if (m_socket.is_open()) {
		m_socket.close(ec);
	}

	m_socket = std::move(peer);

	// Configure accepted socket
	m_socket.non_blocking(true, ec);

	uint32_t bufSize = hb::shared::limits::MsgBufferSize * 2;
	m_socket.set_option(asio::socket_base::receive_buffer_size(bufSize), ec);
	m_socket.set_option(asio::socket_base::send_buffer_size(bufSize), ec);
	m_socket.set_option(tcp::no_delay(true), ec);

	m_cType = sock::Type::Normal;
	m_bIsWriteEnabled = true;
	m_bIsAvailable = true;
	return true;
}

//////////////////////////////////////////////////////////////////////
// _iOnRead - read state machine (header then body) - polling mode
//////////////////////////////////////////////////////////////////////

int ASIOSocket::_iOnRead()
{
	asio::error_code ec;

	if (m_cStatus == sock::Status::ReadingHeader) {
		size_t n = m_socket.read_some(
			asio::buffer(m_rcvBuffer.data() + m_dwTotalReadSize, m_dwReadSize), ec);

		if (ec == asio::error::would_block) return sock::Event::Block;
		if (ec || n == 0) {
			if (ec == asio::error::eof || ec == asio::error::connection_reset || n == 0) {
				m_cType = sock::Type::Shutdown;
				return sock::Event::SocketClosed;
			}
			m_WSAErr = ec.value();
			return sock::Event::SocketError;
		}

		m_dwReadSize -= static_cast<uint32_t>(n);
		m_dwTotalReadSize += static_cast<uint32_t>(n);

		if (m_dwReadSize == 0) {
			m_cStatus = sock::Status::ReadingBody;
			uint16_t* wp = reinterpret_cast<uint16_t*>(m_rcvBuffer.data() + 1);
			m_dwReadSize = static_cast<uint32_t>(*wp - 3);

			if (m_dwReadSize == 0) {
				m_cStatus = sock::Status::ReadingHeader;
				m_dwReadSize = 3;
				m_dwTotalReadSize = 0;
				return sock::Event::ReadComplete;
			}
			else if (m_dwReadSize > m_dwBufferSize) {
				m_cStatus = sock::Status::ReadingHeader;
				m_dwReadSize = 3;
				m_dwTotalReadSize = 0;
				return sock::Event::MsgSizeTooLarge;
			}
		}
		return sock::Event::OnRead;
	}
	else if (m_cStatus == sock::Status::ReadingBody) {
		size_t n = m_socket.read_some(
			asio::buffer(m_rcvBuffer.data() + m_dwTotalReadSize, m_dwReadSize), ec);

		if (ec == asio::error::would_block) return sock::Event::Block;
		if (ec || n == 0) {
			if (ec == asio::error::eof || ec == asio::error::connection_reset || n == 0) {
				m_cType = sock::Type::Shutdown;
				return sock::Event::SocketClosed;
			}
			m_WSAErr = ec.value();
			return sock::Event::SocketError;
		}

		m_dwReadSize -= static_cast<uint32_t>(n);
		m_dwTotalReadSize += static_cast<uint32_t>(n);

		if (m_dwReadSize == 0) {
			m_cStatus = sock::Status::ReadingHeader;
			m_dwReadSize = 3;
			m_dwTotalReadSize = 0;
		}
		else {
			return sock::Event::OnRead;
		}
	}

	return sock::Event::ReadComplete;
}

//////////////////////////////////////////////////////////////////////
// _iSend - send with unsent queue fallback (polling mode)
//////////////////////////////////////////////////////////////////////

int ASIOSocket::_iSend(const char* cData, size_t iSize, bool bSaveFlag)
{
	// If there's queued data, queue this too to preserve ordering
	if (!m_unsentQueue.empty()) {
		if (bSaveFlag) {
			if (!_iRegisterUnsentData(cData, iSize)) {
				return sock::Event::QueueFull;
			}
			return sock::Event::Block;
		}
		else {
			return 0;
		}
	}

	int iOutLen = 0;
	while (iOutLen < iSize) {
		asio::error_code ec;
		size_t n = m_socket.write_some(
			asio::buffer(cData + iOutLen, iSize - iOutLen), ec);

		if (ec) {
			if (ec == asio::error::would_block) {
				m_bIsWriteEnabled = false;
				if (bSaveFlag) {
					if (!_iRegisterUnsentData(cData + iOutLen, iSize - iOutLen)) {
						return sock::Event::QueueFull;
					}
				}
				return sock::Event::Block;
			}
			m_WSAErr = ec.value();
			return sock::Event::SocketError;
		}
		iOutLen += static_cast<int>(n);
	}

	return iOutLen;
}

//////////////////////////////////////////////////////////////////////
// _iSend_ForInternalUse - send without queueing (for draining unsent queue)
//////////////////////////////////////////////////////////////////////

int ASIOSocket::_iSend_ForInternalUse(const char* cData, size_t iSize)
{
	int iOutLen = 0;
	while (iOutLen < iSize) {
		asio::error_code ec;
		size_t n = m_socket.write_some(
			asio::buffer(cData + iOutLen, iSize - iOutLen), ec);

		if (ec) {
			if (ec == asio::error::would_block) {
				return iOutLen;
			}
			m_WSAErr = ec.value();
			return sock::Event::SocketError;
		}
		iOutLen += static_cast<int>(n);
	}
	return iOutLen;
}

//////////////////////////////////////////////////////////////////////
// _iRegisterUnsentData - queue data for later sending
//////////////////////////////////////////////////////////////////////

bool ASIOSocket::_iRegisterUnsentData(const char* cData, size_t iSize)
{
	if (static_cast<int>(m_unsentQueue.size()) >= m_iBlockLimit) {
		printf("[ASIO] Queue full! Dropped packet. Queue size: %d, limit: %d\n",
			static_cast<int>(m_unsentQueue.size()), m_iBlockLimit);
		return false;
	}

	m_unsentQueue.emplace_back(cData, iSize);
	return true;
}

//////////////////////////////////////////////////////////////////////
// _iSendUnsentData - drain the unsent queue
//////////////////////////////////////////////////////////////////////

int ASIOSocket::_iSendUnsentData()
{
	while (!m_unsentQueue.empty()) {
		auto& block = m_unsentQueue.front();
		int iRet = _iSend_ForInternalUse(block.remaining(), block.remainingSize());

		if (iRet == block.remainingSize()) {
			// Entire block sent
			m_unsentQueue.pop_front();
		}
		else if (iRet >= 0) {
			// Partial send - advance offset, wait for next writable
			block.offset += iRet;
			m_bIsWriteEnabled = false;
			return sock::Event::UnsentDataSendBlock;
		}
		else {
			// Error
			return iRet;
		}
	}

	return sock::Event::UnsentDataSendComplete;
}

//////////////////////////////////////////////////////////////////////
// iSendMsg - send message with protocol header and optional encryption
// (synchronous polling mode - used by client and polling server path)
//////////////////////////////////////////////////////////////////////

int ASIOSocket::iSendMsg(char* cData, size_t dwSize, char cKey)
{
	if (dwSize > m_dwBufferSize) return sock::Event::MsgSizeTooLarge;
	if (m_cType != sock::Type::Normal) return sock::Event::SocketMismatch;
	if (m_cType == 0) return sock::Event::NotInitialized;

	// If async mode is active, delegate to async path
	if (m_bAsyncMode.load(std::memory_order_relaxed)) {
		return iSendMsgAsync(cData, dwSize, cKey);
	}

	// Build protocol message: [key:1][size:2][payload:N]
	m_sndBuffer[0] = cKey;

	uint16_t* wp = reinterpret_cast<uint16_t*>(m_sndBuffer.data() + 1);
	*wp = static_cast<uint16_t>(dwSize + 3);

	std::memcpy(m_sndBuffer.data() + 3, cData, dwSize);

	// Encrypt payload if key is set
	if (cKey != 0) {
		for (uint32_t i = 0; i < dwSize; i++) {
			m_sndBuffer[3 + i] += static_cast<char>(i ^ cKey);
			m_sndBuffer[3 + i] = static_cast<char>(m_sndBuffer[3 + i] ^ (cKey ^ static_cast<char>(dwSize - i)));
		}
	}

	int iRet;
	if (m_bIsWriteEnabled == false) {
		if (!_iRegisterUnsentData(m_sndBuffer.data(), dwSize + 3)) {
			return sock::Event::QueueFull;
		}
		iRet = static_cast<int>(dwSize + 3);
	}
	else {
		iRet = _iSend(m_sndBuffer.data(), dwSize + 3, true);
	}

	if (iRet < 0) return iRet;
	else return (iRet - 3);
}

//////////////////////////////////////////////////////////////////////
// iSendMsgAsync - async send (posts to strand, builds frame per-message)
// Called from game thread. Errors come via error callback.
//////////////////////////////////////////////////////////////////////

int ASIOSocket::iSendMsgAsync(char* cData, size_t dwSize, char cKey)
{
	if (dwSize > m_dwBufferSize) return sock::Event::MsgSizeTooLarge;
	if (m_cType != sock::Type::Normal) return sock::Event::SocketMismatch;

	// Build frame into a per-message vector
	std::vector<char> frame(dwSize + 3);
	frame[0] = cKey;

	uint16_t* wp = reinterpret_cast<uint16_t*>(frame.data() + 1);
	*wp = static_cast<uint16_t>(dwSize + 3);

	std::memcpy(frame.data() + 3, cData, dwSize);

	// Encrypt payload if key is set
	if (cKey != 0) {
		for (uint32_t i = 0; i < dwSize; i++) {
			frame[3 + i] += static_cast<char>(i ^ cKey);
			frame[3 + i] = static_cast<char>(frame[3 + i] ^ (cKey ^ static_cast<char>(dwSize - i)));
		}
	}

	// Post to strand to serialize with other writes
	asio::post(m_strand, [this, buf = std::move(frame)]() mutable {
		bool wasEmpty = m_asyncWriteQueue.empty();
		if (m_asyncWriteQueue.size() >= MAX_ASYNC_WRITE_QUEUE) {
			// Queue full - invoke error callback
			if (m_onError) {
				m_onError(m_iSocketIndex, sock::Event::QueueFull);
			}
			return;
		}
		m_asyncWriteQueue.push_back(std::move(buf));
		if (wasEmpty && !m_bAsyncWriteInProgress) {
			_DoAsyncWrite();
		}
	});

	// Return optimistic success
	return static_cast<int>(dwSize);
}

//////////////////////////////////////////////////////////////////////
// _DoAsyncWrite - drain async write queue (runs on strand)
//////////////////////////////////////////////////////////////////////

void ASIOSocket::_DoAsyncWrite()
{
	if (m_asyncWriteQueue.empty()) {
		m_bAsyncWriteInProgress = false;
		return;
	}

	m_bAsyncWriteInProgress = true;
	auto& front = m_asyncWriteQueue.front();

	asio::async_write(m_socket, asio::buffer(front.data(), front.size()),
		asio::bind_executor(m_strand,
			[this](const asio::error_code& ec, std::size_t /*bytesWritten*/) {
				if (ec) {
					m_bAsyncWriteInProgress = false;
					if (m_onError) {
						m_onError(m_iSocketIndex, sock::Event::SocketError);
					}
					return;
				}
				m_asyncWriteQueue.pop_front();
				_DoAsyncWrite();
			}));
}

//////////////////////////////////////////////////////////////////////
// iSendMsgBlockingMode - blocking send for shutdown phase
//////////////////////////////////////////////////////////////////////

int ASIOSocket::iSendMsgBlockingMode(char* buf, int nbytes)
{
	int nleft = nbytes;
	while (nleft > 0) {
		asio::error_code ec;
		// Temporarily set blocking for this operation
		m_socket.non_blocking(false, ec);
		size_t nwritten = m_socket.write_some(asio::buffer(buf, nleft), ec);
		m_socket.non_blocking(true, ec);

		if (ec) return -1;
		if (nwritten == 0) break;
		nleft -= static_cast<int>(nwritten);
		buf += nwritten;
	}
	return (nbytes - nleft);
}

//////////////////////////////////////////////////////////////////////
// pGetRcvDataPointer - get received message with decryption (polling mode)
//////////////////////////////////////////////////////////////////////

char* ASIOSocket::pGetRcvDataPointer(size_t* pMsgSize, char* pKey)
{
	char cKey = m_rcvBuffer[0];
	if (pKey != nullptr) *pKey = cKey;

	uint16_t* wp = reinterpret_cast<uint16_t*>(m_rcvBuffer.data() + 1);
	*pMsgSize = (*wp) - 3;
	size_t dwSize = (*wp) - 3;

	if (dwSize > hb::shared::limits::MsgBufferSize) dwSize = hb::shared::limits::MsgBufferSize;

	// Decrypt payload if key is set
	if (cKey != 0) {
		for (size_t i = 0; i < dwSize; i++) {
			m_rcvBuffer[3 + i] = static_cast<char>(m_rcvBuffer[3 + i] ^ (cKey ^ static_cast<char>(dwSize - i)));
			m_rcvBuffer[3 + i] -= static_cast<char>(i ^ cKey);
		}
	}
	return (m_rcvBuffer.data() + 3);
}

//////////////////////////////////////////////////////////////////////
// DrainToQueue - read all available packets into the receive queue
//////////////////////////////////////////////////////////////////////

int ASIOSocket::DrainToQueue()
{
	int iPacketsQueued = 0;
	constexpr int MAX_DRAIN_PER_CALL = 300;

	while (iPacketsQueued < MAX_DRAIN_PER_CALL &&
		m_RecvQueue.size() < MAX_QUEUE_SIZE)
	{
		int iRet = _iOnRead();

		switch (iRet) {
		case sock::Event::ReadComplete:
		{
			size_t dwSize = 0;
			char* pData = pGetRcvDataPointer(&dwSize);

			if (pData != nullptr && dwSize > 0) {
				m_RecvQueue.emplace_back(pData, dwSize);
				iPacketsQueued++;
			}
		}
		break;

		case sock::Event::Block:
			return iPacketsQueued;

		case sock::Event::SocketError:
		case sock::Event::MsgSizeTooLarge:
			return -1;
		case sock::Event::SocketClosed:
			// If we already queued packets before the close, return them
			// so the caller can process the response before handling disconnect.
			return (iPacketsQueued > 0) ? iPacketsQueued : -1;

		case sock::Event::OnRead:
			break;

		default:
			break;
		}
	}

	return iPacketsQueued;
}

bool ASIOSocket::PeekPacket(NetworkPacket& outPacket) const
{
	if (m_RecvQueue.empty()) return false;
	outPacket = m_RecvQueue.front();
	return true;
}

bool ASIOSocket::PopPacket()
{
	if (m_RecvQueue.empty()) return false;
	m_RecvQueue.pop_front();
	return true;
}

//////////////////////////////////////////////////////////////////////
// Connection info
//////////////////////////////////////////////////////////////////////

int ASIOSocket::iGetPeerAddress(char* pAddrString)
{
	asio::error_code ec;
	auto ep = m_socket.remote_endpoint(ec);
	if (!ec) {
		std::string addr = ep.address().to_string();
		std::snprintf(pAddrString, 46, "%s", addr.c_str()); // 46 = INET6_ADDRSTRLEN
		return 0;
	}
	return -1;
}

NativeSocketHandle ASIOSocket::iGetSocket()
{
	return m_socket.native_handle();
}

//////////////////////////////////////////////////////////////////////
// CloseConnection - graceful shutdown and close
//////////////////////////////////////////////////////////////////////

void ASIOSocket::CloseConnection()
{
	asio::error_code ec;

	if (m_acceptor.is_open()) {
		m_acceptor.close(ec);
	}

	if (!m_socket.is_open()) return;

	// Shutdown send side
	m_socket.shutdown(tcp::socket::shutdown_send, ec);

	// Drain receive buffer
	char cTmp[100];
	for (;;) {
		size_t n = m_socket.read_some(asio::buffer(cTmp, sizeof(cTmp)), ec);
		if (ec || n == 0) break;
	}

	m_socket.close(ec);
	m_cType = sock::Type::Shutdown;
}

//////////////////////////////////////////////////////////////////////
// CancelAsync - cancel all pending async operations
//////////////////////////////////////////////////////////////////////

void ASIOSocket::CancelAsync()
{
	m_bAsyncMode.store(false, std::memory_order_relaxed);

	asio::error_code ec;
	if (m_socket.is_open()) {
		m_socket.cancel(ec);
	}
	if (m_acceptor.is_open()) {
		m_acceptor.cancel(ec);
	}
}

//////////////////////////////////////////////////////////////////////
// SetCallbacks - set async message and error callbacks
//////////////////////////////////////////////////////////////////////

void ASIOSocket::SetCallbacks(MessageCallback onMessage, ErrorCallback onError)
{
	m_onMessage = std::move(onMessage);
	m_onError = std::move(onError);
}

//////////////////////////////////////////////////////////////////////
// StartAsyncRead - begin async read chain (header -> body -> callback -> repeat)
//////////////////////////////////////////////////////////////////////

void ASIOSocket::StartAsyncRead()
{
	m_bAsyncMode.store(true, std::memory_order_relaxed);
	_DoAsyncReadHeader();
}

void ASIOSocket::_DoAsyncReadHeader()
{
	// Read exactly 3 bytes: [key:1][size:2]
	asio::async_read(m_socket, asio::buffer(m_asyncRcvBuffer.data(), 3),
		asio::bind_executor(m_strand,
			[this](const asio::error_code& ec, std::size_t /*bytesRead*/) {
				if (ec) {
					if (m_onError) {
						int errCode = sock::Event::SocketError;
						if (ec == asio::error::eof || ec == asio::error::connection_reset ||
							ec == asio::error::connection_aborted || ec == asio::error::operation_aborted) {
							errCode = sock::Event::SocketClosed;
						}
						m_onError(m_iSocketIndex, errCode);
					}
					return;
				}

				// Parse header: byte 0 = key, bytes 1-2 = total size (including header)
				uint16_t* wp = reinterpret_cast<uint16_t*>(m_asyncRcvBuffer.data() + 1);
				size_t totalSize = static_cast<size_t>(*wp);

				if (totalSize <= 3) {
					// Header-only message (no body)
					char cKey = m_asyncRcvBuffer[0];
					if (m_onMessage) {
						m_onMessage(m_iSocketIndex, nullptr, 0, cKey);
					}
					_DoAsyncReadHeader();
					return;
				}

				size_t bodySize = totalSize - 3;
				if (bodySize > m_dwBufferSize) {
					// Message too large
					if (m_onError) {
						m_onError(m_iSocketIndex, sock::Event::MsgSizeTooLarge);
					}
					return;
				}

				_DoAsyncReadBody(bodySize);
			}));
}

void ASIOSocket::_DoAsyncReadBody(size_t bodySize)
{
	// Read body into buffer starting at offset 3 (after header)
	asio::async_read(m_socket, asio::buffer(m_asyncRcvBuffer.data() + 3, bodySize),
		asio::bind_executor(m_strand,
			[this, bodySize](const asio::error_code& ec, std::size_t /*bytesRead*/) {
				if (ec) {
					if (m_onError) {
						int errCode = sock::Event::SocketError;
						if (ec == asio::error::eof || ec == asio::error::connection_reset ||
							ec == asio::error::connection_aborted || ec == asio::error::operation_aborted) {
							errCode = sock::Event::SocketClosed;
						}
						m_onError(m_iSocketIndex, errCode);
					}
					return;
				}

				// Decrypt payload
				char cKey = m_asyncRcvBuffer[0];
				if (cKey != 0) {
					for (uint32_t i = 0; i < bodySize; i++) {
						m_asyncRcvBuffer[3 + i] = static_cast<char>(
							m_asyncRcvBuffer[3 + i] ^ (cKey ^ static_cast<char>(bodySize - i)));
						m_asyncRcvBuffer[3 + i] -= static_cast<char>(i ^ cKey);
					}
				}

				// Deliver message via callback
				if (m_onMessage) {
					m_onMessage(m_iSocketIndex, m_asyncRcvBuffer.data() + 3, bodySize, cKey);
				}

				// Chain to next header read
				_DoAsyncReadHeader();
			}));
}

//////////////////////////////////////////////////////////////////////
// StartAsyncAccept - begin async accept loop on listen socket
//////////////////////////////////////////////////////////////////////

void ASIOSocket::StartAsyncAccept(AcceptCallback callback)
{
	if (m_cType != sock::Type::Listen) return;

	m_onAccept = std::move(callback);
	m_bAsyncMode.store(true, std::memory_order_relaxed);

	// Start the accept loop
	auto doAccept = [this]() {
		m_acceptor.async_accept(
			[this](const asio::error_code& ec, tcp::socket peer) {
				if (!ec) {
					if (m_onAccept) {
						m_onAccept(std::move(peer));
					}
				}
				else if (ec == asio::error::operation_aborted) {
					return; // Shutting down
				}
				// Continue accepting (even after errors like too many open files)
				if (m_bAsyncMode.load(std::memory_order_relaxed)) {
					StartAsyncAccept(m_onAccept);
				}
			});
	};
	doAccept();
}

} // namespace hb::shared::net
