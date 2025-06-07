#include "evk_config.h"
#include <stdint.h>
#include "platform_api.h"


evk_config_t evk_config = {0};

void evk_config_init(void)
{
    evk_config.bin_addr = EVK_BIN_ADDR;
    evk_config.sw_dir = UART_DUT_2_EVK;
    evk_config.protocol = PROTOCOL_2_BURN;
    evk_config.work = 0;
}

uint8_t set_evk_protocol(uint8_t protocol)
{
    if(protocol < PROTOCOL_2_BURN || protocol > PROTOCOL_2_TEST)
    {
        return 0; 
    }
    evk_config.protocol = protocol;
    platform_printf("protocol is %x\n",evk_config.protocol);
    return 1;
}

void set_evk_work_mode(uint8_t work)
{
    evk_config.work = work;
}


uint8_t get_evk_protocol(void)
{
    return evk_config.protocol;
}

uint8_t get_evk_work(void)
{
    return evk_config.work;
}

evk_config_t *get_evk_config(void)
{
    return &evk_config;
}

void delay_ms(uint32_t ms)
{
    for(uint32_t i = 0; i < ms * 1000; i++)
    {
        __asm__ volatile ("nop");
    }
}
