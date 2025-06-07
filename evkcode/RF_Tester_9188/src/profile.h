#ifndef _PROFILESTASK_H_
#define _PROFILESTASK_H_

#include <stdint.h>
#include "ll_api.h"

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


//VD, debug version; VR, release version
#define VERSION  VD_1.0.0

#define USER_Tx_RAW_PKT 0
#define USER_Rx_RAW_PKT 1
#define USER_END_TEST 2
#define USER_SET_CHANNEL 3
#define USER_SET_RX_PKT 4
#define USER_SET_TX_PKT 5

#define USER_MSG_FREQ_OFFSET_BEGIN_RX     10
#define USER_MSG_FREQ_OFFSET_DONE_RX      11
#define USER_MSG_FREQ_OFFSET_END_RX       12
#define USER_MSG_FREQ_OFFSET_TUNE         13


#define DUT_PORT_CLK_GATE      SYSCTRL_ClkGate_APB_UART1
#define DUT_IO_SOURCE_TXD    IO_SOURCE_UART1_TXD
#define DUT_PORT               APB_UART1
#define DUT_UART               UART_PORT_1
#define DUT_UART_IRQ_CB        PLATFORM_CB_IRQ_UART1
#define DUT_TX_PIN             GIO_GPIO_4
#define DUT_RX_PIN             GIO_GPIO_5


#define UPPER_PORT_CLK_GATE    SYSCTRL_ClkGate_APB_UART0
#define UPPER_IO_SOURCE_TXD    IO_SOURCE_UART0_TXD
#define UPPER_PORT             APB_UART0
#define UPPER_UART             UART_PORT_0
#define UPPER_UART_IRQ_CB      PLATFORM_CB_IRQ_UART0
#define UPPER_TX_PIN           GIO_GPIO_2
#define UPPER_RX_PIN           GIO_GPIO_3


// #define     UART0_TX        	GIO_GPIO_2
// #define     UART0_RX        	GIO_GPIO_3

// #define     UART1_TX        	GIO_GPIO_4
// #define     UART1_RX        	GIO_GPIO_5


uint32_t setup_profile(void *data, void *user_data);

#endif


