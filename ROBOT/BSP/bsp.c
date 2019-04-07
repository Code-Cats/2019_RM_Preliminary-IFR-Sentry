#include "bsp.h"
/******************************************
函数名称;BSP_Init
函数功能：对板级设备初始化配置
函数参数：无
函数返回值：无
函数描述：无
******************************************/
u8 Flag_bspOK=0;
void BSP_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
	delay_init(168);    //初始化延时函数
	
	/********GPIO*******/
	LED_Init();
	Laser_Init();
	Power_Ctrl_Init();	//电源管理
	delay_ms(100);
	
	power_ctrl_on(POWER1_CTRL_SWITCH);
	power_ctrl_on(POWER2_CTRL_SWITCH);
	power_ctrl_on(POWER3_CTRL_SWITCH);
	power_ctrl_on(POWER4_CTRL_SWITCH);
	
	/********CAN********/
	CAN1_Motors_Config();
	CAN2_Motors_Config();
	delay_ms(2000);
	
	/************PWM************/
	Pwm_Buzzer_Init(10-1,pre_calc(0,0));
	PWM_Config();
	PWM_Heat_Init();	//陀螺仪加热电阻
	
	/************USART***********/
	USART1_DMA_Config(100000);
	
	USART2_wifidebug_Init(1000000);
	USART2_DMA_Tx_Init();
	
	USART3_Configuration(115200);
	
	UART7_Vision_Init(576000);
	
	/*******MPU********/
	IMU_SPI5_Init();
	MPU_Device_Init();	
	
	
	/********TIM********/
	
	TIM6_Configuration();	////延时处理应当在定时器中，以便检测遥控器和陀螺仪	//设置BSP标志位
	delay_ms(1000);
//	delay_ms(1600+0);
	TIM6_Start();	
Flag_bspOK=1;	
}

