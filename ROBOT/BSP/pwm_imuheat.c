#include "pwm_imuheat.h"

void PWM_Heat_Init(void)
{
	GPIO_InitTypeDef gpio;
	TIM_TimeBaseInitTypeDef tim_base;
	TIM_OCInitTypeDef  tim_oc;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	gpio.GPIO_Pin = GPIO_Pin_5;           
	gpio.GPIO_Mode = GPIO_Mode_AF;        
	gpio.GPIO_Speed = GPIO_Speed_100MHz;	
	gpio.GPIO_OType = GPIO_OType_PP;     
	gpio.GPIO_PuPd = GPIO_PuPd_UP;   
	
	GPIO_Init(GPIOB,&gpio);
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5 , GPIO_AF_TIM3);	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	tim_base.TIM_ClockDivision = TIM_CKD_DIV1;
	tim_base.TIM_CounterMode = TIM_CounterMode_Up;
	tim_base.TIM_Period = 20000 - 1;
	tim_base.TIM_Prescaler = 84 - 1;
	tim_base.TIM_RepetitionCounter = 0;
	
	TIM_TimeBaseInit(TIM3,&tim_base);
	
	tim_oc.TIM_OCMode = TIM_OCMode_PWM1;
	tim_oc.TIM_OutputState = TIM_OutputState_Enable;
	tim_oc.TIM_Pulse = 0;
	tim_oc.TIM_OCPolarity = TIM_OCPolarity_High;
	
	TIM_OC2Init(TIM3, &tim_oc);
	
	TIM_OC2PreloadConfig(TIM3, ENABLE);
	
	TIM_ARRPreloadConfig(TIM3, ENABLE);
	TIM_CtrlPWMOutputs(TIM3, ENABLE);

}

void imu_Heat_Config(u16 dc)
{
	u32 tmp = 20000*dc/20000;
	
	TIM3->CCR2 = tmp;
}
