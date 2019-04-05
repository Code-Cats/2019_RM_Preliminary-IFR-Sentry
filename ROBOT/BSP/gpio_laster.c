#include "gpio_laster.h"


/*************************
��������Laser_Init
�������ܣ������GPIO����
�������룺��
��������ֵ�� ��
���������õ����� PG13
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

