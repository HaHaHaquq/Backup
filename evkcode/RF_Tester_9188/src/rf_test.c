#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "rf_test.h"
#include "platform_api.h"
#include "profile.h"
#include "uart_golden_DUT.h"
#include "raw_pkt.h"
#include "evk_config.h"
#include "uart_ota.h"
#include "freq_offset.h"
#include "sw_gpio.h"


rf_test_data_t rf_test_data = {
    .tx_cnt = 0,
    .rx_cnt = 0,
    .tx_power = 0,
    .rx_sen = 0,
    .freq_offset = 0,
    .status = {
        .tx_cnt_pass = false,
        .rx_cnt_pass = false,
        .tx_power_pass = false,
        .rx_sen_pass = false,
        .freq_offset_pass = false
    }
};

static rf_test_ctrl_t rf_test_ctrl = {
    .ch_id = 2,  // Default channel ID
    .frequency = 2404, //// Default frequency
    .hrng_addr = 0,
    .acc_addr = FREQ_ACC_ADDRESS  // Default frequency accuracy address
};

volatile TestStateMachine test_sm = {
    .current_state = STATE_IDLE,
    .substate = SUBSTATE_START,
    .test_start_time = 0,
};

static void rf_test_result_print(void)
{
    if(rf_test_data.status.tx_cnt_pass)
    {
        DBG_PRINTF("TX cnt:%d\n", rf_test_data.tx_cnt);
    }
    else{
        DBG_PRINTF("TX cnt Failed");
    }

    if(rf_test_data.status.tx_power_pass)
    {
        DBG_PRINTF("TX power:%d\n", rf_test_data.tx_power);
    }
    else{
        DBG_PRINTF("TX power Failed");
    }

    if(rf_test_data.status.rx_cnt_pass)
    {
        DBG_PRINTF("RX cnt:%d\n", rf_test_data.rx_cnt);
    }
    else{
        DBG_PRINTF("RX cnt Failed");
    }

    if(rf_test_data.status.rx_sen_pass)
    {
        DBG_PRINTF("RX sensitivity:%d\n", rf_test_data.rx_sen);
    }
    else{
        DBG_PRINTF("RX sensitivity Failed");
    }

    if(rf_test_data.status.freq_offset_pass)
    {
        DBG_PRINTF("Frequency offset:%d\n", rf_test_data.freq_offset);
    }
    else{
        DBG_PRINTF("Frequency offset Failed");
    }
}

RFTest_Result_t rf_test_start(uint8_t *buffer){
    rf_test_ctrl.ch_id = set_rf_channel(buffer);
    rf_test_ctrl.frequency = ((uint16_t)buffer[0] | (uint16_t)(buffer[1]) << 8);
    rf_test_ctrl.hrng_addr = get_rf_acc_addr();//Here, after assigning acc_addr value, send it to the slave
    DBG_PRINTF("[HRNG] IS %x\n",rf_test_ctrl.hrng_addr);
    if(test_sm.current_state == STATE_IDLE) {
        test_sm.current_state = STATE_FLASH_FIRMWARE;
        test_sm.substate = SUBSTATE_START;
        DBG_PRINTF("[TEST] Starting RF tests...\n");
        return TEST_SUCCESS;
    } else {
        DBG_PRINTF("[ERROR] RF tests already in progress!\n");
        return TEST_RUNNING;
    }
}

RFTest_Result_t rf_test_get_result(rf_test_data_t *data) {
    if(test_sm.current_state == STATE_DONE) {
        data = &rf_test_data;
        test_sm.current_state = STATE_IDLE;
        test_sm.substate = SUBSTATE_START;
        DBG_PRINTF("[TEST] Starting RF tests...\n");
        return TEST_SUCCESS;
    } else {
        DBG_PRINTF("[TEST] RF tests already in progress!\n");
        return TEST_RUNNING;
    }
}

