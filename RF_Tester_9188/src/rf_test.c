#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "evk_config.h"
#include "sw_gpio.h"
#include "rf_test.h"
#include "platform_api.h"
#include "profile.h"
#include "tx_rx_test.h"
#include "uart_ota.h"

evk_config_t evk_config = {0};
volatile rf_test_data_t rf_test_data = {
    .tx_cnt = 0,
    .rx_cnt = 0,
    .tx_power = 0,
    .rx_sen = 0,
    .freq_offset = 0,
    .err_cnt = 0,
    .status.raw_status = 0
};

volatile TestStateMachine test_sm = {
    .current_state = STATE_IDLE,
    .substate = SUBSTATE_START,
    .test_start_time = 0,
};

void rf_test_state_machine(void) {
    uint16_t test_data = 0;
    switch (test_sm.current_state) {
        case STATE_IDLE:
            // 执行触发操作，等待串口命令，这里可以根据逻辑修改执行测试、烧录单独或者执行所有的选择
            //全部执行则默认是先烧录测试程序
            break;
        case STATE_FLASH_FIRMWARE:
        {
            switch(test_sm.substate)
            {
                case SUBSTATE_START:
                {
                    if(!evk_config.work)
                    {
                        boot_check();
                        break;
                    }
                    sw_uart_direction(UART_DUT_2_EVK);
                    test_sm.substate = SUBSTATE_RUNNING;
                }break;
                case SUBSTATE_RUNNING:
                {
                    if(get_buner_state() == BURN_STATE_CMPL){
                        test_sm.substate = SUBSTATE_COMPLETE;
                    }
                    else
                    {
                        rf_test_data.err_cnt++;
                        boot_check();   //循环三次处理，三次烧录失败则返回错误，这里可以根据返回的work_mode来switch做更精细的处理
                        if(rf_test_data.err_cnt >= 3)
                        {
                            DBG_PRINTF("[ERROR] Firmware flash failed after 3 attempts!\n");
                            //最好不是重置工作状态，需要有一个结果返回的结构体来直接返回测试失败，原因为烧录失败或者是evk本身没有bin固件在flash里
                            rf_test_data.error_code = RF_FIRMWARE_FAIL;
                            rf_test_data.err_cnt = 0;
                            evk_config.work = 0; //重置工作状态
                            //这里处理接下来跳转什么流程
                        }
                    }
                }break;
                case SUBSTATE_COMPLETE:
                {
                    test_sm.current_state = STATE_TEST_TX_CNT;
                    test_sm.substate = SUBSTATE_START;
                    //烧录结果的返回，可以做拓展处理（是否发送测试程序烧录完毕，可以开始测试的条件）
                }break;
                default:
                    break;
            }
        }break;
        case STATE_TEST_TX_CNT://需要切换串口
        {
            switch (test_sm.substate) {
                case SUBSTATE_START:
                    if(rf_test_tx_cnt_begin() == TEST_SUCCESS)
                    {
                        test_sm.test_start_time = platform_get_us_time();
                        test_sm.substate = SUBSTATE_RUNNING;
                        DBG_PRINTF("[TEST] Initializing TX count test...\n");
                    }
                    else
                    {
                        DBG_PRINTF("[ERROR] TX count test failed to start!\n");
                        test_sm.current_state = STATE_IDLE;
                    }
                    break;

                case SUBSTATE_RUNNING:
                    if (platform_get_us_time() - test_sm.test_start_time < RF_TEST_TX_CNT_TIMEOUT) {
                        int status = rx_test_tx_cnt_status(&test_data);
                        if (status == TEST_SUCCESS) {
                            rf_test_data.tx_cnt = test_data;
                            rf_test_data.status.test_status.tx_cnt_pass = true;
                            test_sm.substate = SUBSTATE_COMPLETE;
                            DBG_PRINTF("[TEST] TX count test result: %d\n", test_data);
                        } else if (status == TEST_FAILED) {
                            DBG_PRINTF("[ERROR] TX count test hardware error!\n");
                            rf_test_data.status.test_status.tx_cnt_pass = false;
                            test_sm.substate = SUBSTATE_COMPLETE;
                        }
                    }
                    else {
                        DBG_PRINTF("[ERROR] TX count test timeout!\n");
                        rf_test_data.status.test_status.tx_cnt_pass = false;
                        test_sm.substate = SUBSTATE_COMPLETE;
                    }
                    break;

                case SUBSTATE_COMPLETE:
                    if (rf_test_data.status.test_status.tx_cnt_pass) {
                        DBG_PRINTF("[TEST] TX count test passed! Received=%d\n", rf_test_data.tx_cnt);
                        test_sm.current_state = STATE_TEST_RX_SEN;
                        test_sm.substate = SUBSTATE_START;
                    } else {
                        DBG_PRINTF("[TEST] TX count test failed. Received=%d\n", rf_test_data.tx_cnt);
                        test_sm.current_state = STATE_IDLE;
                    }
                    break;
            }
        }break;
        case STATE_TEST_TX_POWER:
        {
        }break;
        case STATE_TEST_RX_SEN:
        {
        }break;
        case STATE_TEST_RX_CNT:
        {
        }break;
        case STATE_TEST_FREQ_OFFSET:
        {
        }break;
        case STATE_VENDOR_FIRMWARE:
        {
            switch(test_sm.substate)
            {
                case SUBSTATE_START:{
                    //开始控制amo
                }break;
                case SUBSTATE_RUNNING:{

                }break;
                case SUBSTATE_COMPLETE:{

                }break;
                default:
                    break;
            }
        }break;
        case STATE_DONE:
        {
            DBG_PRINTF("[TEST] All tests completed.\n");
            test_sm.current_state = STATE_IDLE;
			//通过串口把结果发送给PC,ring_buf填数据
            break;
        }
        default:
            break;
    }
}


void evk_config_init(void)
{
    evk_config.bin_addr = EVK_BIN_ADDR;
    evk_config.sw_dir = UART_DUT_2_EVK;
    evk_config.protocol = PROTOCOL_2_PC;
    evk_config.work = 0;
}

evk_config_t *get_evk_config(void)
{
    return &evk_config;
}

