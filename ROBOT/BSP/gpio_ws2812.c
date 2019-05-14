#include "gpio_ws2812.h"
#include "pwm.h"
/*************************
��������WS2812_Init
�������ܣ�WS2812��PWM����
�������룺��
��������ֵ�� ��
���������õ����� PH12
*************************/
void WS2812_Init(void)
{
	GPIO_InitTypeDef          gpio;
	TIM_TimeBaseInitTypeDef   tim;
	TIM_OCInitTypeDef         oc;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH ,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI ,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);   

	gpio.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 ;	//CH1,2,3
	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOH,&gpio);

	gpio.GPIO_Pin = GPIO_Pin_0;	//CH4
	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOI,&gpio);

	GPIO_PinAFConfig(GPIOH,GPIO_PinSource10,GPIO_AF_TIM5);//��ʱ��5 ͨ��1
	GPIO_PinAFConfig(GPIOH,GPIO_PinSource11,GPIO_AF_TIM5);//��ʱ��5 ͨ��2
	GPIO_PinAFConfig(GPIOH,GPIO_PinSource12,GPIO_AF_TIM5);//��ʱ��5 ͨ��3
	GPIO_PinAFConfig(GPIOI,GPIO_PinSource0,GPIO_AF_TIM5);//��ʱ��5 ͨ��4

	/* TIM5 */
	tim.TIM_Prescaler = 19-1;	//18OK
	tim.TIM_CounterMode = TIM_CounterMode_Up;
	tim.TIM_Period = 1*5;   //4->1us	//0.5HIGH 0.75LOW 0�룻 0.75HIGH 0.5LOW 1��
	tim.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM5,&tim);	//��ʼ��ʱ�������λ

	oc.TIM_OCMode = TIM_OCMode_PWM2;
	oc.TIM_OutputState = TIM_OutputState_Enable;
	oc.TIM_OutputNState = TIM_OutputState_Disable;
	oc.TIM_Pulse = 0;
	oc.TIM_OCPolarity = TIM_OCPolarity_Low;
	oc.TIM_OCNPolarity = TIM_OCPolarity_High;
	oc.TIM_OCIdleState = TIM_OCIdleState_Reset;
	oc.TIM_OCNIdleState = TIM_OCIdleState_Set;

	TIM_OC1Init(TIM5,&oc);//��ʱ��5 ͨ��1
	TIM_OC2Init(TIM5,&oc);//��ʱ��5 ͨ��2
	TIM_OC3Init(TIM5,&oc);//��ʱ��5 ͨ��3
	TIM_OC4Init(TIM5,&oc);//��ʱ��5 ͨ��4

	TIM_OC1PreloadConfig(TIM5,TIM_OCPreload_Enable);
	TIM_OC2PreloadConfig(TIM5,TIM_OCPreload_Enable);
	TIM_OC3PreloadConfig(TIM5,TIM_OCPreload_Enable);
	TIM_OC4PreloadConfig(TIM5,TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM5,ENABLE);

	TIM_Cmd(TIM5,DISABLE);
}

