#pragma once
#include <stdio.h>
#include <stdint.h>

#define EVK_BIN_ADDR 0x0002E000

typedef enum :uint8_t{
    UART_DUT_2_EVK,
    UART_DUR_2_AMO,
    UART_AMO_2_EVK,
    UART_AMO_2_IDKE
}uart_direction;

typedef enum : uint8_t{
    PROTOCOL_2_PC,
    PROTOCOL_2_DUT,
}evk_direction;

typedef struct{
    uart_direction sw_dir;
    evk_direction protocol;
    uint8_t work;
    uint32_t bin_addr;

}evk_config_t;


evk_config_t *get_evk_config(void);
void evk_config_init(void);