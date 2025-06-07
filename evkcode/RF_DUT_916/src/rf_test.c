#include "platform_api.h"
#include <stdint.h>
#include <stdio.h>

#include "profile.h"
#include "rf_test.h"
#include "uart_golden_DUT.h"
#include "raw_pkt.h"

#include "freq_offset.h"

#define CHECK_TEST_RUNNING(state) \
    do { \
        if ((state) <= STATE_FREQ_OFFSET_RUNNING) { \
            return TEST_RUNNING; \
        } \
    } while (0)

RF_test_state_t rf_test_state = STATE_TX_RX_IDLE;
rf_test_data_t rf_test_data = {
    .tx_cnt = 0,
    .rx_cnt = 0,
    .tx_power = 0,
    .rx_sen = 0,
    .freq_offset = 0
};

void loop_rf_test(void)
{
    // This function is a placeholder for the main loop of the RF test.
    // It can be used to call the test functions in a loop or based on events.
}

//just for test, will be removed later
//todo: implement the actual test logic
void tx_cnt_test_over(void)
{
    DBG_PRINTF("Tx cnt test over\n");
    if (rf_test_state == STATE_TX_CNT_RUNNING) {
        rf_test_state = STATE_TX_CNT_COMPLETE;
        rf_test_tx_cnt_end();
        // Here you can add code to handle the end of the TX count test.
    }
}

//just for test, will be removed later
//todo: implement the actual test logic
void tx_power_test_over(void)
{
    DBG_PRINTF("Tx power test over\n");
    if (rf_test_state == STATE_TX_POWER_RUNNING) {
        rf_test_state = STATE_TX_POWER_COMPLETE;
        rf_test_tx_power_end();
        // Here you can add code to handle the end of the TX power test.
    }
}

void set_dut_test_state(RF_test_state_t state)
{
    rf_test_state = state;
    DBG_PRINTF("DUT test state set to: %d\n", rf_test_state);
}

uint16_t get_rx_cnt_result(void)
{
    rf_test_data.rx_cnt = get_raw_pkt_count();
    return rf_test_data.rx_cnt;
}

int8_t get_rx_power_result(void)
{
    rf_test_data.rx_sen = get_raw_pkt_power();
    return rf_test_data.rx_sen;
}

RFTest_Result_t rf_test_tx_cnt_begin(uint8_t *buffer)
{
    CHECK_TEST_RUNNING(rf_test_state);
    //todo: deal with the actual TX count test logic
    set_rf_channel(buffer);
    set_rf_acc_addr(&buffer[2]);
    restart_raw_pkt_tx();
    rf_test_state = STATE_TX_CNT_RUNNING;
    DBG_PRINTF("Begin TX count test...\n");
    platform_set_timer(start_raw_tx_rx, 320);
    return TEST_SUCCESS;
}

RFTest_Result_t rf_test_tx_power_begin(void)
{
    CHECK_TEST_RUNNING(rf_test_state);
    //todo: deal with the actual TX power test logic
    rf_test_state = STATE_TX_POWER_RUNNING;
    DBG_PRINTF("Begin TX Power test...\n");
    platform_set_timer(tx_power_test_over, 320);
    return TEST_SUCCESS;
}

RFTest_Result_t rf_test_rx_cnt_begin(void)
{
    CHECK_TEST_RUNNING(rf_test_state);
    //todo: deal with the actual RX count test logic
    restart_raw_pkt_rx();
    rf_test_state = STATE_RX_CNT_RUNNING;
    DBG_PRINTF("Begin RX count test...\n");
    start_raw_tx_rx();
    return TEST_SUCCESS;
}

RFTest_Result_t rf_test_rx_sen_begin(void)
{
    CHECK_TEST_RUNNING(rf_test_state);
    //todo: deal with the actual RX sensitivity test logic
    rf_test_state = STATE_RX_SEN_RUNNING;
    DBG_PRINTF("Begin RX Sensitivity test...\n");
    return TEST_SUCCESS;
}

RFTest_Result_t rf_test_freq_offset_begin(uint8_t ch_id)
{
    CHECK_TEST_RUNNING(rf_test_state);
    //todo: deal with the actual Frequency Offset test logic
    freq_offset_tx_begin(ch_id);
    rf_test_state = STATE_FREQ_OFFSET_RUNNING;
    DBG_PRINTF("Begin Frequency Offset test ch:%d...\n", ch_id);
    return TEST_SUCCESS;
}

RFTest_Result_t rf_test_tx_cnt_end(void)
{
    if(rf_test_state == STATE_TX_CNT_COMPLETE) {
        DUT_cmd_rf_tx_cnt(0, rf_test_data.tx_cnt);
        return TEST_SUCCESS;
    }
    else if(rf_test_state == STATE_TX_CNT_RUNNING) {
        DBG_PRINTF("TX count test is still running...\n");
        return TEST_RUNNING;
    }
    else {
        DBG_PRINTF("TX count test is not started yet...\n");
        return TEST_FAILED;
    }
}


RFTest_Result_t rf_test_tx_power_end()
{
    if(rf_test_state == STATE_TX_POWER_COMPLETE) {
        DUT_cmd_rf_tx_power(0, rf_test_data.tx_power);
        return TEST_SUCCESS;
    }
    else if(rf_test_state == STATE_TX_POWER_RUNNING) {
        DBG_PRINTF("TX power test is still running...\n");
        return TEST_RUNNING;
    }
    else {
        DBG_PRINTF("TX power test is not started yet...\n");
        return TEST_FAILED;
    }
}

RFTest_Result_t rf_test_rx_cnt_end(void)
{
    if(rf_test_state == STATE_RX_CNT_RUNNING) {
        //todo: implement RX count end logic
        stop_raw_tx_rx();
        rf_test_state = STATE_RX_CNT_COMPLETE;

        return TEST_SUCCESS;
    }
    else {
        DBG_PRINTF("RX count test is not started yet...\n");
        return TEST_FAILED;
    }
}



RFTest_Result_t rf_test_rx_sen_end(void)
{
    if(rf_test_state == STATE_RX_SEN_RUNNING) {
        //todo: implement RX sensitivity end logic
        rf_test_state = STATE_RX_SEN_COMPLETE;
        return TEST_SUCCESS;
    }
    else {
        DBG_PRINTF("RX sensitivity test is not started yet...\n");
        return TEST_FAILED;
    }
}

RFTest_Result_t rf_test_freq_offset_end(void)
{
    if(rf_test_state == STATE_FREQ_OFFSET_RUNNING) {
        //todo: implement Frequency Offset end logic
        freq_offset_tx_end();
        rf_test_state = STATE_FREQ_OFFSET_COMPLETE;
        return TEST_SUCCESS;
    }
    else{
        DBG_PRINTF("Frequency offset test is not started yet...\n");
        return TEST_FAILED;
    }
}


