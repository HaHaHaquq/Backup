#include <stdint.h>
#include <stdio.h>
#include "platform_api.h"
#include "ll_api.h"
#include "btstack_event.h"
#include "profile.h"
#include "str_util.h"

#include "freq_offset.h"

struct ll_raw_packet *raw_packet_tx;


rf_freq_offset_ctrl_t rf_fo_cmd = {
    .ch_id = 1,
    .run = 0,
    .tune = 0x1d, // Default tune value
};

void freq_offset_tx_begin_do(void)
{
    if (!rf_fo_cmd.run) {
        DBG_PRINTF("Frequency offset test finished.\n");
        return;
    }
    // DBG_PRINTF("Freq offset 1 time\n");

    platform_config(PLATFORM_CFG_24M_OSC_TUNE, rf_fo_cmd.tune);

    ll_raw_packet_set_param(raw_packet_tx,
                          3,        // tx_power
                          rf_fo_cmd.ch_id,    // phy_channel_id
                          1,        // phy
                          FREQ_ACC_ADDRESS,
                          0x555555);
    ll_raw_packet_set_tx_cte(raw_packet_tx, 0, 20, 0, NULL);
    ll_raw_packet_set_tx_data(raw_packet_tx, 0, NULL, 0);
    int status = ll_raw_packet_send(raw_packet_tx, platform_get_us_time() + 30000);
    if (status != 0) {
        DBG_PRINTF("Error FREQ sending raw packet: %d\n", status);
    } else {
        DBG_PRINTF("FREQ Raw packet sent successfully.\n");
    }
}

void freq_offset_tx_begin(uint8_t ch_id)
{
    rf_fo_cmd.ch_id = ch_id;
    rf_fo_cmd.run = 1;
    btstack_push_user_msg(USER_MSG_RAW_PACKET_BEGIN_TX, NULL, 0);
}

void freq_offset_tune(uint8_t tune)
{
    rf_fo_cmd.tune = tune;
    DBG_PRINTF("Freq tune to:%d...\n", rf_fo_cmd.tune);
}

void freq_offset_tx_end(void)
{
    rf_fo_cmd.run = 0;
}

void on_raw_packet_done_tx(struct ll_raw_packet *packet, void *user_data)
{
    btstack_push_user_msg(USER_MSG_RAW_PACKET_DONE_TX, NULL, 0);
}

void freq_test_init(void){
    raw_packet_tx = ll_raw_packet_alloc(1, on_raw_packet_done_tx, NULL);
}
