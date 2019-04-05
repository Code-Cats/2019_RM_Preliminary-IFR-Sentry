#include "uart7_vision.h"
#include "uart7_vision_analysis.h"

void UART7_Vision_Init(uint32_t baud_rate)	//���޸�
{

	GPIO_InitTypeDef gpio;
	USART_InitTypeDef usart;
	NVIC_InitTypeDef nvic;
	//    DMA_InitTypeDef dma;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); 	//PE7 RX; PE8 TX
	//     RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART7, ENABLE); 

	GPIO_PinAFConfig(GPIOE,GPIO_PinSource7,GPIO_AF_UART7);	//RX
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource8,GPIO_AF_UART7); 	//TX

	gpio.GPIO_Pin = GPIO_Pin_7;
	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_Speed = GPIO_Speed_100MHz;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE,&gpio);

	gpio.GPIO_Pin = GPIO_Pin_8;
	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_Speed = GPIO_Speed_100MHz;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE,&gpio);

	USART_DeInit(UART7);
	USART_StructInit(&usart);
	usart.USART_BaudRate = baud_rate;
	usart.USART_WordLength = USART_WordLength_8b;
	usart.USART_StopBits = USART_StopBits_1;
	usart.USART_Parity = USART_Parity_No;
	// usart.USART_Parity = USART_Parity_Even;
	usart.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
	usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(UART7, &usart);
//		
//		USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
//		
//	  DMA_DeInit(DMA1_Stream1);
//    DMA_StructInit(&dma);
//    dma.DMA_Channel = DMA_Channel_4;
//    dma.DMA_PeripheralBaseAddr = (uint32_t)(&USART3->DR);
//    dma.DMA_Memory0BaseAddr = (uint32_t)&_USART3_DMA_RX_BUF[0][0];
//    dma.DMA_DIR = DMA_DIR_PeripheralToMemory;
//    dma.DMA_BufferSize = sizeof(_USART3_DMA_RX_BUF)/2;
//    dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//    dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
//    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
//    dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
//    dma.DMA_Mode = DMA_Mode_Circular;
//    dma.DMA_Priority = DMA_Priority_Medium;
//    dma.DMA_FIFOMode = DMA_FIFOMode_Disable;
//    dma.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
//    dma.DMA_MemoryBurst = DMA_MemoryBurst_Single;
//    dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
//    DMA_Init(DMA1_Stream1, &dma);
//		
//		
//		    //����Memory1,Memory0�ǵ�һ��ʹ�õ�Memory
//    DMA_DoubleBufferModeConfig(DMA1_Stream1, (uint32_t)&_USART3_DMA_RX_BUF[1][0], DMA_Memory_0);   //first used memory configuration
//    DMA_DoubleBufferModeCmd(DMA1_Stream1, ENABLE);
//    
//    DMA_Cmd(DMA1_Stream1, ENABLE);
		
    nvic.NVIC_IRQChannel = UART7_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority = 1;
    nvic.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&nvic);
		
	USART_ITConfig(UART7, USART_IT_RXNE, ENABLE);        //usart rx idle interrupt  enabled
    USART_Cmd(UART7, ENABLE);
}

u32 vision_count;
u8 UART7_Res=0;
void UART7_IRQHandler(void)
{

	if(USART_GetITStatus(UART7, USART_IT_RXNE) != RESET)
	{
		UART7_Res=USART_ReceiveData(UART7);

		VisionData_Receive(UART7_Res);
		vision_count++;
		
		(void)UART7->SR;
		(void)UART7->DR;
	}

}


