#include <stdint.h>
#include <stdio.h>
#include "ingsoc.h"
#include "profile.h"
#include "platform_api.h"
#include "uart_golden_DUT.h"
#include "pulse_test_gpio.h"
#include "rf_test.h"
#include "uart_protocol.h"

#include "my_ring_buffer.h"
#include "freq_offset.h"
#include "crc_16.h"

ring_fifo_t *uart_DUT_fifo = NULL;
#define UART_RX_WATER_LEVEL		8

void uart_DUT_process(ring_fifo_t *uart_fifo);

uint32_t uart_dut_isr(void *user_data)
{
    uint32_t status;
    status = apUART_Get_all_raw_int_stat(DUT_PORT);
    if (status & (1 << bsUART_RECEIVE_INTENAB))
    {
        // DBG_PRINTF("UART RX interrupt\n");
        while (apUART_Check_RXFIFO_EMPTY(DUT_PORT) != 1)
        {
			write_ring_fifo_char(uart_DUT_fifo, DUT_PORT->DataRead);
        }
        uart_DUT_process(uart_DUT_fifo);
    }
    else if (status & (1 << bsUART_TIMEOUT_INTENAB))
    {
        while (apUART_Check_RXFIFO_EMPTY(DUT_PORT) != 1)
        {
            write_ring_fifo_char(uart_DUT_fifo, DUT_PORT->DataRead);
        }
		uart_DUT_process(uart_DUT_fifo);
    }
    else if (status & (1 << bsUART_ERROR_INTENAB))
    {
        DBG_PRINTF("UART ERROR isr\n");
    }

    DUT_PORT->IntClear = status;
    return 0;
}

void uart_DUT_init(uint32_t Baud_Rate)
{
//     SYSCTRL_ClearClkGateMulti((1 << SYSCTRL_ClkGate_APB_UART1)
//                         | (1 << SYSCTRL_ClkGate_APB_PinCtrl)
//                         | (1 << SYSCTRL_ClkGate_APB_GPIO0)
//                         | (1 << SYSCTRL_ClkGate_APB_GPIO1));


//     PINCTRL_SetPadMux(DUT_RX_PIN, IO_SOURCE_GENERAL);
//    PINCTRL_SelUartRxdIn(DUT_UART, DUT_RX_PIN);
//    PINCTRL_Pull(DUT_RX_PIN,PINCTRL_PULL_UP);

//    PINCTRL_SetPadMux(DUT_TX_PIN, IO_SOURCE_UART1_TXD);
//    PINCTRL_Pull(DUT_TX_PIN,PINCTRL_PULL_UP);

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
    uart_config.rxfifo_waterlevel = 16;      
    uart_config.txfifo_waterlevel = 1;
    uart_config.ClockFrequency    = OSC_CLK_FREQ;
    uart_config.BaudRate          = Baud_Rate;

    apUART_Initialize(DUT_PORT, &uart_config, (1 << bsUART_RECEIVE_INTENAB) | (1 << bsUART_TIMEOUT_INTENAB) | (1 << bsUART_ERROR_INTENAB));
    platform_set_irq_callback(DUT_UART_IRQ_CB, uart_dut_isr, NULL);

    uart_DUT_fifo = ring_fifo_init(UART_DUT_BUFF_SIZE);
    if (uart_DUT_fifo == NULL) {
        DBG_PRINTF("[ERROR] Failed to initialize UART DUT FIFO\n");
        return;
    }
    DBG_PRINTF("UART DUT FIFO initialized successfully\n");
}


void uart_cmd_packet_assemble(uint8_t *buffer, uint8_t cmd, uint8_t *buffer_data, uint8_t data_len)
{
    uint16_t temp_crc = 0;
    buffer[0] = PROTOCOL_HEADER1; // Frame header 0x4A45
    buffer[1] = PROTOCOL_HEADER2; // Frame header 0x4A45
    buffer[2] = GOLDEN_DUT_RF_CMD_RSP;        // UART type (UART1/UART2)
    buffer[3] = data_len+1  ;                   // length
    buffer[4] = cmd;            // Command
    if((data_len >= 1))
    {
        if((buffer_data != NULL))
        {
            for(uint8_t i=0; i<data_len; i++)
            {
                buffer[5+i] = buffer_data[i];
            }
        }
        else{
            DBG_PRINTF("[ERROR]buffer_data is NULL\n");
        }
    }
    temp_crc = check_crc16((char *)buffer, 4 + (data_len + 1));
    buffer[data_len + 5] = (uint8_t)(temp_crc & 0x00FF);
    buffer[data_len + 6] = (uint8_t)((temp_crc & 0xFF00) >> 8);
    buffer[data_len + 7] = PROTOCOL_TAIL1;       // Frame tail 0x4E44
    buffer[data_len + 8] = PROTOCOL_TAIL2;       // Frame tail 0x4E44
}

