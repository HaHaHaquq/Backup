#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "raw_pkt.h"
#include "platform_api.h"
#include "btstack_event.h"
#include "filter.h"
#include "profile.h"



Filter_CTL ema_filter = {
    .ema_rssi = 0,
    .corr_rssi = 0,
    .ema_cnt = 0,
    .beta = EMA_BETA
};
uint8_t addr_buf[4] = {0};
struct ll_raw_packet *rx_raw_packet = NULL;
struct ll_raw_packet *tx_raw_packet = NULL;
static raw_cnt_ctrl_t raw_cnt_ctrl = {
    .ch_id = 10,
    .for_tx = 0, // 1 for tx, 0 for rx
    .run = 0,
    .rx_delay = 600, // convert ms to seconds
    .tx_speed = 5000, // us
    .rx_speed = 1000 * 1000, // us
    .count = 0,
    .rssi = 0,
    .data = {0},
    .acc_addr = 0x8E89BED6
};

const uint8_t check_buf[] = "RAW PACKET #";
static void show_rx(struct ll_raw_packet *packet)
{
    uint64_t air_time;
    uint8_t header;
    int len;
    int rssi;

    if (ll_raw_packet_get_rx_data(packet, &air_time, &header, raw_cnt_ctrl.data, &len, &rssi) == 0)
    {
        if(!memcmp((const uint8_t *)raw_cnt_ctrl.data, (const uint8_t *)check_buf, sizeof(check_buf)-1))
        {
            raw_cnt_ctrl.count++;
            Smooth_Average(&ema_filter, rssi);
            // DBG_PRINTF("RSSI is %d\n", ema_filter.corr_rssi);
        }
    }
    else
    {
        DBG_PRINTF("Rx error\n");
    }
}

static void raw_pkt_reset(void);
void raw_pkt_tx_test(void)
{
    static int counter = 0;
//  DBG_PRINTF("Tx done\n");
    if(counter >= PACKET_CNT_MAX)
    {
        counter = 0;
        raw_cnt_ctrl.run = 0;
        btstack_push_user_msg(USER_END_TEST, NULL, 0);
    }
    // DBG_PRINTF("[RAW] Tx packet #%d, run:%d\n", counter, raw_cnt_ctrl.run);
    sprintf(raw_cnt_ctrl.data, "RAW PACKET #%d", counter++);
    ll_raw_packet_set_tx_data(tx_raw_packet, counter & 0x7f, raw_cnt_ctrl.data, strlen(raw_cnt_ctrl.data));
    if(raw_cnt_ctrl.run)
    {
        ll_raw_packet_send(tx_raw_packet, platform_get_us_time() + raw_cnt_ctrl.tx_speed);
    }
    else
    {
        raw_pkt_reset();
    } 
}

void raw_pkt_rx_test(void)
{
    if(raw_cnt_ctrl.run)
    {
        show_rx(rx_raw_packet);
        ll_raw_packet_recv(rx_raw_packet, platform_get_us_time() + raw_cnt_ctrl.rx_delay, raw_cnt_ctrl.rx_speed);//The time when the window opens and the duration of the window
    }
    else 
    {
        raw_pkt_reset();
    }
}

void on_raw_packet_done(struct ll_raw_packet *packet, void *user_data)
{
    uint32_t temp = 0;
    if(raw_cnt_ctrl.for_tx == 1)
    {
        temp = USER_Tx_RAW_PKT;
    }
    else
    {
        temp = USER_Rx_RAW_PKT;
    }
    btstack_push_user_msg(temp, NULL, 0);
}

void start_raw_tx_rx(void)
{
    raw_cnt_ctrl.run = 1;
    DBG_PRINTF("Raw Tx/Rx started\n");
    on_raw_packet_done(NULL, NULL);
}

void stop_raw_tx_rx(void)
{
    raw_cnt_ctrl.run = 0;
    DBG_PRINTF("Raw Tx/Rx stopped\n");
}

// void set_raw_pkt_channel(uint8_t *buf)
// {
//     btstack_push_user_msg(USER_SET_CHANNEL, buf, 2);
// }

void restart_raw_pkt_rx(void)
{
    btstack_push_user_msg(USER_SET_RX_PKT, NULL, 0);
}

