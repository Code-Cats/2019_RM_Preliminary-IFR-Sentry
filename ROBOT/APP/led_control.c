#include "led_control.h"
#include "gpio_ws2812.h"

extern u32 time_1ms_count;

//#define BLINK_CYCLE 150
//#define BLINK_INTERVAL 2500

u8 Frequency_Green,Frequency_Red=0;
void LED_Blink_Run(void)	//LED闪烁运行函数10h处理频率
{
	if(Frequency_Green<10&&Frequency_Green>0)
	{
		if(time_1ms_count%BLINK_INTERVAL<=2*Frequency_Green*BLINK_CYCLE)
		{
			if(time_1ms_count%BLINK_INTERVAL==0)
			{
				GREEN_LED_OFF();	//每次循环开始将灯熄灭
			}
			else if(time_1ms_count%BLINK_CYCLE==0)
			{
				GREEN_LED_TOGGLE();
			}
		}
	}
	else if(Frequency_Green>=10)	//常亮
	{
		GREEN_LED_ON();
	}
	else
	{
		GREEN_LED_OFF();
	}
	
	if(Frequency_Red<10&&Frequency_Red>0)
	{
		if(time_1ms_count%BLINK_INTERVAL<=2*Frequency_Red*BLINK_CYCLE)
		{
			if(time_1ms_count%BLINK_INTERVAL==0)
			{
				RED_LED_OFF();	//每次循环开始将灯熄灭
			}
			else if(time_1ms_count%BLINK_CYCLE==0)
			{
				RED_LED_TOGGLE();
			}
		}
	}
	else if(Frequency_Red>=10)	//常亮
	{
		RED_LED_ON();
	}
	else
	{
		RED_LED_OFF();
	}
	
}

void LED_Blink_Set(u8 frequency_green,u8 frequency_red)	//2s内闪烁次数,最高9
{
//	Frequency_Green=frequency_green<0?0:frequency_green;
//	Frequency_Red=frequency_red<0?0:frequency_red;
	Frequency_Green=frequency_green>10?10:frequency_green;
	Frequency_Red=frequency_red>10?10:frequency_red;
}

u8 led_start2normal=0;

u8 WS2812Colors[4][3]={0};
#define MAX_LI 80
void WS2812_Run(void)
{

	if(led_start2normal==0)
	{
		WS2812_SetStart();
		//WS2812_SetError();
	}
	else
	{
		WS2812_SetNormal();
	}
	
	
	WS2812_UpdateColor(WS2812Colors,5);
		
	PWM5_3_DMA_Enable();
	
}

u8 rgbstate=0;
void WS2812_SetNormal(void)
{
	static u8 krg,kgb,kbr=0;
	
	switch(rgbstate)
	{
		case 0:
		{
			krg++;
			if(krg>=MAX_LI)
			{
				rgbstate=1;
				krg=MAX_LI;
				kgb=0;
				kbr=0;
			}
			WS2812Colors[0][WS2812_RED]=(u8)(MAX_LI-krg);
			WS2812Colors[0][WS2812_GREEN]=(u8)(krg);
			WS2812Colors[0][WS2812_BLUE]=0;
			break;
		}
		case 1:
		{
			kgb++;
			if(kgb>=MAX_LI)
			{
				rgbstate=2;
				krg=0;
				kgb=MAX_LI;
				kbr=0;
			}
			WS2812Colors[0][WS2812_RED]=0;
			WS2812Colors[0][WS2812_GREEN]=(u8)(MAX_LI-kgb);
			WS2812Colors[0][WS2812_BLUE]=(u8)(kgb);
			break;
		}
		case 2:
		{
			kbr++;
			if(kbr>=MAX_LI)
			{
				rgbstate=0;
				krg=0;
				kgb=0;
				kbr=MAX_LI;
			}
			WS2812Colors[0][WS2812_RED]=(u8)(kbr);
			WS2812Colors[0][WS2812_GREEN]=0;
			WS2812Colors[0][WS2812_BLUE]=(u8)(MAX_LI-kbr);
			break;
		}
	}
	
	static u8 ledLightstate[5]={0,0,0,0,1};
	static float kk[5]={0,0.1f,0.2f,0.3f,0.4f};
	
	for(int i=0;i<5;i++)
	{
		switch(ledLightstate[i])
		{
			case 0:
			{
				kk[i]+=0.01f;
				if(kk[i]>=0.4f)
				{
					kk[i]=0.4f;
					ledLightstate[i]=1;
				}
				break;
			}
			case 1:
			{
				kk[i]-=0.01f;
				if(kk[i]<=0)
				{
					kk[i]=0;
					ledLightstate[i]=0;
				}
				break;
			}
		}
		
		
	}
	
	
	WS2812Colors[1][WS2812_GREEN]=WS2812Colors[0][WS2812_RED];
	WS2812Colors[1][WS2812_BLUE]=WS2812Colors[0][WS2812_GREEN];
	WS2812Colors[1][WS2812_RED]=WS2812Colors[0][WS2812_BLUE];
//	for(int j=0;j<3;j++)
//	{
//		WS2812Colors[0][j]*=kk[0];
//		WS2812Colors[1][j]*=1-kk[0];
//	}
	
	WS2812Colors[2][WS2812_GREEN]=WS2812Colors[0][WS2812_GREEN];
	WS2812Colors[2][WS2812_BLUE]=WS2812Colors[0][WS2812_RED];
	WS2812Colors[2][WS2812_RED]=WS2812Colors[0][WS2812_BLUE];
	
	WS2812Colors[3][WS2812_GREEN]=WS2812Colors[0][WS2812_BLUE];
	WS2812Colors[3][WS2812_BLUE]=WS2812Colors[0][WS2812_GREEN];
	WS2812Colors[3][WS2812_RED]=WS2812Colors[0][WS2812_RED];
	
	WS2812Colors[4][WS2812_GREEN]=WS2812Colors[0][WS2812_GREEN];
	WS2812Colors[4][WS2812_BLUE]=WS2812Colors[0][WS2812_BLUE];
	WS2812Colors[4][WS2812_RED]=WS2812Colors[0][WS2812_RED];
	
	for(int i=0;i<5;i++)
	{
		for(int j=0;j<3;j++)
		{
			WS2812Colors[i][j]*=kk[i];
			//WS2812Colors[1][j]*=1-kk[0];
		}
	}
}

