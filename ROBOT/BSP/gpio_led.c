#include "gpio_led.h" 

/*************************
��������LED_Init
�������ܣ�˫ɫLED�Ƶ�GPIO����
�������룺��
��������ֵ�� ��
���������õ����� PE11��PF14
*************************/
void LED_Init(void)
{    	 
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOE, &GPIO_InitStructure);//��ʼ��


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOF, &GPIO_InitStructure);//��ʼ��


//	GPIO_SetBits(GPIOE,GPIO_Pin_11);
//	GPIO_SetBits(GPIOF,GPIO_Pin_14);
	GREEN=1;
	RED=0;
	
	Green_LED_8_Init();

}

/*************************
��������Green_LED_8_Init
�������ܣ�8·LED�Ƶ�GPIO����
�������룺��
��������ֵ�� ��
���������õ����� PG-8
*************************/
void Green_LED_8_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOG, &GPIO_InitStructure);//��ʼ��


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
��������Green_LED_8_Set
�������ܣ�8·LED���趨
�������룺u8���� ��λ��Ӧ8��LED
��������ֵ�� ��
���������õ����� PG-8
*************************/
void Green_LED_8_Set(u8 value)
{
	for(int i=1;i<9;i++)
	{
		GREEN1_8(i)=(value>>(i-1))&0x01;
	}
	
}


/*************************
��������Green_LED_8_Set
�������ܣ�8·LED���趨
�������룺s8���� �������������� �����������Ǳ���
��������ֵ�� ��
���������õ����� PG-8
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
	
	temp_v=~temp_v;	//��Ϊ0������
	
	Green_LED_8_Set(temp_v);
}
