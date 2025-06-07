#include <stdint.h>
#include <stdio.h>
#include "ingsoc.h"
#include "profile.h"
#include "platform_api.h"
#include "string.h"

#include "uart_upper_golden.h"
#include "pulse_test_gpio.h"
#include "rf_test.h"
#include "uart_protocol.h"
#include "my_ring_buffer.h"
#include "crc_16.h"

ring_fifo_t *uart_upper_fifo = NULL;

#define UART_RX_WATER_LEVEL		8

void uart_upper_process(ring_fifo_t *uart_fifo);

uint32_t uart_upper_isr(void *user_data)
{
    uint32_t status;
    uint8_t  count = 0;

    status = apUART_Get_all_raw_int_stat(UPPER_PORT);
    if (status & (1 << bsUART_RECEIVE_INTENAB))
    {
        while (apUART_Check_RXFIFO_EMPTY(UPPER_PORT) != 1)
        {
            write_ring_fifo_char(uart_upper_fifo, UPPER_PORT->DataRead);
        }
    }
    else if (status & (1 << bsUART_TIMEOUT_INTENAB))
    {
        while (apUART_Check_RXFIFO_EMPTY(UPPER_PORT) != 1)
        {
            write_ring_fifo_char(uart_upper_fifo, UPPER_PORT->DataRead);
        }
		uart_upper_process(uart_upper_fifo);
    }

    else if (status & (1 << bsUART_ERROR_INTENAB))
    {
    }

    UPPER_PORT->IntClear = status;
    return 0;
}

void uart_upper_init(uint32_t Baud_Rate)
{
    SYSCTRL_ClearClkGateMulti((1 << UPPER_PORT_CLK_GATE)
                        | (1 << SYSCTRL_ClkGate_APB_PinCtrl));

   PINCTRL_SetPadMux(UPPER_RX_PIN, IO_SOURCE_GENERAL);
   PINCTRL_SelUartRxdIn(UPPER_UART, UPPER_RX_PIN);
   PINCTRL_Pull(UPPER_RX_PIN,PINCTRL_PULL_UP);

   PINCTRL_SetPadMux(UPPER_TX_PIN, UPPER_IO_SOURCE_TXD);
   PINCTRL_Pull(UPPER_TX_PIN,PINCTRL_PULL_UP);

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
    uart_config.rxfifo_waterlevel = 12;      
    uart_config.txfifo_waterlevel = 1;
    uart_config.ClockFrequency    = OSC_CLK_FREQ;
    uart_config.BaudRate          = Baud_Rate;

    apUART_Initialize(UPPER_PORT, &uart_config, (1 << bsUART_RECEIVE_INTENAB) | (1 << bsUART_TIMEOUT_INTENAB) | (1 << bsUART_ERROR_INTENAB));
    platform_set_irq_callback(UPPER_UART_IRQ_CB, uart_upper_isr, NULL);

    uart_upper_fifo = ring_fifo_init(UART_UPPER_BUFF_SIZE);
    if (uart_upper_fifo == NULL) {
        DBG_PRINTF("[ERROR] Failed to initialize UART upper FIFO\n");
        return;
    }
}

void uart_upper_send(const uint8_t * buffer, uint16_t len)
{
	for(uint16_t i=0; i<len; i++)
    {
        while(apUART_Check_TXFIFO_FULL(UPPER_PORT) == 1);
        UART_SendData(UPPER_PORT, buffer[i]);
    }
}

void uart_cmd_start_rsp(uint8_t result)
{
    uint8_t buffer[10] = {0};
    uint16_t temp_crc = 0;
    buffer[0] = PROTOCOL_HEADER1; // Frame header 0x4A45
    buffer[1] = PROTOCOL_HEADER2; // Frame header 0x4A45
    buffer[2] = UPPER_GOLDEN_RF_CMD_RSP;        // UART type (UART1/UART2)
    buffer[3] = PROTOCOL_UPPER_CMD_START_RSP_LEN;                   // length
    buffer[4] = UART_UPPER_TEST_START;            // Command
    buffer[5] = result;
    temp_crc = check_crc16((char *)buffer, 6);
    buffer[6] = (uint8_t)(temp_crc & 0x00FF);
    buffer[7] = (uint8_t)((temp_crc & 0xFF00) >> 8);
    buffer[8] = PROTOCOL_TAIL1;       // Frame tail 0x4E44
    buffer[9] = PROTOCOL_TAIL2;       // Frame tail 0x4E44

    uart_upper_send(buffer, sizeof(buffer));
}

