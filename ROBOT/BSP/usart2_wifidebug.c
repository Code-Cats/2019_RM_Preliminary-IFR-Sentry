#include "usart2_wifidebug.h"

#define BSP_USART2_DMA_RX_BUF_LEN 100

uint8_t USART2_DMA_RX_BUF[2][BSP_USART2_DMA_RX_BUF_LEN]={0};

USART_CircularQueueTypeDef USART2_Software_FIFO={0};//需要处理一下头部到了末尾衔接问题

void USART2_wifidebug_Init(uint32_t baud_rate)
{
    GPIO_InitTypeDef gpio;
	USART_InitTypeDef usart;
	NVIC_InitTypeDef nvic;
    DMA_InitTypeDef dma;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);  //使能USART2，GPIOB时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);   //使能DMA传输
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); //使能USART2时钟
	
	USART_DeInit(USART2);//复位串口2
	
	USART_OverSampling8Cmd(USART2,ENABLE);
    
    GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOD,GPIO_PinSource6,GPIO_AF_USART2); 

    gpio.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_Speed = GPIO_Speed_100MHz;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD,&gpio);
    
	nvic.NVIC_IRQChannel = USART2_IRQn;
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
		
    USART_Init(USART2, &usart);  //初始化串口3
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);        //开启空闲中断
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);   //使能串口1 DMA接收
	USART_Cmd(USART2, DISABLE);                 //现在不使能串口，在进入while前使能串口 
		
		//相应的DMA配置
	DMA_DeInit(DMA1_Stream1);
   // DMA_StructInit(&dma);
    dma.DMA_Channel = DMA_Channel_4;
    dma.DMA_PeripheralBaseAddr = (uint32_t)(&USART2->DR);  //DMA外设ADC基地址
    dma.DMA_Memory0BaseAddr = (uint32_t)&USART2_DMA_RX_BUF[0][0]; //DMA内存基地址
    dma.DMA_DIR = DMA_DIR_PeripheralToMemory;
    dma.DMA_BufferSize = BSP_USART2_DMA_RX_BUF_LEN;
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
    DMA_Init(DMA1_Stream5, &dma);
		
		
	nvic.NVIC_IRQChannel = DMA1_Stream5_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 2;
    nvic.NVIC_IRQChannelSubPriority = 1;
    nvic.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&nvic);
	
	DMA_ITConfig(DMA1_Stream5, DMA_IT_TC, ENABLE);        //开启DMA传输完成中断
		
		    //配置Memory1,Memory0是第一个使用的Memory
    DMA_DoubleBufferModeConfig(DMA1_Stream5, (uint32_t)&USART2_DMA_RX_BUF[1][0], DMA_Memory_0);   //first used memory configuration
    DMA_DoubleBufferModeCmd(DMA1_Stream5, ENABLE);
    
    DMA_Cmd(DMA1_Stream5, ENABLE);
  
}

