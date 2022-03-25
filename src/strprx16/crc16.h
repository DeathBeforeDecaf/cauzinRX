#ifndef CRC16_H
#define CRC16_H

#ifdef __BORLANDC__
#include "src/strprx16/stdtypes.h"
#elif _MSC_VER
#include "stdtypes.h"
#endif

void crc16_clear();

uint16_t crc16_get();

void crc16_put( uint8_t inputByte );

#endif
