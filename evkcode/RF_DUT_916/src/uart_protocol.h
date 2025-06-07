#ifndef _UART_PROTOCOL_H_
#define _UART_PROTOCOL_H_

#include <stdint.h>

/* Protocol constants */
#define PROTOCOL_HEADER1                        0x45     // Frame header "JE"
#define PROTOCOL_HEADER2                        0x4A     // Frame header "JE"
#define PROTOCOL_TAIL1                          0x4E     // Frame tail "ND"
#define PROTOCOL_TAIL2                          0x44     // Frame tail "ND"
#define PROTOCOL_MAX_DATA_LEN                   128      // Maximum data length
      
#define PROTOCOL_GOLDEN_CMD_LEN                 3        // Golden command length
#define PROTOCOL_UPPER_CMD_START_RSP_LEN        2        // Upper command start response length
      
#define PROTOCOL_UPPER_CMD_MAX_LEN              1        // Upper command cmd maximum length
#define PROTOCOL_DUT_CMD_MAX_LEN                8  //3        // DUT command cmd maximum length

#define PROTOCOL_DUT_BUF_LENGTH_WITHOUT_DATA    7      // DUT buffer length

typedef enum {
    PARSE_STATE_HEADER1,
    PARSE_STATE_HEADER2,
    PARSE_STATE_TYPE,
    PARSE_STATE_LENGTH,
    PARSE_STATE_CMD,
    PARSE_STATE_DATA,
    PARSE_STATE_CRC,
    PARSE_STATE_TAIL1,
    PARSE_STATE_TAIL2
} parse_state_t;

typedef enum {
    //OTA cmd and data
    GOLDEN_DUT_CMD             = 0x01,
    GOLDEN_DUT_DATA            = 0x02,
    GOLDEN_DUT_CMD_RSP         = 0xF1,
    GOLDEN_DUT_DATA_RSP        = 0xF2,
    //RF test cmd and reply
    GOLDEN_DUT_RF_CMD          = 0x03,
    GOLDEN_DUT_RF_CMD_RSP      = 0xF3,
    UPPER_GOLDEN_RF_CMD        = 0x04,
    UPPER_GOLDEN_RF_CMD_RSP    = 0xF4,
} protocol_type_t;

// typedef enum {
//     UART_DUT_TX_CNT_CMD         = 0xB1,
//     UART_DUT_TX_POWER_CMD       = 0xB2,
//     UART_DUT_RX_CNT_CMD         = 0xB3,
//     UART_DUT_RX_SEN_CMD         = 0xB4,
//     UART_DUT_FREQ_OFFSET_CMD    = 0xB5,
// } protocol_cmd_t;

typedef enum{
    UART_UPPER_TEST_START             = 0xC1,
    UART_UPPER_GET_RESULT             = 0xC2
} uart_upper_cmd_t;

typedef enum{
    UART_DUT_TX_CNT_BEGIN_CMD         = 0xB1,
    UART_DUT_TX_CNT_END_CMD           = 0xB2,
    UART_DUT_TX_POWER_BEGIN_CMD       = 0xB3,
    UART_DUT_TX_POWER_END_CMD         = 0xB4,
    UART_DUT_RX_CNT_BEGIN_CMD         = 0xB5,
    UART_DUT_RX_CNT_END_CMD           = 0xB6,
    UART_DUT_RX_SEN_BEGIN_CMD         = 0xB7,
    UART_DUT_RX_SEN_END_CMD           = 0xB8,
    UART_DUT_FREQ_OFFSET_BEGIN_CMD    = 0xB9,
    UART_DUT_FREQ_OFFSET_END_CMD      = 0xBA,
    UART_DUT_FREQ_OFFSET_TUNE_CMD     = 0xBB,
} uart_dut_cmd_t;

#endif


