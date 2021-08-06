#ifndef __COMMON_H__
#define __COMMON_H__
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "blueNRG1_flash.h"
#include "hal_types.h"

/*********************************** Define General ***********************************/
#define MAX_TRX_BUF_SIZE								32
#define ASCII_CR												0x0A
#define ASCII_LF												0x0D
/*********************************** Typedef Structer ***********************************/
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
#endif

