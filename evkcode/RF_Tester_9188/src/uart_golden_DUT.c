#include <stdint.h>
#include <stdio.h>
#include "ingsoc.h"
#include "profile.h"
#include "platform_api.h"
#include "uart_golden_DUT.h"
#include "pulse_test_gpio.h"
#include "rf_test.h"
#include "uart_protocol.h"
#include "uart_ota.h"
#include "evk_config.h"

#include "my_ring_buffer.h"
#include "crc_16.h"

ring_fifo_t *uart_DUT_fifo = NULL;
#define UART_RX_WATER_LEVEL		8

void uart_DUT_process(ring_fifo_t *uart_fifo);

uint32_t uart_dut_isr(void *user_data)
{
    
    uint32_t status;
    char ch[32] = {0};
    uint8_t  count = 0;
    uint8_t type = get_evk_protocol();

    status = apUART_Get_all_raw_int_stat(DUT_PORT);
    if (status & ((1 << bsUART_RECEIVE_INTENAB) | (1 << bsUART_TIMEOUT_INTENAB)))
    {
        while (apUART_Check_RXFIFO_EMPTY(DUT_PORT) != 1)
        {
            if(type == PROTOCOL_2_BURN)
            {
                char c = (char)DUT_PORT->DataRead;
                ch[count++] = c;
            }
            else 
            {
                write_ring_fifo_char(uart_DUT_fifo, DUT_PORT->DataRead);
            }
        }
        
        if(type == PROTOCOL_2_BURN)
        {
            uart_buner_rx_data(ch, count, status & (1 << bsUART_TIMEOUT_INTENAB));
        }
        else
        {
            uart_DUT_process(uart_DUT_fifo);
        }
    }
    else if (status & (1 << bsUART_ERROR_INTENAB))
    {
    }

    DUT_PORT->IntClear = status;
    return 0;
}

void uart_DUT_init(uint32_t Baud_Rate)
{
    SYSCTRL_ClearClkGateMulti((1 << DUT_PORT_CLK_GATE)
                        | (1 << SYSCTRL_ClkGate_APB_PinCtrl));

    
    PINCTRL_SetPadMux(DUT_RX_PIN, IO_SOURCE_GENERAL);
    PINCTRL_SelUartRxdIn(DUT_UART, DUT_RX_PIN);
    PINCTRL_Pull(DUT_RX_PIN,PINCTRL_PULL_UP);

    PINCTRL_SetPadMux(DUT_TX_PIN, DUT_IO_SOURCE_TXD);
    PINCTRL_Pull(DUT_TX_PIN,PINCTRL_PULL_UP);

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
}

//just for test, remember to remove it
void rx_cnt_cb(void)
{
    DUT_cmd_rf_rx_cnt(0, 0x962); // Start RX count test
    DBG_PRINTF("[DUT] RX count test finish, tell DUT.\n");
}

//just for test, remember to remove it
void rx_sen_cb(void)
{
    DUT_cmd_rf_rx_sen(0, 0x962); // Start RX sensitivity test
    DBG_PRINTF("[DUT] RX sensitivity test finish, tell DUT.\n");
}

//just for test, remember to remove it
// void rx_freq_offset_cb(void)
// {
//     DUT_cmd_rf_freq_offset(0, 0x962); // Start RX frequency offset test
//     DBG_PRINTF("[DUT] RX frequency offset test finish, tell DUT.\n");
// }