void uart_DUT_send(const uint8_t * buffer, uint16_t len)
{
    DBG_PRINTF("[UART]:Sending UART DUT command: %d bytes, 0x%x\n", len, buffer[4]);
	for(uint16_t i=0; i<len; i++)
    {
        //判断发送缓冲区是否为满
        while(apUART_Check_TXFIFO_FULL(DUT_PORT) == 1);
        UART_SendData(DUT_PORT, buffer[i]);
    }
}

void DUT_cmd_rf_tx_cnt(uint8_t en, uint16_t status)
{
    uint8_t buffer[11] = {0};
    uint8_t data[5] = {0};
    data[0] = (uint8_t)status;
    if(en){
        uart_cmd_packet_assemble(buffer, UART_DUT_TX_CNT_BEGIN_CMD, data, 2);
    }
    else{
        uart_cmd_packet_assemble(buffer, UART_DUT_TX_CNT_END_CMD, data, 2);
    }
    uart_DUT_send(buffer, sizeof(buffer));
}

void DUT_cmd_rf_tx_power(uint8_t en, uint8_t status)
{
    uint8_t buffer[10] = {0};
    uint8_t data[2] = {0};
    data[0] = (uint8_t)status;
    if(en)
        uart_cmd_packet_assemble(buffer, UART_DUT_TX_POWER_BEGIN_CMD, data, 1);
    else
        uart_cmd_packet_assemble(buffer, UART_DUT_TX_POWER_END_CMD, data, 1);
    uart_DUT_send(buffer, sizeof(buffer));
}

void DUT_cmd_rf_rx_cnt(uint8_t en, uint8_t status, uint16_t result)
{
    uint8_t buffer[12] = {0};
    uint8_t data[3] = {0};
    data[0] = (uint8_t)status;
    if(en){
        uart_cmd_packet_assemble(buffer, UART_DUT_RX_CNT_BEGIN_CMD, data, 1);
    }
    else
    {
        data[1] = (uint8_t)(result & 0x00FF);
        data[2] = (uint8_t)((result >> 8) & 0x00FF);
        uart_cmd_packet_assemble(buffer, UART_DUT_RX_CNT_END_CMD, data, 3);
    }
    uart_DUT_send(buffer, sizeof(buffer));
}

void DUT_cmd_rf_rx_sen(uint8_t en, uint8_t status, int8_t result)
{
    uint8_t buffer[10] = {0};
    uint8_t data[2] = {0};
    data[0] = (uint8_t)status;
    if(en)
        uart_cmd_packet_assemble(buffer, UART_DUT_RX_SEN_BEGIN_CMD, data, 1);
    else
    {
        data[1] = (uint8_t)result;
        uart_cmd_packet_assemble(buffer, UART_DUT_RX_SEN_END_CMD, &data[1], 1);
    }
    uart_DUT_send(buffer, sizeof(buffer));
}

void DUT_cmd_rf_freq_offset(uint8_t en, uint8_t status)
{
    uint8_t buffer[10] = {0};
    uint8_t data[2] = {0};
    data[0] = (uint8_t)status;
    if(en)
        uart_cmd_packet_assemble(buffer, UART_DUT_FREQ_OFFSET_BEGIN_CMD, data, 1);
    else
        uart_cmd_packet_assemble(buffer, UART_DUT_FREQ_OFFSET_END_CMD, data, 1);
    uart_DUT_send(buffer, sizeof(buffer));
}


