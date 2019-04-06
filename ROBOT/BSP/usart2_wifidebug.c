#include "usart2_wifidebug.h"

#define BSP_USART2_DMA_RX_BUF_LEN 100

uint8_t USART2_DMA_RX_BUF[2][BSP_USART2_DMA_RX_BUF_LEN]={0};

USART_CircularQueueTypeDef USART2_Software_FIFO={0};//��Ҫ����һ��ͷ������ĩβ�ν�����

void USART2_wifidebug_Init(uint32_t baud_rate)
{
    GPIO_InitTypeDef gpio;
	USART_InitTypeDef usart;
	NVIC_InitTypeDef nvic;
    DMA_InitTypeDef dma;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);  //ʹ��USART2��GPIOBʱ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);   //ʹ��DMA����
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); //ʹ��USART2ʱ��
	
	USART_DeInit(USART2);//��λ����2
	
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
    usart.USART_WordLength = USART_WordLength_8b; //�ֳ�Ϊ8λ���ݸ�ʽ
    usart.USART_StopBits = USART_StopBits_1;  //һ��ֹͣλ
    usart.USART_Parity = USART_Parity_No;   //����żУ��λ
    usart.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;  //�շ�ģʽ
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  //��Ӳ������������
		
    USART_Init(USART2, &usart);  //��ʼ������3
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);        //���������ж�
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);   //ʹ�ܴ���1 DMA����
	USART_Cmd(USART2, DISABLE);                 //���ڲ�ʹ�ܴ��ڣ��ڽ���whileǰʹ�ܴ��� 
		
		//��Ӧ��DMA����
	DMA_DeInit(DMA1_Stream1);
   // DMA_StructInit(&dma);
    dma.DMA_Channel = DMA_Channel_4;
    dma.DMA_PeripheralBaseAddr = (uint32_t)(&USART2->DR);  //DMA����ADC����ַ
    dma.DMA_Memory0BaseAddr = (uint32_t)&USART2_DMA_RX_BUF[0][0]; //DMA�ڴ����ַ
    dma.DMA_DIR = DMA_DIR_PeripheralToMemory;
    dma.DMA_BufferSize = BSP_USART2_DMA_RX_BUF_LEN;
    dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma.DMA_Mode = DMA_Mode_Normal;      //��������������ģʽ
    dma.DMA_Priority = DMA_Priority_Medium;   //DMAͨ�� xӵ�������ȼ� 
    dma.DMA_FIFOMode = DMA_FIFOMode_Disable;   //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
    dma.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
    dma.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA1_Stream5, &dma);
		
		
	nvic.NVIC_IRQChannel = DMA1_Stream5_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 2;
    nvic.NVIC_IRQChannelSubPriority = 1;
    nvic.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&nvic);
	
	DMA_ITConfig(DMA1_Stream5, DMA_IT_TC, ENABLE);        //����DMA��������ж�
		
		    //����Memory1,Memory0�ǵ�һ��ʹ�õ�Memory
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


    /* 1.ʹ��DMA2ʱ�� */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

    /* 2.����ʹ��DMA�������� */
    DMA_DeInit(DMA1_Stream6); 

    DMA_InitStructure.DMA_Channel             = DMA_Channel_4;               /* ����DMAͨ�� */
    DMA_InitStructure.DMA_PeripheralBaseAddr  = (uint32_t)(&(USART2->DR));   /* Ŀ�� */
    DMA_InitStructure.DMA_Memory0BaseAddr     = (uint32_t)USART2_DMA_TX_Buffer;             /* Դ */
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_MemoryToPeripheral;    /* ���� */
    DMA_InitStructure.DMA_BufferSize          = USART2_DMA_TX_BUFFER_SIZE;                    /* ���� */                  
    DMA_InitStructure.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;    /* �����ַ�Ƿ����� */
    DMA_InitStructure.DMA_MemoryInc           = DMA_MemoryInc_Enable;         /* �ڴ��ַ�Ƿ����� */
    DMA_InitStructure.DMA_PeripheralDataSize  = DMA_MemoryDataSize_Byte;      /* Ŀ�����ݴ��� */
    DMA_InitStructure.DMA_MemoryDataSize      = DMA_MemoryDataSize_Byte;      /* Դ���ݿ�� */
    DMA_InitStructure.DMA_Mode                = DMA_Mode_Normal;              /* ���δ���ģʽ/ѭ������ģʽ */
    DMA_InitStructure.DMA_Priority            = DMA_Priority_High;             /* DMA���ȼ� */
    DMA_InitStructure.DMA_FIFOMode            = DMA_FIFOMode_Disable;          /* FIFOģʽ/ֱ��ģʽ */
    DMA_InitStructure.DMA_FIFOThreshold       = DMA_FIFOThreshold_HalfFull; /* FIFO��С */
    DMA_InitStructure.DMA_MemoryBurst         = DMA_MemoryBurst_Single;       /* ���δ��� */
    DMA_InitStructure.DMA_PeripheralBurst     = DMA_PeripheralBurst_Single;

    /* 3. ����DMA */
    DMA_Init(DMA1_Stream6, &DMA_InitStructure);

    /* 4.ʹ��DMA�ж� */
    DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, ENABLE);

    /* 5.ʹ�ܴ��ڵ�DMA���ͽӿ� */
    USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);

    /* 6. ����DMA�ж����ȼ� */
    NVIC_InitStructure.NVIC_IRQChannel                   = DMA1_Stream6_IRQn;           
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;          
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1; 
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 7.��ʹ��DMA */                  
    DMA_Cmd(DMA1_Stream6, DISABLE);
}

