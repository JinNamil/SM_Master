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
uint8_t gUpdateBlockSize = 0;
uint32_t gUpdateTotalSize = 0;
uint32_t gStartUpdateClock = 0;
uint32_t gTimeoutUpdateClock = 0;
uint32_t gStatus = 0;
uint8_t gBufferUart[UART_BUFFER_SIZE] = {0,};
uint32_t gBufferUartLen = 0;
uint8_t gUpdateMode = CONN_BLE_MODE;
uint16_t gOtaUpdateHandle = 0;
uint8_t gIsUpdateLastPacket = 0;

hUARTQUEUE	hUartQueue;
static unsigned char g_UartQueue[UART_QUEUE_SIZE] = {0x0, };
uint8_t         gFwUpdateFlag = 0;
uint32_t        gFwSize = 0;
uint16_t        gBlockSize = 0;
OtaUpdateInfoPacket_t gOtaUpdateInfoPacket = {0,};

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

void SetConnUpdateMode(uint8_t mode)
{
  gUpdateMode = mode;
  return;
}

uint8_t GetConnUpdateMode(void)
{
  return gUpdateMode;
}

void SetOtaUpdateHandle(uint16_t handle)
{
  gOtaUpdateHandle = handle + 1;
  return;
}
uint16_t GetOtaUpdateHandle(void)
{
  return gOtaUpdateHandle;
}

void SetIsUpdateLastPacket(uint8_t flag)
{
  gIsUpdateLastPacket = flag;
  return;
}

uint8_t GetIsUpdateLastPacket(void)
{
  return gIsUpdateLastPacket;
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

void InitBleUpdateTimeout(void)
{  
  SetBleStatus(STATUS_PC_REQUEST_COMMAND_WAIT);
  setUpdateMode(FALSE);
  gUpdateTotalSize = 0;
  gStartUpdateClock = 0;
  dmaBufferInit();
}

void PcToUartParse(void)
{
  uint8_t    cmd = 0;
  uint16_t recvBuffLen = dmaReceiveDataLen();
  UpdateStartPacket_t* updateStartPacket = {0,};
  if(recvBuffLen == 0)
    return;
  else
  {
    if(!getUpdateMode())
    {
      if((gBufferUart[recvBuffLen-1] == ASCII_CR) && (gBufferUart[recvBuffLen-2] == ASCII_LF))  //packet structure to be edit
      {
        dmaBufferInit();
        cmd = gBufferUart[recvBuffLen-3];
        if(recvBuffLen == 3)
        {
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
              memcpy(gUpdateBlockData, gBufferUart, BLE_CMD_SIZE); 
              SetBleStatus(STATUS_PC_REQUEST_BANK_SWAP_RECV);
              setUpdatePacketSize(BLE_CMD_SIZE);
          }
          else if(cmd == OTA_COMMAND_COMPLETE_UPDATE)
          {
            memset(gUpdateBlockData, 0x00, 64);
            memcpy(gUpdateBlockData, gBufferUart, BLE_CMD_SIZE); 
            memset(gBufferUart, 0x00, sizeof(gBufferUart));
            setUpdatePacketSize(BLE_CMD_SIZE);
            gUpdateTotalSize = 0; 
            SetBleStatus(STATUS_PC_REQUEST_COMMAND_RECV);
          }
          else if(cmd == OTA_COMMAND_BLE_FLAG_UPDATE)
          {
            gStartUpdateClock = Clock_Time();
            memcpy(gUpdateBlockData, gBufferUart, BLE_CMD_SIZE); 
            SetBleStatus(STATUS_PC_REQUEST_BLE_START_RECV);
            setUpdatePacketSize(BLE_CMD_SIZE);
          }
          else if(cmd == OTA_COMMAND_SET_BLE_UPDATE_MODE)
          {
            SetConnUpdateMode(CONN_BLE_MODE);
            putchar(OTA_COMMAND_SET_BLE_UPDATE_MODE);
          }
          else if(cmd == OTA_COMMAND_SET_MCU_UPDATE_MODE)
          {
            SetConnUpdateMode(CONN_MCU_MODE);
            putchar(OTA_COMMAND_SET_MCU_UPDATE_MODE);
          }
        }
        else
        {
          updateStartPacket = (UpdateStartPacket_t*)gBufferUart;
          if(GetConnUpdateMode() == CONN_MCU_MODE)
          {
            if(updateStartPacket->cmd == OTA_COMMAND_START_UPDATE)
            {
              gStartUpdateClock = Clock_Time();
              gFwSize = updateStartPacket->fwSize;
              gBlockSize = updateStartPacket->blkTotal;
              
              memcpy(gUpdateBlockData, gBufferUart, sizeof(updateStartPacket)); 
              setUpdatePacketSize(BLE_CMD_SIZE);
              SetBleStatus(STATUS_PC_REQUEST_COMMAND_RECV);
              setUpdateMode(TRUE);
            }
          }
          else if(GetConnUpdateMode() == CONN_BLE_MODE)
          {
            if(updateStartPacket->cmd == OTA_COMMAND_START_UPDATE)
            {
              gFwSize = updateStartPacket->fwSize;
              gOtaUpdateInfoPacket.radioRange = 0x08;
              gOtaUpdateInfoPacket.fwSize = updateStartPacket->fwSize;
              gOtaUpdateInfoPacket.baseAddr = 0x10051000;
              memset(&updateStartPacket, 0x00, sizeof(updateStartPacket));
              memcpy(gUpdateBlockData, &gOtaUpdateInfoPacket, sizeof(gOtaUpdateInfoPacket));
              setUpdatePacketSize(sizeof(gOtaUpdateInfoPacket));
              SetBleStatus(STATUS_PC_REQUEST_COMMAND_RECV);
              SetOtaUpdateHandle(masterContext.otaNewImgHandle);
              setUpdateMode(TRUE);
            }
          }
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
          
          if(GetConnUpdateMode() == CONN_BLE_MODE)
            gUpdateTotalSize += 16;
          else if(GetConnUpdateMode() == CONN_MCU_MODE)
            gUpdateTotalSize += BLE_TX_BUFFER_SIZE;
          
          if(gUpdateTotalSize > gFwSize)
            setUpdatePacketSize((BLE_TX_BUFFER_SIZE - (gUpdateTotalSize - gFwSize)));
          else
            setUpdatePacketSize(BLE_TX_BUFFER_SIZE);
          
          if(GetConnUpdateMode() == CONN_BLE_MODE)
          {
            SetOtaUpdateHandle(masterContext.otaNewImgTuContentHandle);
            setUpdatePacketSize(BLE_TX_BUFFER_SIZE);
          }
            
          if(gUpdateTotalSize >= gFwSize)
          {
            SetIsUpdateLastPacket(TRUE);
            setUpdateMode(FALSE);
//            if(GetConnUpdateMode() == CONN_BLE_MODE)
//              Master_CloseConnection(masterContext.connHandle);
          }
          
          memcpy(gUpdateBlockData, gBufferUart, BLE_TX_BUFFER_SIZE);
      }
      else if(recvBuffLen > BLE_TX_BUFFER_SIZE)
        InitBleUpdateTimeout();
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