static void process_complete_packet(uint8_t* buffer, uint8_t len)
{
    RFTest_Result_t status;
    DBG_PRINTF("[DUT]Received complete packet: %d bytes\n", len);
    switch(buffer[4]) {
        //todo:deal with the channel data imput(buffer[5]~buffer[6])
        case UART_DUT_TX_CNT_BEGIN_CMD: {
            status = rf_test_tx_cnt_begin(&buffer[5]);
            if(status == TEST_SUCCESS)
            {
                DUT_cmd_rf_tx_cnt(1, status);
                DBG_PRINTF("TX count test started successfully.\n");
            }
            else
            {
                DBG_PRINTF("TX count test failed.\n");
            }
            break;
        }

        //due to the timing of tx cnt test is controlled by DUT, so the golden will not send this command normally
        case UART_DUT_TX_CNT_END_CMD: {
            status = rf_test_tx_cnt_end();//to do. data in buf is useful?
            if(status == TEST_SUCCESS)
            {
                DUT_cmd_rf_tx_cnt(0, status);
                DBG_PRINTF("TX count test completed successfully.\n");
            }
            else
            {
                DBG_PRINTF("TX count test failed.\n");
            }
           break;
        }

        //todo:deal with the channel data imput(buffer[5]~buffer[6])
        case UART_DUT_TX_POWER_BEGIN_CMD: {
            status = rf_test_tx_power_begin();
            if(status == TEST_SUCCESS)
            {
                DUT_cmd_rf_tx_power(1, status);
                DBG_PRINTF("TX power test started successfully.\n");
            }
            else
            {
                DBG_PRINTF("TX power test failed.\n");
            }
            break;
        }
        case UART_DUT_TX_POWER_END_CMD: {
            status = rf_test_tx_power_end();
            if(status == TEST_SUCCESS)
            {
                DUT_cmd_rf_tx_power(0, status);
                DBG_PRINTF("TX power test completed successfully.\n");
            }
            else
            {
                DBG_PRINTF("TX power test failed.\n");
            }
            break;
        }
        case UART_DUT_RX_CNT_BEGIN_CMD: {
            status = rf_test_rx_cnt_begin();
            if(status == TEST_SUCCESS)
            {
                DUT_cmd_rf_rx_cnt(1, status, 0);//reply to begin command
                DBG_PRINTF("RX count test started successfully.\n");
            }
            else
            {
                DBG_PRINTF("RX count test failed.\n");
            }
            break;
        }
        case UART_DUT_RX_CNT_END_CMD: {
            status = rf_test_rx_cnt_end();
            //todo:get RX count result
            if(status == TEST_SUCCESS)
            {
                DUT_cmd_rf_rx_cnt(0, status, get_rx_cnt_result());
                DBG_PRINTF("RX count test completed successfully.\n");
            }
            else
            {
                DBG_PRINTF("RX count test failed.\n");
            }
           break;
        }
        case UART_DUT_RX_SEN_BEGIN_CMD: {
            status = rf_test_rx_sen_begin();
            if(status == TEST_SUCCESS)
            {
                DUT_cmd_rf_rx_sen(1, status, 0);//reply to begin command
                DBG_PRINTF("RX sensitivity test started successfully.\n");
            }
            else
            {
                DBG_PRINTF("RX sensitivity test failed.\n");
            }
            break;
        }
        case UART_DUT_RX_SEN_END_CMD: {
            status = rf_test_rx_sen_end();
            //todo: return RX sensitivity result
            if(status == TEST_SUCCESS)
            {
                platform_printf("RX sensitivity is %d\n", get_rx_power_result());
                DUT_cmd_rf_rx_sen(0, status, get_rx_power_result());
                DBG_PRINTF("RX sensitivity test completed successfully.\n");
            }
            else
            {
                DBG_PRINTF("RX sensitivity test failed.\n");
            }
            break;
        }
        case UART_DUT_FREQ_OFFSET_BEGIN_CMD: {
            status = rf_test_freq_offset_begin(buffer[5]);
            if(status == TEST_SUCCESS)
            {
                DUT_cmd_rf_freq_offset(1, status);
                DBG_PRINTF("Frequency offset test started successfully.\n");
            }
            else
            {
                DBG_PRINTF("Frequency offset test failed.\n");
            }
            break;
        }
        case UART_DUT_FREQ_OFFSET_TUNE_CMD: {
            if(buffer[5] > 0x3F) //the max tune value is 0x3F
            {
                freq_offset_tune(0x3F); // Set to max tune value
            }
            else{
                freq_offset_tune(buffer[5]);
            }
            break;
        }
        case UART_DUT_FREQ_OFFSET_END_CMD: {
                status = rf_test_freq_offset_end();
                if(status == TEST_SUCCESS)
                {
                    DUT_cmd_rf_freq_offset(0, status);
                    set_dut_test_state(STATE_TX_RX_IDLE);
                    DBG_PRINTF("Frequency offset test completed successfully.\n");
                }
                else
                {
                    DBG_PRINTF("Frequency offset test failed.\n");
                }
            break;
        }
       default:
           DBG_PRINTF("[ERROR] Unknown command: %02X\n", buffer[4]);
           break;
   }
}

