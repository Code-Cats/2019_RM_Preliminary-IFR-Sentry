#include "usart3_judge.h"
#include "usart3_judge_analysis.h"

#define BSP_USART3_DMA_RX_BUF_LEN 100
uint8_t USART3_DMA_RX_BUF[2][BSP_USART3_DMA_RX_BUF_LEN]={0};

char g_judgeFlag = 0;
char g_judgeLost = 0;


void USART3_Configuration(uint32_t baud_rate)
{
    GPIO_InitTypeDef gpio;
	USART_InitTypeDef usart;
	NVIC_InitTypeDef nvic;
    DMA_InitTypeDef dma;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);  //使能USART3，GPIOB时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);   //使能DMA传输
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); //使能USART3时钟
	
	USART_DeInit(USART3);//复位串口3
    
    GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_USART3);	//TX
    GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_USART3);	//RX

    gpio.GPIO_Pin = GPIO_Pin_8;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_Speed = GPIO_Speed_100MHz;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD,&gpio);
	
	gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_OType = GPIO_OType_OD;
    gpio.GPIO_Speed = GPIO_Speed_100MHz;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOD,&gpio);
    
	nvic.NVIC_IRQChannel = USART3_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 2;
    nvic.NVIC_IRQChannelSubPriority = 1;
    nvic.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&nvic);
    
    USART_StructInit(&usart);
    usart.USART_BaudRate = baud_rate;
    usart.USART_WordLength = USART_WordLength_8b; //字长为8位数据格式
    usart.USART_StopBits = USART_StopBits_1;  //一个停止位
    usart.USART_Parity = USART_Parity_No;   //无奇偶校验位
    usart.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;  //收发模式
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  //无硬件数据流控制
		
    USART_Init(USART3, &usart);  //初始化串口3
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);        //开启空闲中断
	USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);   //使能串口1 DMA接收
	USART_Cmd(USART3, ENABLE);                 //使能串口 
		
		//相应的DMA配置
	DMA_DeInit(DMA1_Stream1);
   // DMA_StructInit(&dma);
    dma.DMA_Channel = DMA_Channel_4;
    dma.DMA_PeripheralBaseAddr = (uint32_t)(&USART3->DR);  //DMA外设ADC基地址
    dma.DMA_Memory0BaseAddr = (uint32_t)&USART3_DMA_RX_BUF[0][0]; //DMA内存基地址
    dma.DMA_DIR = DMA_DIR_PeripheralToMemory;
    dma.DMA_BufferSize = BSP_USART3_DMA_RX_BUF_LEN;
    dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma.DMA_Mode = DMA_Mode_Normal;      //工作在正常缓存模式
    dma.DMA_Priority = DMA_Priority_Medium;   //DMA通道 x拥有中优先级 
    dma.DMA_FIFOMode = DMA_FIFOMode_Disable;   //DMA通道x没有设置为内存到内存传输
    dma.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
    dma.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA1_Stream1, &dma);
		
		
		    //配置Memory1,Memory0是第一个使用的Memory
    DMA_DoubleBufferModeConfig(DMA1_Stream1, (uint32_t)&USART3_DMA_RX_BUF[1][0], DMA_Memory_0);   //first used memory configuration
    DMA_DoubleBufferModeCmd(DMA1_Stream1, ENABLE);
    
    DMA_Cmd(DMA1_Stream1, ENABLE);	
    
}

uint8_t	this_dma_type_usart3=0;
uint32_t	this_frame_rx_len_usart3 = 0;
void USART3_IRQHandler(void)
{
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
	{
		(void)USART3->SR;
		(void)USART3->DR;
		if(DMA_GetCurrentMemoryTarget(DMA1_Stream1) == 0)
		{
			this_dma_type_usart3=0;
			DMA_Cmd(DMA1_Stream1, DISABLE);
			this_frame_rx_len_usart3 = BSP_USART3_DMA_RX_BUF_LEN - DMA_GetCurrDataCounter(DMA1_Stream1);
			DMA1_Stream1->NDTR = (uint16_t)BSP_USART3_DMA_RX_BUF_LEN;     //??relocate the dma memory pointer to the beginning position
			DMA1_Stream1->CR |= (uint32_t)(DMA_SxCR_CT);                  //enable the current selected memory is Memory 1
			DMA_Cmd(DMA1_Stream1, ENABLE);

			judgeData_analysis(USART3_DMA_RX_BUF[0], this_frame_rx_len_usart3);
			//judgementDataHandler();
		}
		else 
		{
			this_dma_type_usart3=1;
			DMA_Cmd(DMA1_Stream1, DISABLE);
			this_frame_rx_len_usart3 = BSP_USART3_DMA_RX_BUF_LEN - DMA_GetCurrDataCounter(DMA1_Stream1);
			DMA1_Stream1->NDTR = (uint16_t)BSP_USART3_DMA_RX_BUF_LEN;      //??relocate the dma memory pointer to the beginning position
			DMA1_Stream1->CR &= ~(uint32_t)(DMA_SxCR_CT);                  //enable the current selected memory is Memory 0
			DMA_Cmd(DMA1_Stream1, ENABLE);

			judgeData_analysis(USART3_DMA_RX_BUF[1], this_frame_rx_len_usart3);
			// judgementDataHandler();
		}
	}
}


/*
入口:裁判反馈的实时枪口热量
     双目反馈的目标距离
     计算得到的目标速度（由另一个函数生成）
出口:射击频率
*/
