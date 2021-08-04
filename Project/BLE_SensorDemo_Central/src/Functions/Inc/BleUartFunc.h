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
#define OTA_UPDATE_PACKET_BLOCK_SIZE 18
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
/************************************ Define Functions ************************************/
void UartWrite(const unsigned char * buffer, size_t size);
int UartRead(int time_out); 
void PcToUartParse(void);
void InitUartQueue(void);
void UartEnQueue(unsigned char nCmd);
char UartDeQueue(void);
void DeInitUartQueue(void);


#endif /* __BLEUARTFUNC_H__ */


