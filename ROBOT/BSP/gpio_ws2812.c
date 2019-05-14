#include "gpio_ws2812.h"
#include "pwm.h"
/*************************
函数名：WS2812_Init
函数功能：WS2812的PWM配置
函数输入：无
函数返回值： 无
描述：配置的引脚 PH12
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

	GPIO_PinAFConfig(GPIOH,GPIO_PinSource10,GPIO_AF_TIM5);//定时器5 通道1
	GPIO_PinAFConfig(GPIOH,GPIO_PinSource11,GPIO_AF_TIM5);//定时器5 通道2
	GPIO_PinAFConfig(GPIOH,GPIO_PinSource12,GPIO_AF_TIM5);//定时器5 通道3
	GPIO_PinAFConfig(GPIOI,GPIO_PinSource0,GPIO_AF_TIM5);//定时器5 通道4

	/* TIM5 */
	tim.TIM_Prescaler = 19-1;	//18OK
	tim.TIM_CounterMode = TIM_CounterMode_Up;
	tim.TIM_Period = 1*5;   //4->1us	//0.5HIGH 0.75LOW 0码； 0.75HIGH 0.5LOW 1码
	tim.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM5,&tim);	//初始化时间基数单位

	oc.TIM_OCMode = TIM_OCMode_PWM2;
	oc.TIM_OutputState = TIM_OutputState_Enable;
	oc.TIM_OutputNState = TIM_OutputState_Disable;
	oc.TIM_Pulse = 0;
	oc.TIM_OCPolarity = TIM_OCPolarity_Low;
	oc.TIM_OCNPolarity = TIM_OCPolarity_High;
	oc.TIM_OCIdleState = TIM_OCIdleState_Reset;
	oc.TIM_OCNIdleState = TIM_OCIdleState_Set;

	TIM_OC1Init(TIM5,&oc);//定时器5 通道1
	TIM_OC2Init(TIM5,&oc);//定时器5 通道2
	TIM_OC3Init(TIM5,&oc);//定时器5 通道3
	TIM_OC4Init(TIM5,&oc);//定时器5 通道4

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
#define WS2812_SIZE 24*WS2812_NUMS+1	//最后一bit为reset电平	//还有另一种实现思路是使能DMA传输完成中断 并在该中断中将CCR寄存器置0
u32 Pwm5_3_DMABuffer[WS2812_SIZE]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,4,4,4,4,4,4,4,4,4,4,4,4,4,1,1,1,1,1,1,1,4,4,4,4,4,4,4,4,4,0};
void WS2812_TIM5_3_DMA_Init(void)
{
	NVIC_InitTypeDef nvic;
    DMA_InitTypeDef dma;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);   //使能DMA传输
    delay_ms(5);

	DMA_Cmd(DMA1_Stream0, DISABLE);
	DMA_DeInit(DMA1_Stream0);
    dma.DMA_Channel = DMA_Channel_6;
    dma.DMA_PeripheralBaseAddr = (uint32_t)(&TIM5->CCR3);  /* DMA外设基地址 *///DMA外设TIM5-CCR3地址/
    dma.DMA_Memory0BaseAddr = (uint32_t)Pwm5_3_DMABuffer; ///* DMA内存基地址 */DMA内存基地址/
    dma.DMA_DIR = DMA_DIR_MemoryToPeripheral;/* 数据传输方向，从内存读取发送到外设 */
    dma.DMA_BufferSize = WS2812_SIZE;/* DMA通道的DMA缓存的大小 */
    dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;/* 外设地址寄存器不变 */
    dma.DMA_MemoryInc = DMA_MemoryInc_Enable;/* 内存地址寄存器递增 */
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;/* 数据宽度为32位 */
    dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;/* 数据宽度为32位 */
    dma.DMA_Mode = DMA_Mode_Normal;      ///* 工作在正常模式 */工作在正常缓存模式
    dma.DMA_Priority = DMA_Priority_Medium;   //DMA通道 x拥有中优先级 
    dma.DMA_FIFOMode = DMA_FIFOMode_Disable;   //DMA通道x没有设置为内存到内存传输
    dma.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;//DMA_FIFOThreshold_HalfFull;//DMA_FIFOThreshold_1QuarterFull;
    dma.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
		
	    /* 配置DMA */
    DMA_Init(DMA1_Stream0, &dma);

    /*使能TIM的DMA接口 */
	//TIM_SelectCCDMA(TIM5,ENABLE);
	//TIM_DMAConfig(TIM5, TIM_DMABase_CCR3, TIM_DMABurstLength_16Bytes);
	
    TIM_DMACmd(TIM5, TIM_DMA_Update, ENABLE);	/* 如果是要调节占空比就把这行去掉注释，注释另一行，再修改相应通道 */
	//TIM_DMACmd(TIM3, TIM_DMA_CC1, ENABLE);	/* 如果是要调节频率就把这行去掉注释，注释另一行，再修改相应通道 */

    DMA_Cmd(DMA1_Stream0, DISABLE);	 /*不使能DMA */  
	TIM_Cmd(TIM5, ENABLE);  /* 使能TIM5 */
}

bool PWM5_3_DMA_Enable(void)
{
	static u8 start_flag=0;
	if(DMA_GetFlagStatus(DMA1_Stream0,DMA_FLAG_TCIF0)!= RESET || start_flag!=1)	//传输完成标志，若在传输过程中再次设置将会打乱时序
	{
		start_flag=1;
		DMA_ClearFlag(DMA1_Stream0,DMA_IT_TCIF0);
		DMA_Cmd(DMA1_Stream0, DISABLE );
		DMA_SetCurrDataCounter(DMA1_Stream0,WS2812_SIZE);

		DMA_Cmd(DMA1_Stream0, ENABLE);
		//TIM_Cmd(TIM5, ENABLE);  /* 使能TIM3 */
		TIM5->EGR |= 0x00000001;	/* 由于最后一次ARR值为0，这是为了停止定时器对io口的操作，但是不要忽略了一点：CNT并没有停止计数，而且是不会再停下来，如果没有手动操作的话，所以需要在每次dma使能时加上一句，将EGR里的UG位置1，清零计数器 */
		return true;
	}
	else
	{
		return false;
	}
}

bool WS2812_UpdateColor(u8 colors[][3],u16 led_nums)	//GRB 高位先发
{
	if(led_nums>WS2812_NUMS)	return false;
	for(int i=0;i<led_nums;i++)
	{
		for(int channel=WS2812_GREEN;channel<WS2812_BLUE+1;channel++)	//这两个for可合并为一个
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