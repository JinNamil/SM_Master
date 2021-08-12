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
#include "clock.h"

extern int putchar(int c);
extern uint8_t gUpdateBlockData[64];
uint8_t gUpdateBlockSize;
uint32_t gUpdateTotalSize = 0;
uint32_t gStartUpdateClock = 0;
uint32_t gTimeoutUpdateClock = 0;
uint32_t gStatus = 0;
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

uint32_t GetBleUpdateTimeout(void)
{
  return gTimeoutUpdateClock;
}

void SetBleStatus(uint32_t status)
{
  gStatus = status;
  return;
}

uint32_t GetBleStatus(void)
{
  return gStatus;
}

void initBleUpdateTimeout(void)
{  
  SetBleStatus(STATUS_PC_REQUEST_COMMAND_WAIT);
  dmaBufferInit();
  setUpdateMode(FALSE);
  gUpdateTotalSize = 0;
  gStartUpdateClock = 0;
}

uint16_t dmaReceiveDataLen(void)
{
  return (UART_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA_CH_UART_RX));
}

void dmaBufferInit(void)
{
  DMA_CH_UART_RX->CCR_b.EN = RESET;
  DMA_SetCurrDataCounter(DMA_CH_UART_RX, UART_BUFFER_SIZE);
  DMA_CH_UART_RX->CCR_b.EN = SET;
}

void PcToUartParse(void)
{
  uint8_t    cmd = 0;
  uint16_t recvBuffLen = dmaReceiveDataLen();
  CommandPacket_t* commandPacket = {0,};

  if(recvBuffLen == 0)
    return;
  else
  {
    if(!getUpdateMode())
    {
      if((gBufferUart[recvBuffLen-1] == ASCII_LF) && (gBufferUart[recvBuffLen-2] == ASCII_CR))
      {
        dmaBufferInit();
        commandPacket = (CommandPacket_t*)&gBufferUart[recvBuffLen-sizeof(CommandPacket_t)];
        cmd = commandPacket->cmd;
        if(cmd == OTA_COMMAND_DISCOVERY)
        {
          if(!gConnectionContext.isBleConnection)
          {
            if (deviceDiscovery() != BLE_STATUS_SUCCESS)
              putchar(OTA_COMMAND_NACK);
          }
          else
            putchar(OTA_COMMAND_ACK);
        }
        else if(cmd == OTA_COMMAND_COMPLETE_BANK_SWAP)
        {
            gStartUpdateClock = 0;
            memcpy(gUpdateBlockData, gBufferUart, 3);
            SetBleStatus(STATUS_PC_REQUEST_BANK_SWAP_RECV);
            setUpdatePacketSize(3);
        }
        else if(cmd == OTA_COMMAND_COMPLETE_UPDATE)
        {
			memset(gUpdateBlockData, 0x00, 64);
			memcpy(gUpdateBlockData, gBufferUart, 3);
			memset(gBufferUart, 0x00, sizeof(gBufferUart));
			setUpdatePacketSize(3);
			gUpdateTotalSize = 0;
			SetBleStatus(STATUS_PC_REQUEST_COMMAND_RECV);
        }
        else if(cmd == OTA_COMMAND_START_UPDATE)
        {
			gStartUpdateClock = Clock_Time();
			gFwSize = commandPacket->fwSize;
			gBlockSize = commandPacket->blkTotal;

			memcpy(gUpdateBlockData, gBufferUart, sizeof(CommandPacket_t));
			setUpdatePacketSize(3);
			SetBleStatus(STATUS_PC_REQUEST_COMMAND_RECV);
			setUpdateMode(TRUE);
        }
      }
    }
    else if(getUpdateMode())
    {
      if(recvBuffLen == BLE_TX_BUFFER_SIZE)
      {
		SetBleStatus(STATUS_PC_REQUEST_DATA_RECV);
		dmaBufferInit();
		gStartUpdateClock = Clock_Time();
		gUpdateTotalSize += BLE_TX_BUFFER_SIZE;

		if(gUpdateTotalSize > gFwSize)
		  setUpdatePacketSize((BLE_TX_BUFFER_SIZE - (gUpdateTotalSize - gFwSize)));
		else
		  setUpdatePacketSize(BLE_TX_BUFFER_SIZE);

		if(gUpdateTotalSize >= gFwSize)
		  setUpdateMode(FALSE);

		memcpy(gUpdateBlockData, gBufferUart, BLE_TX_BUFFER_SIZE);
      }
      else if(recvBuffLen > BLE_TX_BUFFER_SIZE)
        initBleUpdateTimeout();
    }
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
