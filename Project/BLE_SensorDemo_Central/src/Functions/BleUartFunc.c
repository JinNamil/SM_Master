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

extern int putchar(int c);
extern uint8_t gUpdateBlockData[32];
uint8_t gUpdateBlockSize;
uint32_t gUpdateTotalSize = 0;

hUARTQUEUE	hUartQueue;
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

void setUpdateMode(uint8_t flag)
{
  gFwUpdateFlag = flag;
}

uint8_t getUpdateMode(void)
{
  return gFwUpdateFlag;
}

void setUpdatePacketSize(uint8_t size)
{
  gUpdateBlockSize = size;
}

uint8_t getUpdatePacketSize(void)
{
  return gUpdateBlockSize;
}

void PcToUartParse(void)
{
  static unsigned char nRxData[MAX_TRX_BUF_SIZE]={0,};
  static unsigned char chIndex = 0;
  uint8_t    ch = 0;
  uint8_t    cmd = 0;
  UpdateStartPacket_t* updateStartPacket = {0,};

  if ( IsFifoEmpty(&hUartQueue.FifoRx) == true )
    return;
  else
  {
        ch = FifoPop(&hUartQueue.FifoRx);
        
        nRxData[chIndex++] = ch;
        if(!getUpdateMode() && ((ch == ASCII_CR) && (nRxData[chIndex-2] == ASCII_LF)))
        {
          cmd = nRxData[chIndex-3];
          if(cmd == OTA_COMMAND_DISCOVERY)
          {
            chIndex = 0;
            if(!gConnectionContext.isBleConnection)
            {
              if (deviceDiscovery() != BLE_STATUS_SUCCESS) {
                PRINTF("Error during the device discovery procedure\r\n");
              }
            }
            else
              putchar(OTA_COMMAND_ACK);
          }
          else if(cmd == OTA_COMMAND_RESPONSE_COMPLETE_BANK_SWAP)
          {
              chIndex = 0;
              memcpy(gUpdateBlockData, nRxData, sizeof(updateStartPacket)); 
              masterContext.mainWriteEnable = TRUE;
              setUpdatePacketSize(3);
          }
          else
          {
            chIndex = 0;
            updateStartPacket = (UpdateStartPacket_t*)nRxData;
            if(updateStartPacket->cmd == 0x0A)
            {
              gFwSize = updateStartPacket->fwSize;
              gBlockSize = updateStartPacket->blkTotal;
              
              memcpy(gUpdateBlockData, nRxData, sizeof(updateStartPacket)); 
              masterContext.mainWriteEnable = TRUE;
              setUpdatePacketSize(3);
              setUpdateMode(TRUE);
            }
          }
        }
        else if(getUpdateMode())
        {
          if(chIndex == BLE_TX_BUFFER_SIZE)
          {
            gUpdateTotalSize += chIndex;
            if(gUpdateTotalSize > gFwSize)
              setUpdatePacketSize((BLE_TX_BUFFER_SIZE - (gUpdateTotalSize - gFwSize)));
            else
              setUpdatePacketSize(BLE_TX_BUFFER_SIZE);
              
            chIndex = 0;
            memcpy(gUpdateBlockData, nRxData, BLE_TX_BUFFER_SIZE);
            masterContext.mainWriteEnable = TRUE;
          }
          else if(((ch == ASCII_CR) && (nRxData[chIndex-2] == ASCII_LF)))
          {
            cmd = nRxData[chIndex-3];
            if(cmd == OTA_COMMAND_RESPONSE_COMPLETE_UPDATE)
            {
              chIndex = 0;
              setUpdateMode(FALSE);
              memset(gUpdateBlockData, 0x00, 32);
              memcpy(gUpdateBlockData, nRxData, 3); 
              setUpdatePacketSize(3);
              gUpdateTotalSize = 0;
              masterContext.updateStart = FALSE;
              masterContext.mainWriteEnable = TRUE;
            }
          }
        }
        if(chIndex >= MAX_TRX_BUF_SIZE)
          chIndex = 0;
  }
  return;
}

void InitUartQueue(void)
{
  FifoInit( &hUartQueue.FifoRx, g_UartQueue, UART_QUEUE_SIZE );
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
    return -1;
  }
  return (FifoPop(&hUartQueue.FifoRx));
}

void DeInitUartQueue(void)
{
	FifoFlush(&hUartQueue.FifoRx);
	memset(g_UartQueue, 0L, UART_QUEUE_SIZE);
  return;
}
