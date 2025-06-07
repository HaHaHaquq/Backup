#ifndef _ANO_PROTOCOL_H_
#define _ANO_PROTOCOL_H_
#include <stdint.h>
#include <string.h>

#define PROTOCOL_HEAD 0xAB

#define COMMODN_LH      0xf1

#pragma pack(1)

typedef struct 
{
    uint8_t head;
    uint8_t src_addr;
    uint8_t dst_addr;
    uint8_t commond;
    uint16_t data_len;
    uint8_t data[0];
}ano_protocol_t;

#pragma pack()


#define PROTICOL_SIZE(len)          (sizeof(ano_protocol_t)+len+2)
#define PROTICOL_NOEND_SIZE(len)    (sizeof(ano_protocol_t)+len)

uint8_t packed_ano(uint8_t *out, uint8_t *in, uint32_t len);
#endif
