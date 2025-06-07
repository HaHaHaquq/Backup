#ifndef _TX_RX_TEST_H_
#define _TX_RX_TEST_H_

#include <stdint.h>

RFTest_Result_t rf_test_tx_cnt_run(void);
RFTest_Result_t rx_test_tx_cnt_status(uint16_t *test_result);
RFTest_Result_t rf_test_tx_cnt_begin(void);

#endif


