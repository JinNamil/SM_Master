/******************** (C) COPYRIGHT 2012 STMicroelectronics ********************
* File Name          : gatt_db.c
* Author             : AMS - VMA division
* Version            : V1.0.0
* Date               : 25-August-2016
* Description        : Functions to build GATT DB and handle GATT events.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/


#include "hal_types.h"
#include <string.h>
#include "ble_const.h"
#include <stdio.h>
#include <stdlib.h>
#include "SDK_EVAL_Config.h"
#include "master_basic_profile.h"
#include "master_config.h"
#include "gatt_db.h"
#include "BleUartFunc.h"
#include "BlueNRG1_it.h"

#define BLE_MAC_ADDR_LEN								6
#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/* Primary Service UUID expected from Sensor demo peripheral */
uint8_t GENERIC_ACCESS_PROFILE_UUID[]={0x00, 0x18};
uint8_t GENERIC_ATTRIBUTE_PROFILE_UUID[]={0x01, 0x18};

/* Service and Characteristic UUID expected from slave peripheral */
uint8_t MAIN_FIRMWARE_SERVICE_UUID[] = {0xE5,0x49,0xD2,0x79,  0x4F,0x32,  0x18,0xBD,  0x48,0x41,  0x54,0x50,0x00,0x90,0x58,0xDE};
uint8_t MAIN_FIRMWARE_WRITE_CHAR_UUID[] = {0xE5,0x49,0xD2,0x79,  0x4F,0x32,  0x18,0xBD,  0x48,0x41,  0x54,0x50,0x01,0x90,0x58,0xDE};
uint8_t MAIN_FIRMWARE_READ_CHAR_UUID[] = {0xE5,0x49,0xD2,0x79,  0x4F,0x32,  0x18,0xBD,  0x48,0x41,  0x54,0x50,0x02,0x90,0x58,0xDE};

extern uint8_t getUpdatePacketSize(void);
masterRoleContextType masterContext;
securitySetType sec_param;
connectionContexts_t gConnectionContext;
uint8_t gUpdateBlockData[64] = {0,};


uint8_t Device_Security(void)
{
  uint8_t ret; 

  ret = Master_ClearSecurityDatabase();
  
  sec_param.ioCapability = IO_CAPABILITY;
  sec_param.mitm_mode = MITM_MODE;
  
  sec_param.Min_Encryption_Key_Size = MIN_KEY_SIZE; 
  sec_param.Max_Encryption_Key_Size = MAX_KEY_SIZE;
  sec_param.bonding_mode = BONDING;
  sec_param.use_fixed_pin = FIXED_PIN_POLICY;
    
  sec_param.fixed_pin = MASTER_PASS_KEY; /* used only if use fixed pin policy is enabled */
  
  /* BLE Security v4.2 is supported: BLE stack FW version >= 2.x */
  sec_param.sc_support = SECURE_CONNECTION_SUPPORT; 
  sec_param.keypress_support = KEYPRESS_NOTIFICATION; 
  sec_param.identity_address_type = IDENTITY_ADDRESS; 
       
  ret = Master_SecuritySet(&sec_param);
  if (ret != BLE_STATUS_SUCCESS) {
    PRINTF("Error in Master_SecuritySet() 0x%02x\r\n", ret);
  
  }
  return ret; 
}
/*******************************************************************************
* Function Name  : deviceInit
* Description    : Device init
* Input          : None
* Return         : Status of the call
*******************************************************************************/
uint8_t deviceInit(void)
{
  uint8_t ret;
  uint8_t bdaddr[] = MASTER_PUBLIC_ADDRESS;
  uint8_t devName[18] = "SensorDemo Central";
  initDevType param;

  /* Master context init */
  masterContext.startDeviceDisc = FALSE;
  masterContext.findCharacOfService = FALSE;
  masterContext.genAttFlag = FALSE;
  masterContext.mainFlag = FALSE;
  masterContext.enableNotif = FALSE;
  gConnectionContext.isBleConnection = FALSE;
  gConnectionContext.isUartConnection = FALSE;
  masterContext.connHandle = 0xFFFF;

  /* Master device init */
  memcpy(param.public_addr, bdaddr, 6);
  param.device_name_len = 18;
  param.device_name = devName;
  param.txPowerLevel = 4; // -2.1 dBm
  ret = Master_Init(&param);
  if (ret != BLE_STATUS_SUCCESS) {
    PRINTF("Master_Init() status: 0x%02x\r\n", ret);
  }
  return ret;
}

