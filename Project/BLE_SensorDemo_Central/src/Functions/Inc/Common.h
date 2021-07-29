#ifndef __COMMON_H__
#define __COMMON_H__
//#include <stddef.h>
//#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "blueNRG1_flash.h"
#include "hal_types.h"

/*********************************** Define General ***********************************/
#define BLE_PACKET_PREHEADER_SIZE				8 				
#define RX_PACKET_CHECKSUM_MINUS_LEN		4
#define RX_PACKET_CRC_POS_PLUS					4

#define BLE_PACKET_CMD_POS							2
#define BLE_PACKET_CMD_MINUS_LEN				2

//STX (1Byte) + LENGTH (1Byte) + CMD (1Byte) + MAGIC CODE (1Byte) + CHECKSUM (1Byte) + EDX (1Byte) + CRLF (2Bytes) 
#define MAX_TRX_BUF_SIZE								32
#define ASCII_CR												0x0A
#define ASCII_LF												0x0D

#define	UC_BLE_ST												0xE8
#define	UC_BLE_ESC											0xE9
#define	UC_BLE_END											0xEA
#define	UC_BLE_COMP											0x40
#define	UC_MAX_PACKET_SIZE							64

#define	BLE_MAGIC_CODE_MCU_TO_BLE				0x11	 /* STM4 ==> BLE */
#define	BLE_MAGIC_CODE_BLE_TO_MCU				0x22		/* BLE ===> STM4 */

#define BLE_LIB_CENTRAL									1
#define BLE_LIB_PERIPHERAL							0
#define BLE_SENSOR_MAX_NUM							5

#define BLE_SENSOR_ALL_CHECKED					0x1F

#define DATA_STORAGE_PAGE       (N_PAGES-4)	//56Pages
#define DATA_STORAGE_ADDR       (((N_PAGES-4)*N_BYTES_PAGE) + FLASH_START)	//0x26000

#define SENSOR_DATA_PREFIX				3
#define SENSOR_DATA_POS						5+SENSOR_DATA_PREFIX
#define SCAN_REQ_CNT							2

#define MANHOLE_EMERGENCY						0xE0
//#define __USE_BLE_DEBUG_RAW__
//#define __USE_BLE_CENTRAL_TEST_ALONE__
/*********************************** Typedef Structer ***********************************/
typedef enum __eBLECMD_
{
	eBLE_CMD_NONE = 0x0,
	eBLE_CMD_DEBUG_MSG = 0x10,						/* display ble debug packet log  */
	eBLE_CMD_PERI_CENTRAL_MODE = 0x20,		/* set BLE mode */
	eBLE_CMD_BLE_STATUS,									/* BLE status */
	eBLE_CMD_FIRMWARE_INFO,								/* firmware info */
	eBLE_CMD_FIRMWARE_PROGNOTI,						/* firmware process noti */
	eBLE_CMD_FIRMWARE_PAGEDATA,						/* firmware data   */
	eBLE_CMD_FIRMWARE_CONTROL,						/* firmware control */
	eBLE_CMD_SENSOR_WRITE = 0x30,					/* Sensor Write from BLE to STM */
	eBLE_CMD_WRITE_SENSOR_SCANTIME,						/* Sensor Write Respone from STM to BLE */
	eBLE_CMD_SET_NUM_RECORD = 0x40,				/* Total Log Record number from STM to BLE  */
	eBLE_CMD_LOG_RECORD_NUM,							/* Log Record number from BLT to STM */
	eBLE_CMD_SEND_LOG_DATA,								/* Send Log Data */
	eBLE_CMD_MAC = 0x80, 									/* Mac address setting */
	eBLE_CMD_DEV_NAME,	 									/* Mac address setting */
	eBLE_CMD_WRITE_BLE_SENSOR,						/* WRITE BLE Sensor Info MCU<-->BLE */
	eBLE_CMD_SENSOR_LIST_PRINT,
	/*eBLE_CMD_WRITE_BLE_SENSOR Info
	 [Data Packet] = [0] -> Tot BleSensor Num, [1] -> Currnet BLE Sensor Num, [2] -> Sensor Type, 
	 [3]:[8] -> BLE SENSOR MAC, [9] -> Connectable(1) or Non Connectable(0) */
	eBLE_CMD_SENSOR,											/* read sensor value */
	eBLE_CMD_FLASH_WRITE = 0x90,					/* Write Flash*/
	eBLE_CMD_AUTO = 0xFF									/* automatic handling */
}E_BLE_CMD;

typedef enum __ePROGNOTY_
{
	ePROGNOTI_MODE_UPDATE_NOT_READY = 0x01,
	ePROGNOTI_MODE_PAGE_REQUEST,
	ePROGNOTI_MODE_PAGE_RCVERRPR,
	ePROGNOTI_MODE_FLASHWRITE_FAIL,
	ePROGNOTI_MODE_UPDATGE_START,
	ePROGNOTI_MODE_UPDATGE_COMP,
	ePROGNOTI_MODE_FLASH_INFO,
	ePROGNOTI_MODE_FLASH_ERASE_PRC,
	ePROGNOTI_MODE_ILLEGAL_CONTROL
}E_PROGNOTY;

typedef enum __eSensorType
{
	eO2_SENSOR = 0x1,  /*Oxyzen 산소농도센서*/
	eCO_SENSOR,				/*화재감지, 일산화탄소 센서*/
	eCO2_SENSOR,			/*이산화탄소 농도센서*/
	eWATER_SENSOR   /*수위감지 센서 Cm단원*/
}E_SENSOR_TYPE;

#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define TRUE				1
#define FALSE				0


/************************************ Define Functions ************************************/
unsigned char hex2Digit(unsigned char ch);
unsigned char MakeCheckSum(unsigned char *pPacket, unsigned char nCnt);
int RxPacketCheckCRC(unsigned char *pPacket);
int MakeTxPacket(unsigned char *pPacket, unsigned char nBleCmd, unsigned char nMagicCode, unsigned char *pMsg, int nMsgLen);
int FlashInitParameter(void);
int FlashGetParameter(void *ptr);
void FlashWriteParameter(unsigned int nWriteAddr, unsigned char *pBuffer, unsigned int nWriteSize);
#endif

