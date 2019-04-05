#include "gpio_laster.h"


/*************************
函数名：Laser_Init
函数功能：激光的GPIO配置
函数输入：无
函数返回值： 无
描述：配置的引脚 PG13
*************************/
void Laser_Init(void)
{
	GPIO_InitTypeDef gpio;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	gpio.GPIO_Pin = GPIO_Pin_13;
	gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOG, &gpio);
	GPIO_ResetBits(GPIOG, GPIO_Pin_13);
}

