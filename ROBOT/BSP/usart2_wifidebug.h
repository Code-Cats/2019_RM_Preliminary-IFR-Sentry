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
	u16 FIFO_Front;	//队列头索引
	u16 FIFO_Rear;	//队列尾索引
	u16 Effective_data_length;
	u16 Data_state;	//FIFO Data当前状态	//储存当前是否。。算了还是写通用型的FIFO，队头队尾自动设置，这样while中每处理一个数据尾部就前进一个，剩下空间就多一个。可以达到空间利用率高的特点。不然如果接受到一个长帧，还在处理，新的一帧又来了，如果采用FIFO方法，前面处理过的就已经空出来了，效率高
}USART_CircularQueueTypeDef;	//USART2_Software_FIFO.Data_state==1 有个BUG，如果传输数据正好=DMA_BUFFER_SIZE，就不会进串口中断

/*计划封装为3部分：
1.队列入队，会改变队头索引，进行实际储存，在DMA或者空闲中断中使用
2.队列出队，会改变队尾索引，进行实际读取，在裁判判断完整帧过后进行：内部对一个一个查和一块一块查做优化？
3.查询队列某个值或者某部分值，不改变队列索引，只起到查询作用，用于裁判判断队尾？
*/

bool InsertQueue(USART_CircularQueueTypeDef *q, const u8* source,u16 len);
bool RemoveQueue(u8* destination,USART_CircularQueueTypeDef *q,u16 len);
char RemoveAQueue(USART_CircularQueueTypeDef *q);	//移出一个队列元素并直接返回
bool RemoveAllQueue(u8* destination,USART_CircularQueueTypeDef *q);	//移出当前所有队列可读元素
bool QueryQueue(u8* destination, USART_CircularQueueTypeDef const * queue, u16 start, u16 len);
char QueryAQueue(USART_CircularQueueTypeDef const * queue, u16 distance);	//查询一个队列并直接返回
bool DeleteQueue(USART_CircularQueueTypeDef * q, u16 len);	//就是在移出队列的基础上删除了memcpy
void DeleteAllQueue(void);//直接把队头队尾置零
u16 GetQueueReadCount(USART_CircularQueueTypeDef const * queue);
u16 GetQueueWriteCount(USART_CircularQueueTypeDef const * queue);

#endif
