#ifndef _PULSE_TEST_GPIO_H_
#define _PULSE_TEST_GPIO_H_

#include <stdint.h>

#define TEST_GPIO_NUM 6

#define TEST_PIN_0_GPIO 12
#define TEST_PIN_1_GPIO 13
#define TEST_PIN_2_GPIO 16
#define TEST_PIN_3_GPIO 17
#define TEST_PIN_4_GPIO 18
#define TEST_PIN_5_GPIO 19
#define TEST_PIN_6_GPIO 27
#define TEST_PIN_7_GPIO 28

void test_gpio_init(void);
void gpio_pin_toggle(uint8_t gpio_array_index);
void gpio_pin_pulse(uint8_t gpio_array_index);
void gpio_pin_pulse_multi(uint8_t gpio_array_index, uint32_t pulse_num);

#endif