#define USART2_DMA_TX_BUFFER_SIZE 400
u8 USART2_DMA_TX_Buffer[USART2_DMA_TX_BUFFER_SIZE]={0};
void USART2_DMA_Tx_Init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;


    /* 1.使能DMA2时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

    /* 2.配置使用DMA发送数据 */
    DMA_DeInit(DMA1_Stream6); 

    DMA_InitStructure.DMA_Channel             = DMA_Channel_4;               /* 配置DMA通道 */
    DMA_InitStructure.DMA_PeripheralBaseAddr  = (uint32_t)(&(USART2->DR));   /* 目的 */
    DMA_InitStructure.DMA_Memory0BaseAddr     = (uint32_t)USART2_DMA_TX_Buffer;             /* 源 */
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_MemoryToPeripheral;    /* 方向 */
    DMA_InitStructure.DMA_BufferSize          = USART2_DMA_TX_BUFFER_SIZE;                    /* 长度 */                  
    DMA_InitStructure.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;    /* 外设地址是否自增 */
    DMA_InitStructure.DMA_MemoryInc           = DMA_MemoryInc_Enable;         /* 内存地址是否自增 */
    DMA_InitStructure.DMA_PeripheralDataSize  = DMA_MemoryDataSize_Byte;      /* 目的数据带宽 */
    DMA_InitStructure.DMA_MemoryDataSize      = DMA_MemoryDataSize_Byte;      /* 源数据宽度 */
    DMA_InitStructure.DMA_Mode                = DMA_Mode_Normal;              /* 单次传输模式/循环传输模式 */
    DMA_InitStructure.DMA_Priority            = DMA_Priority_High;             /* DMA优先级 */
    DMA_InitStructure.DMA_FIFOMode            = DMA_FIFOMode_Disable;          /* FIFO模式/直接模式 */
    DMA_InitStructure.DMA_FIFOThreshold       = DMA_FIFOThreshold_HalfFull; /* FIFO大小 */
    DMA_InitStructure.DMA_MemoryBurst         = DMA_MemoryBurst_Single;       /* 单次传输 */
    DMA_InitStructure.DMA_PeripheralBurst     = DMA_PeripheralBurst_Single;

    /* 3. 配置DMA */
    DMA_Init(DMA1_Stream6, &DMA_InitStructure);

    /* 4.使能DMA中断 */
    DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, ENABLE);

    /* 5.使能串口的DMA发送接口 */
    USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);

    /* 6. 配置DMA中断优先级 */
    NVIC_InitStructure.NVIC_IRQChannel                   = DMA1_Stream6_IRQn;           
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;          
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1; 
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 7.不使能DMA */                  
    DMA_Cmd(DMA1_Stream6, DISABLE);
}

u8 USART2_Use_DMA_Tx_Flag;
void USART2_DMA_Send(const uint8_t *data,uint16_t size)  
{  
    /* 等待空闲 */
    while (USART2_Use_DMA_Tx_Flag);  
    USART2_Use_DMA_Tx_Flag = 1;  
    /* 复制数据 */
    memcpy(USART2_DMA_TX_Buffer,data,size);  
    /* 设置传输数据长度 */  
    DMA_SetCurrDataCounter(DMA1_Stream6,size);  
    /* 打开DMA,开始发送 */  
    DMA_Cmd(DMA1_Stream6,ENABLE);  
} 

void DMA1_Stream6_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_Stream6,DMA_IT_TCIF6) != RESET)   
    {  
        /* 清除标志位 */
        DMA_ClearFlag(DMA1_Stream6,DMA_IT_TCIF6);
        /* 关闭DMA */
        DMA_Cmd(DMA1_Stream6,DISABLE);
        /* 打开发送完成中断,确保最后一个字节发送成功 */
        USART_ITConfig(USART2,USART_IT_TC,ENABLE);  
    }  
}

void USART2_TX_TC_IRQ_Deal(void)	//发送完成中断处理函数
{  
    if(USART_GetITStatus(USART2, USART_IT_TXE) == RESET)  
    {  
        /* 关闭发送完成中断  */ 
        USART_ITConfig(USART2,USART_IT_TC,DISABLE);  
        /* 发送完成  */
        USART2_Use_DMA_Tx_Flag = 0;  
    }    
}