void _rol_(u8 *x)
{
	u8 t=*x>>7&0x01;
	*x=*x<<1;
	*x |= t;
}

void _ror_(u8 *x)
{
	u8 t=*x&0x01;
	*x=*x>>1;
	*x |= t<<7;
}


void WS2812_SetStart(void)	//调用周期100ms
{
	/**********************************************
	static u8 ledLightstate[5]={0,0,0,1,1};
	static float kk[5]={0,0.2f,0.4f,0.2f,0.0f};
	
	for(int i=0;i<5;i++)
	{
		switch(ledLightstate[i])
		{
			case 0:
			{
				kk[i]+=0.01f;
				if(kk[i]>=0.4f)
				{
					kk[i]=0.4f;
					ledLightstate[i]=1;
				}
				break;
			}
			case 1:
			{
				kk[i]-=0.01f;
				if(kk[i]<=0)
				{
					kk[i]=0;
					ledLightstate[i]=0;
				}
				break;
			}
		}
	}
	
	for(int i=0;i<5;i++)
	{
		for(int j=0;j<3;j++)
		{
			WS2812Colors[i][j]=40*kk[i];
			//WS2812Colors[1][j]*=1-kk[0];
		}
	}
	***************************************************/
	
	static u8 ledstate=1;
	static u8 ledchangecnt=0;
	static u8 ledstartnum=0;
	if(time_1ms_count%100==0)
	{
		for(int i=0;i<5;i++)
		{
			for(int j=0;j<3;j++)
			{
				WS2812Colors[i][j]=100*((ledstate>>i)&0x01);
			}
		}
		_ror_(&ledstate);
		ledchangecnt++;
		if(ledchangecnt%8==0&&ledstartnum<=5)
		{
			ledstartnum++;
		}
		
		for(int i=0;i<ledstartnum;i++)
		{
			for(int j=0;j<3;j++)
			{
				WS2812Colors[i][j]=100;
			}
		}
//		if(ledstartnum==1)
//		{
//			for(int i=0;i<1;i++)
//			{
//				for(int j=0;j<3;j++)
//				{
//					WS2812Colors[i][j]=100;
//				}
//			}
//		}
//		else if(ledstartnum==2)
//		{
//			
//		}
//		else if(ledstartnum==3)
//		{
//			
//		}
//		else if(ledstartnum==4)
//		{
//			
//		}
//		else if(ledstartnum>=5)
//		{
//			
//		}
	}
	
}


void WS2812_SetError(void)
{
	static u8 errorledstate=0;
	
	if(time_1ms_count%500==0)
	{
		for(int i=0;i<5;i++)
		{
//			for(int j=0;j<3;j++)
//			{
				WS2812Colors[i][WS2812_RED]=100*errorledstate;
				WS2812Colors[i][WS2812_GREEN]=0;
				WS2812Colors[i][WS2812_BLUE]=0;
//			}
		}
		errorledstate=!errorledstate;
	}
	
}

