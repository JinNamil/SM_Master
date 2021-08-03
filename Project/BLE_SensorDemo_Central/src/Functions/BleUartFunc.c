#include <stdio.h>
#include <string.h>
#include "BlueNRG1_it.h"
#include "BlueNRG1_conf.h"
#include "ble_const.h"
#include "SDK_EVAL_Config.h"
#include "sleep.h"
#include "gatt_db.h"
#include "master_basic_profile.h"
#include "master_config.h"
#include "BleUartFunc.h"
#include "Common.h"
#include "Fifo.h"
#include "AppGeneral.h"

extern int putchar(int c);
extern uint8_t gUpdateBlockData[32];
//extern uint8_t* gUpdateBlockData;
uint8_t gUpdateBlockSize;
uint32_t gUpdateTotalSize = 0;

hUARTQUEUE	hUartQueue;
unsigned char g_UartQRxChar;
static unsigned char g_UartQueue[UART_QUEUE_SIZE] = {0x0, };
uint8_t         gFwUpdateFlag = 0;
uint32_t        gFwSize = 0;
uint16_t        gBlockSize = 0;


void UartWrite(const unsigned char * buffer, size_t size)
{   
	while(size--)
	{
		putchar(*buffer++);
	}
	return;
}

int UartRead(int time_out) 
{
	while(time_out--)
	{	
		while(!UART_GetFlagStatus(UART_FLAG_RXFE)) 
		{
			unsigned char chv =(unsigned char) (UART_ReceiveData() & 0xFF);
		  /* Read byte from the receive FIFO */
			UartEnQueue(chv);
		 }
		return TRUE;
	}
	return FALSE;
}

