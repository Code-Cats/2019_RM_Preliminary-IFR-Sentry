#include "pwm.h"

/***********************************
函数名称：PWM_Config
函数功能：配置定时器5的通道1234
函数参数： 无
函数返回值： 无
描述：
************************************/
void PWM5_Config(void)
{
	GPIO_InitTypeDef          gpio;
	TIM_TimeBaseInitTypeDef   tim;
	TIM_OCInitTypeDef         oc;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH ,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI ,ENABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);   

	gpio.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 ;
	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOH,&gpio);

	gpio.GPIO_Pin = GPIO_Pin_0;
	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOI,&gpio);//定时器5通道3

	GPIO_PinAFConfig(GPIOH,GPIO_PinSource10,GPIO_AF_TIM5);//定时器5 通道3
	GPIO_PinAFConfig(GPIOH,GPIO_PinSource11,GPIO_AF_TIM5);//定时器5 通道3
	GPIO_PinAFConfig(GPIOH,GPIO_PinSource12,GPIO_AF_TIM5);//定时器5 通道3
	GPIO_PinAFConfig(GPIOI,GPIO_PinSource0,GPIO_AF_TIM5);//定时器5 通道4

	/* TIM5 */
	tim.TIM_Prescaler = 840-1;
	tim.TIM_CounterMode = TIM_CounterMode_Up;
	tim.TIM_Period = 1000-1;   //10ms
	tim.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM5,&tim);

	oc.TIM_OCMode = TIM_OCMode_PWM2;
	oc.TIM_OutputState = TIM_OutputState_Enable;
	oc.TIM_OutputNState = TIM_OutputState_Disable;
	oc.TIM_Pulse = 1000;
	oc.TIM_OCPolarity = TIM_OCPolarity_Low;
	oc.TIM_OCNPolarity = TIM_OCPolarity_High;
	oc.TIM_OCIdleState = TIM_OCIdleState_Reset;
	oc.TIM_OCNIdleState = TIM_OCIdleState_Set;

	TIM_OC1Init(TIM5,&oc);//定时器5 通道1
	TIM_OC2Init(TIM5,&oc);//定时器5 通道2
	TIM_OC3Init(TIM5,&oc);//定时器5 通道3
	TIM_OC4Init(TIM5,&oc);//定时器5 通道4

	TIM_OC1PreloadConfig(TIM5,TIM_OCPreload_Enable);
	TIM_OC2PreloadConfig(TIM5,TIM_OCPreload_Enable);
	TIM_OC3PreloadConfig(TIM5,TIM_OCPreload_Enable);
	TIM_OC4PreloadConfig(TIM5,TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM5,ENABLE);

	TIM_Cmd(TIM5,ENABLE);

	PWM5_1=100;
	PWM5_2=100;
	PWM5_3=100;
	PWM5_4=100;
}


