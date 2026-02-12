#include "LocalCacheManager.h"
#include "CRC32.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>

LocalCacheManager& LocalCacheManager::Get()
{
	static LocalCacheManager instance;
	return instance;
}

void LocalCacheManager::Initialize()
{
	std::filesystem::create_directory("cache");
	for (int i = 0; i < static_cast<int>(ConfigCacheType::COUNT); i++) {
		m_state[i] = {};
		m_accum[i] = {};
		_LoadHeader(static_cast<ConfigCacheType>(i));
	}
}

void LocalCacheManager::Shutdown()
{
	for (int i = 0; i < static_cast<int>(ConfigCacheType::COUNT); i++) {
		m_state[i] = {};
		m_accum[i].data.clear();
		m_accum[i].active = false;
	}
}

bool LocalCacheManager::HasCache(ConfigCacheType type) const
{
	return m_state[static_cast<int>(type)].hasCache;
}

uint32_t LocalCacheManager::GetHash(ConfigCacheType type) const
{
	return m_state[static_cast<int>(type)].hash;
}

void LocalCacheManager::AccumulatePacket(ConfigCacheType type, const char* pData, uint32_t size)
{
	if (m_bIsReplaying) return;

	auto& acc = m_accum[static_cast<int>(type)];
	acc.active = true;

	uint16_t len = static_cast<uint16_t>(size);
	const uint8_t* lenBytes = reinterpret_cast<const uint8_t*>(&len);
	acc.data.push_back(lenBytes[0]);
	acc.data.push_back(lenBytes[1]);
	acc.data.insert(acc.data.end(),
		reinterpret_cast<const uint8_t*>(pData),
		reinterpret_cast<const uint8_t*>(pData) + size);
}

bool LocalCacheManager::FinalizeAndSave(ConfigCacheType type)
{
	if (m_bIsReplaying) return false;

	int idx = static_cast<int>(type);
	auto& acc = m_accum[idx];
	if (!acc.active || acc.data.empty()) return false;

	uint32_t hash = _ComputePayloadHash(acc.data);

	CacheHeader hdr{};
	hdr.magic = CACHE_MAGIC;
	hdr.version = CACHE_VERSION;
	hdr.configType = static_cast<uint32_t>(type);
	hdr.crc32 = hash;
	hdr.payloadSize = static_cast<uint32_t>(acc.data.size());

	std::ofstream file(_GetFilename(type), std::ios::binary);
	if (!file) return false;

	file.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));
	file.write(reinterpret_cast<const char*>(acc.data.data()), acc.data.size());

	if (!file) return false;

	m_state[idx].hasCache = true;
	m_state[idx].hash = hash;

	acc.data.clear();
	acc.active = false;

	return true;
}

bool LocalCacheManager::ReplayFromCache(ConfigCacheType type, PacketCallback cb, void* ctx)
{
	m_bIsReplaying = true;

	std::ifstream file(_GetFilename(type), std::ios::binary);
	if (!file) {
		m_bIsReplaying = false;
		return false;
	}

	CacheHeader hdr{};
	if (!file.read(reinterpret_cast<char*>(&hdr), sizeof(hdr))) {
		m_bIsReplaying = false;
		return false;
	}

	if (hdr.magic != CACHE_MAGIC || hdr.version != CACHE_VERSION) {
		m_bIsReplaying = false;
		return false;
	}

	std::vector<uint8_t> payload(hdr.payloadSize);
	if (!file.read(reinterpret_cast<char*>(payload.data()), hdr.payloadSize)) {
		m_bIsReplaying = false;
		return false;
	}

	uint32_t check = hb::shared::util::hb_crc32(payload.data(), payload.size());
	if (check != hdr.crc32) {
		m_bIsReplaying = false;
		return false;
	}

	size_t offset = 0;
	while (offset + 2 <= payload.size()) {
		uint16_t pktLen = 0;
		std::memcpy(&pktLen, payload.data() + offset, 2);
		offset += 2;

		if (offset + pktLen > payload.size()) break;

		if (!cb(reinterpret_cast<char*>(payload.data() + offset), pktLen, ctx)) {
			m_bIsReplaying = false;
			return false;
		}
		offset += pktLen;
	}

	m_bIsReplaying = false;
	return true;
}

void LocalCacheManager::ResetAccumulator(ConfigCacheType type)
{
	auto& acc = m_accum[static_cast<int>(type)];
	acc.data.clear();
	acc.active = false;
}

const char* LocalCacheManager::_GetFilename(ConfigCacheType type) const
{
	switch (type) {
	case ConfigCacheType::Items:  return "cache/{7A3F8B2E-4D1C-9E5A-B6F0-2C8D4E1A3B5F}.bin";
	case ConfigCacheType::Magic:  return "cache/{D9E2A1C4-8F37-4B6D-A5C0-1E9F3D7B2A4C}.bin";
	case ConfigCacheType::Skills: return "cache/{B4C8E6F1-2A5D-4739-8E1B-6F0C3D9A5E2B}.bin";
	case ConfigCacheType::Npcs:   return "cache/{E3A7F5D2-1B8C-4E6A-9D0F-5C2B7A4E8F1D}.bin";
	default: return "";
	}
}

bool LocalCacheManager::_LoadHeader(ConfigCacheType type)
{
	int idx = static_cast<int>(type);
	m_state[idx].hasCache = false;
	m_state[idx].hash = 0;

	std::ifstream file(_GetFilename(type), std::ios::binary);
	if (!file) return false;

	CacheHeader hdr{};
	if (!file.read(reinterpret_cast<char*>(&hdr), sizeof(hdr))) {
		return false;
	}

	if (hdr.magic != CACHE_MAGIC || hdr.version != CACHE_VERSION) {
		return false;
	}

	if (hdr.configType != static_cast<uint32_t>(type)) {
		return false;
	}

	m_state[idx].hasCache = true;
	m_state[idx].hash = hdr.crc32;
	return true;
}

uint32_t LocalCacheManager::_ComputePayloadHash(const std::vector<uint8_t>& data) const
{
	return hb::shared::util::hb_crc32(data.data(), data.size());
}
