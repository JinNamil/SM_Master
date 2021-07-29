/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Implements a FIFO buffer

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/

#include "fifo.h"
#include "Common.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define/constants---------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

static uint16_t FifoNext( Fifo_t *fifo, uint16_t index )
{
    return ( index + 1 ) % fifo->Size;
}

void FifoInit( Fifo_t *fifo, uint8_t *buffer, uint16_t size )
{
    fifo->Begin = 0;
    fifo->End = 0;
    fifo->Peek = 0;
    fifo->Data = buffer;
    fifo->Size = size;
}

void FifoPush( Fifo_t *fifo, uint8_t data )
{
    fifo->End = FifoNext( fifo, fifo->End );
    fifo->Data[fifo->End] = data;
}

uint8_t FifoPop( Fifo_t *fifo )
{
    uint8_t data = fifo->Data[FifoNext( fifo, fifo->Begin )];

    fifo->Begin = FifoNext( fifo, fifo->Begin );
    fifo->Peek = fifo->Begin;
    return data;
}

uint8_t FifoPeek( Fifo_t *fifo )
{
    uint8_t data = fifo->Data[FifoNext( fifo, fifo->Peek )];

    fifo->Peek = FifoNext( fifo, fifo->Peek );
    return data;
}

void FifoPeekFlush( Fifo_t *fifo )
{
    fifo->Peek = fifo->Begin;
}

void FifoFlush( Fifo_t *fifo )
{
    fifo->Begin = 0;
    fifo->End = 0;
    fifo->Peek = 0;  
}

bool IsFifoEmpty( Fifo_t *fifo )
{
    return ( fifo->Begin == fifo->End );
}

bool IsFifoFull( Fifo_t *fifo )
{
    return ( FifoNext( fifo, fifo->End ) == fifo->Begin );
}

uint16_t GetFifoDataSize( Fifo_t *fifo )
{
  uint16_t length;
  
  if ( fifo->End >= fifo->Begin) 
    length = fifo->End - fifo->Begin;
  else
    length = fifo->Size - fifo->Begin + fifo->End;
  
  return length;
}
