#ifndef __MY_IIC_H
#define __MY_IIC_H

#include "stm32f4xx.h"

#define IIC_SCL_GPIO_PORT       GPIOF
#define IIC_SCL_GPIO_PIN				GPIO_Pin_10
#define IIC_SCL_GPIO_CLK				RCC_AHB1Periph_GPIOF

#define IIC_SDA_GPIO_PORT       GPIOI
#define IIC_SDA_GPIO_PIN				GPIO_Pin_9
#define IIC_SDA_GPIO_CLK				RCC_AHB1Periph_GPIOI

#define IIC_GPIO_Clock_FUN			RCC_AHB1PeriphClockCmd


#define IIC_SCL_H    						GPIO_SetBits(IIC_SCL_GPIO_PORT, IIC_SCL_GPIO_PIN)
#define IIC_SCL_L    						GPIO_ResetBits(IIC_SCL_GPIO_PORT, IIC_SCL_GPIO_PIN)

#define IIC_SDA_H    						GPIO_SetBits(IIC_SDA_GPIO_PORT, IIC_SDA_GPIO_PIN)
#define IIC_SDA_L    						GPIO_ResetBits(IIC_SDA_GPIO_PORT, IIC_SDA_GPIO_PIN)

#define READ_SDA   							GPIO_ReadInputDataBit(IIC_SDA_GPIO_PORT,IIC_SDA_GPIO_PIN) 
#define READ_SCL   							GPIO_ReadInputDataBit(IIC_SCL_GPIO_PORT,IIC_SCL_GPIO_PIN) 

void IIC_Init(void);                //初始化IIC的IO口				 
void IIC_Start(void);				//发送IIC开始信号
void IIC_Stop(void);	  			//发送IIC停止信号
void IIC_Send_Byte(u8 txd);			//IIC发送一个字节
u8 IIC_Read_Byte(unsigned char ack);//IIC读取一个字节
u8 IIC_Wait_Ack(void); 				//IIC等待ACK信号
void IIC_Ack(u8 rg);					//IIC发送ACK信号

int IIC_ReadData(u8 dev_addr,u8 reg_addr,u8 *pdata,u8 count);

#endif