u8 USART2_Use_DMA_Tx_Flag;
void USART2_DMA_Send(const uint8_t *data,uint16_t size)  
{  
    /* �ȴ����� */
    while (USART2_Use_DMA_Tx_Flag);  
    USART2_Use_DMA_Tx_Flag = 1;  
    /* �������� */
    memcpy(USART2_DMA_TX_Buffer,data,size);  
    /* ���ô������ݳ��� */  
    DMA_SetCurrDataCounter(DMA1_Stream6,size);  
    /* ��DMA,��ʼ���� */  
    DMA_Cmd(DMA1_Stream6,ENABLE);  
} 

void DMA1_Stream6_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_Stream6,DMA_IT_TCIF6) != RESET)   
    {  
        /* �����־λ */
        DMA_ClearFlag(DMA1_Stream6,DMA_IT_TCIF6);
        /* �ر�DMA */
        DMA_Cmd(DMA1_Stream6,DISABLE);
        /* �򿪷�������ж�,ȷ�����һ���ֽڷ��ͳɹ� */
        USART_ITConfig(USART2,USART_IT_TC,ENABLE);  
    }  
}

void USART2_TX_TC_IRQ_Deal(void)	//��������жϴ�����
{  
    if(USART_GetITStatus(USART2, USART_IT_TXE) == RESET)  
    {  
        /* �رշ�������ж�  */ 
        USART_ITConfig(USART2,USART_IT_TC,DISABLE);  
        /* �������  */
        USART2_Use_DMA_Tx_Flag = 0;  
    }    
}

u8 this_dma_type_usart2=0;	//���ܻ������ĸ���
uint32_t this_frame_rx_len_usart2 = 0;	//����֡����
void USART2_RX_IDLE_IRQ_Deal(void)	//���ڿ����жϴ�����
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
		
		this_dma_type_usart2=!DMA_GetCurrentMemoryTarget(DMA1_Stream5);	//��ȡ��ǰ���壬������ΪDMA���˺���Զ��л���һ����������Ҫȡ��
		(void)USART2->SR;	//ֻ��ִ����������ܱ��⴮�ڿ����ж�
		(void)USART2->DR;
		
		InsertQueue(&USART2_Software_FIFO,USART2_DMA_RX_BUF[this_dma_type_usart2],BSP_USART2_DMA_RX_BUF_LEN);
		
	}
}