int RxPacketParse(unsigned char *pPacket)
{
	unsigned char nCmd = 0x0;
	unsigned char nMagicCode = 0x0;
	unsigned char nPacketMsgLen = 0x0;
	unsigned char *pMsg;
	unsigned char nPacket[MAX_TRX_BUF_SIZE]={0,};
	unsigned char i=0, j=0;
	
	for(i=0, j=0; j<strlen((char*)pPacket)/2; j++, i+=2)
	{   
		nPacket[j] = ((hex2Digit(pPacket[i]) << 4)|hex2Digit(pPacket[i+1]));
	}
	
	if(RxPacketCheckCRC(nPacket) == TRUE)
	{
		// [ST][LNE][CMD][MAGIC][Data Packet][ChecSum][End]
		nPacketMsgLen = nPacket[1];// - BLE_PACKET_PREHEADER_SIZE;
		nCmd = nPacket[2];
		nMagicCode = nPacket[3];
		pMsg = &nPacket[4];

		switch(nCmd)
		{
		case eBLE_CMD_DEBUG_MSG:
		break;

		case eBLE_CMD_PERI_CENTRAL_MODE:
			if(GetBleMode() != pMsg[0])
			{
				SetBleMode((E_BLE_MODE)pMsg[0]);
				TxCurrBleMode(GetBleMode());	//jdkimkey Mode from BLE then Reply  BLE Mode to MCU 19-04-30-10:15:59
			}
			//SetBleStatus(eBLE_STATUS_NRG_READY);
		break;

		case eBLE_CMD_BLE_STATUS:
		break;

		case eBLE_CMD_FIRMWARE_INFO:
		break;

		case eBLE_CMD_FIRMWARE_PROGNOTI:
		break;

		case eBLE_CMD_FIRMWARE_PAGEDATA:
		break;

		case eBLE_CMD_FIRMWARE_CONTROL:
		break;

		case eBLE_CMD_SENSOR_WRITE:
		break;

		case eBLE_CMD_WRITE_SENSOR_SCANTIME:
		{
			P_FLASH_CFG pFlashCfg = NULL;//GetFlashCfgGlobalPtr();
			pFlashCfg = GetFlashCfgGlobalPtr();

			pFlashCfg->nBleSensorScanTimeOut = pMsg[0];
			SetBleCentralScanTimeOut(pFlashCfg->nBleSensorScanTimeOut);

			FlashWriteParameter(DATA_STORAGE_ADDR, (unsigned char*)pFlashCfg, sizeof(ST_FLASH_CFG));
			PRINTF("eBLE_CMD_WRITE_SENSOR_SCANTIME\r\n");
		}
		break;

		case eBLE_CMD_SET_NUM_RECORD:
		break;

		case eBLE_CMD_LOG_RECORD_NUM:
		break;

		case eBLE_CMD_SEND_LOG_DATA:
		break;

		case eBLE_CMD_MAC:
		{
			P_FLASH_CFG pFlashCfg = NULL;//GetFlashCfgGlobalPtr();
			pFlashCfg = GetFlashCfgGlobalPtr();
			memcpy(pFlashCfg->nBlePeriOwnMacAddr, pMsg, BLE_MAC_ADDR_LEN);
			FlashWriteParameter(DATA_STORAGE_ADDR, (unsigned char*)pFlashCfg, sizeof(ST_FLASH_CFG));
		}
		break;

		case eBLE_CMD_DEV_NAME:
			SetAdvDevName((char*)pMsg);
		break;

		case eBLE_CMD_WRITE_BLE_SENSOR:
		{
			P_FLASH_CFG pFlashCfg = NULL;//GetFlashCfgGlobalPtr();
			pFlashCfg = GetFlashCfgGlobalPtr();

			// [ST][LNE][CMD][MAGIC][Data Packet][ChecSum][End]
			/*eBLE_CMD_WRITE_BLE_SENSOR Info
			 [Data Packet] = [0] -> Tot BleSensor Num, [1] -> Currnet BLE Sensor Num, [2] -> Sensor Type, 
			 [3]:[8] -> BLE SENSOR MAC, [9] -> Connectable(1) or Non Connectable(0) */
			pFlashCfg->nBleSensorTotNum = pMsg[0];
			SetBleSensorTotNum(pFlashCfg->nBleSensorTotNum);
			memcpy(&pFlashCfg->stBleSensorDevList[*(pMsg+1)], &pMsg[2], sizeof(ST_BLE_SENSOR_DEV));
			PRINTF("BLE_SENSOR[%d] =0x%02x%02x%02x%02x%02x%02x\r\n", *(pMsg+1), pFlashCfg->stBleSensorDevList[*(pMsg+1)].nSensorMacAddr[0]
			, pFlashCfg->stBleSensorDevList[*(pMsg+1)].nSensorMacAddr[1], pFlashCfg->stBleSensorDevList[*(pMsg+1)].nSensorMacAddr[2]
			, pFlashCfg->stBleSensorDevList[*(pMsg+1)].nSensorMacAddr[3], pFlashCfg->stBleSensorDevList[*(pMsg+1)].nSensorMacAddr[4]
			, pFlashCfg->stBleSensorDevList[*(pMsg+1)].nSensorMacAddr[5]);
		}
		break;

		case eBLE_CMD_FLASH_WRITE:
		{
			P_FLASH_CFG pFlashCfg = NULL;//GetFlashCfgGlobalPtr();
			pFlashCfg = GetFlashCfgGlobalPtr();

			for(i=pFlashCfg->nBleSensorTotNum; i<BLE_SENSOR_MAX_NUM; i++)
			{
				memset(&pFlashCfg->stBleSensorDevList[i], 0xFF, sizeof(ST_BLE_SENSOR_DEV));
			}
			FlashWriteParameter(DATA_STORAGE_ADDR, (unsigned char*)pFlashCfg, sizeof(ST_FLASH_CFG));	//jdkimkey Store Sensor Infor Directly, If U write Ten of BleSensor Info, Write Function be excuted 10 Times 19-05-03-10:43:07

			PRINTF("eBLE_CMD_FLASH_WRITE\r\n");
		}
		break;

		case eBLE_CMD_SENSOR:
		break;

		case eBLE_CMD_NONE:
			//UartTxBlePacket();
		break;

		default:

		break;
		}
	}
	return FALSE;
}

void TxBleStatus(unsigned char nStatus)
{
	unsigned char mBlePacket[MAX_TRX_BUF_SIZE]={0,};	
	int xret = 0;
	
	xret = MakeTxPacket(mBlePacket, eBLE_CMD_BLE_STATUS, BLE_MAGIC_CODE_BLE_TO_MCU, (unsigned char *)&nStatus, 1);
	UartWrite(mBlePacket, xret); 				
}

void TxCurrBleMode(unsigned char nMode)
{
	unsigned char mBlePacket[MAX_TRX_BUF_SIZE]={0,};	
	int xret = 0;

	xret = MakeTxPacket(mBlePacket, eBLE_CMD_PERI_CENTRAL_MODE, BLE_MAGIC_CODE_BLE_TO_MCU, (unsigned char *)&nMode, 1);
	UartWrite(mBlePacket, xret); 				

	return;
}

