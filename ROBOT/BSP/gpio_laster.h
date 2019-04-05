#ifndef __GPIO_LASTER_H__
#define __GPIO_LASTER_H__
#include "sys.h"


void Laser_Init(void);

#define LASER_SWITCH PGout(13)

#define LASER_ON()  GPIO_SetBits(GPIOG, GPIO_Pin_13)
#define LASER_OFF()  GPIO_ResetBits(GPIOG, GPIO_Pin_13)

#endif