////FIFO������أ�
//�����
bool InsertQueue(USART_CircularQueueTypeDef *q, const u8* source,u16 len)//USART_FIFO_SIZE
{
	if(len<=GetQueueWriteCount(q)&&len>0)
	{
		if(q->FIFO_Front + len < USART_FIFO_SIZE)	//����������δ�����б߽�
		{
			memcpy(&q->SoftWare_FIFO[q->FIFO_Front], source, len);
			q->FIFO_Front+=len;
		}
		else
		{
			unsigned int len_front=USART_FIFO_SIZE - q->FIFO_Front;
			memcpy(&q->SoftWare_FIFO[q->FIFO_Front], source, len_front);//��front��end
			memcpy(&q->SoftWare_FIFO[0], (source+len_front), len-len_front);	//��ͷ��ʣ��led	//����source+len_front��Ҫ����  
			q->FIFO_Front=len-len_front;
		}
	}
	else
	{
		return false;
	}
	
	return true;
}

//�����в���
bool RemoveQueue(u8* destination,USART_CircularQueueTypeDef *q,u16 len)
{
	if(len<=GetQueueReadCount(q)&&len>0)
	{
		if(q->FIFO_Rear + len < USART_FIFO_SIZE)	//��ȡ�����ݷ�Χδ�����б߽�
		{
			memcpy(destination, &q->SoftWare_FIFO[q->FIFO_Rear], len);
			q->FIFO_Rear+=len;
		}
		else
		{
			unsigned int len_front=USART_FIFO_SIZE - q->FIFO_Rear;
			memcpy(destination, &q->SoftWare_FIFO[q->FIFO_Rear], len_front);//��front��end
			memcpy(destination, &q->SoftWare_FIFO[0], len-len_front);	//��ͷ��ʣ��led
			q->FIFO_Rear=len-len_front;
		}
	}
	else
	{
		return false;
	}
	
	return true;
}

