#ifndef __PWM_H__
#define __PWM_H__

#include "main.h"

#define PWM4_1  TIM4->CCR1    //
#define PWM4_2  TIM4->CCR2    //
#define PWM4_3  TIM4->CCR3    //
#define PWM4_4  TIM4->CCR4    //

#define PWM5_1 TIM5->CCR1
#define PWM5_2 TIM5->CCR2
#define PWM5_3 TIM5->CCR3
#define PWM5_4 TIM5->CCR4

#define SetFrictionWheelSpeed(x) \
        PWM5_1 = x;                \
        PWM5_2 = x;
				
				
#define FRICTION_INIT      1000

#define PWM_IO_ON	20000
#define PWM_IO_OFF	0

void PWM4_Config(void);
void PWM5_Config(void);
void PWM4_ZGX_Init(void);

#endif


