#ifndef __BLEUARTFUNC_H__
#define __BLEUARTFUNC_H__

#include "Fifo.h"
#include "Common.h"
/*********************************** Define General ***********************************/
/* Prohibit using 0x0D, 0x0A for PACKET Define, It should be reserved for Uart Rx Parsing */
/* Even IU have Packet Size 0x0A(10) or 0x0D(13) eraborately, Make Packet 11 or 14 adding Dummy Packet 1byte! */
/* ========> Packet Structure <====================
 [ST][LNE][CMD][MAGIC][Data Packet][ChecSum][End]
  =========================================== */
#define UART_QUEUE_SIZE		        64
#define UART_BUFFER_SIZE                32

/*********************************** Typedef Structer ***********************************/
typedef struct _UartFifoTag
{
    unsigned char   UartId;
    Fifo_t    			FifoTx;
    Fifo_t    			FifoRx;
} hUartBuf_t, hUARTQUEUE;

#pragma pack(push, 1)
typedef struct
{
	uint8_t		cmd;
	uint32_t	fwSize;
	uint16_t	blkTotal;
	uint8_t		cr;
	uint8_t		lf;
} UpdateStartPacket_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct
{
	uint8_t	 checksum;
	uint8_t	 data[16];
	uint8_t  isNoti;
	uint16_t blkOffset;
} BleUpdatePacket_t;
#pragma pack(pop)

typedef enum
{
        STATUS_PC_REQUEST_COMMAND_WAIT = 0,
        STATUS_PC_REQUEST_COMMAND_RECV,
        STATUS_PC_REQUEST_DATA_WAIT,
        STATUS_PC_REQUEST_DATA_RECV,
        STATUS_BLE_SEND_DATA_COMPLETE,
        STATUS_PC_REQUEST_BANK_SWAP_RECV,
        STATUS_PC_REQUEST_BANK_SWAP_COMPLETE,
        STATUS_BLE_SEND_COMMAND_COMPLETE,
}bleStatus_t;

/************************************ Define Functions ************************************/
void UartWrite(const unsigned char * buffer, size_t size);
int UartRead(int time_out); 
void PcToUartParse(void);
void InitUartQueue(void);
void UartEnQueue(unsigned char nCmd);
char UartDeQueue(void);
void DeInitUartQueue(void);
void SetBleUpdateTimeout(unsigned int nTimeOut);
void InitBleUpdateTimeout(void);
void SetBleStatus(uint32_t status);
uint32_t GetBleStatus(void);
uint32_t GetBleUpdateTimeout(void);
#endif /* __BLEUARTFUNC_H__ */


