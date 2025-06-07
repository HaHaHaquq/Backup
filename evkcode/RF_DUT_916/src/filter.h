#ifndef __FILTER_H
#define __FILTER_H
#include <stdint.h>

#define EMA_BETA	(0.8f)
#define EMA_THRESHOLD (-30)
#define EMA_MAX_COUNT	(1000)//29

typedef struct {
    int8_t ema_rssi;
    int8_t corr_rssi;
    uint16_t ema_cnt;
    float beta; //Adjust as needed, generally between 0.8-0.99
} Filter_CTL;

int8_t Smooth_Average(Filter_CTL * ema, int8_t RSSI);

#endif