u8 this_dma_type_usart2=0;	//接受缓冲在哪个区
uint32_t this_frame_rx_len_usart2 = 0;	//接受帧长度
void USART2_RX_IDLE_IRQ_Deal(void)	//串口空闲中断处理函数
{
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
	{
		USART2_Software_FIFO.Data_state=1;
		(void)USART2->SR;
		(void)USART2->DR;
		if(DMA_GetCurrentMemoryTarget(DMA1_Stream5) == 0)
		{
			this_dma_type_usart2=0;

			DMA_ITConfig(DMA1_Stream5, DMA_IT_TC, DISABLE);
			
			DMA_Cmd(DMA1_Stream5, DISABLE);

			DMA_ClearITPendingBit(DMA1_Stream5,DMA_IT_TCIF5);
			this_frame_rx_len_usart2 = BSP_USART2_DMA_RX_BUF_LEN - DMA_GetCurrDataCounter(DMA1_Stream5);
			DMA1_Stream5->NDTR = (uint16_t)BSP_USART2_DMA_RX_BUF_LEN;     //??relocate the dma memory pointer to the beginning position
			DMA1_Stream5->CR |= (uint32_t)(DMA_SxCR_CT);                  //enable the current selected memory is Memory 1

			//test_memcpy
			//memcpy(test_memcpy,USART2_DMA_RX_BUF[this_dma_type_usart2],1);
			InsertQueue(&USART2_Software_FIFO,USART2_DMA_RX_BUF[this_dma_type_usart2],this_frame_rx_len_usart2);
			
			DMA_ITConfig(DMA1_Stream5, DMA_IT_TC, ENABLE);
			
			DMA_Cmd(DMA1_Stream5, ENABLE);
			
			//judgementDataHandler();
		}
		else 
		{
			this_dma_type_usart2=1;
			
			DMA_ITConfig(DMA1_Stream5, DMA_IT_TC, DISABLE);
			
			DMA_Cmd(DMA1_Stream5, DISABLE);

			DMA_ClearITPendingBit(DMA1_Stream5,DMA_IT_TCIF5);
			this_frame_rx_len_usart2 = BSP_USART2_DMA_RX_BUF_LEN - DMA_GetCurrDataCounter(DMA1_Stream5);
			DMA1_Stream5->NDTR = (uint16_t)BSP_USART2_DMA_RX_BUF_LEN;      //??relocate the dma memory pointer to the beginning position
			DMA1_Stream5->CR &= ~(uint32_t)(DMA_SxCR_CT);                  //enable the current selected memory is Memory 0

			InsertQueue(&USART2_Software_FIFO,USART2_DMA_RX_BUF[this_dma_type_usart2],this_frame_rx_len_usart2);
			
			DMA_ITConfig(DMA1_Stream5, DMA_IT_TC, ENABLE);
			
			DMA_Cmd(DMA1_Stream5, ENABLE);

			//judgementDataHandler();
		}
	}
}


void USART2_IRQHandler(void)
{
//
	USART2_TX_TC_IRQ_Deal();
	USART2_RX_IDLE_IRQ_Deal();
}

void DMA1_Stream5_IRQHandler()
{
	if(DMA_GetITStatus(DMA1_Stream5, DMA_IT_TCIF5) != RESET)
	{
		DMA_ClearFlag(DMA1_Stream5,DMA_IT_TCIF5);
		
		this_dma_type_usart2=!DMA_GetCurrentMemoryTarget(DMA1_Stream5);	//获取当前缓冲，但是因为DMA满了后会自动切换另一个，所以需要取反
		(void)USART2->SR;	//只有执行了这个才能避免串口空闲中断
		(void)USART2->DR;
		
		InsertQueue(&USART2_Software_FIFO,USART2_DMA_RX_BUF[this_dma_type_usart2],BSP_USART2_DMA_RX_BUF_LEN);
		
	}
}


////FIFO队列相关：
//入队列
bool InsertQueue(USART_CircularQueueTypeDef *q, const u8* source,u16 len)//USART_FIFO_SIZE
{
	if(len<=GetQueueWriteCount(q)&&len>0)
	{
		if(q->FIFO_Front + len < USART_FIFO_SIZE)	//若存入数据未到队列边界
		{
			memcpy(&q->SoftWare_FIFO[q->FIFO_Front], source, len);
			q->FIFO_Front+=len;
		}
		else
		{
			unsigned int len_front=USART_FIFO_SIZE - q->FIFO_Front;
			memcpy(&q->SoftWare_FIFO[q->FIFO_Front], source, len_front);//从front到end
			memcpy(&q->SoftWare_FIFO[0], (source+len_front), len-len_front);	//从头至剩下led	//这里source+len_front需要测试  
			q->FIFO_Front=len-len_front;
		}
	}
	else
	{
		return false;
	}
	
	return true;
}

