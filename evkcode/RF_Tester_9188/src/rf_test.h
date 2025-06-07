#ifndef _RF_TEST_H_
#define _RF_TEST_H_

#include <stdbool.h>
#include <stdint.h>
#include "ingsoc.h"

#define RF_TEST_TX_CNT_TIMEOUT 12000*1000    // units: us
#define RF_TEST_TX_POWER_TIMEOUT 12000*1000  // units: us
#define RF_TEST_RX_CNT_TIMEOUT 12000*1000    // units: us
#define RF_TEST_RX_SEN_TIMEOUT 12000*1000    // units: us
#define RF_TEST_FREQ_OFFSET_TIMEOUT 5000*1000 // units: us

#define FREQ_ACC_ADDRESS          0xBED8A379  // Frequency accuracy address
#define LOSE_PKT_ERROR              (-1)
#define POWER_PKT_ERROR             (0)

typedef struct {
    uint16_t tx_cnt;
    uint16_t rx_cnt;
    int8_t tx_power;
    int8_t rx_sen;
    int8_t freq_offset;

    struct {
        bool tx_cnt_pass    : 1;  // 1-bit flag for pass/fail
        bool rx_cnt_pass    : 1;
        bool tx_power_pass  : 1;
        bool rx_sen_pass    : 1;
        bool freq_offset_pass : 1;
    } status;
} rf_test_data_t;

typedef struct {
    uint8_t ch_id;  // Channel ID
    uint16_t frequency;
    uint32_t hrng_addr;
    uint32_t acc_addr;  // Frequency accuracy address
} rf_test_ctrl_t;

typedef enum {
    STATE_IDLE,
    STATE_FLASH_FIRMWARE,
    STATE_TEST_TX_CNT,
    STATE_TEST_TX_POWER,
    STATE_TEST_RX_CNT,
    STATE_TEST_RX_SEN,
    STATE_TEST_FREQ_OFFSET,
    STATE_DONE
} RFTestState;

typedef enum {
    SUBSTATE_START,
    SUBSTATE_START_WAIT_RSP,      
    SUBSTATE_RUNNING,
    SUBSTATE_RUNNING_WAIT_RSP,    
    SUBSTATE_COMPLETE    
} TestSubState;

typedef enum {
    TEST_SUCCESS,
    TEST_RUNNING,
    TEST_FAILED
} RFTest_Result_t;

typedef struct {
    RFTestState current_state;  
    TestSubState substate;
    uint64_t test_start_time;
} TestStateMachine;


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

RFTest_Result_t rf_test_start(uint8_t *buffer);
RFTest_Result_t rf_test_get_result(rf_test_data_t *data);
void loop_rf_test_state_machine(void);
void tf_test_rx_pkt_end(void);

RFTest_Result_t rf_test_tx_power_begin(void);
RFTest_Result_t rf_test_tx_power_end(void);

RFTest_Result_t rf_test_tx_cnt_begin(void);
RFTest_Result_t rf_test_tx_cnt_end(void);

RFTest_Result_t rf_test_rx_cnt_begin(void);
RFTest_Result_t rf_test_rx_cnt_end(uint16_t test_data);

RFTest_Result_t rf_test_rx_sen_begin(void);
RFTest_Result_t rf_test_rx_sen_end(int8_t test_data);

RFTest_Result_t rf_test_freq_offset_begin(void);
RFTest_Result_t rf_test_freq_offset_end(int8_t offset_data);

#endif


