#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_


#ifdef	__cplusplus
extern "C" {	/* allow C++ to use these headers */
#endif	/* __cplusplus */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define POLY 0x8005
#define HEAD    0x58
#define TAIL    0X16

#define START_UPLOAD 0x51

#pragma pack(1)
typedef struct
{
    uint8_t head;
    uint32_t addr;
    uint32_t cmd;
    uint32_t len;
    uint8_t data[0];
}protocol_t;

typedef struct
{
    uint32_t hardware;
    uint32_t software;
    uint32_t one_pack_len;
    uint32_t all_pack_num;
    uint16_t crc;
}startup_pack_t;

typedef struct
{
    uint32_t hardware;
    uint32_t software;
    uint32_t all_pack_num;
    uint8_t crc_state;//0:crc_ok 1:crc_error
}end_pack_t;

typedef struct
{
    uint32_t now_pack_num;
    uint32_t all_pack_num;
}end_pack_t;

#define FARM_MIN_LEN (sizeof(protocol_t) + 1)

#pragma pack()

#ifdef __cplusplus
} /* allow C++ to use these headers */
#endif	/* __cplusplus */

#endif
