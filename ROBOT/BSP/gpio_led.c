#include "gpio_led.h" 

/*************************
函数名：LED_Init
函数功能：双色LED灯的GPIO配置
函数输入：无
函数返回值： 无
描述：配置的引脚 PE11；PF14
*************************/
void LED_Init(void)
{    	 
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOF, &GPIO_InitStructure);//初始化


//	GPIO_SetBits(GPIOE,GPIO_Pin_11);
//	GPIO_SetBits(GPIOF,GPIO_Pin_14);
	GREEN=1;
	RED=0;
	
	Green_LED_8_Init();

}

/*************************
函数名：Green_LED_8_Init
函数功能：8路LED灯的GPIO配置
函数输入：无
函数返回值： 无
描述：配置的引脚 PG-8
*************************/
void Green_LED_8_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOG, &GPIO_InitStructure);//初始化


	GPIO_SetBits(GPIOG,GPIO_Pin_1);
	GPIO_SetBits(GPIOG,GPIO_Pin_2);
	GPIO_SetBits(GPIOG,GPIO_Pin_3);
	GPIO_SetBits(GPIOG,GPIO_Pin_4);
	GPIO_SetBits(GPIOG,GPIO_Pin_5);
	GPIO_SetBits(GPIOG,GPIO_Pin_6);
	GPIO_SetBits(GPIOG,GPIO_Pin_7);
	GPIO_SetBits(GPIOG,GPIO_Pin_8);
}


/*************************
函数名：Green_LED_8_Set
函数功能：8路LED灯设定
函数输入：u8变量 按位对应8个LED
函数返回值： 无
描述：配置的引脚 PG-8
*************************/
void Green_LED_8_Set(u8 value)
{
	for(int i=1;i<9;i++)
	{
		GREEN1_8(i)=(value>>(i-1))&0x01;
	}
	
}


/*************************
函数名：Green_LED_8_Set
函数功能：8路LED灯设定
函数输入：s8变量 数量决定亮几个 正负决定从那边亮
函数返回值： 无
描述：配置的引脚 PG-8
*************************/
void Green_LED_8_SetNum(s8 num)
{
	u8 temp_v=0;
	
	num=num>8?8:num;
	num=num<-8?-8:num;
	
	if(num>=0)
	{
		for(int i=0;i<num;i++)
		{
			temp_v|=0x01<<i;
		}
	}
	else
	{
		for(int i=0;i<-num;i++)
		{
			temp_v|=0x01<<(8-i);
		}
	}
	
	temp_v=~temp_v;	//因为0才是亮
	
	Green_LED_8_Set(temp_v);
}