#define WS2812BIT_HIGH 3
#define WS2812BIT_LOW 1
const u8 WS2812BitDef[2] ={WS2812BIT_LOW,WS2812BIT_HIGH};
#define WS2812_NUMS 5
#define WS2812_SIZE 24*WS2812_NUMS+1	//���һbitΪreset��ƽ	//������һ��ʵ��˼·��ʹ��DMA��������ж� ���ڸ��ж��н�CCR�Ĵ�����0
u32 Pwm5_3_DMABuffer[WS2812_SIZE]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,4,4,4,4,4,4,4,4,4,4,4,4,4,1,1,1,1,1,1,1,4,4,4,4,4,4,4,4,4,0};
void WS2812_TIM5_3_DMA_Init(void)
{
	NVIC_InitTypeDef nvic;
    DMA_InitTypeDef dma;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);   //ʹ��DMA����
    delay_ms(5);

	DMA_Cmd(DMA1_Stream0, DISABLE);
	DMA_DeInit(DMA1_Stream0);
    dma.DMA_Channel = DMA_Channel_6;
    dma.DMA_PeripheralBaseAddr = (uint32_t)(&TIM5->CCR3);  /* DMA�������ַ *///DMA����TIM5-CCR3��ַ/
    dma.DMA_Memory0BaseAddr = (uint32_t)Pwm5_3_DMABuffer; ///* DMA�ڴ����ַ */DMA�ڴ����ַ/
    dma.DMA_DIR = DMA_DIR_MemoryToPeripheral;/* ���ݴ��䷽�򣬴��ڴ��ȡ���͵����� */
    dma.DMA_BufferSize = WS2812_SIZE;/* DMAͨ����DMA����Ĵ�С */
    dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;/* �����ַ�Ĵ������� */
    dma.DMA_MemoryInc = DMA_MemoryInc_Enable;/* �ڴ��ַ�Ĵ������� */
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;/* ���ݿ��Ϊ32λ */
    dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;/* ���ݿ��Ϊ32λ */
    dma.DMA_Mode = DMA_Mode_Normal;      ///* ����������ģʽ */��������������ģʽ
    dma.DMA_Priority = DMA_Priority_Medium;   //DMAͨ�� xӵ�������ȼ� 
    dma.DMA_FIFOMode = DMA_FIFOMode_Disable;   //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
    dma.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;//DMA_FIFOThreshold_HalfFull;//DMA_FIFOThreshold_1QuarterFull;
    dma.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
		
	    /* ����DMA */
    DMA_Init(DMA1_Stream0, &dma);

    /*ʹ��TIM��DMA�ӿ� */
	//TIM_SelectCCDMA(TIM5,ENABLE);
	//TIM_DMAConfig(TIM5, TIM_DMABase_CCR3, TIM_DMABurstLength_16Bytes);
	
    TIM_DMACmd(TIM5, TIM_DMA_Update, ENABLE);	/* �����Ҫ����ռ�ձȾͰ�����ȥ��ע�ͣ�ע����һ�У����޸���Ӧͨ�� */
	//TIM_DMACmd(TIM3, TIM_DMA_CC1, ENABLE);	/* �����Ҫ����Ƶ�ʾͰ�����ȥ��ע�ͣ�ע����һ�У����޸���Ӧͨ�� */

    DMA_Cmd(DMA1_Stream0, DISABLE);	 /*��ʹ��DMA */  
	TIM_Cmd(TIM5, ENABLE);  /* ʹ��TIM5 */
}

bool PWM5_3_DMA_Enable(void)
{
	static u8 start_flag=0;
	if(DMA_GetFlagStatus(DMA1_Stream0,DMA_FLAG_TCIF0)!= RESET || start_flag!=1)	//������ɱ�־�����ڴ���������ٴ����ý������ʱ��
	{
		start_flag=1;
		DMA_ClearFlag(DMA1_Stream0,DMA_IT_TCIF0);
		DMA_Cmd(DMA1_Stream0, DISABLE );
		DMA_SetCurrDataCounter(DMA1_Stream0,WS2812_SIZE);

		DMA_Cmd(DMA1_Stream0, ENABLE);
		//TIM_Cmd(TIM5, ENABLE);  /* ʹ��TIM3 */
		TIM5->EGR |= 0x00000001;	/* �������һ��ARRֵΪ0������Ϊ��ֹͣ��ʱ����io�ڵĲ��������ǲ�Ҫ������һ�㣺CNT��û��ֹͣ�����������ǲ�����ͣ���������û���ֶ������Ļ���������Ҫ��ÿ��dmaʹ��ʱ����һ�䣬��EGR���UGλ��1����������� */
		return true;
	}
	else
	{
		return false;
	}
}

bool WS2812_UpdateColor(u8 colors[][3],u16 led_nums)	//GRB ��λ�ȷ�
{
	if(led_nums>WS2812_NUMS)	return false;
	for(int i=0;i<led_nums;i++)
	{
		for(int channel=WS2812_GREEN;channel<WS2812_BLUE+1;channel++)	//������for�ɺϲ�Ϊһ��
		{
			for(int bit=7;bit>=0;bit--)
			{
				Pwm5_3_DMABuffer[i*24+channel*8+(7-bit)]=WS2812BitDef[*(colors[i] + channel)>>bit&0x01];
			}
		}
	}
	Pwm5_3_DMABuffer[led_nums*24+8]=0;
	PWM5_3_DMA_Enable();
	return true;
}