#include "main.h"
#include "bsp.h"
#include "led_control.h"
#include "usart2_wifidebug.h"
#include "usart2_wifidebug_analysis.h"
#include "wifi_debug.h"
#include "brain.h"
#include "buzzer.h"

////////
volatile u16 testu16=4;
volatile float testf=5.2f;
volatile u32 iii=0;
////////

extern u8 led_start2normal;

s8 led_num=0;


//encoder  108  331-> 15  60
int main(void)//40-508-1289-1705
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2

	BSP_Init();
	
	for(int i=0;i<22;i++)	//40//86
	{
		if(led_num<8)
		led_num+=3;
		
		buzzer_on(TIM12->ARR/2);
		Buzzer_SetFre(10-1,pre_calc(Star_Sky[i][0],Star_Sky[i][1]));
		delay_ms(Star_Sky[i][2]/2);
		buzzer_off();
		delay_ms(30);
		if((i+1)%4==1)
		{
		//	delay_ms(80);   
		}
	}

#if (WIFI_DEBUG_ENABLE == 1)
	USART_Cmd(USART2, ENABLE);                 //使能串口 
	USART2_DMA_Send("+++",3);
	delay_ms(600);
#endif 
//	while(ESP8266_OSPF_Config(0)==false)
//	{
//		delay_ms(22);
//	}
	
	GREEN=0;
	
	LED_Blink_Set(1,8);
	
	led_start2normal=1;
	while(1)
	{
//		Green_LED_8_SetNum(3);
		
		if(time_1ms_count%1000==0)
		{
			led_num=8;
		}
		
#if (WIFI_DEBUG_ENABLE == 1)
		ESP8266_ConfigCheck();
		Wifi_Debug_Main();

		WFDBG_DataSampling();
#endif		
	}
	
}


float ABS(float x)
{
	x=x<0?-x:x;
	return x;
}

