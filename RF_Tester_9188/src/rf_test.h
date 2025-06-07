#ifndef __RF_TEST_H
#define __RF_TEST_H

#include <stdbool.h>
#include <stdint.h>
#include "ingsoc.h"

#define RF_TEST_TX_CNT_TIMEOUT 1000*1000    // 1 second

typedef enum : uint8_t{
    RF_SUCCESS,
    RF_FIRMWARE_FAIL,
    RF_TEST_FAIL,
    RF_TEST_VENDOR_FIRMWARE_FAIL
}Errorcode;

typedef enum {
    STATE_IDLE,
    STATE_FLASH_FIRMWARE,
    STATE_TEST_TX_POWER,
    STATE_TEST_TX_CNT,
    STATE_TEST_RX_SEN,
    STATE_TEST_RX_CNT,
    STATE_TEST_FREQ_OFFSET,
    STATE_VENDOR_FIRMWARE,
    STATE_DONE
} RFTestState;

typedef enum {
    SUBSTATE_START,      
    SUBSTATE_RUNNING,    
    SUBSTATE_COMPLETE    
} TestSubState;

typedef enum {
    TEST_SUCCESS,
    TEST_RUNNING,
    TEST_FAILED
} RFTest_Result_t;

typedef struct {
    uint16_t tx_cnt;
    uint16_t rx_cnt;
    uint8_t tx_power;
    uint8_t rx_sen;
    uint8_t freq_offset;
    uint8_t err_cnt;
    union{
        struct {
            bool tx_cnt_pass    : 1;  // 1-bit flag for pass/fail
            bool rx_cnt_pass    : 1;
            bool tx_power_pass  : 1;
            bool rx_sen_pass    : 1;
            bool freq_offset_pass : 1;
        }test_status;
        uint8_t raw_status;
    }status;
    Errorcode error_code;
} rf_test_data_t;

typedef struct {
    RFTestState current_state;  
    TestSubState substate;
    uint64_t test_start_time;
} TestStateMachine;

#endif