void reset_test_state(void) {
    test_sm.current_state = STATE_IDLE;
    test_sm.substate = SUBSTATE_START;
    memset(&rf_test_data, 0, sizeof(rf_test_data));
}

void loop_rf_tx_rx_test(void)
{
    // This function is a placeholder for the main loop of the RF test.
    // It can be used to call the test functions in a loop or based on events.
}

RFTest_Result_t rf_test_tx_cnt_begin(void)
{
    if((test_sm.current_state == STATE_TEST_TX_CNT)&&(test_sm.substate == SUBSTATE_START_WAIT_RSP)) {
        //todo: deal with the actual test logic
        test_sm.substate = SUBSTATE_RUNNING;
        DBG_PRINTF("Begin TX count test...\n");
        start_raw_tx_rx();
        return TEST_SUCCESS;
    }
    return TEST_RUNNING;
}

RFTest_Result_t rf_test_tx_power_begin(void)
{
    if((test_sm.current_state == STATE_TEST_TX_POWER) && (test_sm.substate == SUBSTATE_START_WAIT_RSP)) {
        //todo: deal with the actual test logic
        test_sm.substate = SUBSTATE_RUNNING;
        DBG_PRINTF("Begin TX Power test...\n");
        return TEST_SUCCESS;
    }
    return TEST_RUNNING;
}

RFTest_Result_t rf_test_rx_cnt_begin(void)
{
    if((test_sm.current_state == STATE_TEST_RX_CNT) && (test_sm.substate == SUBSTATE_START_WAIT_RSP)) {
        //todo:deal with the actual test logic
        start_raw_tx_rx();
        test_sm.substate = SUBSTATE_RUNNING;
        DBG_PRINTF("Begin RX count test...\n");
        return TEST_SUCCESS;
    }
    return TEST_RUNNING;
}

RFTest_Result_t rf_test_rx_sen_begin(void)
{
    if((test_sm.current_state == STATE_TEST_RX_SEN) && (test_sm.substate == SUBSTATE_START_WAIT_RSP)) {
        //todo:deal with the actual test logic
        test_sm.substate = SUBSTATE_RUNNING;
        DBG_PRINTF("Begin RX Sensitivity test...\n");
        return TEST_SUCCESS;
    }
    return TEST_RUNNING;
}

RFTest_Result_t rf_test_freq_offset_begin(void)
{
    if(test_sm.current_state == STATE_TEST_FREQ_OFFSET) {
        freq_offset_rx_begin(rf_test_ctrl.ch_id);
        test_sm.substate = SUBSTATE_RUNNING;
        DBG_PRINTF("Begin Frequency Offset test...\n");
        return TEST_SUCCESS;
    }
    return TEST_RUNNING;
}

RFTest_Result_t rf_test_tx_cnt_end(void)
{
    if((test_sm.current_state == STATE_TEST_TX_CNT) && (test_sm.substate == SUBSTATE_RUNNING_WAIT_RSP)) {
        //todo: deal with the actual test data
        stop_raw_tx_rx();
        if(get_raw_pkt_loss() == LOSE_PKT_ERROR)
        {
            rf_test_data.status.tx_cnt_pass = false;
        }
        else
        {
            rf_test_data.status.tx_cnt_pass = true;
        }
        test_sm.substate = SUBSTATE_COMPLETE;
        return TEST_SUCCESS;
    }
    else{
        test_sm.substate = SUBSTATE_COMPLETE;
        return TEST_RUNNING;
    }
}

RFTest_Result_t rf_test_tx_power_end(void)
{
    if((test_sm.current_state == STATE_TEST_TX_POWER) && (test_sm.substate == SUBSTATE_RUNNING_WAIT_RSP)) {
        //todo: deal with the actual test data
        if(get_raw_pkt_power() == POWER_PKT_ERROR)
        {
            rf_test_data.status.tx_power_pass = false;
        }
        else
        {
            rf_test_data.status.tx_power_pass = true;
        }
        test_sm.substate = SUBSTATE_COMPLETE;
        return TEST_SUCCESS;
    }
    else{
        test_sm.substate = SUBSTATE_COMPLETE;
        return TEST_RUNNING;
    }
}