/*******************************************************************************
* Function Name  : deviceDiscovery
* Description    : Starts the device discovery proceudre
* Input          : None
* Return         : Status of the call
*******************************************************************************/
uint8_t deviceDiscovery(void)
{
  uint8_t ret;
  devDiscoveryType devDiscParam;

  devDiscParam.procedure = GENERAL_DISCOVERY_PROCEDURE;
  devDiscParam.scanInterval = LIM_DISC_SCAN_INT;
  devDiscParam.scanWindow = LIM_DISC_SCAN_WIND;
  devDiscParam.own_address_type = PUBLIC_DEVICE_ADDRESS;
  ret = Master_DeviceDiscovery(&devDiscParam);
  if (ret != BLE_STATUS_SUCCESS) {
    PRINTF("Master_DeviceDiscovery() status: 0x%02x\r\n", ret);
  } else {
    PRINTF("****Start Device Discovery Procedure\r\n");
  }
  return ret;
}

/*******************************************************************************
* Function Name  : deviceConnection
* Description    : Starts the connection proceudre with the peripheral device
* Input          : None
* Return         : Status of the call
*******************************************************************************/
uint8_t deviceConnection(void)
{
  uint8_t ret;
  connDevType connParam;

  connParam.procedure = DIRECT_CONNECTION_PROCEDURE;
  connParam.fastScanDuration = FAST_SCAN_DURATION;
  connParam.fastScanInterval = FAST_SCAN_INTERVAL;  
  connParam.fastScanWindow = FAST_SCAN_WINDOW;
  connParam.reducedPowerScanInterval = REDUCED_POWER_SCAN_INTERVAL;
  connParam.reducedPowerScanWindow = REDUCED_POWER_SCAN_WINDOW;
  connParam.peer_addr_type = masterContext.peer_addr_type;
  memcpy(connParam.peer_addr, masterContext.peer_addr, 6);
  connParam.own_addr_type =0;
  connParam.conn_min_interval = FAST_MIN_CONNECTION_INTERVAL;
  connParam.conn_max_interval = FAST_MAX_CONNECTION_INTERVAL;
  connParam.conn_latency = FAST_CONNECTION_LATENCY;
  connParam.supervision_timeout = SUPERVISION_TIMEOUT;
  connParam.min_conn_length = 5;
  connParam.max_conn_length = 5;
  ret = Master_DeviceConnection(&connParam);
  if (ret != BLE_STATUS_SUCCESS) 
    PRINTF("Master_DeviceConnection() failed with status: 0x%02x\r\n", ret);
  return ret;
}

/*******************************************************************************
* Function Name  : primaryServiceFound
* Description    : Function used to print the primary service found
* Input          : 
* Return         : 
*******************************************************************************/
void primaryServiceFound(void)
{
  uint8_t i, index, sizeElement;
  uint8_t uuid[16];

  index = 0;
  for (i=0; i<masterContext.numPrimarySer; i++) { 
    if (masterContext.primarySer[index] == 1)
      sizeElement = 6;
    else
      sizeElement = 20;
    index++;
    PRINTF("%d-- ", i+1);
    memcpy (uuid, &masterContext.primarySer[index+4], (sizeElement-4));
    if (memcmp(uuid, GENERIC_ACCESS_PROFILE_UUID, (sizeElement-4)) == 0) {
      PRINTF("Generic Access Profile\r\n");
    }
    if (memcmp (uuid, GENERIC_ATTRIBUTE_PROFILE_UUID, (sizeElement-4)) == 0) {
      PRINTF("Generic Attribute Profile\r\n");
      masterContext.genAttFlag = TRUE;
      masterContext.findCharacOfService = TRUE;
    }
    if (memcmp (uuid, MAIN_FIRMWARE_SERVICE_UUID, (sizeElement-4)) == 0) {
      PRINTF("Main FW Update Service\r\n");
    }
    index += sizeElement;
  }
  PRINTF("********************************************************\r\n");
}