//��һ��Ԫ�ز���
char RemoveAQueue(USART_CircularQueueTypeDef *q)
{
	u16 len=1;
	char re_char;
	if(len<=GetQueueReadCount(q)&&len>0)
	{
		if(q->FIFO_Rear + len < USART_FIFO_SIZE)	//��ȡ�����ݷ�Χδ�����б߽�
		{
			//memcpy(destination, &q->SoftWare_FIFO[q->FIFO_Rear], len);
			
			re_char = q->SoftWare_FIFO[q->FIFO_Rear];
			q->FIFO_Rear+=len;
		}
		else
		{
			unsigned int len_front=USART_FIFO_SIZE - q->FIFO_Rear;
			//memcpy(destination, &q->SoftWare_FIFO[q->FIFO_Rear], len_front);//��front��end
			
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

/**    �Ƴ���ǰ���ж��пɶ�Ԫ��   **/
bool RemoveAllQueue(u8* destination,USART_CircularQueueTypeDef *q)	//�Ƴ���ǰ���ж��пɶ�Ԫ��
{
	u16 len=GetQueueReadCount(q);
	if(len>0)
	{
		if(q->FIFO_Rear + len < USART_FIFO_SIZE)	//��ȡ�����ݷ�Χδ�����б߽�
		{
			memcpy(destination, &q->SoftWare_FIFO[q->FIFO_Rear], len);
			q->FIFO_Rear+=len;
		}
		else
		{
			unsigned int len_front=USART_FIFO_SIZE - q->FIFO_Rear;
			memcpy(destination, &q->SoftWare_FIFO[q->FIFO_Rear], len_front);//��front��end
			memcpy(destination, &q->SoftWare_FIFO[0], len-len_front);	//��ͷ��ʣ��led
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

//��ѯ����
bool QueryQueue(u8* destination, USART_CircularQueueTypeDef const * queue, u16 distance, u16 len)
{
	if(distance+len<=GetQueueReadCount(queue)&&len>0)	//��ȡ��Χ�ڶ��пɶ���Χ��
	{
		unsigned int start=(queue->FIFO_Rear+distance)%USART_FIFO_SIZE;
	
		if(start + len < USART_FIFO_SIZE)	//��ȡ�����ݷ�Χδ�����б߽�
		{
			memcpy(destination, &queue->SoftWare_FIFO[start], len);
			//q->FIFO_Rear+=len;
		}
		else
		{
			unsigned int len_front=USART_FIFO_SIZE - start;
			memcpy(destination, &queue->SoftWare_FIFO[start], len_front);//��front��end
			memcpy(destination, &queue->SoftWare_FIFO[0], len-len_front);	//��ͷ��ʣ��led
			//q->FIFO_Rear=len-len_front;
		}
	}
	else
	{
		return false;
	}
	
	return true;
}

//��ѯ����һ��Ԫ��
char QueryAQueue(USART_CircularQueueTypeDef const * queue, u16 distance)
{
	u16 len=1;
	if(distance+len<=GetQueueReadCount(queue)&&len>0)	//��ȡ��Χ�ڶ��пɶ���Χ��
	{ 
		unsigned int start=(queue->FIFO_Rear+distance)%USART_FIFO_SIZE;
	
		return queue->SoftWare_FIFO[start];
//		if(start + len < USART_FIFO_SIZE)	//��ȡ�����ݷ�Χδ�����б߽�
//		{
//			memcpy(destination, &queue->SoftWare_FIFO[start], len);
//			//q->FIFO_Rear+=len;
//			return q->SoftWare_FIFO[q->FIFO_Rear];
//		}
//		else
//		{
//			unsigned int len_front=USART_FIFO_SIZE - start;
//			memcpy(destination, &queue->SoftWare_FIFO[start], len_front);//��front��end
//			memcpy(destination, &queue->SoftWare_FIFO[0], len-len_front);	//��ͷ��ʣ��led
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

bool DeleteQueue(USART_CircularQueueTypeDef * q, u16 len)	//�������Ƴ����еĻ�����ɾ����memcpy
{
	if(len<=GetQueueReadCount(q)&&len>0)
	{
		if(q->FIFO_Rear + len < USART_FIFO_SIZE)	//��ȡ�����ݷ�Χδ�����б߽�
		{
			//memcpy(destination, &q->SoftWare_FIFO[q->FIFO_Rear], len);
			q->FIFO_Rear+=len;
		}
		else
		{
			unsigned int len_front=USART_FIFO_SIZE - q->FIFO_Rear;
			//memcpy(destination, &q->SoftWare_FIFO[q->FIFO_Rear], len_front);//��front��end
			//memcpy(destination, &q->SoftWare_FIFO[0], len-len_front);	//��ͷ��ʣ��led
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

u16 GetQueueReadCount(USART_CircularQueueTypeDef const * queue)	//����ָ�����ֵ���Խ�Լջ�ؼ�������Ч�ʣ�����const���α��ⱻ�޸�
{
	if(queue->FIFO_Front==queue->FIFO_Rear)	return 0;//return QueueEmpty;
	if(queue->FIFO_Rear-queue->FIFO_Front==1||queue->FIFO_Rear-queue->FIFO_Front==-(USART_FIFO_SIZE-1))	return USART_FIFO_SIZE-1;//return QueueFull;
	return (queue->FIFO_Front - queue->FIFO_Rear)>=0?(queue->FIFO_Front - queue->FIFO_Rear):(queue->FIFO_Front - queue->FIFO_Rear+USART_FIFO_SIZE);
}

u16 GetQueueWriteCount(USART_CircularQueueTypeDef const * queue)	//����ָ�����ֵ���Խ�Լջ�ؼ�������Ч�ʣ�����const���α��ⱻ�޸�
{
	if(queue->FIFO_Front==queue->FIFO_Rear)	return USART_FIFO_SIZE-1;//return QueueEmpty;
	if(queue->FIFO_Rear-queue->FIFO_Front==1||queue->FIFO_Rear-queue->FIFO_Front==-(USART_FIFO_SIZE-1))	return 0;
	return (queue->FIFO_Rear - queue->FIFO_Front-1)>=0?(queue->FIFO_Rear - queue->FIFO_Front-1):(queue->FIFO_Rear - queue->FIFO_Front-1+USART_FIFO_SIZE);
}
