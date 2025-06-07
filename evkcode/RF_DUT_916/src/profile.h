#ifndef _PROFILESTASK_H_
#define _PROFILESTASK_H_

#include <stdint.h>
#include "ll_api.h"

#define VERSION  VD_1.0.0

#define DEBUG 1
#ifdef DEBUG
    #define DBG_PRINTF(fmt, ...) \
        platform_printf("[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

    #define CUSTOM_ASSERT(expr) \
    if (!(expr)) { \
        platform_printf("[MY Assertion]: %s, file %s, line %d\n", #expr, __FILE__, __LINE__); \
        while (1); \
    }
#else
    #define DBG_PRINTF(fmt, ...) ((void)0)
    #define CUSTOM_ASSERT(expr) ((void)0)
#endif


#define DUT_PORT            APB_UART0
#define DUT_UART            UART_PORT_0
#define DUT_UART_IRQ_CB     PLATFORM_CB_IRQ_UART0
#define DUT_RX_PIN          UART1_RX
#define DUT_TX_PIN          UART1_TX


#define USER_Tx_RAW_PKT 0
#define USER_Rx_RAW_PKT 1
#define USER_END_TEST 2
#define USER_SET_CHANNEL 3
#define USER_SET_RX_PKT 4
#define USER_SET_TX_PKT 5
#define USER_MSG_RAW_PACKET_BEGIN_TX     10
#define USER_MSG_RAW_PACKET_DONE_TX      11

// #define UPPER_PORT            APB_UART1
// #define UPPER_UART            UART_PORT_1
// #define UPPER_UART_IRQ_CB     PLATFORM_CB_IRQ_UART1


// #define     UART0_TX        	GIO_GPIO_2
// #define     UART0_RX        	GIO_GPIO_3

#define     UART1_TX        	GIO_GPIO_35
#define     UART1_RX        	GIO_GPIO_31


uint32_t setup_profile(void *data, void *user_data);

#endif


