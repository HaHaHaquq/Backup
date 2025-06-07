#ifndef __CRC_16_H
#define __CRC_16_H
#include <stdint.h>

unsigned short check_crc16(char *puchMsg , unsigned short usDataLen);
unsigned short check_crc16_918(uint8_t  *puchMsg , uint16_t usDataLen);


#endif