RFTest_Result_t rf_test_rx_cnt_end(uint16_t test_data)
{
    if((test_sm.current_state == STATE_TEST_RX_CNT) && (test_sm.substate == SUBSTATE_RUNNING_WAIT_RSP)) {
        rf_test_data.rx_cnt = test_data;
        platform_printf("[RX count] Received packets: %d\n", rf_test_data.rx_cnt);
        (PACKET_CNT_MAX - rf_test_data.rx_cnt) > PCK_THRESHOLDS ? rf_test_data.status.rx_cnt_pass = false : (rf_test_data.status.rx_cnt_pass = true);
        test_sm.substate = SUBSTATE_COMPLETE;
        return TEST_SUCCESS;
    }
    else{
        test_sm.substate = SUBSTATE_COMPLETE;
        return TEST_RUNNING;
    }
}
//dut returns the number of packets received and RSSI here
RFTest_Result_t rf_test_rx_sen_end(int8_t test_data)
{
    if((test_sm.current_state == STATE_TEST_RX_SEN) && (test_sm.substate == SUBSTATE_RUNNING_WAIT_RSP)) {
        rf_test_data.rx_sen = test_data;
        platform_printf("[RX Sensitivity] RSSI: %d\n", rf_test_data.rx_sen);
        rf_test_data.rx_sen > POWER_THRESHOLDS ? rf_test_data.status.rx_sen_pass = true : (rf_test_data.status.rx_sen_pass = false);
        test_sm.substate = SUBSTATE_COMPLETE;
        return TEST_SUCCESS;
    }
    else{
        test_sm.substate = SUBSTATE_COMPLETE;
        return TEST_RUNNING;
    }
}

RFTest_Result_t rf_test_freq_offset_end(int8_t offset_data)
{
    DUT_cmd_rf_freq_offset(0, rf_test_ctrl.frequency); // Stop frequency offset test
    if((test_sm.current_state == STATE_TEST_FREQ_OFFSET) && (test_sm.substate == SUBSTATE_RUNNING_WAIT_RSP)) {
        rf_test_data.freq_offset = offset_data;
        rf_test_data.status.freq_offset_pass = true; // todo: deal with the pass logic
        test_sm.substate = SUBSTATE_COMPLETE;
        return TEST_SUCCESS;
    }
    else{
        test_sm.substate = SUBSTATE_COMPLETE;
        return TEST_RUNNING;
    }
}

void freq_offset_begin(void)
{
    DBG_PRINTF("timer start, Begin Frequency Offset test...\n");
    DUT_cmd_rf_freq_offset(1, (uint16_t)(rf_test_ctrl.ch_id)); // Start frequency offset test
}

void tf_test_rx_pkt_end(void)
{
    stop_raw_tx_rx();
    DUT_cmd_rf_rx_cnt(0, 0x962);
}



