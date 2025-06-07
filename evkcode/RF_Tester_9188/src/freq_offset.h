#ifndef _FREQ_OFFSET_H_
#define _FREQ_OFFSET_H_

#include <stdint.h>


#define RX_DELAY                600
#define SCAN_WINDOW         (1000 * 1000)

#define FREQ_OFFSET_THR_LOW        -10
#define FREQ_OFFSET_THR_HIGH        10
#define FREQ_OFFSET_THR_STEP        2
#define FREQ_OFFSET_TUNE_RADIO      2

#define FREQ_ACC_ADDRESS    0xBED8A379

typedef struct 
{
    uint8_t run;
    uint8_t ch_id;
    uint8_t tune;
    int8_t offset_data;
    uint8_t offset_data_index;
} rf_freq_offset_ctrl_t;

void freq_test_init(void);
void freq_offset_rx_begin(uint8_t ch_id);
void freq_offset_rx_end(void);
void freq_offset_rx_done(void);
void freq_offset_rx_begin_do(void);
void freq_offset_tune(void);

#endif


