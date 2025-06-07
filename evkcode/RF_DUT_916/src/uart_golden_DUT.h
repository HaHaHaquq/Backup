#ifndef _UART_GOLDEN_DUT_H_
#define _UART_GOLDEN_DUT_H_

#include <stdint.h>

#define UART_DUT_BUFF_SIZE 64

void uart_DUT_init(uint32_t Baud_Rate);
// void uart_DUT_send(const uint8_t * buffer, uint16_t len);

void DUT_cmd_rf_tx_cnt(uint8_t en, uint16_t status);
void DUT_cmd_rf_tx_power(uint8_t en, uint8_t status);
void DUT_cmd_rf_rx_cnt(uint8_t en, uint8_t status, uint16_t result);
void DUT_cmd_rf_rx_sen(uint8_t en, uint8_t status, int8_t result);
void DUT_cmd_rf_freq_offset(uint8_t en, uint8_t status);


#endif