void uart_cmd_get_result_rsp(uint8_t status, rf_test_data_t *data)
{
    uint8_t buffer[20] = {0};

    buffer[0] = PROTOCOL_HEADER1; // Frame header 0x4A45
    buffer[1] = PROTOCOL_HEADER2; // Frame header 0x4A45
    buffer[2] = UPPER_GOLDEN_RF_CMD_RSP;        // UART type (UART1/UART2)
    buffer[3] = PROTOCOL_UPPER_CMD_START_RSP_LEN;                   // length
    buffer[4] = UART_UPPER_GET_RESULT;            // Command
    if(status == TEST_SUCCESS)
    {
        if(data != NULL)
        {
            buffer[5] = TEST_SUCCESS;         //test running/failed
            memcpy(&buffer[6], data, sizeof(rf_test_data_t));
            buffer[6 + sizeof(rf_test_data_t)] = PROTOCOL_TAIL1;       // Frame tail 0x4E44
            buffer[7 + sizeof(rf_test_data_t)] = PROTOCOL_TAIL2;       // Frame tail 0x4E44
        }
        else {
            buffer[5] = TEST_FAILED;         //test running/failed
            buffer[6] = PROTOCOL_TAIL1;       // Frame tail 0x4E44
            buffer[7] = PROTOCOL_TAIL2;       // Frame tail 0x4E44
        }
    }
    else
    {
        buffer[5] = TEST_RUNNING;         //test running/failed
        buffer[6] = PROTOCOL_TAIL1;       // Frame tail 0x4E44
        buffer[7] = PROTOCOL_TAIL2;       // Frame tail 0x4E44
    }

    uart_upper_send(buffer, sizeof(buffer));
}


static void process_complete_packet(uint8_t* buffer, uint8_t len)
{
    DBG_PRINTF("[Upper]Received complete packet: %d bytes\n", len);
    switch(buffer[4]) {
        case UART_UPPER_TEST_START: {
            RFTest_Result_t status;
            DBG_PRINTF("UART_UPPER_TEST_START status: %d\n", status);
            status = rf_test_start(&buffer[5]);
            uart_cmd_start_rsp(status);
            break;
        }
        case UART_UPPER_GET_RESULT: {
            rf_test_data_t data;
            RFTest_Result_t status;
            // status = rf_test_get_result(&data);
            DBG_PRINTF("UART_UPPER_GET_RESULT status: %d\n", status);
            uart_cmd_get_result_rsp(status, &data);
            break;
        }
        default:
            DBG_PRINTF("[ERROR] Unknown command: %02X\n", buffer[4]);
            break;
    }
}

