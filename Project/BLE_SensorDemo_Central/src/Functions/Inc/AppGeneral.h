#ifndef __APPGENERAL_H__
#define __APPGENERAL_H__

#include "Common.h"
/*********************************** Define General ***********************************/
#define ADV_DEVICE_NAME_LEN							10
#define BLE_SENSOR_DATA									4
#define BLE_MAC_ADDR_LEN								6
#define BLE_ADV_MAX_LEN									31
#define BLE_ADV_5BYTES									5
//#define __USE_PRINT_SENSOR_RAW__
/*********************************** Typedef Structer ***********************************/
typedef enum __eBLEStatus_
{
	eBLE_STATUS_NONE = 0x0,
	eBLE_STATUS_CENTRAL_SCANNING,
	eBLE_STATUS_CENTRAL_SCANDONE,
	eBLE_STATUS_CENTRAL_SENSOR_CONNECT,
	eBLE_STATUS_PERI_ADVERTISING,
	eBLE_STATUS_PERI_CONNECTED,
	eBLE_STATUS_SCANNED_BLE_TX_DONE
} E_BLE_STATUS;

typedef enum __eBLEMODE_
{
	eBLE_MODE_NONE		=0x0,
	eBLE_MODE_NRG_BOOT,
	eBLE_MODE_NRG_READY,
	eBLE_MODE_CENTRAL,
	eBLE_MODE_PERI
}	E_BLE_MODE;

typedef struct __stBLESENSORDATA_
{
	unsigned char nSensorData[BLE_SENSOR_DATA];
	unsigned char bFlagChecked;
}ST_BLE_SENSOR_DATA, *P_BLE_SENSOR_DATA;

typedef struct __stBLESENSOR_
{
	unsigned char nSensorType;																		//Info from Flash or MCU
	unsigned char nSensorMacAddr[BLE_MAC_ADDR_LEN];								//Info from Flash or MCU
	unsigned char bFlagIsConnectable;
	unsigned char nScanReq;
}ST_BLE_SENSOR_DEV, *P_BLE_SENSOR_DEV;		//9 Bytes

#pragma pack(push)
#pragma pack(1)
/*
	unsigned char nSensorData[BLE_SENSOR_DATA];
	unsigned char bFlagChecked;
*/
typedef struct __stFLASHCFG_
{
	ST_BLE_SENSOR_DEV stBleSensorDevList[BLE_SENSOR_MAX_NUM];	//45Bytes
	unsigned char nBleSensorTotNum;	//1Byte
	unsigned char nBleSensorScanTimeOut;	//1Byte
	unsigned char nBlePeriOwnMacAddr[BLE_MAC_ADDR_LEN];	//6Bytes
	unsigned char nZeroPad[3];
}ST_FLASH_CFG, *P_FLASH_CFG;		//88 Bytes	Must be 4bytes Align
/*Do not Make a Mistake to Define Structure for Saving Parameters to Flash, it must be 4Bytes Align.*/

#pragma pack(pop)

/************************************ Define Functions ************************************/
void SetBleStatus(E_BLE_STATUS nBleStatus);
E_BLE_STATUS GetBleStatus(void);
void SetBleMode(E_BLE_MODE eBleMode);
E_BLE_MODE GetBleMode(void);
P_BLE_SENSOR_DEV GetBleSensorDevPtr(void);
void SetBleSensorDevPtr(P_BLE_SENSOR_DEV pBleSensorDevList);
void SetBleSensorTotNum(unsigned char nNum);
unsigned char GetBleSensorTotNum(void);
void ClearCollectBleSensor(void);
void SetCollectBleSensor(unsigned char nBit);
unsigned char GetCollectBleSensor(void);
void SetAdvDevName(char *pName);
char *GetAdvDevName(void);
void SetBlePeriMacAddr(unsigned char *pMacAddr);
unsigned char *GetBlePeriMacAddr(void);
P_FLASH_CFG GetFlashCfgGlobalPtr(void);
void SetFlashCfgGlobalPtr(P_FLASH_CFG pFlashCfg);
void SetBleCentralScanTimeOut(unsigned int nTimeOut);
unsigned int GetBleCentralScanTimeOut(void);
P_BLE_SENSOR_DATA GetBleSensorDataPtr(void);
void InitSensorDataPtr(void);
void SetEmergency(unsigned char bFlagEmergency);
unsigned char GetEmergency(void);
void SetEmergencyTime(unsigned int nTime);
unsigned int GetEmergencyTime(void);
#endif