/*******************************************************************************
* Function Name  : findCharacOfService
* Description    : Search the characteristics of a service
* Input          : 
* Return         : 
*******************************************************************************/
void findCharcOfService(void)
{
  uint8_t ret, i, index, sizeElement, uuid[16], max_size = 0;
  uint16_t startHandle, endHandle;
  uint8_t *numCharac = NULL, *charac = NULL;

  index = 0;
  for (i=0; i<masterContext.numPrimarySer; i++) { 
    if (masterContext.primarySer[index] == 1)
      sizeElement = 6;
    else
      sizeElement = 20;
    index++;
    memcpy (uuid, &masterContext.primarySer[index+4], (sizeElement-4));
    /* Discovery all the characteristics of the Generic Attribute Profile */
    if ((memcmp(uuid, GENERIC_ATTRIBUTE_PROFILE_UUID, (sizeElement-4)) == 0) && masterContext.genAttFlag) {
      memcpy((uint8_t*)&startHandle, &masterContext.primarySer[index], 2);
      memcpy((uint8_t*)&endHandle, &masterContext.primarySer[index+2], 2);
      numCharac = &masterContext.numCharacGenAttSer;
      charac = masterContext.characGenAttSer;
      max_size = sizeof(masterContext.characGenAttSer);
    }
    
    /* Discovery all the characteristics of the Slave Peripheral Service */
    if ((memcmp(uuid, MAIN_FIRMWARE_SERVICE_UUID, (sizeElement-4)) == 0) && masterContext.mainFlag) {
      memcpy((uint8_t*)&startHandle, &masterContext.primarySer[index], 2);
      memcpy((uint8_t*)&endHandle, &masterContext.primarySer[index+2], 2);
      numCharac = &masterContext.numCharacMainSer;
      charac = masterContext.characMainSer;
      max_size = sizeof(masterContext.characMainSer);
    }
    
    index += sizeElement;
  }

  ret = Master_GetCharacOfService(masterContext.connHandle, startHandle, endHandle,
  				  numCharac, charac, max_size);

  if (ret != BLE_STATUS_SUCCESS) {
    ;
  }
}

/*******************************************************************************
* Function Name  : extractCharacInfo
* Description    : Extracts the handle information for all the characteristics found
* Input          : 
* Return         : 
*******************************************************************************/
void extractCharacInfo(void)
{
  uint8_t i;

  /* Store in the master context the characteristic handles for the tempereture */
  for (i=0; i<masterContext.numCharacMainSer; i++) {
    if (memcmp(&masterContext.characMainSer[6+(i*22)], MAIN_FIRMWARE_WRITE_CHAR_UUID, 16) == 0) {
      memcpy((uint8_t*)&masterContext.mainHandle, &masterContext.characMainSer[1+(i*22)], 2);
    }
    if (memcmp(&masterContext.characMainSer[6+(i*22)], MAIN_FIRMWARE_READ_CHAR_UUID, 16) == 0) {
      memcpy((uint8_t*)&masterContext.mainGetInfoHandle, &masterContext.characMainSer[1+(i*22)], 2);
    }
  }
}

/*******************************************************************************
* Function Name  : Master_DeviceDiscovery_CB
* Description    : Device discovery callback from master basic profile library
* Input          : 
* Return         : 
*******************************************************************************/
void Master_DeviceDiscovery_CB(uint8_t *status, uint8_t *addr_type, uint8_t *addr, 
			       uint8_t *data_length, uint8_t *data,
			       int8_t *RSSI)
{
  uint8_t i, deviceOffset = 0;
  uint8_t scannedMacAddr[BLE_MAC_ADDR_LEN] = {0,};
  uint8_t slaveMacAddr[BLE_MAC_ADDR_LEN] = {0xBC, 0x9A, 0x78, 0x56, 0x34, 0x11};
//  uint8_t slaveMacAddr[BLE_MAC_ADDR_LEN] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC};
  uint8_t deviceName[6] = {0,};
  uint32_t deviceNameLen = 0;
  
  for(int j = 5; j < 12; j++)
  {
    deviceName[deviceOffset++] = data[j];
  }
  deviceNameLen = *data_length;

  if (*status == DEVICE_DISCOVERED) {
    for(i=0; i<BLE_MAC_ADDR_LEN; i++)
    {
      scannedMacAddr[i] = addr[BLE_MAC_ADDR_LEN-i-1];
    }	// Ordering Mac Address
    
    
    
    if(memcmp(slaveMacAddr, scannedMacAddr, BLE_MAC_ADDR_LEN) == 0)
    {
      /* If the device found is a BlueNRG try to connect */
//      if (!memcmp(&data[5], "2MANHOLE", 8))
      { 
        PRINTF("BlueNRG device found\r\n");
        PRINTF("MAC Address: ");
        for(i = 0; i < BLE_MAC_ADDR_LEN; i++)
        {
            PRINTF("%02x:", scannedMacAddr[i]);
        }
        PRINTF("\r\n");
        
        masterContext.peer_addr_type = *addr_type;
        memcpy(masterContext.peer_addr, addr, 6);
        if (deviceConnection() != BLE_STATUS_SUCCESS)
          PRINTF("Error during the device connection procedure\r\n");      
      } /*Find Defined Sensors*/
    }
  }
  if (*status == DEVICE_DISCOVERY_PROCEDURE_ENDED)
    PRINTF("***Device Discovery Procedure ended from the application\r\n");

  if (*status == DEVICE_DISCOVERY_PROCEDURE_TIMEOUT) {
    PRINTF("***Device Discovery Procedure Timeout\r\n");
    if (deviceDiscovery() != BLE_STATUS_SUCCESS) {
      PRINTF("Error during the device discovery procedure\r\n");
    }
  }    
}

