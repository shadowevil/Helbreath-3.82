#pragma once

#include <cstddef>

class CItem;

class ItemLog
{
public:
	static ItemLog& Get();
	void Initialize();

	// Split log writers
	void LogDrop(const char* playerName, const char* ip, const char* mapName, int sX, int sY, CItem* pItem);
	void LogPickup(const char* playerName, const char* ip, const char* mapName, int sX, int sY, CItem* pItem);
	void LogTrade(const char* giverName, const char* giverIP, const char* receiverName, const char* mapName, int sX, int sY, CItem* pItem);
	void LogExchange(const char* giverName, const char* giverIP, const char* receiverName, const char* mapName, int sX, int sY, CItem* pItem);
	void LogShop(const char* action, const char* playerName, const char* ip, const char* mapName, int sX, int sY, CItem* pItem);
	void LogCraft(const char* playerName, const char* ip, const char* mapName, int sX, int sY, CItem* pItem);
	void LogUpgrade(bool success, const char* playerName, const char* ip, const char* mapName, int sX, int sY, CItem* pItem);
	void LogBank(const char* action, const char* playerName, const char* ip, const char* mapName, int sX, int sY, CItem* pItem);
	void LogMisc(const char* action, const char* playerName, const char* ip, const char* mapName, int sX, int sY, CItem* pItem);

private:
	ItemLog() = default;
	void WriteToFile(const char* filename, const char* line);
	void FormatItemInfo(char* buf, size_t bufSize, CItem* pItem);
	bool IsItemSuspicious(CItem* pItem);
	bool m_bInitialized = false;
};
