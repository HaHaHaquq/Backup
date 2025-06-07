#include "filter.h"
#include <stdio.h>

static uint32_t Pow_Smooth(float x,uint8_t y)
{
	uint32_t Result = 1;
	for(;y>0;y--)
	{
		Result = Result * x;
	}
	return Result;
}

int8_t Smooth_Average(Filter_CTL * ema, int8_t RSSI)
{
    if(ema->ema_cnt == 0)ema->ema_rssi = RSSI;
	else
		{
			ema->ema_rssi = ema->beta * ema->ema_rssi + (1 - ema->beta) * RSSI;
			ema->corr_rssi = ema->ema_rssi/(1 - Pow_Smooth(ema->beta,ema->ema_cnt));
		}
	ema->ema_cnt++;
	if(ema->ema_cnt >EMA_MAX_COUNT)
	{
		ema->ema_cnt = 1;
		ema->ema_rssi = 0;
//		ema->ema_cnt = 0;
		return ema->corr_rssi;
	}
	return 0;
}





























