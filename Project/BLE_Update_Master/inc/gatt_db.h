
#ifndef _GATT_DB_H_
#define _GATT_DB_H_


#define BLE_TX_BUFFER_SIZE 20

#define OTA_COMMAND_START_UPDATE		0x0A
#define OTA_COMMAND_SEND_PACKET			0x0B
#define OTA_COMMAND_COMPLETE_UPDATE		0x0C
#define OTA_COMMAND_CONNECTION			0x0D
#define OTA_COMMAND_COMPLETE_BANK_SWAP		0x0E
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
  uint16_t mainHandle;
  uint16_t mainGetInfoHandle;
  uint8_t enableNotif;
} masterRoleContextType;

typedef struct connectionContexts{
  uint8_t isBleConnection;
  uint8_t isUartConnection;
}connectionContexts_t;
  
extern connectionContexts_t gConnectionContext;
extern masterRoleContextType masterContext;

uint8_t deviceInit(void);
uint8_t deviceDiscovery(void);
uint8_t deviceConnection(void);
void findCharcOfService(void);
void enableSensorNotifications(void);
void readTemperature(void);
void readMainFwTest(void);
void bleWriteTask(void);
void pcResponseTask(void);
uint8_t Device_Security(void);
  
#endif /* _GATT_DB_H_ */
