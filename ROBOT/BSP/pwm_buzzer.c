#include "pwm_buzzer.h"

void Pwm_Buzzer_Init(uint16_t psc,uint16_t arr)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);

    RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM12, ENABLE);
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM12, DISABLE);

    GPIO_PinAFConfig(GPIOH, GPIO_PinSource6, GPIO_AF_TIM12);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOH, &GPIO_InitStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM12, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM12, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM12, ENABLE);

    TIM_TimeBaseInitStructure.TIM_Period = arr - 1;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc - 1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;

    TIM_TimeBaseInit(TIM12, &TIM_TimeBaseInitStructure);
    TIM_Cmd(TIM12, ENABLE);

    buzzer_off();
//TIM_SetCompare1(TIM12, 1000);
}

const u16 tonality_fre[3][8]= \
{\
	{10,262,294,330,349,392,440,494},\
	{10,523,587,659,698,784,880,988},\
	{10,1046,1175,1318,1397,1568,1760,1967}\
};\

u16 pre_calc(u8 scale,u8 drm)
{
	u16 result=16061;
//	float result_f=16061;
	result=(u16)(8400000/tonality_fre[scale][drm]);
	return result;
}

void Buzzer_SetFre(uint16_t psc, uint16_t arr)
{
	TIM12->PSC = psc;
	TIM12->ARR = arr;
}

void buzzer_on(uint16_t pwm)
{
    TIM_SetCompare1(TIM12, pwm);
}


void buzzer_off(void)
{
    TIM_SetCompare1(TIM12, 0);
}

