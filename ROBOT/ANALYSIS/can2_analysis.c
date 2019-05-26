#include "can2_analysis.h"
#include "protect.h"

#define ZGYRO_FEEDBACK_MSG_ID    0x401
float Z_Angle_Tem=0.0f;
float ZGyroModuleAngle=0.0f; 
int angel_10x=0;
//LIFT_POSITION_ENCODER chassis_position_encoder[4]={0};

//extern CHASSIS_DATA chassis_Data;

/******************************************
函数名：CAN2_Feedback_Analysis
函数功能：对底盘电机以及云台电机进行数据解析
          得到反馈数据
函数参数：无
函数返回值：无
函数描述：无
*******************************************/
u32 t_yun_count=0;
void CAN2_Feedback_Analysis(CanRxMsg *rx_message)
{		
		CAN_Receive(CAN2, CAN_FIFO0, rx_message);//读取数据	
		switch(rx_message->StdId)
		{
			 case 0x201:
			{
//				Speed_Data_deal(&chassis_Data.lf_wheel_fdbV,rx_message);
//				Position_Data_deal_DIV8(&chassis_Data.lf_wheel_fdbP,&chassis_position_encoder[LF],rx_message);
//				LostCountFeed(&Error_Check.count[LOST_CM1]);
				break;
			}
			case 0x202:
			{
//				Speed_Data_deal(&chassis_Data.rf_wheel_fdbV,rx_message);
//				Position_Data_deal_DIV8(&chassis_Data.rf_wheel_fdbP,&chassis_position_encoder[RF],rx_message);
//				LostCountFeed(&Error_Check.count[LOST_CM2]);
				break;
			}
			case 0x203:
			{
//				Speed_Data_deal(&chassis_Data.lb_wheel_fdbV,rx_message);
//				Position_Data_deal_DIV8(&chassis_Data.lb_wheel_fdbP,&chassis_position_encoder[LB],rx_message);
//				LostCountFeed(&Error_Check.count[LOST_CM3]);
				break;
			}
			case 0x204:
			{
//				Speed_Data_deal(&chassis_Data.rb_wheel_fdbV,rx_message);
//				Position_Data_deal_DIV8(&chassis_Data.rb_wheel_fdbP,&chassis_position_encoder[RB],rx_message);
//				LostCountFeed(&Error_Check.count[LOST_CM4]);
				break;
			}
			case 0x205:	//新英雄改
			{
			  
			  break;
			}
			case 0x206:	//新英雄改
			{
			  
			  break;
			}
			case ZGYRO_FEEDBACK_MSG_ID:
			{
//				ZGyroModuleAngle = 0.01f*((int32_t)(hcan->pRxMsg->Data[0]<<24)|(int32_t)(hcan->pRxMsg->Data[1]<<16) \
//				| (int32_t)(hcan->pRxMsg->Data[2]<<8) | (int32_t)( hcan->pRxMsg->Data[3]))*0.8571428571f; 
				
				Zgyro_Data_deal(&Z_Angle_Tem,rx_message);
				if(Z_Angle_Tem>180)
				{
					ZGyroModuleAngle=Z_Angle_Tem-360*((int)(Z_Angle_Tem-180)/360+1);
				}
				else if(Z_Angle_Tem<-180)
				{
					ZGyroModuleAngle=Z_Angle_Tem-360*((int)(Z_Angle_Tem+180)/360-1);
				}
				else
				{
					ZGyroModuleAngle=Z_Angle_Tem;
				}
				
				ZGyroModuleAngle=-ZGyroModuleAngle;
				
				angel_10x=(int)(ZGyroModuleAngle*10);
				LostCountFeed(&Error_Check.count[LOST_IMU2]);
				DeviceFpsFeed(LOST_IMU2);
			}break;
			 default:
			 break;
		}
}

void RM_ADI_IMU_Cali(void)	//外挂RM陀螺仪进行校准
{
	CanTxMsg TxMessage;
	TxMessage.StdId = 0x404;      //帧ID为传入参数的CAN_ID
	TxMessage.IDE = CAN_ID_STD;    //标准帧
	TxMessage.RTR = CAN_RTR_DATA;  //数据帧
	TxMessage.DLC = 0x08;          //帧长度为8

	TxMessage.Data[0] =0x00;
	TxMessage.Data[1] =0x01;
	TxMessage.Data[2] =0x02;
	TxMessage.Data[3] =0x03;
	TxMessage.Data[4] =0x04;
	TxMessage.Data[5] =0x05;
	TxMessage.Data[6] =0x06;
	TxMessage.Data[7] =0x07;
	 
	CAN_Transmit(CAN2,&TxMessage);
}

void Zgyro_Data_deal(float * zgyro,CanRxMsg * msg)	//外挂陀螺仪反馈数据解析
{
	*zgyro = 0.01f*((int32_t)(msg->Data[0]<<24)|(int32_t)(msg->Data[1]<<16) \
	| (int32_t)(msg->Data[2]<<8) | (int32_t)( msg->Data[3]))*0.8571428571f; 
}
