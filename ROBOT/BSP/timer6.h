#ifndef __HEARTBEAT_H__
#define __HEARTBEAT_H__
#include "main.h"

void TIM6_Configuration(void);
void TIM6_Start(void);

extern int speed;


extern u32 time_1ms_count;

#endif
