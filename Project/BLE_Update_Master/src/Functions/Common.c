#include "Common.h"

unsigned char hex2Digit(unsigned char ch)
{
	if (ch >= 'A' && ch <= 'F')
	{
		return ch - 'A' + 10;
	}
	else if (ch >= 'a' && ch <= 'f')
	{
		return ch - 'a' + 10;
	}
	else
	{
		return ch - '0';
	}
	//return 0xFF;
}

unsigned char MakeCheckSum(unsigned char *pPacket, unsigned char nCnt)
{
	unsigned char i = 0;
	unsigned char nCheckSum = 0;
	for(i = 0; i < nCnt; i++)
	{
		nCheckSum += pPacket[i];
	}
	return nCheckSum;
}

unsigned char MakeBleCheckSum(unsigned char* pPacket, unsigned char nCnt)
{
	unsigned char i = 0;
	unsigned char nCheckSum = 0;
	for (i = 0; i < nCnt; i++)
	{
		nCheckSum ^= pPacket[i];
	}
	return nCheckSum;
}
