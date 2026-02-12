// Misc.h: interface for the CMisc namespace.

#pragma once

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)
#include "Platform.h"
#include "CommonTypes.h"
#include "GameGeometry.h"
#include "DirectionHelpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace CMisc
{
	static inline void GetMyCursorPos(short * pX, short * pY)
	{
		POINT point;

		GetCursorPos(&point);
		*pX = (short)point.x;
		*pY = (short)point.y;
	}

	static inline char cGetNextMoveDir(short sX, short sY, short dX, short dY)
	{
		short absX, absY;
		char  cRet = 0;

		absX = sX - dX;
		absY = sY - dY;

		if ((absX == 0) && (absY == 0)) cRet = 0;

		if (absX == 0) {
			if (absY > 0) cRet = 1;
			if (absY < 0) cRet = 5;
		}
		if (absY == 0) {
			if (absX > 0) cRet = 7;
			if (absX < 0) cRet = 3;
		}
		if ( (absX > 0)	&& (absY > 0) ) cRet = 8;
		if ( (absX < 0)	&& (absY > 0) ) cRet = 2;
		if ( (absX > 0)	&& (absY < 0) ) cRet = 6;
		if ( (absX < 0)	&& (absY < 0) ) cRet = 4;

		return cRet;
	}

	static inline void GetPoint(int x0, int y0, int x1, int y1, int * pX, int * pY, int * pError)
	{
		int dx, dy, x_inc, y_inc, error, index;
		int iResultX, iResultY, iDstCnt;

		if ((x0 == x1) && (y0 == y1)) {
			*pX = x0;
			*pY = y0;
			return;
		}

		error = *pError;

		iResultX = x0;
		iResultY = y0;
		iDstCnt  = 0;

		dx = x1-x0;
		dy = y1-y0;

		if(dx>=0)
		{
			x_inc = 1;
		}
		else
		{
			x_inc = -1;
			dx = -dx;
		}

		if(dy>=0)
		{
			y_inc = 1;
		}
		else
		{
			y_inc = -1;
			dy = -dy;
		}

		if(dx>dy)
		{
			for(index = 0; index<=dx; index++)
			{
				error+=dy;
				if(error>dx)
				{
					error-=dx;
					iResultY += y_inc;
				}
				iResultX += x_inc;
				goto CALC_OK;
			}
		}
		else
		{
			for(index=0; index<=dy; index++)
			{
				error+=dx;
				if(error>0)
				{
					error-=dy;
					iResultX += x_inc;
				}
				iResultY += y_inc;
				goto CALC_OK;
			}
		}

	CALC_OK:;

		*pX = iResultX;
		*pY = iResultY;
		*pError = error;
	}

	// Forwards to shared implementation in GameGeometry.h
	static inline void GetPoint2(int x0, int y0, int x1, int y1, int* pX, int* pY, int* pError, int iCount)
	{
		hb::shared::geometry::GetPoint2(x0, y0, x1, y1, pX, pY, pError, iCount);
	}

	static inline void GetDirPoint(char cDir, int * pX, int * pY)
	{
		hb::shared::direction::ApplyOffset(cDir, *pX, *pY);
	}

	static inline bool bCheckValidName(char *pStr)
	{
		size_t iLen = strlen(pStr);
		for(int i = 0; i < iLen; i++)
		{
			if ( (pStr[i] == ',')  || (pStr[i] == '=')  || (pStr[i] == ' ') ||
				 (pStr[i] == '\n') || (pStr[i] == '\t') || /*(pStr[i] == '.') ||*/
				 (pStr[i] == '\\') || (pStr[i] == '/')  || (pStr[i] == ':') ||
				 (pStr[i] == '*')  || (pStr[i] == '?')  || (pStr[i] == '<') ||
				 (pStr[i] == '>')  || (pStr[i] == '|')  || (pStr[i] == '"') ) return false;

			if ((i <= iLen-2) && ((unsigned char)pStr[i] >= 128)) {
				if (((unsigned char)pStr[i] == 164) && ((unsigned char)pStr[i+1] >= 161) &&
					((unsigned char)pStr[i+1] <= 211)) {

				}
				else
				if (((unsigned char)pStr[i] >= 176) && ((unsigned char)pStr[i] <= 200) &&
					((unsigned char)pStr[i+1] >= 161) && ((unsigned char)pStr[i+1] <= 254)) {

				}
				else return false;
				i++;
			}
		}

		return true;
	}

	static inline void Temp()
	{
		FILE * pSrcFile, * pDestFile, * pSrcFileA, * pSrcFileB;
		
		char cTemp[100000];

		pSrcFile = fopen("middleland.amd", "rb");
		pDestFile = fopen("middleland.amd.result", "wb");

		pSrcFileA = fopen("middleland1.amd", "rb");
		pSrcFileB = fopen("middleland2.amd", "rb");

		fread(cTemp, 1, 256, pSrcFile);
		fread(cTemp, 1, 256, pSrcFileA);
		fread(cTemp, 1, 256, pSrcFileB);
		for(int i = 1; i <= 444; i++)
			fread(cTemp, 1, 5240, pSrcFileB);

		std::memset(cTemp, 0, sizeof(cTemp));
		strcpy(cTemp, "MAPSIZEX = 824 MAPSIZEY = 824 TILESIZE = 10");

		fwrite(cTemp, 1, 256, pDestFile);

		for(int i = 1; i <= 80; i++) {
			std::memset(cTemp, 0, sizeof(cTemp));
			fread((cTemp + 1500), 1, 5240, pSrcFileA);
			fwrite(cTemp, 1, 824*10, pDestFile);
		}

		std::memset(cTemp, 0, sizeof(cTemp));
		for(int i = 1; i <= 68; i++) fwrite(cTemp, 1, 824*10, pDestFile);

		//148
		/*
		std::memset(cTemp, 0, sizeof(cTemp));
		for(int i = 1; i <= 150; i++) fwrite(cTemp, 1, 824*10, pDestFile);
		*/

		for(int i = 1; i <= 524; i++) {
			std::memset(cTemp, 0, sizeof(cTemp));
			fread((cTemp + 1500), 1, 5240, pSrcFile);
			fwrite(cTemp, 1, 824*10, pDestFile);
		}

		std::memset(cTemp, 0, sizeof(cTemp));
		for(int i = 1; i <= 68; i++) fwrite(cTemp, 1, 824*10, pDestFile);

		for(int i = 1; i <= 80; i++) {
			std::memset(cTemp, 0, sizeof(cTemp));
			fread((cTemp + 1500), 1, 5240, pSrcFileB);
			fwrite(cTemp, 1, 824*10, pDestFile);
		}

		std::memset(cTemp, 0, sizeof(cTemp));
		for(int i = 1; i <= 2; i++) fwrite(cTemp, 1, 824*10, pDestFile);

		/*
		std::memset(cTemp, 0, sizeof(cTemp));
		for(int i = 1; i <= 150; i++) fwrite(cTemp, 1, 824*10, pDestFile);
		*/

		fclose(pSrcFile);
		fclose(pDestFile);
		fclose(pSrcFileA);
		fclose(pSrcFileB);
	}
}
