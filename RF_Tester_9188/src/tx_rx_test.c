#include "platform_api.h"
#include <stdint.h>
#include <stdio.h>

#include "rf_test.h"

void rf_test_tx_cnt(void)
{
    platform_printf("tx cnt test\n");
}

void rf_test_rx_cnt(void)
{
    platform_printf("tx cnt test\n");
}

RFTest_Result_t rx_test_tx_cnt_status(uint16_t *test_result)
{
    *test_result = 1;
    return TEST_SUCCESS;
}

RFTest_Result_t rf_test_tx_cnt_begin(void)
{
    platform_printf("Begin TX count test...\n");
    return TEST_SUCCESS;
}