void UartTxBlePacket(char *msg, unsigned char nCmd)
{
	unsigned char mBlePacket[MAX_TRX_BUF_SIZE]={0,};
	int xret = 0;

	xret = MakeTxPacket(mBlePacket, nCmd, BLE_MAGIC_CODE_BLE_TO_MCU, (unsigned char *)msg, sizeof(ST_BLE_SENSOR_DATA));
	UartWrite(mBlePacket, xret); 				

	return;
}	//jdkimkey Msg Size must not be over 128 19-04-23-15:02:05

void TxBleSensorList(void)
{
	P_BLE_SENSOR_DEV pBleSensorDevList;
	unsigned char mBlePacket[MAX_TRX_BUF_SIZE]={0,};
	int xret = 0;
	unsigned char i;
	pBleSensorDevList = GetBleSensorDevPtr();

	for(i=0; i<BLE_SENSOR_MAX_NUM; i++)
	{
		xret = MakeTxPacket( mBlePacket, eBLE_CMD_SENSOR_LIST_PRINT, BLE_MAGIC_CODE_BLE_TO_MCU, (unsigned char *)&pBleSensorDevList[i], sizeof(ST_BLE_SENSOR_DEV));
		UartWrite(mBlePacket, xret); 
	}

	return;
}	//jdkimkey Msg Size must not be over 128 19-04-23-15:02:05

void UartParse(void)
{
	static unsigned char nRxData[MAX_TRX_BUF_SIZE]={0,};
	static unsigned char chIndex = 0;
  char    ch;
	
	#if 1
  if ( IsFifoEmpty(&hUartQueue.FifoRx) == true )
  {
		return;
  }
  else
  {
		ch = FifoPop(&hUartQueue.FifoRx);
//		PRINTF("ch = %02x, Index = %d\r\n", ch, chIndex);

		nRxData[chIndex++] = ch;
		if(ch == ASCII_CR || ch == ASCII_LF)
		{
    	nRxData[chIndex++] = '\0';
    	chIndex = 0;
    	RxPacketParse(nRxData);
    	memset(nRxData, 0L, MAX_TRX_BUF_SIZE);    	
		}

		if(chIndex >= MAX_TRX_BUF_SIZE)
		{
			chIndex = 0;
		}
  }

	#else
	ch = UartDeQueue();

	if(ch == ASCII_CR || ch == ASCII_LF)
  {
    if (chIndex)
    {
    	nRxData[chIndex] = '\0';
    	chIndex = 0;
    	RxPacketParse(nRxData);
    	memset(nRxData, 0L, MAX_TRX_BUF_SIZE);
    }
  }
  else
  {
		if(chIndex < (MAX_TRX_BUF_SIZE-1))
		{
			if(ch != Q_FALSE)
			{
				nRxData[chIndex] = ch;
				chIndex = chIndex+1;
			}
		}
		else
		{
			chIndex = 0;    /* TODO : OVERFLOW INDICATION */
		}
	}
	#endif
	return;
}

int RxUpdatePacketParse(uint8_t* packet, uint32_t packetLen)
{
  uint8_t makeChecksum = 0;
  BlockPacket_t* blockPacket = {0,};
  blockPacket = (BlockPacket_t*)packet;
  makeChecksum = MakeCheckSum(packet, packetLen);
  if(makeChecksum == blockPacket->verify)
    return 0;
  
  return -1;
}