void uart_upper_process(ring_fifo_t *uart_fifo)
{
    static parse_state_t state = PARSE_STATE_HEADER1;
    static uint8_t packet[20]; // Temporary storage of complete packages
    static uint8_t data_len = 0;
    static uint8_t data_index = 0;
    static uint8_t expected_len = 0;
    
    uint8_t ch;
    fifo_status_t status;

    while (get_ring_buff_used(uart_fifo) > 0) {
        status = peek_ring_fifo_char(uart_fifo, &ch);
        if (status != FIFO_OK) break;

        switch (state) {
            case PARSE_STATE_HEADER1:
                gpio_pin_pulse(0);
                if (ch == PROTOCOL_HEADER1) {
                    packet[0] = ch;
                    DBG_PRINTF("[DUT]Header1:0x%x\n", ch);
                    state = PARSE_STATE_HEADER2;
                    read_ring_fifo_char(uart_fifo, &ch); // use the byte
                } else {
                    read_ring_fifo_char(uart_fifo, &ch); // discard invalid data
                }
                break;

            case PARSE_STATE_HEADER2:
                gpio_pin_pulse(1);
                if (ch == PROTOCOL_HEADER2) {
                    packet[1] = ch;
                    state = PARSE_STATE_TYPE;
                    DBG_PRINTF("[DUT]Header2:0x%x\n", ch);
                    read_ring_fifo_char(uart_fifo, &ch);
                } else {
                    state = PARSE_STATE_HEADER1; // reset state
                }
                break;

            case PARSE_STATE_TYPE:
                gpio_pin_pulse(2);
                if (ch == UPPER_GOLDEN_RF_CMD) {
                    packet[2] = ch;
                    state = PARSE_STATE_LENGTH;
                    DBG_PRINTF("[DUT]type:0x%x\n", ch);
                    read_ring_fifo_char(uart_fifo, &ch);
                } else {
                    state = PARSE_STATE_HEADER1; // reset state
                }
                break;

            case PARSE_STATE_LENGTH:
                gpio_pin_pulse(3);
                if (PROTOCOL_UPPER_CMD_MAX_LEN < ch) {
                    state = PARSE_STATE_HEADER1; // reset state
                    break;
                }
                DBG_PRINTF("[DUT]length:0x%x\n", ch);
                packet[3] = ch;
                data_len = ch;
                expected_len = 2 + 1 + 1 + data_len + 2 + 2; //Calculate the full package length
                state = PARSE_STATE_CMD;
                read_ring_fifo_char(uart_fifo, &ch);
                break;

            case PARSE_STATE_CMD:
                gpio_pin_pulse(4);
                if(ch != UART_UPPER_TEST_START && ch != UART_UPPER_GET_RESULT)
                {
                    state = PARSE_STATE_HEADER1; // reset state
                    break;
                }
                DBG_PRINTF("[DUT]cmd:0x%x\n", ch);
                packet[4] = ch;
                data_index = 0;
                if (data_len > 1) {
                    state = PARSE_STATE_DATA;
                } else {
                    state = PARSE_STATE_CRC;
                }
                read_ring_fifo_char(uart_fifo, &ch);
                break;

            case PARSE_STATE_DATA:
                gpio_pin_pulse(5);
                DBG_PRINTF("[DUT]data:0x%x\n", ch);
                packet[5 + data_index] = ch;
                data_index++;
                if (data_index >= (data_len -1)) {//data_len contains cmd and data
                    state = PARSE_STATE_CRC;
                }
                read_ring_fifo_char(uart_fifo, &ch);
                break;
            case PARSE_STATE_CRC:
                DBG_PRINTF("[DUT]crc:0x%x\n", ch);
                packet[5 + data_index] = ch;
                data_index++;
                if(data_index >= (data_len + 1))
                {
                    uint16_t temp_crc = ((uint16_t)(packet[4 + data_index]) << 8) | (uint16_t)packet[4 + data_index - 1];
                    DBG_PRINTF("[DUT]%x\n", temp_crc);
                    uint16_t comp_crc = check_crc16((char *)packet, 4 + packet[3]);
                    DBG_PRINTF("[DUT]%x\n", comp_crc);
                    if(temp_crc == comp_crc){
                        state = PARSE_STATE_TAIL1;
                    }
                    else {
                        state = PARSE_STATE_HEADER1;
                    }
                }
                read_ring_fifo_char(uart_fifo, &ch);
                break;
            case PARSE_STATE_TAIL1:
                // gpio_pin_pulse(6);
                DBG_PRINTF("[DUT]tail:0x%x\n", ch);
                if (ch == PROTOCOL_TAIL1) {
                    packet[expected_len - 2] = ch;
                    state = PARSE_STATE_TAIL2;
                    read_ring_fifo_char(uart_fifo, &ch);
                } else {
                    state = PARSE_STATE_HEADER1;// Invalid data, reset
                }
                break;

            case PARSE_STATE_TAIL2:
                gpio_pin_pulse(7);
                DBG_PRINTF("[DUT]receive:0x%x\n", ch);
                if (ch == PROTOCOL_TAIL2) {
                    DBG_PRINTF("Received complete packet: %d bytes\n", expected_len);
                    packet[expected_len - 1] = ch;
                    // The complete package has been received, the package is processed
                    read_ring_fifo_char(uart_fifo, &ch);
                    process_complete_packet(packet, expected_len);
                }
                else{
                    DBG_PRINTF("Received incomplete packet: %d bytes\n", expected_len);
                }
                state = PARSE_STATE_HEADER1; // Reset the state machine
                break;
        }
    }
}