#include "usart3_judge_analysis.h"
#include "string.h"
#include "CRC_check.h"
#include "protect.h"
#include "heat_limit.h"
#include "brain.h"

#include "friction_wheel.h"

ext_game_state_t game_state_judge = {0};
ext_game_result_t game_result_judge = {0};
ext_game_robot_HP_t game_robot_HP_judge = {0};
ext_event_data_t event_data_judge = {0};
ext_supply_projectile_action_t supply_action_judge = {0};
ext_supply_projectile_booking_t supply_booking_send = {0};
ext_game_robot_state_t robot_state_judge = {0};
ext_power_heat_data_t heat_data_judge = {0};
ext_game_robot_pos_t robot_pos_judge = {0};
ext_buff_musk_t buff_musk_judge = {0};
ext_robot_hurt_t robot_hurt_judge = {0};
ext_shoot_data_t shoot_data_judge = {0};
ext_bullet_remaining_t bullet_remaining_judge = {0};
ext_student_interactive_header_data_t interactive_data_send = {0};
client_custom_data_t custom_data_send = {0};
robot_interactive_data_t interactive_data_judge = {0};

uint16_t judge_errorData_count[2] = {0};

static void judge_Process(uint16_t CmdID, uint8_t *Data, uint8_t len)
{
	static uint32_t last_Systime_heat = 0;
	extern uint32_t time_Sys;
	switch(CmdID)
	{
		case GAMEStateId		: memcpy(&game_state_judge, &Data[1], 2); game_state_judge.game_type = Data[0]&0x0F; game_state_judge.game_progress = (Data[0]>>4)&0x0F;break;
		case GameResultId	 	: memcpy(&game_result_judge, Data, 1); break;
		case RobotHPId			: memcpy(&game_robot_HP_judge, Data, 2); break;
		case EventDataId 		: memcpy(&event_data_judge, Data, 4); break;
		case SupplyActionId	 	: memcpy(&supply_action_judge, Data, 3);break;
		case RobotStateId	 	:
		{
			memcpy(&robot_state_judge, Data, 14);
			robot_state_judge.mains_power_gimbal_output = Data[14]&0x01;
			robot_state_judge.mains_power_chassis_output = (Data[14]>>1)&0x01;
			robot_state_judge.mains_power_shooter_output = (Data[14]>>2)&0x01;
			break;
		}
		case RobotHeatDataId	: 
		{
			memcpy(&heat_data_judge, Data, 14); 
			DeviceFpsFeed(LOST_REFEREE);
			break;
		}
		case RobotPosId	 		: memcpy(&robot_pos_judge, Data, 16); break;
		case BuffMuskId	 		: memcpy(&buff_musk_judge, Data, 1); break;
		case RobotHurtId	 	: robot_hurt_judge.armor_id = Data[0] & 0x0F;robot_hurt_judge.hurt_type = (Data[0]>>4) & 0x0F; RobotHurtCallback(robot_hurt_judge.armor_id,robot_hurt_judge.hurt_type);break;
		case ShootDataId	 	: 
		{
			memcpy(&shoot_data_judge, Data, 6);
			if(shoot_data_judge.DataNum<0xFE)
				shoot_data_judge.DataNum++;
			else
				shoot_data_judge.DataNum = 0;
			
			BulletNum_Simu_ADD(); 
			AutoAdjust_FrictionSpeed(shoot_data_judge.bullet_speed);
			break;
		}
		case Bullet_remainingID : memcpy(&bullet_remaining_judge, Data, 2); break;
		case RobotInteractiveId	: memcpy(&interactive_data_judge, Data, len);Avoid_drones_MsgDeal(&interactive_data_judge); break;
		default:break;
	}
	
}

