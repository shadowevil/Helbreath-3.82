// GameMonitor.cpp: implementation of the CGameMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "GameMonitor.h"
#include "CommonTypes.h"
using namespace hb::client::config;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGameMonitor::CGameMonitor()
{
	int i;

	for (i = 0; i < MaxBadWord; i++)
		m_pWordList[i] = 0;
}

CGameMonitor::~CGameMonitor()
{
	int i;

	for (i = 0; i < MaxBadWord; i++)
		if (m_pWordList[i] != 0) delete m_pWordList[i];
}

int CGameMonitor::iReadBadWordFileList(char* pFn)
{
	char* pContents, * token;
	char seps[] = "/,\t\n";
	char cReadMode = 0;
	int  iIndex = 0;
	FILE* pFile;
	uint32_t dwFileSize;

	// Get file size using standard C
	pFile = fopen(pFn, "rb");
	if (!pFile) return 0;
	fseek(pFile, 0, SEEK_END);
	dwFileSize = static_cast<uint32_t>(ftell(pFile));
	fclose(pFile);

	pFile = fopen(pFn, "rt");
	if (pFile == 0) return 0;
	else
	{
		pContents = new char[dwFileSize + 1];
		std::memset(pContents, 0, dwFileSize + 1);
		fread(pContents, dwFileSize, 1, pFile);
		fclose(pFile);
	}
	token = strtok(pContents, seps);
	while (token != 0)
	{
		m_pWordList[iIndex] = new class CMsg(0, token, 0);
		iIndex++;
		if (iIndex >= MaxBadWord) break;
		token = strtok(NULL, seps);
	}
	delete[] pContents;
	return iIndex;
}

bool CGameMonitor::bCheckBadWord(char* pWord)
{
#ifndef _DEBUG
	int i;
	char cBuffer[500];
	std::memset(cBuffer, 0, sizeof(cBuffer));
	strcpy(cBuffer, pWord);
	i = 0;
	while ((m_pWordList[i] != 0) && (strlen(m_pWordList[i]->m_pMsg) != 0))
	{
		if (memcmp(cBuffer, m_pWordList[i]->m_pMsg, strlen(m_pWordList[i]->m_pMsg)) == 0)
		{
			return true;
		}
		i++;
	}
	return false;
#else
	return false;
#endif
}