void uart_DUT_process(ring_fifo_t *uart_fifo)
{
    static parse_state_t state = PARSE_STATE_HEADER1;
    static uint8_t packet[20]; // Temporary storage of complete package
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
                // DBG_PRINTF("Processing header1: %02X\n", ch);
                gpio_pin_pulse(0);
                if (ch == PROTOCOL_HEADER1) {
                    packet[0] = ch;
                    state = PARSE_STATE_HEADER2;
                    read_ring_fifo_char(uart_fifo, &ch); // use the byte
                } else {
                    read_ring_fifo_char(uart_fifo, &ch); // discard invalid data
                }
                break;

            case PARSE_STATE_HEADER2:
                // DBG_PRINTF("Processing header2: %02X\n", ch);
                gpio_pin_pulse(1);
                if (ch == PROTOCOL_HEADER2) {
                    packet[1] = ch;
                    state = PARSE_STATE_TYPE;
                    read_ring_fifo_char(uart_fifo, &ch);
                } else {
                    state = PARSE_STATE_HEADER1; // reset state
                }
                break;

            case PARSE_STATE_TYPE:
                // DBG_PRINTF("Processing type: %02X\n", ch);
                gpio_pin_pulse(2);
                if (ch == GOLDEN_DUT_RF_CMD) {
                    packet[2] = ch;
                    state = PARSE_STATE_LENGTH;
                    read_ring_fifo_char(uart_fifo, &ch);
                } else {
                    state = PARSE_STATE_HEADER1; // reset state
                }
                break;

            case PARSE_STATE_LENGTH:
                // DBG_PRINTF("Processing length: %02X\n", ch);
                gpio_pin_pulse(3);
                if (PROTOCOL_DUT_CMD_MAX_LEN < ch) {
                    state = PARSE_STATE_HEADER1; // reset state
                    break;
                }

                packet[3] = ch;
                data_len = ch;
                expected_len = 2 + 1 + 1 + data_len + 2 + 2; // Calculate the full package length
                state = PARSE_STATE_CMD;
                read_ring_fifo_char(uart_fifo, &ch);
                break;

            case PARSE_STATE_CMD:
                // DBG_PRINTF("Processing command: %02X\n", ch);
                gpio_pin_pulse(4);
                if((ch < UART_DUT_TX_CNT_BEGIN_CMD) || (ch > UART_DUT_FREQ_OFFSET_TUNE_CMD))
                {
                    state = PARSE_STATE_HEADER1; // reset state
                    break;
                }

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
                // DBG_PRINTF("Processing data: %02X\n", ch);
                gpio_pin_pulse(5);
                packet[5 + data_index] = ch;
                data_index++;
                if (data_index >= (data_len-1)) { //data_len contains cmd and data
                    state = PARSE_STATE_CRC;
                }
                read_ring_fifo_char(uart_fifo, &ch);
                break;

            case PARSE_STATE_CRC:
				// DBG_PRINTF("[DUT]crc:0x%x\n", ch);
                packet[5 + data_index] = ch;
                data_index++;
                if(data_index >= (data_len + 1))
                {
                    uint16_t temp_crc = ((uint16_t)(packet[4 + data_index]) << 8) | (uint16_t)packet[4 + data_index - 1];
                    uint16_t comp_crc = check_crc16((char *)packet, 4 + packet[3]);
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
                // DBG_PRINTF("Processing tail1: %02X\n", ch);
                gpio_pin_pulse(6);
                if (ch == PROTOCOL_TAIL1) {
                    packet[expected_len - 2] = ch;
                    state = PARSE_STATE_TAIL2;
                    read_ring_fifo_char(uart_fifo, &ch);
                } else {
                    state = PARSE_STATE_HEADER1; //Invalid data, reset
                }
                break;

            case PARSE_STATE_TAIL2:
                // DBG_PRINTF("Processing tail2: %02X\n", ch);
                gpio_pin_pulse(7);
                if (ch == PROTOCOL_TAIL2) {
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

