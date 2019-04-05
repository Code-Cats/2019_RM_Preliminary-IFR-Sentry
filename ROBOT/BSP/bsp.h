#ifndef __BSP_H__
#define __BSP_H__

//#include "common_definition.h"
#include "sys.h"
#include "MyDefine.h"
#include "delay.h"
#include "pid.h"

	/********GPIO*******/
#include "gpio_laster.h"
#include "gpio_led.h"
#include "gpio_key.h"
#include "gpio_power.h"
	
	/********CAN********/
#include "can1.h"
#include "can2.h"
#include "can1_analysis.h"
#include "can2_analysis.h"
	
	/************PWM************/
#include "pwm.h"
#include "pwm_buzzer.h"
#include "pwm_imuheat.h"
	
	/************USART***********/
#include "usart1_remote.h"
#include "usart1_remote_analysis.h"
#include "usart3_judge.h"
#include "usart6_viceboard.h"
#include "uart7_vision.h"
	
	/*******MPU********/
#include "My_IIC.h"
#include "SPI5_imu.h"
#include "imu_analysis.h"
	
	
	/********TIM********/
#include "timer6.h"




//#include "main.h"

void BSP_Init(void);



#endif 

