#include <stdint.h>
#include <stdio.h>
#include "ingsoc.h"
#include "profile.h"
#include "platform_api.h"
#include "uart_golden_DUT.h"
#include "pulse_test_gpio.h"

#define CHECK_BUFFER_OVERFLOW(var, max_size) \
    do { \
        if ((var) >= (max_size)) { \
            platform_printf("[ERROR] Buffer overflow! (%s=%d, max=%d)\n", \
                          #var, (var), (max_size)); \
            var = 0; \
            return 1; \
        } \
    } while (0)

static uint8_t uart_golden_buff[UART_GOLDEN_BUFF_SIZE] = {0};
static uint16_t recive_len = 0; 
#define UART_RX_WATER_LEVEL		8

void uart_golden_process(uint8_t *buffer, uint16_t len);

uint32_t uart_dut_isr(void *user_data)
{
    
    uint32_t status;
    uint8_t  count = 0;

    status = apUART_Get_all_raw_int_stat(DUT_PORT);
    if (status & (1 << bsUART_RECEIVE_INTENAB))
    {
        while (apUART_Check_RXFIFO_EMPTY(DUT_PORT) != 1)
        {
			uart_golden_buff[recive_len] = DUT_PORT->DataRead;
			recive_len++;
            CHECK_BUFFER_OVERFLOW(recive_len, UART_GOLDEN_BUFF_SIZE);
			count++;
			if(count >= (UART_RX_WATER_LEVEL-1))
			{
				count = 0;
				break;
			}
        }
    }
    else if (status & (1 << bsUART_TIMEOUT_INTENAB))
    {
        while (apUART_Check_RXFIFO_EMPTY(DUT_PORT) != 1)
        {
            uart_golden_buff[recive_len] = DUT_PORT->DataRead;
			recive_len++;
            CHECK_BUFFER_OVERFLOW(recive_len, UART_GOLDEN_BUFF_SIZE);
        }
		uint16_t temp_len = recive_len;
		uart_golden_process(uart_golden_buff,temp_len);
		recive_len = 0;
		
    }
    
    APB_UART1->IntClear = status;
    return 0;
}

void uart_DUT_init(uint32_t Baud_Rate)
{
    SYSCTRL_ClearClkGateMulti((1 << SYSCTRL_ClkGate_APB_UART0)
                        | (1 << SYSCTRL_ClkGate_APB_PinCtrl));

    //设置UART PIN
//    PINCTRL_SetPadMux(UART0_RX, IO_SOURCE_GENERAL);
//    PINCTRL_SelUartRxdIn(DUT_UART, UART0_RX);
//    PINCTRL_Pull(UART0_RX,PINCTRL_PULL_UP);

//    PINCTRL_SetPadMux(UART0_TX, IO_SOURCE_UART0_TXD);
//    PINCTRL_Pull(UART0_TX,PINCTRL_PULL_UP);

    UART_sStateStruct uart_config;

    uart_config.word_length       = UART_WLEN_8_BITS;
    uart_config.parity            = UART_PARITY_NOT_CHECK;
    uart_config.fifo_enable       = 1;
    uart_config.two_stop_bits     = 0;
    uart_config.receive_en        = 1;
    uart_config.transmit_en       = 1;
    uart_config.UART_en           = 1;
    uart_config.cts_en            = 0;
    uart_config.rts_en            = 0;
    uart_config.rxfifo_waterlevel = 8;      
    uart_config.txfifo_waterlevel = 1;
    uart_config.ClockFrequency    = OSC_CLK_FREQ;
    uart_config.BaudRate          = Baud_Rate;

    apUART_Initialize(DUT_PORT, &uart_config, (1 << bsUART_RECEIVE_INTENAB) | (1 << bsUART_TIMEOUT_INTENAB));
    platform_set_irq_callback(DUT_UART_IRQ_CB, uart_dut_isr, NULL);
}

void uart_golden_process(uint8_t *buffer, uint16_t len)
{
/*test code*/
    // platform_printf("Received %d bytes: ", len);
    // for (uint16_t i = 0; i < len; i++) {
    //     platform_printf("%02X ", buffer[i]);
    // }
    // platform_printf("\n");
/*test code end*/
}