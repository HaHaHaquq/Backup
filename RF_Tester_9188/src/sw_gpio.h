#ifndef __SW_GPIO_H
#define __SW_GPIO_H
#include <stdint.h>

//uart switch
#define SW_UART_DUT   GIO_GPIO_8
#define SW_UART_AMO   GIO_GPIO_17

//reset switch
#define SW_REST_CTRL  GIO_GPIO_11
#define SW_REST_COMM  GIO_GPIO_10

//ext_int switch
#define SW_EXT_INT    GIO_GPIO_7

#define SW_ON  1
#define SW_OFF 0

void sw_uart_gpio_init(void);
void sw_uart_direction(uart_direction dir);
void boot_check(void);

#endif