//出队列操作
bool RemoveQueue(u8* destination,USART_CircularQueueTypeDef *q,u16 len)
{
	if(len<=GetQueueReadCount(q)&&len>0)
	{
		if(q->FIFO_Rear + len < USART_FIFO_SIZE)	//若取出数据范围未到队列边界
		{
			memcpy(destination, &q->SoftWare_FIFO[q->FIFO_Rear], len);
			q->FIFO_Rear+=len;
		}
		else
		{
			unsigned int len_front=USART_FIFO_SIZE - q->FIFO_Rear;
			memcpy(destination, &q->SoftWare_FIFO[q->FIFO_Rear], len_front);//从front到end
			memcpy(destination, &q->SoftWare_FIFO[0], len-len_front);	//从头至剩下led
			q->FIFO_Rear=len-len_front;
		}
	}
	else
	{
		return false;
	}
	
	return true;
}

//出一个元素操作
char RemoveAQueue(USART_CircularQueueTypeDef *q)
{
	u16 len=1;
	char re_char;
	if(len<=GetQueueReadCount(q)&&len>0)
	{
		if(q->FIFO_Rear + len < USART_FIFO_SIZE)	//若取出数据范围未到队列边界
		{
			//memcpy(destination, &q->SoftWare_FIFO[q->FIFO_Rear], len);
			
			re_char = q->SoftWare_FIFO[q->FIFO_Rear];
			q->FIFO_Rear+=len;
		}
		else
		{
			unsigned int len_front=USART_FIFO_SIZE - q->FIFO_Rear;
			//memcpy(destination, &q->SoftWare_FIFO[q->FIFO_Rear], len_front);//从front到end
			
			re_char = q->SoftWare_FIFO[q->FIFO_Rear];
			q->FIFO_Rear=len-len_front;
		}
		return re_char;
	}
	else
	{
		return false;
	}
	
	return true;
}

/**    移出当前所有队列可读元素   **/
bool RemoveAllQueue(u8* destination,USART_CircularQueueTypeDef *q)	//移出当前所有队列可读元素
{
	u16 len=GetQueueReadCount(q);
	if(len>0)
	{
		if(q->FIFO_Rear + len < USART_FIFO_SIZE)	//若取出数据范围未到队列边界
		{
			memcpy(destination, &q->SoftWare_FIFO[q->FIFO_Rear], len);
			q->FIFO_Rear+=len;
		}
		else
		{
			unsigned int len_front=USART_FIFO_SIZE - q->FIFO_Rear;
			memcpy(destination, &q->SoftWare_FIFO[q->FIFO_Rear], len_front);//从front到end
			memcpy(destination, &q->SoftWare_FIFO[0], len-len_front);	//从头至剩下led
			q->FIFO_Rear=len-len_front;
		}
	}
	else
	{
		return false;
	}
	
	return true;
}

//char* GetCurrentQueuePoint()

//查询队列
bool QueryQueue(u8* destination, USART_CircularQueueTypeDef const * queue, u16 distance, u16 len)
{
	if(distance+len<=GetQueueReadCount(queue)&&len>0)	//读取范围在队列可读范围内
	{
		unsigned int start=(queue->FIFO_Rear+distance)%USART_FIFO_SIZE;
	
		if(start + len < USART_FIFO_SIZE)	//若取出数据范围未到队列边界
		{
			memcpy(destination, &queue->SoftWare_FIFO[start], len);
			//q->FIFO_Rear+=len;
		}
		else
		{
			unsigned int len_front=USART_FIFO_SIZE - start;
			memcpy(destination, &queue->SoftWare_FIFO[start], len_front);//从front到end
			memcpy(destination, &queue->SoftWare_FIFO[0], len-len_front);	//从头至剩下led
			//q->FIFO_Rear=len-len_front;
		}
	}
	else
	{
		return false;
	}
	
	return true;
}