static void process_complete_packet(uint8_t* buffer, uint8_t len)
{
    RFTest_Result_t status;
    DBG_PRINTF("[DUT]Received complete packet: %d bytes\n", len);
    switch(buffer[4]) {
        case UART_DUT_TX_CNT_BEGIN_CMD: {
            status = rf_test_tx_cnt_begin();
            if(status == TEST_SUCCESS)
            {
                DBG_PRINTF("TX count test started successfully.\n");
            }
            else
            {
                DBG_PRINTF("TX count test failed.\n");
            }
            break;
        }
        case UART_DUT_TX_CNT_END_CMD: {
            status = rf_test_tx_cnt_end();
            if(status == TEST_SUCCESS)
            {
                DBG_PRINTF("TX count test completed successfully.\n");
            }
            else
            {
                DBG_PRINTF("TX count test failed.\n");
            }
           break;
        }
        case UART_DUT_TX_POWER_BEGIN_CMD: {
            status = rf_test_tx_power_begin();
            if(status == TEST_SUCCESS)
            {
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
                //todo: deal with the actual test logic and remove the test callback
                // platform_set_timer(rx_cnt_cb, 3200);
                DBG_PRINTF("RX count test started successfully.\n");
            }
            else
            {
                DBG_PRINTF("RX count test failed.\n");
            }
            break;
        }
        case UART_DUT_RX_CNT_END_CMD: {
            uint16_t test_data = (uint16_t)buffer[6] | ((uint16_t)buffer[7] << 8);
            platform_printf("data 1 is %x\n",buffer[6]);
            platform_printf("data 2 is %x\n",buffer[7]);
            platform_printf("RX count test data: %d\n", test_data);
            status = rf_test_rx_cnt_end(test_data);
            if(status == TEST_SUCCESS)
            {
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
                rx_sen_cb();
                //todo: deal with the actual test logic and remove the test callback
                // platform_set_timer(rx_sen_cb, 3200);
                DBG_PRINTF("RX sensitivity test started successfully.\n");
            }
            else
            {
                DBG_PRINTF("RX sensitivity test failed.\n");
            }
            break;
        }
        case UART_DUT_RX_SEN_END_CMD: {
            status = rf_test_rx_sen_end(buffer[5]);
            if(status == TEST_SUCCESS)
            {
            DBG_PRINTF("RX sensitivity test completed successfully.\n");
            }
            else
            {
            DBG_PRINTF("RX sensitivity test failed.\n");
            }
            break;
        }
        case UART_DUT_FREQ_OFFSET_BEGIN_CMD: {
            status = rf_test_freq_offset_begin();
            if(status == TEST_SUCCESS)
            {
                //todo: deal with the actual test logic and remove the test callback
                // platform_set_timer(rx_freq_offset_cb, 3200);
                DBG_PRINTF("Frequency offset test started successfully.\n");
            }
            else
            {
                DBG_PRINTF("Frequency offset test failed.\n");
            }
            break;
        }
        case UART_DUT_FREQ_OFFSET_END_CMD: {
            // status = rf_test_freq_offset_end();
            if(status == TEST_SUCCESS)
            {
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
                // DBG_PRINTF("[DUT]Header1:0x%x\n", ch);
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
                // DBG_PRINTF("[DUT]Header2:0x%x\n", ch);
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
                // DBG_PRINTF("[DUT]Type:0x%x\n", ch);
                gpio_pin_pulse(2);
                if (ch == GOLDEN_DUT_RF_CMD_RSP) {
                    packet[2] = ch;
                    state = PARSE_STATE_LENGTH;
                    read_ring_fifo_char(uart_fifo, &ch);
                } else {
                    state = PARSE_STATE_HEADER1; // reset state
                }
                break;

            case PARSE_STATE_LENGTH:
                // DBG_PRINTF("[DUT]Length:0x%x\n", ch);
                gpio_pin_pulse(3);
                if (PROTOCOL_DUT_CMD_RSP_MAX_LEN < ch) {
                    state = PARSE_STATE_HEADER1; // reset state
                    break;
                }

                packet[3] = ch;
                data_len = ch;
                expected_len = 2 + 1 + 1 + data_len + 2 + 2;// Calculate the full package length
                state = PARSE_STATE_CMD;
                read_ring_fifo_char(uart_fifo, &ch);
                break;

            case PARSE_STATE_CMD:
                // DBG_PRINTF("[DUT]CMD:0x%x\n", ch);
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
                // DBG_PRINTF("[DUT]Data:0x%x\n", ch);
                gpio_pin_pulse(5);
                packet[5 + data_index] = ch;
                data_index++;
                if (data_index >= data_len-1) {
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
                    // DBG_PRINTF("[DUT]CRC:0x%x\n", temp_crc);
                    uint16_t comp_crc = check_crc16((char *)packet, 4 + packet[3]);
                    // DBG_PRINTF("[DUT]CRC:0x%x\n", comp_crc);
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
                // DBG_PRINTF("[DUT]Tail1:0x%x\n", ch);
                // gpio_pin_pulse(6);
                if (ch == PROTOCOL_TAIL1) {
                    packet[expected_len - 2] = ch;
                    state = PARSE_STATE_TAIL2;
                    read_ring_fifo_char(uart_fifo, &ch);
                } else {
                    state = PARSE_STATE_HEADER1; // Invalid data, reset
                }
                break;

            case PARSE_STATE_TAIL2:
                // DBG_PRINTF("[DUT]Tail2:0x%x\n", ch);
                gpio_pin_pulse(7);
                if (ch == PROTOCOL_TAIL2) {
                    packet[expected_len - 1] = ch;
                    // The complete package has been received, the processing package is
                    read_ring_fifo_char(uart_fifo, &ch);
                    process_complete_packet(packet, expected_len);
                }
                state = PARSE_STATE_HEADER1; // Reset the state machine
                break;
        }
    }
}

//Finally merged into one API, only used in the DEBUG phase
void uart_cmd_ch_addr_pkt_assemble(uint8_t *buffer, uint8_t cmd, uint16_t ch_id, uint32_t acc_addr)
{
    uint16_t temp_crc = 0;
    buffer[0] = PROTOCOL_HEADER1; // Frame header 0x4A45
    buffer[1] = PROTOCOL_HEADER2; // Frame header 0x4A45
    buffer[2] = GOLDEN_DUT_RF_CMD;        // UART type (UART1/UART2)
    buffer[3] = PROTOCOL_GOLDEN_CH_LEN;                   // length
    buffer[4] = cmd;            // Command
    buffer[5] = (uint8_t)(ch_id & 0x00FF);
    buffer[6] = (uint8_t)((ch_id & 0xFF00) >> 8);
    buffer[7] = (uint8_t)(acc_addr & 0x000000FF);
    buffer[8] = (uint8_t)((acc_addr & 0x0000FF00) >> 8);
    buffer[9] = (uint8_t)((acc_addr & 0x00FF0000) >> 16);
    buffer[10] = (uint8_t)((acc_addr & 0xFF000000) >> 24);

    temp_crc = check_crc16((char *)buffer, 11);
    buffer[11] = (uint8_t)(temp_crc & 0x00FF);
    buffer[12] = (uint8_t)((temp_crc & 0xFF00) >> 8);

    buffer[13] = PROTOCOL_TAIL1;       // Frame tail 0x4E44
    buffer[14] = PROTOCOL_TAIL2;       // Frame tail 0x4E44
}

void uart_cmd_packet_assemble(uint8_t *buffer, uint8_t cmd, uint16_t data)
{
    uint16_t temp_crc = 0;
    buffer[0] = PROTOCOL_HEADER1; // Frame header 0x4A45
    buffer[1] = PROTOCOL_HEADER2; // Frame header 0x4A45
    buffer[2] = GOLDEN_DUT_RF_CMD;        // UART type (UART1/UART2)
    buffer[3] = PROTOCOL_GOLDEN_CMD_LEN;                   // length
    buffer[4] = cmd;            // Command
    buffer[5] = (uint8_t)data;
    buffer[6] = (uint8_t)(data>>8);

    temp_crc = check_crc16((char *)buffer, 7);
    // DBG_PRINTF("[DUT]%x\n", temp_crc);
    buffer[7] = (uint8_t)(temp_crc & 0x00FF);
    buffer[8] = (uint8_t)((temp_crc & 0xFF00) >> 8);

    buffer[9] = PROTOCOL_TAIL1;       // Frame tail 0x4E44
    buffer[10] = PROTOCOL_TAIL2;       // Frame tail 0x4E44
}

void uart_DUT_send(const uint8_t * buffer, uint16_t len)
{   
    // DBG_PRINTF("[UART]:Sending UART DUT command: %d bytes, 0x%x\n", len, buffer[4]);
	for(uint16_t i=0; i<len; i++)
    {
        // Determine whether the sending buffer is full   
        while(apUART_Check_TXFIFO_FULL(DUT_PORT) == 1);
        UART_SendData(DUT_PORT, buffer[i]);
    }
}

void DUT_cmd_rf_tx_cnt(uint8_t en, uint16_t channel, uint32_t acc_addr)
{
    uint8_t buffer[16] = {0};
    if(en)
        uart_cmd_ch_addr_pkt_assemble(buffer, UART_DUT_TX_CNT_BEGIN_CMD, channel, acc_addr);
        // uart_cmd_packet_assemble(buffer, UART_DUT_TX_CNT_BEGIN_CMD, channel);
    else
        uart_cmd_packet_assemble(buffer, UART_DUT_TX_CNT_END_CMD, channel);
    uart_DUT_send(buffer, PROTOCOL_CH_ADDR_BUF_LENGTH);
}

void DUT_cmd_rf_tx_power(uint8_t en, uint16_t channel)
{
    uint8_t buffer[16] = {0};
    if(en)
        uart_cmd_packet_assemble(buffer, UART_DUT_TX_POWER_BEGIN_CMD, channel);
    else
        uart_cmd_packet_assemble(buffer, UART_DUT_TX_POWER_END_CMD, channel);
    uart_DUT_send(buffer, PROTOCOL_DUT_BUF_LENGTH);
}

void DUT_cmd_rf_rx_cnt(uint8_t en, uint16_t channel)
{
    uint8_t buffer[16] = {0};
    if(en)
        uart_cmd_packet_assemble(buffer, UART_DUT_RX_CNT_BEGIN_CMD, channel);
    else
        uart_cmd_packet_assemble(buffer, UART_DUT_RX_CNT_END_CMD, channel);
    uart_DUT_send(buffer, PROTOCOL_DUT_BUF_LENGTH);
}

void DUT_cmd_rf_rx_sen(uint8_t en, uint16_t channel)
{
    uint8_t buffer[16] = {0};
    if(en)
        uart_cmd_packet_assemble(buffer, UART_DUT_RX_SEN_BEGIN_CMD, channel);
    else
        uart_cmd_packet_assemble(buffer, UART_DUT_RX_SEN_END_CMD, channel);
    uart_DUT_send(buffer, PROTOCOL_DUT_BUF_LENGTH);
}

void DUT_cmd_rf_freq_offset(uint8_t en, uint16_t channel)
{
    uint8_t buffer[16] = {0};
    if(en)
        uart_cmd_packet_assemble(buffer, UART_DUT_FREQ_OFFSET_BEGIN_CMD, channel);
    else
        uart_cmd_packet_assemble(buffer, UART_DUT_FREQ_OFFSET_END_CMD, channel);
    uart_DUT_send(buffer, PROTOCOL_DUT_BUF_LENGTH);
}

void DUT_cmd_rf_freq_offset_tune(uint8_t tune_value)
{
    uint8_t buffer[16] = {0};
    uart_cmd_packet_assemble(buffer, UART_DUT_FREQ_OFFSET_TUNE_CMD, (uint16_t)tune_value);
    uart_DUT_send(buffer, PROTOCOL_DUT_BUF_LENGTH);
}

void driver_append_tx_data(const void *data, uint16_t len)
{
    uart_DUT_send((uint8_t *)data, len);
}


