#ifndef CRC16_H
#define CRC16_H

#include "stdtypes.h"

void crc16_clear();

uint16_t crc16_get();

void crc16_put( uint8_t inputByte );

#endif
