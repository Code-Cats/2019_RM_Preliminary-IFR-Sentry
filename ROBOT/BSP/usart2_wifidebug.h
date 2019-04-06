#ifndef __USART2_WIFIDEBUG_H
#define __USART2_WIFIDEBUG_H
#include <stdbool.h>
#include "sys.h"

void USART2_wifidebug_Init(uint32_t baud_rate);

void USART2_DMA_Send(const uint8_t *data,uint16_t size);
void USART2_DMA_Tx_Init(void);

typedef enum
{
	QueueEmpty=0,
	QueueFull=-1,
	QueueNormal=1,
}USART_QueueStateTypeDef;

#define USART_FIFO_SIZE 200
typedef struct
{
	uint8_t SoftWare_FIFO[USART_FIFO_SIZE];
	u16 FIFO_Front;	//����ͷ����
	u16 FIFO_Rear;	//����β����
	u16 Effective_data_length;
	u16 Data_state;	//FIFO Data��ǰ״̬	//���浱ǰ�Ƿ񡣡����˻���дͨ���͵�FIFO����ͷ��β�Զ����ã�����while��ÿ����һ������β����ǰ��һ����ʣ�¿ռ�Ͷ�һ�������Դﵽ�ռ������ʸߵ��ص㡣��Ȼ������ܵ�һ����֡�����ڴ����µ�һ֡�����ˣ��������FIFO������ǰ�洦����ľ��Ѿ��ճ����ˣ�Ч�ʸ�
}USART_CircularQueueTypeDef;	//USART2_Software_FIFO.Data_state==1 �и�BUG�����������������=DMA_BUFFER_SIZE���Ͳ���������ж�

/*�ƻ���װΪ3���֣�
1.������ӣ���ı��ͷ����������ʵ�ʴ��棬��DMA���߿����ж���ʹ��
2.���г��ӣ���ı��β����������ʵ�ʶ�ȡ���ڲ����ж�����֡������У��ڲ���һ��һ�����һ��һ������Ż���
3.��ѯ����ĳ��ֵ����ĳ����ֵ�����ı����������ֻ�𵽲�ѯ���ã����ڲ����ж϶�β��
*/

bool InsertQueue(USART_CircularQueueTypeDef *q, const u8* source,u16 len);
bool RemoveQueue(u8* destination,USART_CircularQueueTypeDef *q,u16 len);
char RemoveAQueue(USART_CircularQueueTypeDef *q);	//�Ƴ�һ������Ԫ�ز�ֱ�ӷ���
bool RemoveAllQueue(u8* destination,USART_CircularQueueTypeDef *q);	//�Ƴ���ǰ���ж��пɶ�Ԫ��
bool QueryQueue(u8* destination, USART_CircularQueueTypeDef const * queue, u16 start, u16 len);
char QueryAQueue(USART_CircularQueueTypeDef const * queue, u16 distance);	//��ѯһ�����в�ֱ�ӷ���
bool DeleteQueue(USART_CircularQueueTypeDef * q, u16 len);	//�������Ƴ����еĻ�����ɾ����memcpy
void DeleteAllQueue(void);//ֱ�ӰѶ�ͷ��β����
u16 GetQueueReadCount(USART_CircularQueueTypeDef const * queue);
u16 GetQueueWriteCount(USART_CircularQueueTypeDef const * queue);

#endif