/*******************************************************************************
* Function Name  : Master_Connection_CB
* Description    : Connection callback from master basic profile library
* Input          : 
* Return         : 
*******************************************************************************/
void Master_Connection_CB(uint8_t *connection_evt, uint8_t *status, uint16_t *connection_handle, connUpdateParamType *param)
{
  uint8_t ret;

  switch (*connection_evt) {
  case CONNECTION_ESTABLISHED_EVT:
    PRINTF("\r\n****Connection Established (handle 0x%04x)\r\n", *connection_handle);
    /* Start the discovery all primary services procedure */
    if (*status == BLE_STATUS_SUCCESS) {
      masterContext.connHandle = *connection_handle;
      ret = Master_GetPrimaryServices(masterContext.connHandle, &masterContext.numPrimarySer,
				     masterContext.primarySer, sizeof(masterContext.primarySer));
      if (ret != BLE_STATUS_SUCCESS) 
	      PRINTF("\r\n****Master_GetPrimaryService() procedure fails with status 0x%02x\r\n", ret);
    }
    break;
  case CONNECTION_FAILED_EVT:
    PRINTF("****CONNECTION FAILED EVENT\r\n");
    break;
  case DISCONNECTION_EVT:
    PRINTF("****DISCONNECTION EVENT\r\n");
    /* Restart Discovery & Connection procedure */
   gConnectionContext.isBleConnection = FALSE;
//    deviceDiscovery();
    masterContext.connHandle = 0xFFFF;
//    deviceConnection();
    if(getUpdateMode())
      initBleUpdateTimeout();
    break;
  case STOP_CONNECTION_PROCEDURE_EVT:
    PRINTF("****STOP CONNECTION PROCEDURE EVENT\r\n");
    break;
  case CONNECTION_UPDATE_PARAMETER_REQUEST_EVT:
    PRINTF("****CONNECTION UPDATE PARAMETER REQUEST EVENT\r\n");
    break;
  }
}

/*******************************************************************************
* Function Name  : Master_ServiceCharacPeerDiscovery_CB
* Description    : Service and Characteristics discovery callback from master basic profile library
* Input          : 
* Return         : 
*******************************************************************************/
void Master_ServiceCharacPeerDiscovery_CB(uint8_t *procedure, uint8_t *status, uint16_t *connection_handle)
{
  switch(*procedure) {
  case PRIMARY_SERVICE_DISCOVERY:
    if (*status == BLE_STATUS_SUCCESS) {
      primaryServiceFound();
    }
    break;
  case GET_CHARACTERISTICS_OF_A_SERVICE:
    {
      if (*status == BLE_STATUS_SUCCESS)
      {
        /* Start the enable indication procedure for the service changed charac */
        if(masterContext.genAttFlag)
        {
          masterContext.genAttFlag = FALSE;
          masterContext.mainFlag = TRUE;
          masterContext.findCharacOfService = TRUE;
        }
        else if(masterContext.mainFlag)
        {
          extractCharacInfo();
          masterContext.genAttFlag = TRUE;
          masterContext.mainFlag = FALSE;
          masterContext.enableNotif = TRUE;
        }
      }
     }
    break;
  }
}
/*******************************************************************************
* Function Name  : enableSensorNotifications
* Description    : Enables the sensor notifications and indications
* Input          : 
* Return         : 
*******************************************************************************/
void enableSensorNotifications(void)
{
  uint8_t ret;
  uint16_t handle;

  /* Enable Service changed notification */
  if (masterContext.genAttFlag) {
    memcpy ((uint8_t*)&handle, &masterContext.characGenAttSer[4], 2);
    ret = Master_NotifIndic_Status(masterContext.connHandle, (handle+2), FALSE, TRUE);
    if (ret != BLE_STATUS_SUCCESS)
      ;
  }

  if(masterContext.mainFlag)
  {
    if(--masterContext.numCharacMainSer != 0)
      handle = masterContext.mainHandle + 2;
    else
      handle = masterContext.mainGetInfoHandle + 2;
    
   ret = Master_NotifIndic_Status(masterContext.connHandle, handle, TRUE, FALSE);
   if(ret != BLE_STATUS_SUCCESS)
      ;
  }
}