void PcToUartParse(void)
{
  static unsigned char nRxData[MAX_TRX_BUF_SIZE]={0,};
  static unsigned char chIndex = 0;
  char    ch;
  UpdateStartPacket_t* updateStartPacket = {0,};

  if ( IsFifoEmpty(&hUartQueue.FifoRx) == true )
    return;
  else
  {
        ch = FifoPop(&hUartQueue.FifoRx);
        
        nRxData[chIndex++] = ch;
        if(!gFwUpdateFlag && ((ch == ASCII_CR) && (nRxData[chIndex-2] == ASCII_LF)))
        {
          if(nRxData[chIndex-3] == OTA_COMMAND_DISCOVERY)
          {
            chIndex = 0;
            /* Start Master Device Discovery procedure */
            if(!gConnectionContext.isBleConnection)
            {
              if (deviceDiscovery() != BLE_STATUS_SUCCESS) {
                PRINTF("Error during the device discovery procedure\r\n");
              }
            }
            else
            {
              putchar(OTA_COMMAND_NACK);
            }
          }
          else if(nRxData[chIndex-3] == OTA_COMMAND_RESPONSE_COMPLETE_BANK_SWAP)
          {
              chIndex = 0;
              memset(gUpdateBlockData, 0x00, 32);
              memcpy(gUpdateBlockData, nRxData, sizeof(updateStartPacket)); 
              masterContext.mainWriteEnable = TRUE;
              gUpdateBlockSize = 3;
          }
          else
          {
            chIndex = 0;
            updateStartPacket = (UpdateStartPacket_t*)nRxData;
            if(updateStartPacket->cmd == 0x0A)
            {
              gFwSize = updateStartPacket->fwSize;
              gBlockSize = updateStartPacket->blkTotal;
              
              memset(gUpdateBlockData, 0x00, 32);
              memcpy(gUpdateBlockData, nRxData, sizeof(updateStartPacket)); 
              masterContext.mainWriteEnable = TRUE;
              gUpdateBlockSize = 3;
              gFwUpdateFlag = TRUE;
            }
          }
        }
        else if(gFwUpdateFlag && (chIndex == BLE_TX_BUFFER_SIZE))
        {
          gUpdateTotalSize += chIndex;
          if(gUpdateTotalSize > gFwSize)
            gUpdateBlockSize = (BLE_TX_BUFFER_SIZE - (gUpdateTotalSize - gFwSize));
          else
            gUpdateBlockSize = BLE_TX_BUFFER_SIZE;
            
          chIndex = 0;
          memset(gUpdateBlockData, 0x00, 32);
          memcpy(gUpdateBlockData, nRxData, BLE_TX_BUFFER_SIZE);
          masterContext.mainWriteEnable = TRUE;
//          DeInitUartQueue();
        }
        else if(gFwUpdateFlag && ((ch == ASCII_CR) && (nRxData[chIndex-2] == ASCII_LF)) && (nRxData[chIndex-3] == OTA_COMMAND_RESPONSE_COMPLETE_UPDATE))
        {
            chIndex = 0;
            gFwUpdateFlag = FALSE;
            memset(gUpdateBlockData, 0x00, 32);
            memcpy(gUpdateBlockData, nRxData, 3); 
            gUpdateBlockSize = 3;
            gUpdateTotalSize = 0;
            masterContext.updateStart = FALSE;
            masterContext.mainWriteEnable = TRUE;
        }
        if(chIndex >= MAX_TRX_BUF_SIZE)
          chIndex = 0;
  }
  return;
}

void DiscoveryPeripeheral(void)
{
  static unsigned char nRxData[MAX_TRX_BUF_SIZE]={0,};
  static unsigned char chIndex = 0;
  char    ch;
  
  if ( IsFifoEmpty(&hUartQueue.FifoRx) == true )
    return;
  else
  {
    ch = FifoPop(&hUartQueue.FifoRx);
    if((ch == ASCII_CR) && (nRxData[chIndex-2] == ASCII_LF))
    {
      if(nRxData[chIndex-3] == 0xCC)
      {
        if (deviceDiscovery() != BLE_STATUS_SUCCESS) 
          PRINTF("Error during the device discovery procedure\r\n");
        memset(nRxData, 0x00, sizeof(nRxData));
        chIndex = 0;
      }
    }
  }
}

void InitUartQueue(void)
{
  FifoInit( &hUartQueue.FifoRx, g_UartQueue, UART_QUEUE_SIZE );
//  gUpdateBlockData = malloc(BLE_TX_BUFFER_SIZE);
  return;
}

void UartEnQueue(unsigned char nCmd)
{
  if( IsFifoFull( &hUartQueue.FifoRx ) == false )
  {
    FifoPush( &hUartQueue.FifoRx, nCmd);
  }
  return;
}

char UartDeQueue(void)
{
  if ( IsFifoEmpty(&hUartQueue.FifoRx) == true )
  {
    return Q_FALSE;
  }
  return (FifoPop(&hUartQueue.FifoRx));
}

void DeInitUartQueue(void)
{
	FifoFlush(&hUartQueue.FifoRx);
	memset(g_UartQueue, 0L, UART_QUEUE_SIZE);
  return;
}
