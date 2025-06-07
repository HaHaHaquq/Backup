#ifndef __RAW_PKT_H
#define __RAW_PKT_H

#include <stdint.h>


#define PACKET_CNT_MAX  100
#define PCK_THRESHOLDS  30
#define POWER_THRESHOLDS -70

typedef struct
{
    uint8_t ch_id;          // channel id
    uint8_t for_tx;         // for tx or rx
    char data[300];         // data buffer
    uint8_t run;            // run flag
    uint16_t rx_delay;      // length of data
    uint16_t count;         // count of packets
    int rssi;               // RSSI value
    uint32_t tx_speed;      // speed of tx
    uint32_t rx_speed;      // speed of rx
    uint32_t acc_addr;
}raw_cnt_ctrl_t;

void start_raw_tx_rx(void);
void stop_raw_tx_rx(void);
// void set_raw_pkt_channel(uint8_t *buf);

void raw_packet_set_rx_param(void);
void raw_packet_set_tx_param(void);

void packet_action(void);

uint16_t get_raw_pkt_count(void);
int8_t get_raw_pkt_loss(void);

void restart_raw_pkt_rx(void);
void restart_raw_pkt_tx(void);


int get_raw_pkt_power(void);
uint32_t get_rf_acc_addr(void);
uint8_t set_rf_channel(uint8_t *ch);
uint32_t set_rf_acc_addr(uint8_t *buffer);


void raw_pkt_malloc_init(void);
void raw_pkt_tx_test(void);
void raw_pkt_rx_test(void);
#endif