/***********************************
函数名称：PWM_Config
函数功能：配置定时器4的通道1234
函数参数： 无
函数返回值： 无
描述：
************************************/
void PWM4_Config(void)
{
    GPIO_InitTypeDef          gpio;
    TIM_TimeBaseInitTypeDef   tim;
    TIM_OCInitTypeDef         oc;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD ,ENABLE);	//PD12-15 -> CH123
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);   

	
    gpio.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOD,&gpio);
		
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource12,GPIO_AF_TIM4);//定时器4 通道1
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource13,GPIO_AF_TIM4);//定时器4 通道2
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource14,GPIO_AF_TIM4);//定时器4 通道3
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource15,GPIO_AF_TIM4);//定时器4 通道4
		
	/* TIM4 */
	tim.TIM_Prescaler = 84-1;
    tim.TIM_CounterMode = TIM_CounterMode_Up;
    tim.TIM_Period = 7700-1;   //7ms  8333-120hz
    tim.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM4,&tim);
		
    oc.TIM_OCMode = TIM_OCMode_PWM2;
    oc.TIM_OutputState = TIM_OutputState_Enable;
    oc.TIM_OutputNState = TIM_OutputState_Disable;
    oc.TIM_Pulse = 1000;
    oc.TIM_OCPolarity = TIM_OCPolarity_Low;
    oc.TIM_OCNPolarity = TIM_OCPolarity_High;
    oc.TIM_OCIdleState = TIM_OCIdleState_Reset;
    oc.TIM_OCNIdleState = TIM_OCIdleState_Set;

	TIM_OC1Init(TIM4,&oc);//定时器4 通道1
	TIM_OC2Init(TIM4,&oc);//定时器4 通道2
	TIM_OC3Init(TIM4,&oc);//定时器4 通道3
	TIM_OC4Init(TIM4,&oc);//定时器4 通道4
		
	TIM_OC1PreloadConfig(TIM4,TIM_OCPreload_Enable);
	TIM_OC2PreloadConfig(TIM4,TIM_OCPreload_Enable);
	TIM_OC3PreloadConfig(TIM4,TIM_OCPreload_Enable);
	TIM_OC4PreloadConfig(TIM4,TIM_OCPreload_Enable);
           
    TIM_ARRPreloadConfig(TIM4,ENABLE);  
		
	TIM_Cmd(TIM4,ENABLE);
		
	PWM4_1=1000;
	PWM4_2=1000;
	PWM4_3=1000;
	PWM4_4=1000;
}


void PWM4_ZGX_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;	
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
		GPIO_InitTypeDef gpio;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
	gpio.GPIO_Pin = GPIO_Pin_0;           
	gpio.GPIO_Mode = GPIO_Mode_AF;        
	gpio.GPIO_Speed = GPIO_Speed_100MHz;	
	gpio.GPIO_OType = GPIO_OType_PP;     
	gpio.GPIO_PuPd = GPIO_PuPd_UP;   
	
	GPIO_Init(GPIOI,&gpio);
	
	gpio.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_11 | GPIO_Pin_10;
	GPIO_Init(GPIOH,&gpio);
	
	gpio.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOD,&gpio);												//PWMH
		
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource0 , GPIO_AF_TIM5);			//PWMA
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource12, GPIO_AF_TIM5);			//PWMB
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource11, GPIO_AF_TIM5);			//PWMC
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource10, GPIO_AF_TIM5);			//PWMD
	
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource12 , GPIO_AF_TIM4);			//PWMH
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 20000 - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = 84 - 1;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	
	TIM_TimeBaseInit(TIM5,&TIM_TimeBaseStructure);
	
	TIM_TimeBaseStructure.TIM_Prescaler = 840 - 1;
	TIM_TimeBaseStructure.TIM_Period = 1000 - 1;
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);				//PWMH
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 1000;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	
	TIM_OC4Init(TIM5, &TIM_OCInitStructure);    				//PWMA
	TIM_OC3Init(TIM5, &TIM_OCInitStructure);					//PWMB
	TIM_OC2Init(TIM5, &TIM_OCInitStructure);					//PWMC
	TIM_OC1Init(TIM5, &TIM_OCInitStructure);					//PWMD
	
	TIM_OC1PreloadConfig(TIM5, ENABLE);
	TIM_OC2PreloadConfig(TIM5, ENABLE);
	TIM_OC3PreloadConfig(TIM5, ENABLE);
	TIM_OC4PreloadConfig(TIM5, ENABLE);
	
	TIM_ARRPreloadConfig(TIM5, ENABLE);
	TIM_CtrlPWMOutputs(TIM5, ENABLE);
	
	TIM_Cmd(TIM5, ENABLE);
	
	TIM_OCInitStructure.TIM_Pulse = 500;						//PWMH
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);
	
	TIM_OC1PreloadConfig(TIM4, ENABLE);
	
	TIM_ARRPreloadConfig(TIM4, ENABLE);
	TIM_CtrlPWMOutputs(TIM4, ENABLE);
	
	TIM_Cmd(TIM4, ENABLE);
}