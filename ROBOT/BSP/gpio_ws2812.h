#ifndef __GPIO_WS2812_H
#define __GPIO_WS2812_H
#include "sys.h"
#include <stdbool.h>

#define WS2812_GREEN 0
#define WS2812_RED 1
#define WS2812_BLUE 2

void WS2812_Init(void);
void WS2812_TIM5_3_DMA_Init(void);
bool PWM5_3_DMA_Enable(void);
bool WS2812_UpdateColor(u8 colors[][3],u16 led_nums);	//GRB 高位先发
#endif
