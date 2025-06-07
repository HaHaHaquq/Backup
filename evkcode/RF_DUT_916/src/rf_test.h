#ifndef _RF_TEST_H_
#define _RF_TEST_H_

#include <stdint.h>
#include "rf_test.h"

typedef enum {
    STATE_TX_CNT_RUNNING  = 0,
    STATE_TX_POWER_RUNNING,
    STATE_RX_CNT_RUNNING,
    STATE_RX_SEN_RUNNING,
    STATE_FREQ_OFFSET_RUNNING,

    STATE_TX_CNT_COMPLETE = 0x10,
    STATE_TX_POWER_COMPLETE,
    STATE_RX_CNT_COMPLETE,
    STATE_RX_SEN_COMPLETE,
    STATE_FREQ_OFFSET_COMPLETE,
    STATE_TX_RX_IDLE,
} RF_test_state_t;

typedef struct {
    uint16_t tx_cnt;
    uint16_t rx_cnt;
    int8_t tx_power;
    int8_t rx_sen;
    int8_t freq_offset;
} rf_test_data_t;

typedef enum {
    TEST_SUCCESS,
    TEST_RUNNING,
    TEST_FAILED
} RFTest_Result_t;

RFTest_Result_t rf_test_tx_power_begin(void);
RFTest_Result_t rf_test_tx_power_end(void);

RFTest_Result_t rf_test_tx_cnt_begin(uint8_t *buffer);
RFTest_Result_t rf_test_tx_cnt_end(void);

RFTest_Result_t rf_test_rx_sen_begin(void);
RFTest_Result_t rf_test_rx_sen_end(void);

RFTest_Result_t rf_test_rx_cnt_begin(void);
RFTest_Result_t rf_test_rx_cnt_end(void);

RFTest_Result_t rf_test_freq_offset_begin(uint8_t ch_id);
RFTest_Result_t rf_test_freq_offset_end(void);

void tx_cnt_test_over(void);
uint16_t get_rx_cnt_result(void);
int8_t get_rx_power_result(void);
void set_dut_test_state(RF_test_state_t state);

#endif


