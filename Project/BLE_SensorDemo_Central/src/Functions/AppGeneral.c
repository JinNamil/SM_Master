#include "AppGeneral.h"
#include "Common.h"
#include "bluenrg1_gap.h"
#include "clock.h"

P_BLE_SENSOR_DEV g_pBleSensorDevList;
E_BLE_STATUS g_nBleStatus = eBLE_STATUS_NONE;
E_BLE_MODE g_eBleMode = eBLE_MODE_NONE;
ST_FLASH_CFG g_stFlashCfg;		//jdkimkey Variable Malloc 19-04-29-15:33:38
ST_BLE_SENSOR_DATA stBleSensorData[BLE_SENSOR_MAX_NUM]={{0,0},};

unsigned int g_nCentralScanTimeOut = 0;
unsigned int g_nEmergencyTime = 0;
unsigned char g_nCollectBleSensorBit = 0;
unsigned char g_nBleTotNum = 0;
unsigned char g_nMacAddr[6]={0x0,};
unsigned char g_bFlagEmergency = FALSE;
char g_strDevName[ADV_DEVICE_NAME_LEN] = {AD_TYPE_COMPLETE_LOCAL_NAME,'2','M','A','N','H','O','L','E'};

void SetBleStatus(E_BLE_STATUS nBleStatus)
{
	g_nBleStatus = nBleStatus;
	return;
}

E_BLE_STATUS GetBleStatus(void)
{
	return g_nBleStatus;
}

void SetBleMode(E_BLE_MODE eBleMode)
{
	g_eBleMode = eBleMode;
}

E_BLE_MODE GetBleMode(void)
{
	return g_eBleMode;
}

P_BLE_SENSOR_DEV GetBleSensorDevPtr(void)
{
	return g_pBleSensorDevList;
}

void SetBleSensorDevPtr(P_BLE_SENSOR_DEV pBleSensorDevList)
{
	g_pBleSensorDevList = pBleSensorDevList;
	return;
}

void SetBleSensorTotNum(unsigned char nNum)
{
	g_nBleTotNum = nNum;	
}

unsigned char GetBleSensorTotNum(void)
{
	return g_nBleTotNum;
}

void ClearCollectBleSensor(void)
{
	g_nCollectBleSensorBit = 0;
}

void SetCollectBleSensor(unsigned char nBit)
{
  g_nCollectBleSensorBit |= nBit;
  return;
}

unsigned char GetCollectBleSensor(void)
{
	return g_nCollectBleSensorBit;
}

void SetAdvDevName(char *pName)
{
	memcpy(g_strDevName, pName, ADV_DEVICE_NAME_LEN);
	return;
}

char *GetAdvDevName(void)
{
	return g_strDevName;
}

void SetBlePeriMacAddr(unsigned char *pMacAddr)
{
	memcpy(g_nMacAddr, pMacAddr, BLE_MAC_ADDR_LEN);
	return;
}

unsigned char *GetBlePeriMacAddr(void)
{
	return g_nMacAddr;
}

P_FLASH_CFG GetFlashCfgGlobalPtr(void)
{
	return &g_stFlashCfg;
}

void SetFlashCfgGlobalPtr(P_FLASH_CFG pFlashCfg)
{
	memcpy(&g_stFlashCfg, pFlashCfg, sizeof(ST_FLASH_CFG));
	return;
}

void SetBleCentralScanTimeOut(unsigned int nTimeOut)
{
	g_nCentralScanTimeOut = nTimeOut;/*(unsigned int)(nTimeOut * 1000);*/
	return;
}

unsigned int GetBleCentralScanTimeOut(void)
{
	return g_nCentralScanTimeOut;
}

P_BLE_SENSOR_DATA GetBleSensorDataPtr(void)
{
	return stBleSensorData;
}

void InitSensorDataPtr(void)
{
	memset(stBleSensorData, 0L, (sizeof(ST_BLE_SENSOR_DATA)*BLE_SENSOR_MAX_NUM));
	return;
}

void SetEmergency(unsigned char bFlagEmergency)
{
	g_bFlagEmergency = bFlagEmergency;
	return;
}

unsigned char GetEmergency(void)
{
	return g_bFlagEmergency;
}

void SetEmergencyTime(unsigned int nTime)
{
	g_nEmergencyTime = nTime;
}

unsigned int GetEmergencyTime(void)
{
	return g_nEmergencyTime;
}