//查询队列一个元素
char QueryAQueue(USART_CircularQueueTypeDef const * queue, u16 distance)
{
	u16 len=1;
	if(distance+len<=GetQueueReadCount(queue)&&len>0)	//读取范围在队列可读范围内
	{ 
		unsigned int start=(queue->FIFO_Rear+distance)%USART_FIFO_SIZE;
	
		return queue->SoftWare_FIFO[start];
//		if(start + len < USART_FIFO_SIZE)	//若取出数据范围未到队列边界
//		{
//			memcpy(destination, &queue->SoftWare_FIFO[start], len);
//			//q->FIFO_Rear+=len;
//			return q->SoftWare_FIFO[q->FIFO_Rear];
//		}
//		else
//		{
//			unsigned int len_front=USART_FIFO_SIZE - start;
//			memcpy(destination, &queue->SoftWare_FIFO[start], len_front);//从front到end
//			memcpy(destination, &queue->SoftWare_FIFO[0], len-len_front);	//从头至剩下led
//			//q->FIFO_Rear=len-len_front;
//			return q->SoftWare_FIFO[q->FIFO_Rear];
//		}
	}
	else
	{
		return false;
	}
	
	return true;
}

bool DeleteQueue(USART_CircularQueueTypeDef * q, u16 len)	//就是在移出队列的基础上删除了memcpy
{
	if(len<=GetQueueReadCount(q)&&len>0)
	{
		if(q->FIFO_Rear + len < USART_FIFO_SIZE)	//若取出数据范围未到队列边界
		{
			//memcpy(destination, &q->SoftWare_FIFO[q->FIFO_Rear], len);
			q->FIFO_Rear+=len;
		}
		else
		{
			unsigned int len_front=USART_FIFO_SIZE - q->FIFO_Rear;
			//memcpy(destination, &q->SoftWare_FIFO[q->FIFO_Rear], len_front);//从front到end
			//memcpy(destination, &q->SoftWare_FIFO[0], len-len_front);	//从头至剩下led
			q->FIFO_Rear=len-len_front;
		}
	}
	else
	{
		return false;
	}
	
	return true;
}

void DeleteAllQueue(void)
{
	USART2_Software_FIFO.FIFO_Front=0;
	USART2_Software_FIFO.FIFO_Rear=0;
}

u16 GetQueueReadCount(USART_CircularQueueTypeDef const * queue)	//传递指针而非值可以节约栈控件并提升效率，加上const修饰避免被修改
{
	if(queue->FIFO_Front==queue->FIFO_Rear)	return 0;//return QueueEmpty;
	if(queue->FIFO_Rear-queue->FIFO_Front==1||queue->FIFO_Rear-queue->FIFO_Front==-(USART_FIFO_SIZE-1))	return USART_FIFO_SIZE-1;//return QueueFull;
	return (queue->FIFO_Front - queue->FIFO_Rear)>=0?(queue->FIFO_Front - queue->FIFO_Rear):(queue->FIFO_Front - queue->FIFO_Rear+USART_FIFO_SIZE);
}

u16 GetQueueWriteCount(USART_CircularQueueTypeDef const * queue)	//传递指针而非值可以节约栈控件并提升效率，加上const修饰避免被修改
{
	if(queue->FIFO_Front==queue->FIFO_Rear)	return USART_FIFO_SIZE-1;//return QueueEmpty;
	if(queue->FIFO_Rear-queue->FIFO_Front==1||queue->FIFO_Rear-queue->FIFO_Front==-(USART_FIFO_SIZE-1))	return 0;
	return (queue->FIFO_Rear - queue->FIFO_Front-1)>=0?(queue->FIFO_Rear - queue->FIFO_Front-1):(queue->FIFO_Rear - queue->FIFO_Front-1+USART_FIFO_SIZE);
}
