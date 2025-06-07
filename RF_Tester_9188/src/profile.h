#ifndef _PROFILESTASK_H_
#define _PROFILESTASK_H_

#include <stdint.h>

#define DEBUG 1
#ifdef DEBUG
    #define DBG_PRINTF(fmt, ...) \
        platform_printf("[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define DBG_PRINTF(fmt, ...) ((void)0)
#endif

//VD, debug version; VR, release version
#define VERSION  VD_1.0.0

#define DUT_PORT            APB_UART0
#define DUT_UART            UART_PORT_0
#define DUT_UART_IRQ_CB     PLATFORM_CB_IRQ_UART0

#define UPPER_PORT            APB_UART1
#define UPPER_UART            UART_PORT_1
#define UPPER_UART_IRQ_CB     PLATFORM_CB_IRQ_UART1


#define     UART0_TX        	GIO_GPIO_2//GIO_GPIO_5
#define     UART0_RX        	GIO_GPIO_3//GIO_GPIO_4


#define     UART1_TX        	GIO_GPIO_7//GIO_GPIO_4
#define     UART1_RX        	GIO_GPIO_8


uint32_t setup_profile(void *data, void *user_data);

#endif


