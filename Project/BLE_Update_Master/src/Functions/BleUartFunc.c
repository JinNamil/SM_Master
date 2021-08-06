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
uint32_t gStartUpdateClock = 0;
uint32_t gTimeoutUpdateClock = 0;
uint8_t gBufferUart[UART_BUFFER_SIZE] = {0,};
uint32_t gBufferUartLen = 0;

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

void SetBleUpdateTimeout(unsigned int nTimeOut)
{
	gTimeoutUpdateClock = nTimeOut;/*(unsigned int)(nTimeOut * 1000);*/
	return;
}

unsigned int GetBleUpdateTimeout(void)
{
	return gTimeoutUpdateClock;
}

void initBleUpdateTimeout(void)
{  
  masterContext.updateStart = FALSE;
  masterContext.mainWriteEnable = FALSE;
  gUpdateTotalSize = 0;
  gStartUpdateClock = 0;
  setUpdateMode(FALSE);
}

void PcToUartParse(void)
{
  uint8_t    cmd = 0;
  uint16_t recvBuffLen = 0;
  UpdateStartPacket_t* updateStartPacket = {0,};
  recvBuffLen = (UART_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA_CH_UART_RX));
  if(recvBuffLen == 0)
    return;
  else
  {
    if(!getUpdateMode())
    {
      if((gBufferUart[recvBuffLen-1] == ASCII_CR) && (gBufferUart[recvBuffLen-2] == ASCII_LF))
      {
        DMA_CH_UART_RX->CCR_b.EN = RESET;
        DMA_SetCurrDataCounter(DMA_CH_UART_RX, UART_BUFFER_SIZE);
        DMA_CH_UART_RX->CCR_b.EN = SET;
        cmd = gBufferUart[recvBuffLen-3];
        if(cmd == OTA_COMMAND_DISCOVERY)
        {
          recvBuffLen = 0;
          if(!gConnectionContext.isBleConnection)
          {
            if (deviceDiscovery() != BLE_STATUS_SUCCESS)
              putchar(OTA_COMMAND_NACK);
          }
          else
            putchar(OTA_COMMAND_ACK);
        }
        else if(cmd == OTA_COMMAND_RESPONSE_COMPLETE_BANK_SWAP)
        {
            recvBuffLen = 0;
            gStartUpdateClock = 0;
            memcpy(gUpdateBlockData, gBufferUart, sizeof(updateStartPacket)); 
            masterContext.mainWriteEnable = TRUE;
            setUpdatePacketSize(3);
        }
        else if(cmd == OTA_COMMAND_RESPONSE_COMPLETE_UPDATE)
        {
          recvBuffLen = 0;
          memset(gUpdateBlockData, 0x00, 32);
          memcpy(gUpdateBlockData, gBufferUart, 3); 
          setUpdatePacketSize(3);
          gUpdateTotalSize = 0;
          masterContext.updateStart = FALSE;
          masterContext.mainWriteEnable = TRUE;
        }
        else
        {
          recvBuffLen = 0;
          updateStartPacket = (UpdateStartPacket_t*)gBufferUart;
          if(updateStartPacket->cmd == 0x0A)
          {
            gFwSize = updateStartPacket->fwSize;
            gBlockSize = updateStartPacket->blkTotal;
            
            memcpy(gUpdateBlockData, gBufferUart, sizeof(updateStartPacket)); 
            masterContext.mainWriteEnable = TRUE;
            setUpdatePacketSize(3);
            setUpdateMode(TRUE);
          }
        }
      }
    }
    else if(getUpdateMode())
    {
      if(recvBuffLen == BLE_TX_BUFFER_SIZE)
      {
        DMA_CH_UART_RX->CCR_b.EN = RESET;
        DMA_SetCurrDataCounter(DMA_CH_UART_RX, UART_BUFFER_SIZE);
        DMA_CH_UART_RX->CCR_b.EN = SET;
        gStartUpdateClock = Clock_Time();
        gUpdateTotalSize += BLE_TX_BUFFER_SIZE;
        if(gUpdateTotalSize > gFwSize)
        {
          setUpdatePacketSize((BLE_TX_BUFFER_SIZE - (gUpdateTotalSize - gFwSize)));
        }
        else
          setUpdatePacketSize(BLE_TX_BUFFER_SIZE);
        
        if(gUpdateTotalSize >= gFwSize)
          setUpdateMode(FALSE);
          
        recvBuffLen = 0;
        memcpy(gUpdateBlockData, gBufferUart, BLE_TX_BUFFER_SIZE);
        masterContext.mainWriteEnable = TRUE;
      }
    }
    if(recvBuffLen >= UART_BUFFER_SIZE)
      recvBuffLen = 0;
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
