#ifndef __LED_CONTROL_H__
#define __LED_CONTROL_H__

#include "sys.h"


#define GREEN_LED_ON()      GPIO_ResetBits(GPIOF, GPIO_Pin_14)
#define GREEN_LED_OFF()     GPIO_SetBits(GPIOF, GPIO_Pin_14)
#define GREEN_LED_TOGGLE()      GPIO_ToggleBits(GPIOF, GPIO_Pin_14)

#define RED_LED_ON()            GPIO_ResetBits(GPIOE, GPIO_Pin_11)
#define RED_LED_OFF()           GPIO_SetBits(GPIOE, GPIO_Pin_11)
#define RED_LED_TOGGLE()        GPIO_ToggleBits(GPIOE, GPIO_Pin_11)

#define DELAY_CNT  200000    //闪烁时间间隔

#define BLINK_CYCLE 150
#define BLINK_INTERVAL (BLINK_CYCLE*2*8)
void LED_Blink_Run(void);
void LED_Blink_Set(u8 frequency_green,u8 frequency_red);	//2s内闪烁次数//LED闪烁运行函数10h处理频率



#endif