void loop_rf_test_state_machine(void) {
    switch (test_sm.current_state) {
        case STATE_IDLE:
            break;
        case STATE_FLASH_FIRMWARE:
        {
            switch(test_sm.substate){
                case SUBSTATE_START:
                    if(!get_evk_work())
                    {
                        boot_check();
                        break;
                    }
                    test_sm.substate = SUBSTATE_RUNNING;
                    break;
                case SUBSTATE_RUNNING:
                    if(get_buner_state() == BURN_STATE_CMPL)
                    {
                        test_sm.substate = SUBSTATE_COMPLETE;
                    }
                    break;
                case SUBSTATE_COMPLETE:
                    set_evk_protocol(1);
                    reset_dut_2_run();
                    platform_printf("[OK] wait for test\n");
					delay_ms(1000);
                    test_sm.current_state = STATE_TEST_TX_CNT;
                    test_sm.substate = SUBSTATE_START;
                    break;
                default:break;
            }
            //FINISHI Then switch the protocol to test set_evk_protocol(1);
        }break;
        case STATE_TEST_TX_CNT:
        {
            switch (test_sm.substate) {
                case SUBSTATE_START:
                    DBG_PRINTF("[TEST] Initializing TX count test...\n");
                    restart_raw_pkt_rx();
                    DBG_PRINTF("fr IS : %d\n", rf_test_ctrl.frequency);
                    DBG_PRINTF("ad IS : %x\n", rf_test_ctrl.hrng_addr);
                    DUT_cmd_rf_tx_cnt(1, rf_test_ctrl.frequency, rf_test_ctrl.hrng_addr); // Start TX count test
                    test_sm.test_start_time = platform_get_us_time();
                    test_sm.substate = SUBSTATE_START_WAIT_RSP;
                    break;
                case SUBSTATE_START_WAIT_RSP:
                    // Wait for the DUT to respond with the start confirmation via UART
                    if (platform_get_us_time() - test_sm.test_start_time > RF_TEST_TX_CNT_TIMEOUT) {
                        DBG_PRINTF("[ERROR] TX count test start timeout!\n");
                        test_sm.current_state = STATE_TEST_TX_POWER;
                        test_sm.substate = SUBSTATE_START;
                    }
                    break;
                case SUBSTATE_RUNNING:
                    test_sm.substate = SUBSTATE_RUNNING_WAIT_RSP;
                case SUBSTATE_RUNNING_WAIT_RSP:
                    if (platform_get_us_time() - test_sm.test_start_time > RF_TEST_TX_CNT_TIMEOUT) {
                        DBG_PRINTF("[ERROR] TX count test start timeout!\n");
                        test_sm.current_state = STATE_TEST_TX_POWER;
                        test_sm.substate = SUBSTATE_START;
                    }
                    break;
                case SUBSTATE_COMPLETE:
                    if (rf_test_data.status.tx_cnt_pass) {
                        //deal with the actual test data to PC
                        rf_test_data.tx_cnt = get_raw_pkt_count();
                        DBG_PRINTF("[TEST] TX count test passed! Received=%d\n", rf_test_data.tx_cnt);
                    } else {
                        DBG_PRINTF("[ERROR] TX count test failed. Received=%d\n", rf_test_data.tx_cnt);
                    }
                    test_sm.current_state = STATE_TEST_TX_POWER;
                    test_sm.substate = SUBSTATE_START;
                    break;
            }
        }break;

        case STATE_TEST_TX_POWER:
        {
            switch (test_sm.substate) {
                case SUBSTATE_START:
                    DBG_PRINTF("[TEST] Initializing TX power test...\n");
                    DUT_cmd_rf_tx_power(1, rf_test_ctrl.frequency); // Start TX power test
                    test_sm.test_start_time = platform_get_us_time();
                    test_sm.substate = SUBSTATE_START_WAIT_RSP;
                    break;

                case SUBSTATE_START_WAIT_RSP:
                    // Wait for the DUT to respond with the start confirmation via UART
                    if (platform_get_us_time() - test_sm.test_start_time > RF_TEST_TX_POWER_TIMEOUT) {
                        DBG_PRINTF("[ERROR] TX power test start timeout!\n");
                        rf_test_data.status.tx_power_pass = false;
                        test_sm.current_state = STATE_TEST_RX_CNT;//continue to next test
                        test_sm.substate = SUBSTATE_START;
                    }
                    break;

                case SUBSTATE_RUNNING:
                    test_sm.substate = SUBSTATE_RUNNING_WAIT_RSP;
                case SUBSTATE_RUNNING_WAIT_RSP:
                    // Wait for the DUT to send the TX power data
                    if (platform_get_us_time() - test_sm.test_start_time > RF_TEST_TX_POWER_TIMEOUT) {
                        DBG_PRINTF("[ERROR] TX power test timeout!\n");
                        rf_test_data.status.tx_power_pass = false;
                        test_sm.current_state = STATE_TEST_RX_CNT;
                        test_sm.substate = SUBSTATE_START;
                    }
                    break;

                case SUBSTATE_COMPLETE:
                    if (rf_test_data.status.tx_power_pass) {
                        DBG_PRINTF("[TEST] TX power test passed!\n");
                        rf_test_data.tx_power = get_raw_pkt_power();
                        platform_printf("[RESULT] Tx power is %d\n",rf_test_data.tx_power);// here can redeal to trainsmit the data(uart)
                    } else {
                        DBG_PRINTF("[ERROR] TX power test failed!\n");
                        platform_printf("[RESULT] Tx power is %d\n",rf_test_data.tx_power);
                    }
                    test_sm.current_state = STATE_TEST_RX_CNT;
                    test_sm.substate = SUBSTATE_START;
                    break;
            }
        }break;

        case STATE_TEST_RX_CNT:
        {
            switch (test_sm.substate) {
                case SUBSTATE_START:
                    DBG_PRINTF("[TEST] Initializing RX count test...\n");
                    restart_raw_pkt_tx();
                    DUT_cmd_rf_rx_cnt(1, rf_test_ctrl.frequency); // Start RX count test
                    test_sm.test_start_time = platform_get_us_time();
                    test_sm.substate = SUBSTATE_START_WAIT_RSP;
                    break;

                case SUBSTATE_START_WAIT_RSP:
                    // Wait for the DUT to respond with the start confirmation via UART
                    if (platform_get_us_time() - test_sm.test_start_time > RF_TEST_RX_CNT_TIMEOUT) {
                        DBG_PRINTF("[ERROR] RX count test start timeout!\n");
                        rf_test_data.status.rx_cnt_pass = false;
                        test_sm.current_state = STATE_TEST_RX_SEN;//continue to next test
                        test_sm.substate = SUBSTATE_START;
                    }
                    break;
                
                case SUBSTATE_RUNNING:
                    test_sm.substate = SUBSTATE_RUNNING_WAIT_RSP;
                case SUBSTATE_RUNNING_WAIT_RSP:
                    // Wait for the DUT to send the RX count data
                    if (platform_get_us_time() - test_sm.test_start_time > RF_TEST_RX_CNT_TIMEOUT) {
                        DBG_PRINTF("[ERROR] RX count test timeout!\n");
                        rf_test_data.status.rx_cnt_pass = false;
                        test_sm.current_state = STATE_TEST_RX_SEN;
                        test_sm.substate = SUBSTATE_START;
                    }
                    break;

                case SUBSTATE_COMPLETE:
                    if (rf_test_data.status.rx_cnt_pass) {
                        DBG_PRINTF("[TEST] RX count test passed!\n");
                        platform_printf("[RESULT] RX count is %d\n",rf_test_data.rx_cnt);
                    } else {
                        DBG_PRINTF("[ERROR] RX count test failed!\n");
                        platform_printf("[RESULT] RX count is %d\n",rf_test_data.rx_cnt);
                    }
                    test_sm.current_state = STATE_TEST_RX_SEN;
                    test_sm.substate = SUBSTATE_START;
                    break;
            }
        }break;

        case STATE_TEST_RX_SEN:
        {
            switch (test_sm.substate) {
                case SUBSTATE_START:
                    DBG_PRINTF("[TEST] Initializing RX sensitivity test...\n");
                    DUT_cmd_rf_rx_sen(1, rf_test_ctrl.frequency); // Start RX sensitivity test
                    test_sm.test_start_time = platform_get_us_time();
                    test_sm.substate = SUBSTATE_START_WAIT_RSP;
                    break;

                case SUBSTATE_START_WAIT_RSP:
                    // Wait for the DUT to respond with the start confirmation via UART
                    if (platform_get_us_time() - test_sm.test_start_time > RF_TEST_RX_SEN_TIMEOUT) {
                        DBG_PRINTF("[ERROR] RX sensitivity test start timeout!\n");
                        rf_test_data.status.rx_sen_pass = false;
                        test_sm.current_state = STATE_TEST_FREQ_OFFSET;//continue to next test
                        test_sm.substate = SUBSTATE_START;
                    }
                    break;

                case SUBSTATE_RUNNING:
                    test_sm.substate = SUBSTATE_RUNNING_WAIT_RSP;
                case SUBSTATE_RUNNING_WAIT_RSP:
                    // Wait for the DUT to send the RX sensitivity data
                    if (platform_get_us_time() - test_sm.test_start_time > RF_TEST_RX_SEN_TIMEOUT) {
                        DBG_PRINTF("[ERROR] RX sensitivity test timeout!\n");
                        rf_test_data.status.rx_sen_pass = false;
                        test_sm.current_state = STATE_TEST_FREQ_OFFSET;
                        test_sm.substate = SUBSTATE_START;
                    }
                    break;

                case SUBSTATE_COMPLETE:
                    if (rf_test_data.status.rx_sen_pass) {
                        DBG_PRINTF("[TEST] RX sensitivity test passed!\n");
                        platform_printf("[RESULT] RX sen is %d\n",rf_test_data.rx_sen);
                    } else {
                        DBG_PRINTF("[ERROR] RX sensitivity test failed!\n");
                        platform_printf("[RESULT] RX sen is %d\n",rf_test_data.rx_sen);
                    }
                    test_sm.current_state = STATE_TEST_FREQ_OFFSET;
                    test_sm.substate = SUBSTATE_START;
                    break;
            }
        }break;

        case STATE_TEST_FREQ_OFFSET:
        {
            switch (test_sm.substate) {
                case SUBSTATE_START:
                    DBG_PRINTF("[TEST] Initializing frequency offset test:%d\n", (uint16_t)rf_test_ctrl.ch_id);
                    test_sm.test_start_time = platform_get_us_time();
                    test_sm.substate = SUBSTATE_START_WAIT_RSP;
                    //set a delay to avoid rf window conflict
                    platform_set_timer(freq_offset_begin,1600);
                    break;

                case SUBSTATE_START_WAIT_RSP:
                    // Wait for the DUT to respond with the start confirmation via UART
                    if (platform_get_us_time() - test_sm.test_start_time > RF_TEST_FREQ_OFFSET_TIMEOUT) {
                        DBG_PRINTF("[ERROR] Frequency offset test start timeout!\n");
                        rf_test_data.status.freq_offset_pass = false;
                        test_sm.current_state = STATE_DONE;//continue to next test
                        test_sm.substate = SUBSTATE_START;
                    }
                    break;

                case SUBSTATE_RUNNING:
                    test_sm.substate = SUBSTATE_RUNNING_WAIT_RSP;
                case SUBSTATE_RUNNING_WAIT_RSP:
                    if (platform_get_us_time() - test_sm.test_start_time > RF_TEST_FREQ_OFFSET_TIMEOUT) {
                        DBG_PRINTF("[ERROR] Frequency offset test timeout!\n");
                        rf_test_data.status.freq_offset_pass = false;
                        test_sm.current_state = STATE_DONE;
                        test_sm.substate = SUBSTATE_START;
                    }
                    break;

                case SUBSTATE_COMPLETE:
                    if (rf_test_data.status.freq_offset_pass) {
                        DBG_PRINTF("[TEST] Frequency offset test passed:%d!\n", rf_test_data.freq_offset);
                    } else {
                        DBG_PRINTF("[ERROR] Frequency offset test failed!\n");
                    }
                    test_sm.current_state = STATE_DONE;
                    test_sm.substate = SUBSTATE_START;
                    break;
            }
        }break;

        //todo: add frequency offset tune procedure according to the test result

        case STATE_DONE:
        {
            DBG_PRINTF("[TEST] All tests completed.\n");
            rf_test_result_print();
            reset_test_state();
            break;
        }
    }
}


