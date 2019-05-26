#include "bsp.h"
/******************************************
��������;BSP_Init
�������ܣ��԰弶�豸��ʼ������
������������
��������ֵ����
������������
******************************************/
u8 Flag_bspOK=0;
void BSP_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
	delay_init(168);    //��ʼ����ʱ����
	
	/********GPIO*******/
	LED_Init();
	Laser_Init();
	Power_Ctrl_Init();	//��Դ����
	delay_ms(100);
	//WS2812_Init();
	//WS2812_TIM5_3_DMA_Init();
	
	power_ctrl_on(POWER1_CTRL_SWITCH);
	power_ctrl_on(POWER2_CTRL_SWITCH);
	power_ctrl_on(POWER3_CTRL_SWITCH);
	power_ctrl_on(POWER4_CTRL_SWITCH);
	
	/********CAN********/
	CAN1_Motors_Config();
	CAN2_Motors_Config();
	RM_ADI_IMU_Cali();
	
	/************PWM************/
	Pwm_Buzzer_Init(10-1,pre_calc(0,0));
	PWM4_Config();
	//PWM4_ZGX_Init();
	PWM5_Config();
	PWM_Heat_Init();	//�����Ǽ��ȵ���
	//delay_ms(2000);
	
	/************USART***********/
	USART1_DMA_Config(100000);
	
#if (WIFI_DEBUG_ENABLE == 1)
	USART2_wifidebug_Init(1000000);
	USART2_DMA_Tx_Init();
#endif
	USART3_Configuration(115200);
	
	//USART6_ViceBoard_Init(115200);
	USART8_ViceBoard_Init(256000);
	
	UART7_Vision_Init(115200);
	//delay_ms(10000);
	/*******MPU********/
	IMU_SPI5_Init();
	MPU_Device_Init();	
	
	
	/********TIM********/
	
	TIM6_Configuration();	////��ʱ����Ӧ���ڶ�ʱ���У��Ա���ң������������	//����BSP��־λ
	delay_ms(1000);
//	delay_ms(1600+0);
	TIM6_Start();	
Flag_bspOK=1;	
}

