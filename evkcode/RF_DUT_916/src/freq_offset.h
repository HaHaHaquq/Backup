#ifndef _FREQ_OFFSET_H_
#define _FREQ_OFFSET_H_

#include <stdint.h>

#define RX_DELAY                600
#define SCAN_WINDOW         (1000 * 1000)

#define FREQ_ACC_ADDRESS    0xBED8A379

typedef struct 
{
    uint8_t run;
    uint8_t ch_id;
    uint8_t tune;
} rf_freq_offset_ctrl_t;


void freq_test_init(void);
void freq_offset_tx_begin(uint8_t ch_id);
void freq_offset_tx_end(void);
void freq_offset_tx_begin_do(void);
void freq_offset_tune(uint8_t tune);

#endif


