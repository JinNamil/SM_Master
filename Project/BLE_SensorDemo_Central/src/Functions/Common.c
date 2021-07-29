#include "Common.h"
#include "AppGeneral.h"

extern ST_FLASH_CFG g_stFlashCfg;

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

int RxPacketCheckCRC(unsigned char *pPacket)
{
	unsigned char nCrc = 0;
	unsigned char nRetval = 0;
	unsigned char nPacketLen = strlen((char*)pPacket);
	unsigned char nMsgCrcPos = pPacket[1] + RX_PACKET_CRC_POS_PLUS;
	
	// [ST][LNE][CMD][MAGIC][Data Packet][ChecSum][End][CR][LF]
	nCrc = pPacket[nMsgCrcPos];
	nRetval = MakeCheckSum(pPacket , pPacket[1] + RX_PACKET_CRC_POS_PLUS);

	#if defined(__USE_BLE_DEBUG_RAW__)
	{
		unsigned char i=0;

		for(i=0; i < nPacketLen; i++)
		{
			PRINTF("%02x", pPacket[i]);
		}
		PRINTF("\r\n");
		PRINTF("nPacketLen = %d, nCrc = %02x, nMakedCRC = %02x\r\n", nPacketLen, nCrc, nRetval);

	}
	#endif

	if(nCrc == nRetval)
	{
		return TRUE;
	}
	return FALSE;
}

int MakeTxPacket(unsigned char *pPacket, unsigned char nBleCmd, unsigned char nMagicCode, unsigned char *pMsg, int nMsgLen)
{
	unsigned char nDest = 0;
	unsigned char nCheckSum = 0;
	unsigned char i=0;
  char strPacketBuf[MAX_TRX_BUF_SIZE]={0,};
  
	strPacketBuf[nDest++] = UC_BLE_ST;		//0xE8
	strPacketBuf[nDest++] = nMsgLen; //0x09
	strPacketBuf[nDest++] = nBleCmd;		//0x20
	strPacketBuf[nDest++] = nMagicCode;		//0x22
	memcpy(&strPacketBuf[nDest], pMsg, nMsgLen); //0x01
	nDest+= nMsgLen;

	nCheckSum = MakeCheckSum((unsigned char*)strPacketBuf, nDest);	//jdkimkey Not Include STX, Packet Size, EDX & CRLF 19-04-23-15:10:23
	strPacketBuf[nDest++] = nCheckSum;		//0x00
	strPacketBuf[nDest++]=UC_BLE_END;	//0xEA

  for(i=0; i<nDest; i++)
  {
      sprintf((char*)&pPacket[i*2],"%02x",strPacketBuf[i]);
  }
  i*=2;
	pPacket[i++] = ASCII_CR;
	pPacket[i++] = ASCII_LF;
	
	return i;
}

int FlashInitParameter(void)
{
	
//	g_pFlashCfg = (P_FLASH_CFG)malloc(sizeof(ST_FLASH_CFG));
	memset(&g_stFlashCfg, 0L, sizeof(ST_FLASH_CFG));
	#if 0
	if(g_pFlashCfg == NULL)
	{
		PRINTF("__%s__[%d] FlashInitParameter Error\r\n", __func__, __LINE__);
		return FALSE;
	}
	#endif
	return TRUE;
}

int FlashGetParameter(void *ptr)
{
	P_FLASH_CFG pFlashCfg = (P_FLASH_CFG)ptr;
	#if 0
	if (pFlashCfg == NULL)
	{
		PRINTF("__%s__[%d] FlashGetParameter Error\r\n", __func__, __LINE__);
		return FALSE;
	}
	memset(pFlashCfg, 0x00, sizeof(ST_FLASH_CFG)); // memory Clear 
	memcpy(pFlashCfg, ((unsigned char *)DATA_STORAGE_ADDR), sizeof(ST_FLASH_CFG));	// bank1 mode ===> laster sector of bank1
	#else
	memset(pFlashCfg, 0L, sizeof(ST_FLASH_CFG)); // memory Clear 
	memcpy(pFlashCfg, ((unsigned char *)DATA_STORAGE_ADDR), sizeof(ST_FLASH_CFG));	// bank1 mode ===> laster sector of bank1
	#endif


	SetBleSensorTotNum(pFlashCfg->nBleSensorTotNum);
	SetBleCentralScanTimeOut(pFlashCfg->nBleSensorScanTimeOut);
	SetBlePeriMacAddr(pFlashCfg->nBlePeriOwnMacAddr);
	return TRUE;
}

void FlashWriteParameter(unsigned int nWriteAddr, unsigned char *pBuffer, unsigned int nWriteSize)
{
	unsigned int *pWriteBufData;
	unsigned int i=0;

	for(i=DATA_STORAGE_PAGE; i<DATA_STORAGE_PAGE+4; i++)
	{
			FLASH_ErasePage((uint16_t)i);
	}
	
	for(i=0, pWriteBufData = (unsigned int *)pBuffer; i < nWriteSize / 4; i++, pWriteBufData++)
	{
		FLASH_ProgramWord(nWriteAddr , *pWriteBufData);
		while(FLASH_GetFlagStatus(Flash_CMDDONE) != SET);
		nWriteAddr = nWriteAddr + 4;
	}
	PRINTF("__%s__[%d]\r\n", __func__, __LINE__);

	return;
}
