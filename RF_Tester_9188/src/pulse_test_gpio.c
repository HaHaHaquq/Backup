#include <stdio.h>
#include <string.h>
#include "ingsoc.h"
#include "platform_api.h"
#include "pulse_test_gpio.h"

GIO_Index_t gpio_test_array[TEST_GPIO_NUM] = {TEST_PIN_0_GPIO, TEST_PIN_1_GPIO, TEST_PIN_2_GPIO, TEST_PIN_3_GPIO, TEST_PIN_4_GPIO, TEST_PIN_5_GPIO};

void test_gpio_init(void)
{
    SYSCTRL_ClearClkGateMulti(  (1 << SYSCTRL_ClkGate_APB_GPIO0)
                            | (1 << SYSCTRL_ClkGate_APB_PinCtrl)
                            | (1 << SYSCTRL_ClkGate_APB_GPIO1));

    for(uint16_t i=0; i<TEST_GPIO_NUM; i++)
    {
        PINCTRL_SetPadMux(gpio_test_array[i], IO_SOURCE_GPIO);
        GIO_SetDirection(gpio_test_array[i], GIO_DIR_OUTPUT);
        GIO_WriteValue(gpio_test_array[i], 0); 
    }

    return;
}

void gpio_pin_toggle(uint8_t gpio_array_index)
{
    if(gpio_array_index >= TEST_GPIO_NUM)
    {
        printf("[ERR] Invalid GPIO pin index %d\n", gpio_array_index);
        return;
    }
    GIO_ToggleBits(1 << gpio_test_array[gpio_array_index]); 
}

void gpio_pin_pulse(uint8_t gpio_array_index)
{
    if(gpio_array_index >= TEST_GPIO_NUM)
    {
        printf("[ERR] Invalid GPIO pin index %d\n", gpio_array_index);
        return;
    }
    GIO_WriteValue(gpio_test_array[gpio_array_index], 1); 
    GIO_WriteValue(gpio_test_array[gpio_array_index], 0); 
}

void gpio_pin_pulse_multi(uint8_t gpio_array_index, uint32_t pulse_num)
{
    if(gpio_array_index >= TEST_GPIO_NUM)
    {
        printf("[ERR] Invalid GPIO pin index %d\n", gpio_array_index);
        return;
    }

    for(uint32_t i=0; i<pulse_num; i++)
    {
        GIO_WriteValue(gpio_test_array[gpio_array_index], 1); 
        GIO_WriteValue(gpio_test_array[gpio_array_index], 0); 
    }
}
