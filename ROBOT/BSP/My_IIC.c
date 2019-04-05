#include "My_IIC.h"

void IIC_Init(void)
{
	GPIO_InitTypeDef gpio;
	
	IIC_GPIO_Clock_FUN(IIC_SCL_GPIO_CLK | IIC_SDA_GPIO_CLK, ENABLE);

  gpio.GPIO_Pin = IIC_SCL_GPIO_PIN;
  gpio.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
  gpio.GPIO_OType = GPIO_OType_PP;//�������
  gpio.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  gpio.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(IIC_SCL_GPIO_PORT, &gpio);//��ʼ��
	
	gpio.GPIO_Pin = IIC_SDA_GPIO_PIN;
	GPIO_Init(IIC_SDA_GPIO_PORT, &gpio);
	IIC_SCL_H;
	IIC_SDA_H;
}

void SDA_IN(void)
{
	GPIO_InitTypeDef gpio;
	
	gpio.GPIO_Pin = IIC_SDA_GPIO_PIN;
  gpio.GPIO_Mode = GPIO_Mode_IN;//��ͨ���ģʽ
  gpio.GPIO_OType = GPIO_OType_PP;//�������
  gpio.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  gpio.GPIO_PuPd = GPIO_PuPd_UP;//����
	
  GPIO_Init(IIC_SDA_GPIO_PORT, &gpio);//��ʼ��
}	

void SDA_OUT(void)
{
	GPIO_InitTypeDef gpio;
	
	gpio.GPIO_Pin = IIC_SDA_GPIO_PIN;
  gpio.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
  gpio.GPIO_OType = GPIO_OType_PP;//�������
  gpio.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  gpio.GPIO_PuPd = GPIO_PuPd_UP;//����
	
  GPIO_Init(IIC_SDA_GPIO_PORT, &gpio);//��ʼ��
}

void IIC_Delay(unsigned int t)
{
	int i;
	for( i=0;i<t;i++)
	{
		int a = 6;//6
		while(a--);
	}
}

//����IIC��ʼ�ź�
void IIC_Start(void)
{
	SDA_OUT();     //sda�����
	IIC_SCL_H;
	IIC_SDA_H;	
	IIC_Delay(1);
	IIC_SCL_L;
	IIC_Delay(1);
 	IIC_SDA_L;//START:when CLK is high,DATA change form high to low 
	IIC_Delay(1);
	IIC_SCL_L;//ǯסI2C���ߣ�׼�����ͻ�������� 
}	  
//����IICֹͣ�ź�
void IIC_Stop(void)
{
	SDA_OUT();//sda�����
	IIC_SCL_L;
	IIC_SDA_L;//STOP:when CLK is high DATA change form low to high
 	IIC_Delay(1);
	IIC_SCL_H; 
	IIC_SDA_H;//����I2C���߽����ź�
	IIC_Delay(1);							   	
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
u8 IIC_Wait_Ack(void)
{
	u16 ucErrTime=0;
	     //SDA����Ϊ����  
	IIC_SDA_H;IIC_Delay(4);	  
	SDA_IN(); 
	IIC_SCL_H;IIC_Delay(4);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>1000)
		{
			IIC_Stop();
			return 0xff;
		}
	}
	IIC_SCL_L;//ʱ�����0 	   
	return 0;  
} 

//����ACKӦ��
void IIC_Ack(u8 rg)  //rg=1,����Ӧ�� rg=0��������Ӧ��
{
	SDA_OUT();
	IIC_SCL_L;
	SDA_OUT();
	if(rg)
		IIC_SDA_H;
	else
		IIC_SDA_L;
	IIC_Delay(2);
	IIC_SCL_H;
	IIC_Delay(2);
	IIC_SCL_L;
}				 				     
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    IIC_SCL_L;//����ʱ�ӿ�ʼ���ݴ���
    for(t=0;t<8;t++)
    {              
        if(txd&0x80)
				{
					IIC_SDA_H;
				}
				else
				{
					IIC_SDA_L;
				}
				txd<<=1;
				IIC_Delay(2);   //��TEA5767��������ʱ���Ǳ����
				IIC_SCL_H;
				IIC_Delay(2); 
				IIC_SCL_L;	
				IIC_Delay(2);
    }	 
} 	    
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA����Ϊ����
  for(i=0;i<8;i++ )
	{
        IIC_SCL_L; 
        IIC_Delay(2);
				IIC_SCL_H;
        receive<<=1;
        if(READ_SDA)
					receive++;   
				IIC_Delay(1); 
   }					 
    if (ack)
        IIC_Ack(1);//����nACK
    else
        IIC_Ack(0); //����ACK   
		
    return receive;
}

void IIC_WriteBit(u8 Temp)
{
	u8 i;
	SDA_OUT();
	IIC_SCL_L;
	for(i=0;i<8;i++)
	{
		if(Temp&0x80)
		{
			IIC_SDA_H;
		}
		else
		{
			IIC_SDA_L;
		}
		Temp<<=1;
		IIC_Delay(1);
		IIC_SCL_H;
		IIC_Delay(1);
		IIC_SCL_L;
	}

}



u8 IIC_ReadBit(void)
{
	u8 i,Temp=0;
	SDA_IN();
	for(i=0;i<8;i++)
	{
		IIC_SCL_L;
		IIC_Delay(1);
		IIC_SCL_H;
		Temp<<=1;
		if(READ_SDA)
		   Temp++;
		IIC_Delay(1);

	}
	IIC_SCL_L;
	return Temp;
}



//д���ݣ��ɹ�����0��ʧ�ܷ���0xff

int IIC_WriteData(u8 dev_addr,u8 reg_addr,u8 data)

{
	IIC_Start();
	IIC_WriteBit(dev_addr);

	if(IIC_Wait_Ack() == 0xff)
  {
        return 0xff;
  }
	IIC_WriteBit(reg_addr);
	 if(IIC_Wait_Ack() == 0xff)
   {
        return 0xff;
   }
    IIC_WriteBit(data);
    if(IIC_Wait_Ack() == 0xff)
    {
        return 0xff;
    }
	IIC_Stop();

    return 0;

}



//�����ݣ��ɹ�����0��ʧ�ܷ���0xff

int IIC_ReadData(u8 dev_addr,u8 reg_addr,u8 *pdata,u8 count)

{
	u8 i;
    IIC_Start();
    IIC_WriteBit(dev_addr);
	if(IIC_Wait_Ack() == 0xff)
    {
//        printf("error 2F\r\n");
        return 0xff;
    }
    IIC_WriteBit(reg_addr);
	if(IIC_Wait_Ack() == 0xff)
    {
//        printf("error 2G\r\n");
        return 0xff;
    }	
    IIC_Start();
    IIC_WriteBit(dev_addr+1);
	if(IIC_Wait_Ack() == 0xff)
    {
//        printf("error 2H\r\n");
        return 0xff;
    }    
    for(i=0;i<(count-1);i++)
    {
        *pdata=IIC_ReadBit();
        IIC_Ack(0);
        pdata++;
    }
    *pdata=IIC_ReadBit();
    IIC_Ack(1); 
    IIC_Stop(); 
    return 0;    

}
