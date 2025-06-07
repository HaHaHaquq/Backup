#ifndef _UART_GOLDEN_DUT_H_
#define _UART_GOLDEN_DUT_H_

#include <stdint.h>

#define UART_GOLDEN_BUFF_SIZE 256

#define UART_DUT_BUFF_SIZE 64

void uart_DUT_init(uint32_t Baud_Rate);
// void uart_DUT_send(const uint8_t * buffer, uint16_t len);

void DUT_cmd_rf_tx_cnt(uint8_t en, uint16_t channel, uint32_t acc_addr);
void DUT_cmd_rf_tx_power(uint8_t en, uint16_t channel);
void DUT_cmd_rf_rx_cnt(uint8_t en, uint16_t channel);
void DUT_cmd_rf_rx_sen(uint8_t en, uint16_t channel);
void DUT_cmd_rf_freq_offset(uint8_t en, uint16_t channel);
void DUT_cmd_rf_freq_offset_tune(uint8_t tune_value);

void driver_append_tx_data(const void *data, uint16_t len);
#endif


