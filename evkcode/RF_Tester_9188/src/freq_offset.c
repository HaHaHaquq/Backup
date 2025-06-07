#include <stdint.h>
#include <stdio.h>
#include "platform_api.h"
#include "ll_api.h"
#include "btstack_event.h"
#include "str_util.h"

#include "freq_offset.h"
#include "profile.h"

struct ll_raw_packet *raw_packet_rx;

static int16_t samples[(8 + 74) * 2];
static char iq_str_buffer[500];
char data[300];
const char *test_base64[100];
uint64_t start_time = 0;

rf_freq_offset_ctrl_t rf_fo_cmd = {
    .run = 0,
    .ch_id = 0,
    .tune = 0x1d,
    .offset_data = 0,
    .offset_data_index = 0
};

void exit(int v)
{
    DBG_PRINTF("Exit\n");
    while (1);
} 

extern int calc_freq_offset(const char *str, double *offset, double *snr);

int calc_freq(int argc, const char *argv[])
{
    if (argc < 2)
    {
        // DBG_PRINTF("usage: %s base64_data\n", argv[0]);
        // DBG_PRINTF("for example:\n");
        // DBG_PRINTF("%s %s\n", argv[0], "q//zABH/mv9vABL/7wB0AIT/4gAr/23/qQAz/8QArwBU/zz/mwDUAH3/If9lAPEAu/8E/zcA/wD2//v+6f8CAT0A//7D//wAXgAL/4H/3gCgAC//UP+7ANAAX/8w/5YA5AB5/xb/aQD+ALr/Af8bAAkB9f8A//L/BgEoAAr/r//0AGgAJf94/9MAnwBL/0L/qADMAHH/KP9+AOYAqf8J/z8A/QDj//3+DgACARUA+/7T//4AWwAK/4//6ACLAB//YP/JAMEASf8z/5sA5AB6/xX/aQD/ALv/Af8fAAcB7v/+/vb/BAEuAAv/r//2AGEAHf+D/9cAmwBJ/0b/tgDAAGD/Mv+UANkAkv8V/1oA9QDI/wH/HQABAQMA+v7l/wABRwAD/5n/6gCSACT/Yv/KAMAASf8+/6gA4QB0/xP/YQD+ALP/Bv85AC==");
        return 1;
    }

    double offset;
    double snr;
    int r = calc_freq_offset(argv[1], &offset, &snr);
    if (r == 0)
    {
        rf_fo_cmd.offset_data = (int8_t)(offset);
        rf_fo_cmd.offset_data_index++;
        // DBG_PRINTF("Freq Offset: %8.2fkHz\n", offset);
        // DBG_PRINTF("Freq Offset: %dkHz\n", rf_fo_cmd.offset_data);
        // if(rf_fo_cmd.offset_data_index == 4)
        // {
        //     freq_offset_rx_end();
        // }
    }
    else
        DBG_PRINTF("Errors in data\n");

    return r;
}

