
#ifndef _GATT_DB_H_
#define _GATT_DB_H_


#define BLE_TX_BUFFER_SIZE 20
#define BLE_CMD_SIZE 3

#define OTA_COMMAND_START_UPDATE		0x0A
#define OTA_COMMAND_SEND_PACKET			0x0B
#define OTA_COMMAND_COMPLETE_UPDATE		0x0C
#define OTA_COMMAND_CONNECTION			0x0D
#define OTA_COMMAND_COMPLETE_BANK_SWAP		0x0E
#define OTA_COMMAND_BLE_FLAG_UPDATE		0x0F
#define OTA_COMMAND_BLE_START_UPDATE		0xAF
#define OTA_COMMAND_SET_BLE_UPDATE_MODE	        0xBF
#define OTA_COMMAND_SET_MCU_UPDATE_MODE	        0xCF
#define OTA_COMMAND_ACK				0xAA
#define OTA_COMMAND_NACK			0xBB
#define OTA_COMMAND_DISCOVERY			0xCC
/* Master context */
typedef struct masterRoleContextS {
  uint8_t startDeviceDisc;
  uint16_t connHandle;
  uint8_t peer_addr_type;
  uint8_t peer_addr[6];
  uint8_t numPrimarySer;
  uint8_t primarySer[100];
  uint8_t findCharacOfService;
  uint8_t genAttFlag;
  uint8_t numCharacGenAttSer;
  uint8_t characGenAttSer[10];
  uint8_t mainFlag;
  uint8_t numCharacMainSer;
  uint8_t characMainSer[100];
  uint8_t otaFlag;
  uint8_t numCharacOtaSer;
  uint8_t characOtaSer[100];
  uint16_t mainHandle;
  uint16_t mainGetInfoHandle;
  uint16_t otaImageHandle;
  uint16_t otaNewImgHandle;
  uint16_t otaNewImgTuContentHandle;
  uint16_t otaExpImgTuSeqHandle;
  uint8_t enableNotif;
  uint8_t otaImageData[20];
  uint16_t otaImageDataLen;
} masterRoleContextType;

typedef struct connectionContexts{
  uint8_t isBleConnection;
  uint8_t isUartConnection;
  uint8_t isBleConnectionAlarm;
}connectionContexts_t;
  
typedef enum
{
  CONN_MCU_MODE = 1,
  CONN_BLE_MODE,
}connectionMode_t;
extern connectionContexts_t gConnectionContext;
extern masterRoleContextType masterContext;

uint8_t deviceInit(void);
uint8_t Device_Security(void);
uint8_t deviceDiscovery(void);
uint8_t deviceConnection(void);
void findCharcOfService(void);
void enableSensorNotifications(void);
void readTemperature(void);
void readMainFwTest(void);
void bleWriteTask(void);
void pcResponseTask(void);

#endif /* _GATT_DB_H_ */
