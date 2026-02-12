// Misc.h: Header-only implementation for CMisc utility functions.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "PlatformCompat.h"
#include <stdio.h>
#include <stdlib.h>
#include "GlobalDef.h"

enum {CODE_ENG,CODE_HAN1,CODE_HAN2};

namespace CMisc
{
	// Movement direction calculation using asymmetric zones (N/S 3:1, E/W 4:1 ratio)
	// Returns direction 1-8 (N, NE, E, SE, S, SW, W, NW) or 0 if same position
	static inline char cGetNextMoveDir(short sX, short sY, short dX, short dY)
	{
		short diffX = dX - sX;
		short diffY = dY - sY;

		if (diffX == 0 && diffY == 0) return 0;

		short absX = (diffX < 0) ? -diffX : diffX;
		short absY = (diffY < 0) ? -diffY : diffY;

		if (absY == 0) {
			return (diffX > 0) ? 3 : 7;  // East or West
		}
		if (absX == 0) {
			return (diffY < 0) ? 1 : 5;  // North or South
		}

		// Asymmetric ratios: N/S uses 3:1, E/W uses 4:1
		if (absY >= absX * 3) {
			return (diffY < 0) ? 1 : 5;  // North or South
		}
		if (absX >= absY * 4) {
			return (diffX > 0) ? 3 : 7;  // East or West
		}

		// Diagonal
		if (diffX > 0 && diffY < 0) return 2;  // NE
		if (diffX > 0 && diffY > 0) return 4;  // SE
		if (diffX < 0 && diffY > 0) return 6;  // SW
		return 8;  // NW
	}

	static inline void GetPoint(int x0, int y0, int x1, int y1, int * pX, int * pY, int * pError, int iCount)
	{
		int dx, dy, x_inc, y_inc, error, index;
		int iResultX, iResultY, iCnt = 0;

		if ((x0 == x1) && (y0 == y1))
		{	*pX = x0;
			*pY = y0;
			return;
		}
		error = *pError;
		iResultX = x0;
		iResultY = y0;
		dx = x1-x0;
		dy = y1-y0;
		if(dx>=0) x_inc = 1;
		else
		{	x_inc = -1;
			dx = -dx;
		}
		if(dy>=0) y_inc = 1;
		else
		{	y_inc = -1;
			dy = -dy;
		}
		if(dx>dy)
		{	for(index = 0; index <= dx; index++)
			{	error += dy;
				if(error > dx)
				{	error -= dx;
					iResultY += y_inc;
				}
				iResultX += x_inc;
				iCnt++;
				if (iCnt >= iCount) break;
			}
		}else
		{	for(index = 0; index <= dy; index++)
			{	error += dx;
				if(error > dy)
				{	error -= dy;
					iResultX += x_inc;
				}
				iResultY += y_inc;
				iCnt++;
				if (iCnt >= iCount) break;
		}	}
		*pX = iResultX;
		*pY = iResultY;
		*pError = error;
	}

	static inline bool bCheckValidString(const char * str)
	{
		size_t len = strlen(str);
		for (size_t i = 0; i < len; i++)
		{	if(str[i] == ' ') return false;
		}
		return true;
	}

	static inline void ReplaceString(char * pStr, char cFrom, char cTo)
	{
		size_t len = strlen(pStr);
		for (size_t i = 0; i < len; i++)
		{	if (pStr[i] == cFrom) pStr[i] = cTo;
		}
	}

	static inline char cCalcDirection(short sX, short sY, short dX, short dY)
	{
		double dTmp1, dTmp2, dTmp3;
		if ((sX == dX) && (sY == dY)) return 1;
		if ((sX == dX) && (sY != dY))
		{	if (sY > dY) return 1;
			else return 5;
		}
		if ((sX != dX) && (sY == dY))
		{	if (sX > dX) return 7;
			else return 3;
		}
		dTmp1 = static_cast<double>(dX - sX);
		dTmp2 = static_cast<double>(dY - sY);
		dTmp3 = dTmp1 / dTmp2;
		if (dTmp3 < -3)
		{	if (sX > dX) return 7;
			else return 3;
		}
		if (dTmp3 > 3)
		{	if (sX > dX) return 7;
			else return 3;
		}
		if ((dTmp3 > -0.3333f) && (dTmp3 <= 0.3333f))
		{	if (sY > dY) return 1;
			else return 5;
		}
		if ((dTmp3 > 0.3333f) && (dTmp3 <= 3.0f))
		{	if (sX > dX) return 8;
			else return 4;
		}
		if ((dTmp3 >= -0.3333f) && (dTmp3 < 3.0f))
		{	if (sX > dX) return 7;
			else return 3;
		}
		if ((dTmp3 >= -3.0f) && (dTmp3 < -0.3333f))
		{	if (sX > dX) return 6;
			else return 2;
		}
		return 1;
	}

	static inline bool bCheckValidName(const char *pStr)
	{
		size_t i, iLen;
		iLen = strlen(pStr);
		for (i = 0; i < iLen; i++)
		{	if ( pStr[i] < 0 )	return false;
			if ( (pStr[i] == ',')  || (pStr[i] == '=') || (pStr[i] == ' ')  || (pStr[i] == '\n') ||
				 (pStr[i] == '\t') || (pStr[i] == '.') || (pStr[i] == '\\') || (pStr[i] == '/')  ||
				 (pStr[i] == ':')  || (pStr[i] == '*') || (pStr[i] == '?')  || (pStr[i] == '<')  ||
				 (pStr[i] == '>')  || (pStr[i] == '|') || (pStr[i] == '"')  || (pStr[i] == '`')  ||
				 (pStr[i] == ';')  || (pStr[i] == '=') || (pStr[i] == '@')  || (pStr[i] == '[')  ||
				 (pStr[i] == ']')  || (pStr[i] == '^') || (pStr[i] == '_')  || (pStr[i] == '\'') ) return false;
			if( (pStr[i] < '0') || (pStr[i] > 'z')) return false;
		}
		return true;
	}

	static inline bool bIsValidEmail(const char *pStr)
	{
		size_t len = strlen( pStr );
		if( len < 7 ) return false;
		char cEmail[52];
		std::memset(cEmail, 0, sizeof(cEmail));
		memcpy( cEmail, pStr, len );
		bool bFlag = false;
		for( size_t i=0 ; i<len ; i++ )
		{
			if( cEmail[i] == '@' ) bFlag = true;
		}
		if( bFlag == false ) return false;
		bFlag = false;
		for( int i=0 ; i<len ; i++ )
		{
			if( cEmail[i] == '.' ) bFlag = true;
		}
		if( bFlag == false ) return false;
		return true;
	}
}
