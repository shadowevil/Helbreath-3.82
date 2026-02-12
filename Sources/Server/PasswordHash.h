#pragma once

#include "Platform.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cstdio>
#include <cstring>


namespace PasswordHash
{
constexpr int SaltHexLen = 33;  // 16 bytes = 32 hex chars + null
constexpr int HashHexLen = 65;  // 32 bytes = 64 hex chars + null

	inline void BytesToHex(const unsigned char* bytes, size_t len, char* outHex, size_t outSize)
	{
		for (size_t i = 0; i < len && (i * 2 + 2) < outSize; i++) {
			std::snprintf(outHex + i * 2, 3, "%02x", bytes[i]);
		}
	}

	inline bool HexToBytes(const char* hex, unsigned char* outBytes, size_t outLen)
	{
		for (size_t i = 0; i < outLen; i++) {
			unsigned int val = 0;
			if (std::sscanf(hex + i * 2, "%02x", &val) != 1) {
				return false;
			}
			outBytes[i] = static_cast<unsigned char>(val);
		}
		return true;
	}

	inline bool GenerateSalt(char* outSaltHex, size_t outSize)
	{
		if (outSize < SaltHexLen) return false;

		unsigned char saltBytes[16] = {};
		if (RAND_bytes(saltBytes, sizeof(saltBytes)) != 1) return false;

		std::memset(outSaltHex, 0, outSize);
		BytesToHex(saltBytes, sizeof(saltBytes), outSaltHex, outSize);
		return true;
	}

	inline bool HashPassword(const char* password, const char* saltHex, char* outHashHex, size_t outSize)
	{
		if (outSize < HashHexLen) return false;

		// Construct input: saltHex + password
		char input[256] = {};
		std::snprintf(input, sizeof(input), "%s%s", saltHex, password);
		size_t inputLen = std::strlen(input);

		unsigned char hashResult[32] = {};
		unsigned int hashLen = 0;

		if (EVP_Digest(input, inputLen, hashResult, &hashLen, EVP_sha256(), nullptr) != 1) {
			OPENSSL_cleanse(input, sizeof(input));
			return false;
		}

		std::memset(outHashHex, 0, outSize);
		BytesToHex(hashResult, sizeof(hashResult), outHashHex, outSize);
		OPENSSL_cleanse(input, sizeof(input));
		return true;
	}

	inline bool VerifyPassword(const char* password, const char* saltHex, const char* storedHashHex)
	{
		char computedHash[HashHexLen] = {};
		if (!HashPassword(password, saltHex, computedHash, sizeof(computedHash))) {
			return false;
		}
		return std::strcmp(computedHash, storedHashHex) == 0;
	}
}
