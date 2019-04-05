#ifndef __PWM_BUZZER_H__
#define __PWM_BUZZER_H__

#include "sys.h"

extern void Pwm_Buzzer_Init(uint16_t psc,uint16_t arr);
extern void buzzer_on(uint16_t pwm);
extern void buzzer_off(void);

u16 pre_calc(u8 scale,u8 drm);
void Buzzer_SetFre(uint16_t psc, uint16_t arr);

extern const u16 tonality_fre[3][8];

#endif