void restart_raw_pkt_tx(void)
{
    btstack_push_user_msg(USER_SET_TX_PKT, NULL, 0);
}

void set_raw_pkt_type(uint8_t type)
{
    raw_cnt_ctrl.for_tx = type; // 1 for tx, 0 for rx
}


uint16_t get_raw_pkt_count(void)
{
    uint16_t cnt = raw_cnt_ctrl.count;
    raw_cnt_ctrl.count = 0; // Reset count after reading
    return cnt;
}

int8_t get_raw_pkt_loss(void)
{
    uint8_t loss_pkt = (PACKET_CNT_MAX - raw_cnt_ctrl.count);
    DBG_PRINTF("Packet loss: %d\n", loss_pkt);
    if(loss_pkt > PCK_THRESHOLDS)
        return  -1;
    else
        return loss_pkt;
}

int get_raw_pkt_power(void)
{
    if(ema_filter.corr_rssi < POWER_THRESHOLDS)
        return 0;
    return ema_filter.corr_rssi;
}

static void raw_pkt_reset(void)
{
    raw_cnt_ctrl.count = 0;
    raw_cnt_ctrl.rssi = 0;
    memset(raw_cnt_ctrl.data, 0, sizeof(raw_cnt_ctrl.data));
}

//This function can only be applied when the channel is between 2402 and 2480.
uint8_t set_rf_channel(uint8_t *ch)
{
    uint16_t default_ch = 2402; // Default channel ID
    uint16_t ch_id  = (uint16_t)(ch[0] | (ch[1] << 8));
    if(ch_id % 2 !=0 || ch_id < default_ch || ch_id > 2480)
    {
        DBG_PRINTF("Invalid channel ID: %d\n", ch_id);
        return -1;
    }
    raw_cnt_ctrl.ch_id = (uint8_t)(ch_id - default_ch)/2;
    return raw_cnt_ctrl.ch_id;
}

uint32_t set_rf_acc_addr(uint8_t *buffer)
{
    uint32_t temp_addr = (uint32_t)buffer[0] | (uint32_t)buffer[1] <<8 |  \
                         (uint32_t)buffer[2] << 16 | (uint32_t)buffer[3] << 24;
    raw_cnt_ctrl.acc_addr = temp_addr;
    return raw_cnt_ctrl.acc_addr;
}

uint32_t get_rf_acc_addr(void)
{
    platform_hrng(addr_buf, 4);
    uint32_t temp_addr = 0;
    memcpy(&temp_addr, addr_buf, sizeof(uint32_t));
    raw_cnt_ctrl.acc_addr = temp_addr;
    return temp_addr;
}

void raw_packet_set_rx_param(void)
{
    raw_cnt_ctrl.for_tx = 0;
    DBG_PRINTF("CHID1 IS : %d\n", raw_cnt_ctrl.ch_id);
    DBG_PRINTF("ADDR1 IS : %x\n", raw_cnt_ctrl.acc_addr);
    ll_raw_packet_set_param(rx_raw_packet,
                        3,        // tx_power
                        raw_cnt_ctrl.ch_id,    // phy_channel_id
                        1,        // phy
                        raw_cnt_ctrl.acc_addr,//0xabcdef,
                        0x555555);
}

void raw_packet_set_tx_param(void)
{
    raw_cnt_ctrl.for_tx = 1;
    DBG_PRINTF("CHID2 IS : %d\n", raw_cnt_ctrl.ch_id);
    DBG_PRINTF("ADDR2 IS : %x\n", raw_cnt_ctrl.acc_addr);
    ll_raw_packet_set_param(tx_raw_packet,
                        3,        // tx_power
                        raw_cnt_ctrl.ch_id,    // phy_channel_id
                        1,        // phy
                        raw_cnt_ctrl.acc_addr,//0xabcdef,
                        0x555555);
}

void raw_pkt_malloc_init(void)
{
    rx_raw_packet = ll_raw_packet_alloc(0, on_raw_packet_done, NULL);//When the sending is completed or the window is opened, the callback will be automatically entered.
    tx_raw_packet = ll_raw_packet_alloc(1, on_raw_packet_done, NULL);//When the sending is completed or the window is opened, the callback will be automatically entered.
    CUSTOM_ASSERT(rx_raw_packet != NULL);
    CUSTOM_ASSERT(tx_raw_packet != NULL);
}
