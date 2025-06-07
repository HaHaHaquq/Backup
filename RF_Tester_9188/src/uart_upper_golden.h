#ifndef _UART_PROTOCOL_H_
#define _UART_PROTOCOL_H_

#include <stdint.h>

typedef struct {
    uint16_t tx_cnt;
    uint16_t rx_cnt;
    uint8_t tx_power;
    uint8_t rx_power;
    uint8_t freq_offset; 
} test_result_t;

#endif


