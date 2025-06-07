#include "evk_config.h"
#include "sw_gpio.h"
#include "peripheral_gpio.h"
#include "platform_api.h"
#include <stdint.h>


void sw_uart_gpio_init(void)
{
    SYSCTRL_ClearClkGateMulti((1 << SYSCTRL_ClkGate_APB_GPIO) | 
                              (1 << SYSCTRL_ClkGate_APB_PinCtrl));

    // Set the GPIO pins for UART switch
    PINCTRL_SetPadMux(SW_UART_DUT, IO_SOURCE_GPIO);
    GIO_SetDirection(SW_UART_DUT, GIO_DIR_OUTPUT);
    GIO_WriteValue(SW_UART_DUT, SW_ON);

    PINCTRL_SetPadMux(SW_UART_AMO, IO_SOURCE_GPIO);
    GIO_SetDirection(SW_UART_AMO, GIO_DIR_OUTPUT);
    GIO_WriteValue(SW_UART_AMO, SW_OFF);

    // Set the GPIO pins for reset switch
    PINCTRL_SetPadMux(SW_REST_CTRL, IO_SOURCE_GPIO);
    GIO_SetDirection(SW_REST_CTRL, GIO_DIR_OUTPUT);
    GIO_WriteValue(SW_REST_CTRL, SW_ON);

    PINCTRL_SetPadMux(SW_REST_COMM, IO_SOURCE_GPIO);
    GIO_SetDirection(SW_REST_COMM, GIO_DIR_OUTPUT);
    GIO_WriteValue(SW_REST_COMM, SW_OFF);

    // Set the GPIO pin for ext_int switch
    PINCTRL_SetPadMux(SW_EXT_INT, IO_SOURCE_GPIO);
    GIO_SetDirection(SW_EXT_INT, GIO_DIR_OUTPUT);
    GIO_WriteValue(SW_EXT_INT, SW_ON);
}


void sw_uart_direction(uart_direction dir)
{
    switch(dir)
    {
        case UART_DUT_2_EVK:
            GIO_WriteValue(SW_UART_DUT, SW_OFF);
            GIO_WriteValue(SW_UART_AMO, SW_OFF);
            break;
        case UART_DUR_2_AMO:
            GIO_WriteValue(SW_UART_DUT, SW_ON);
            GIO_WriteValue(SW_UART_AMO, SW_OFF);
            break;
        case UART_AMO_2_EVK:
            GIO_WriteValue(SW_UART_DUT, SW_ON);
            GIO_WriteValue(SW_UART_AMO, SW_ON);
            break;
        case UART_AMO_2_IDKE:
            GIO_WriteValue(SW_UART_DUT, SW_ON);
            GIO_WriteValue(SW_UART_AMO, SW_OFF);
            break;
        default:
            break;
    }
}

void boot_check(void)
{
    GIO_WriteValue(SW_EXT_INT, SW_ON);
    GIO_WriteValue(SW_REST_COMM, SW_OFF);
    delay_ms(1000);
	GIO_WriteValue(SW_REST_COMM, SW_ON);
    delay_ms(1000);
    GIO_WriteValue(SW_EXT_INT, SW_OFF);
}

void reset_dut_2_run(void)
{
    GIO_WriteValue(SW_REST_COMM, SW_OFF);
    delay_ms(1000);
	GIO_WriteValue(SW_REST_COMM, SW_ON);
}


