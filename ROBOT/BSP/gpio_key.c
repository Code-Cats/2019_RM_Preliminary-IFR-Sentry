#include "gpio_key.h"

void KEY_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//����
	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��
}