void pcResponseTask(void)
{
  if(GetBleStatus() == STATUS_BLE_SEND_DATA_COMPLETE)
  {
    DMA_CH_UART_RX->CCR_b.EN = RESET;
    putchar(OTA_COMMAND_ACK);
    SetBleStatus(STATUS_PC_REQUEST_DATA_WAIT);
    DMA_CH_UART_RX->CCR_b.EN = SET;
  }
  else if(GetBleStatus() == STATUS_PC_REQUEST_BANK_SWAP_COMPLETE)
  {
    DMA_CH_UART_RX->CCR_b.EN = RESET;
    putchar(OTA_COMMAND_ACK);
    SetBleStatus(STATUS_PC_REQUEST_COMMAND_WAIT);
    DMA_CH_UART_RX->CCR_b.EN = SET;
  }
}

void bleWriteTask(void)
{
  if((GetBleStatus() == STATUS_PC_REQUEST_DATA_RECV) || (GetBleStatus() == STATUS_PC_REQUEST_COMMAND_RECV) 
     || (GetBleStatus() == STATUS_PC_REQUEST_BANK_SWAP_RECV))
  {
    if (Master_WriteWithoutResponse_Value(masterContext.connHandle, masterContext.mainHandle+1, getUpdatePacketSize(), gUpdateBlockData) 
        != BLE_STATUS_SUCCESS)
    {
      initBleUpdateTimeout();  
    }
    else
    {      
      if(GetBleStatus() == STATUS_PC_REQUEST_DATA_RECV)
      {
        putchar(OTA_COMMAND_ACK);
        SetBleStatus(STATUS_PC_REQUEST_DATA_WAIT);
//        SetBleStatus(STATUS_BLE_SEND_DATA_COMPLETE);
      }
      else if(GetBleStatus() == STATUS_PC_REQUEST_COMMAND_RECV)
        SetBleStatus(STATUS_BLE_SEND_COMMAND_COMPLETE);
      else if(GetBleStatus() == STATUS_PC_REQUEST_BANK_SWAP_RECV)
      {        
        putchar(OTA_COMMAND_ACK);
        SetBleStatus(STATUS_PC_REQUEST_COMMAND_WAIT);
//        SetBleStatus(STATUS_PC_REQUEST_BANK_SWAP_COMPLETE);
      }
    }
  }
}

/*******************************************************************************
* Function Name  : Master_PeerDataExchange_CB
* Description    : Peer Device data exchange callback from master basic profile library
* Input          : 
* Return         : 
*******************************************************************************/
void Master_PeerDataExchange_CB(uint8_t *procedure, uint8_t *status, uint16_t *connection_handle, dataReceivedType *data)
{
  switch(*procedure) {
  case NOTIFICATION_INDICATION_CHANGE_STATUS:
    {
      if (masterContext.genAttFlag) {
        masterContext.genAttFlag = FALSE;
        masterContext.mainFlag = TRUE;
        masterContext.enableNotif = TRUE;
      } 
      else {
       if (masterContext.mainFlag) {
         if(masterContext.numCharacMainSer == 0)
         {
            masterContext.mainFlag = FALSE;
            gConnectionContext.isBleConnection = TRUE;
            putchar(OTA_COMMAND_DISCOVERY);
            PRINTF("\r\n****Service Changed indication enabled mainFlag complete\r\n");
         }
         else
         {
            masterContext.mainFlag = TRUE;
         }
          masterContext.enableNotif = TRUE;
      }
    }
    }
    break;
  case NOTIFICATION_DATA_RECEIVED:
    {
      if ((masterContext.mainGetInfoHandle+1) == data->attr_handle)
      {
        if((data->data_length == 1) && (data->data_value[0] == OTA_COMMAND_START_UPDATE))
        {
          UartWrite(data->data_value, data->data_length);
          SetBleStatus(STATUS_PC_REQUEST_DATA_WAIT);
        }
        else if((data->data_length == 2) && (data->data_value[0] == OTA_COMMAND_COMPLETE_UPDATE))
        {
            UartWrite(data->data_value, data->data_length);
            SetBleStatus(STATUS_PC_REQUEST_COMMAND_WAIT);
        }
      }
    }
    break;
  }
}

