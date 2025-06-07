#pragma once
#include <stdio.h>
#include <stdint.h>

#define EVK_BIN_ADDR 0x00034000

typedef enum :uint8_t{
    UART_DUT_2_EVK = 0,
    UART_DUR_2_AMO,
    UART_AMO_2_EVK,
    UART_AMO_2_IDKE
}uart_direction;

typedef enum : uint8_t{
    PROTOCOL_2_BURN = 0,
    PROTOCOL_2_TEST,
}evk_direction;

typedef struct{
    uart_direction sw_dir;
    evk_direction protocol;
    uint8_t work;
    uint32_t bin_addr;
}evk_config_t;


evk_config_t *get_evk_config(void);
uint8_t get_evk_protocol(void);
uint8_t get_evk_work(void);

void evk_config_init(void);
uint8_t set_evk_protocol(uint8_t protocol);
void set_evk_work_mode(uint8_t work);

void delay_ms(uint32_t ms);