void freq_offset_rx_done(void)
{
    uint64_t air_time;
    uint8_t header;
    int len;
    int rssi;
    int count = 0;

    if (ll_raw_packet_get_rx_data(raw_packet_rx, &air_time, &header, data, &len, &rssi) != 0)
    {
        // DBG_PRINTF("+ERR: Rx fail\n");
        return;
    }

    if ((0 != ll_raw_packet_get_iq_samples(raw_packet_rx, samples, &count, 0)) || (count < 1))
    {
        // DBG_PRINTF("+ERR: No IQ\n");
        return;
    }
    
    // DBG_PRINTF("count:%d rssi:%d\n", count, rssi);
    
    base64_encode((const uint8_t *)samples,
        2 * count * sizeof(samples[0]),
        iq_str_buffer, sizeof(iq_str_buffer));

    test_base64[1] = iq_str_buffer;

    int status = calc_freq(count, test_base64);
    if(rf_fo_cmd.offset_data_index >= 3)
    {
        if((FREQ_OFFSET_THR_LOW <= rf_fo_cmd.offset_data) && (rf_fo_cmd.offset_data <= FREQ_OFFSET_THR_HIGH))
        {
            freq_offset_rx_end();
        }
        else if((rf_fo_cmd.offset_data < FREQ_OFFSET_THR_LOW))
        {
            uint8_t tune_tmp= rf_fo_cmd.offset_data/FREQ_OFFSET_TUNE_RADIO;
            if (rf_fo_cmd.tune <= tune_tmp) {
                rf_fo_cmd.tune -= tune_tmp; // Ensure tune value does not go below minimum
            }
            else {
                rf_fo_cmd.tune = 0; // Reset to zero if underflow occurs
            }
            freq_offset_tune();
            freq_offset_rx_begin_do();
        }
        else if((rf_fo_cmd.offset_data > FREQ_OFFSET_THR_HIGH))
        {
            if((rf_fo_cmd.tune + rf_fo_cmd.offset_data/FREQ_OFFSET_TUNE_RADIO) >= 0x3f)
            {
                rf_fo_cmd.tune = 0x3f; // Ensure tune value does not exceed maximum
            }
            else
            {
                rf_fo_cmd.tune += rf_fo_cmd.offset_data/FREQ_OFFSET_TUNE_RADIO; // Adjust tune value based on offset
            }
            freq_offset_tune();
            freq_offset_rx_begin_do();
        }
        else
        {
            DBG_PRINTF("Freq offset out of range: %d\n", rf_fo_cmd.offset_data);
            freq_offset_rx_end();
        }
        rf_fo_cmd.offset_data_index = 0;
    }
    else{
        freq_offset_rx_begin_do();
    }
}

void freq_offset_rx_begin_do(void)
{
    if (!rf_fo_cmd.run) {
        DBG_PRINTF("freq offset end test\n");
        return;
    }

    DBG_PRINTF("freq offset begin 1 time\n");
    static const uint8_t pattern[] = {0, 0};

    ll_raw_packet_set_param(raw_packet_rx,
                          0,    // tx_power
                          rf_fo_cmd.ch_id, // phy_channel_id
                          1,    // phy
                          FREQ_ACC_ADDRESS,
                          0x555555);
            
    ll_raw_packet_set_rx_cte(raw_packet_rx,
                            0, // uint8_t cte_type,
                            1, // uint8_t slot_len,
                            sizeof(pattern), // uint8_t switching_pattern_len,
                            pattern, 12, 1);

    int status = ll_raw_packet_recv(raw_packet_rx, platform_get_us_time() + RX_DELAY, SCAN_WINDOW);
    if (status != 0) {
        DBG_PRINTF("Error FREQ sending raw packet: %d\n", status);
    } else {
        DBG_PRINTF("FREQ Raw packet sent successfully.\n");
    }
}

void freq_offset_rx_begin(uint8_t ch_id)
{
    rf_fo_cmd.ch_id = ch_id;
    rf_fo_cmd.run = 1;
    btstack_push_user_msg(USER_MSG_FREQ_OFFSET_BEGIN_RX, NULL, 0);
}

void freq_offset_rx_end(void)
{
    rf_fo_cmd.run = 0;
    btstack_push_user_msg(USER_MSG_FREQ_OFFSET_END_RX, &rf_fo_cmd.offset_data, 1);
}

void freq_offset_tune(void)
{
    DBG_PRINTF("Freq Tune to 0x%x, offset:%d\n", rf_fo_cmd.tune, rf_fo_cmd.offset_data);
    btstack_push_user_msg(USER_MSG_FREQ_OFFSET_TUNE, &rf_fo_cmd.tune, 1);
}

void on_raw_packet_done_rx(struct ll_raw_packet *packet, void *user_data)
{
    DBG_PRINTF("on_raw_packet_done_rx\n");
    btstack_push_user_msg(USER_MSG_FREQ_OFFSET_DONE_RX, NULL, 0);
}

void freq_test_init(void){
    raw_packet_rx = ll_raw_packet_alloc(0, on_raw_packet_done_rx, NULL);
    CUSTOM_ASSERT(raw_packet_rx != NULL)
}