void judgeData_analysis(uint8_t *pata, uint8_t len)
{
	extern Error_check_t Error_Check;
	uint8_t i = 0;
	uint16_t Data_len = 0;
	uint16_t CRC16_recive = 0;
	uint8_t exact_Data[50] = {0};	

	uint16_t CmdID = 0;
	
	while(i<=len-5)
	{
		while(pata[i] != SOF_FIXED)
		{
			i++;
			if(i>len-5)
				break;
		}
		
		Data_len = ((pata[i+2] <<8)| pata[i+1])&0xFFFF;	

		if(Get_CRC8_Check(&pata[i],4)== pata[i+4])
		{
			CRC16_recive = (pata[i+7+Data_len+1]<<8) | pata[i+7+Data_len];
			CmdID = (pata[i+6]<<8) | pata[i+5];
			if(Get_CRC16_Check(&pata[i], 7+Data_len) == CRC16_recive)
			{
				/*检查是否在线*/
				LostCountFeed(&Error_Check.count[LOST_REFEREE]);
				if(CmdID == ShootDataId)
					shoot_data_judge.errorData_count = 0;
				
				memcpy(exact_Data, &pata[i+7], Data_len);
				judge_Process(CmdID, exact_Data, Data_len);			
				i = i+7+Data_len+2;
			}
			else
			{
				i+=5;
				if(judge_errorData_count[1]<0xFFFF)
						judge_errorData_count[1]++;
			}
		}
		else
		{
			i+=5;
			if(judge_errorData_count[0]<0xFFFF)
				judge_errorData_count[0]++;
		}
	}
}


void SendJudge_client_custom(void)
{
	uint8_t sendclientJudgeData[50] = {0};
	uint16_t CRC16 = 0;
	static uint8_t sequence = 0;
	
	sendclientJudgeData[0] = SOF_FIXED;
	sendclientJudgeData[1] = 0x13;
	sendclientJudgeData[2] = 0;
	sendclientJudgeData[3] = sequence;
	if(sequence <= 0xFE)
		sequence++;
	else
		sequence = 0;
	sendclientJudgeData[4] = Get_CRC8_Check(sendclientJudgeData, 4);
	
	sendclientJudgeData[5] = 0x01;
	sendclientJudgeData[6] = 0x03;
	
	sendclientJudgeData[7] = 0x80;
	sendclientJudgeData[8] = 0xD1;
	
	sendclientJudgeData[9] = robot_state_judge.robot_id&0xFF;
	sendclientJudgeData[10] = 0x00;
	sendclientJudgeData[11] = robot_state_judge.robot_id&0xFF;
	sendclientJudgeData[12] = 0x01;
	
	memcpy(&sendclientJudgeData[13], &custom_data_send, 13);
	CRC16 = Get_CRC16_Check(sendclientJudgeData, 26);
	
	sendclientJudgeData[26] = CRC16&0xFF;
	sendclientJudgeData[27] = (CRC16>>8)&0xFF;
	
	//HAL_UART_Transmit_DMA(&huart6, sendclientJudgeData, 28);
}

void SendJudge_communicate_custom(uint8_t com_data)
{
	uint8_t sendclientJudgeData[50] = {0};
	uint16_t CRC16 = 0;
	static uint8_t sequence = 0;
	
	sendclientJudgeData[0] = SOF_FIXED;
	sendclientJudgeData[1] = 8;
	sendclientJudgeData[2] = 0;
	sendclientJudgeData[3] = sequence;
	if(sequence <= 0xFE)
		sequence++;
	else
		sequence = 0;
	sendclientJudgeData[4] = Get_CRC8_Check(sendclientJudgeData, 4);
	
	sendclientJudgeData[5] = 0x01;
	sendclientJudgeData[6] = 0x03;
	
	sendclientJudgeData[7] = 0x00;
	sendclientJudgeData[8] = 0x02;	
	sendclientJudgeData[9] = robot_state_judge.robot_id&0xFF;
	sendclientJudgeData[10] = (robot_state_judge.robot_id>>8)&0xFF;
	if(robot_state_judge.robot_id < 10)
	{
		sendclientJudgeData[11] = 0x04;		
		sendclientJudgeData[12] = 0x00;		
	}
	else
	{
		sendclientJudgeData[11] = 0x11;	
		sendclientJudgeData[12] = 0x00;			
	}
	
	sendclientJudgeData[13] = 0xFE;	
	sendclientJudgeData[14] = com_data;	

	CRC16 = Get_CRC16_Check(sendclientJudgeData, 14);
	
	sendclientJudgeData[13] = CRC16&0xFF;
	sendclientJudgeData[14] = (CRC16>>8)&0xFF;
	
	//HAL_UART_Transmit_DMA(&huart6, sendclientJudgeData, 15);

}
