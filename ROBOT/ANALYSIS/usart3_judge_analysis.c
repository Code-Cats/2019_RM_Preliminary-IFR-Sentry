#include "judge_analysis.h"
#include "CRC_check.h"

ext_game_state_t game_state_judge = {0};
ext_game_result_t game_result_judge = {0};
ext_game_robot_survivors_t game_robot_survivors_judge = {0};
ext_event_data_t event_data_judge = {0};
ext_supply_projectile_action_t supply_action_judge = {0};
ext_supply_projectile_booking_t supply_booking_send = {0};
ext_game_robot_state_t robot_state_judge = {0};
ext_power_heat_data_t heat_data_judge = {0};
ext_game_robot_pos_t robot_pos_judge = {0};
ext_buff_musk_t buff_musk_judge = {0};
ext_robot_hurt_t robot_hurt_judge = {0};
ext_shoot_data_t shoot_data_judge = {0};
ext_student_interactive_header_data_t interactive_data_send = {0};
client_custom_data_t custom_data_send = {0};
robot_interactive_data_t interactive_data_judge = {0};

static void judge_Process(u16 CmdID, u8 *Data, u8 len)
{
	switch(CmdID)
	{
		case GAMEStateId		: memcpy(&game_state_judge, &Data[1], 2); game_state_judge.game_type = Data[0]&0x0F; game_state_judge.game_progress = (Data[0]>>4)&0x0F;break;
		case GameResultId	 	: memcpy(&game_result_judge, Data, 1); break;
		case RobotSurvivorsId	: memcpy(&game_robot_survivors_judge, Data, 2); break;
		case EventDataId 		: memcpy(&event_data_judge, Data, 4); break;
		case SupplyActionId	 	: memcpy(&supply_action_judge, Data, 3);break;
		case RobotStateId	 	: memcpy(&robot_state_judge, Data, 14);robot_state_judge.mains_power_gimbal_output = Data[14]&0x01;robot_state_judge.mains_power_chassis_output = (Data[14]>>1)&0x01;robot_state_judge.mains_power_shooter_output = (Data[14]>>2)&0x01;break;
		case RobotHeatDataId	: memcpy(&heat_data_judge, Data, 14); break;
		case RobotPosId	 		: memcpy(&robot_pos_judge, Data, 16); break;
		case BuffMuskId	 		: memcpy(&buff_musk_judge, Data, 1); break;
		case RobotHurtId	 	: robot_hurt_judge.armor_id = Data[0] & 0x0F;robot_hurt_judge.hurt_type = (Data[0]>>4) & 0x0F;break;
		case ShootDataId	 	: memcpy(&shoot_data_judge, Data, 6); break;
		case RobotInteractiveId	: memcpy(&interactive_data_judge, Data, len); break;
		default:break;
	}
	
}

void judgeData_analysis(u8 *pata, u8 len)
{
	extern Error_check_t Error_Check;
	u8 i = 0;
	u16 Data_len = 0;
	u16 CRC16_recive = 0;
	u8 exact_Data[50] = {0};	

	u16 CmdID = 0;
	
	while(i<=len)
	{
		while(pata[i] != SOF_FIXED)
		{
			i++;
			if(i>=len)
				break;
		}
		
		Data_len = (pata[i+2] <<8)| pata[i+1];	

		if(Get_CRC8_Check(&pata[i],4)== pata[i+4])
		{
			CRC16_recive = (pata[i+7+Data_len+1]<<8) | pata[i+7+Data_len];
			CmdID = (pata[i+6]<<8) | pata[i+5];
			if(Get_CRC16_Check(&pata[i], 7+Data_len) == CRC16_recive)
			{
				/*检查是否在线*/
				LostCountFeed(&(Error_Check.count[LOST_JUDGE]));
				
				memcpy(exact_Data, &pata[i+7], Data_len);
				judge_Process(CmdID, exact_Data, Data_len);			
				i = i+7+Data_len+2;
			}
			else
				i+=5;
		}
		else
			i+=5;
	}
}


void SendJudge_client_custom(void)
{
	u8 sendclientJudgeData[50] = {0};
	u16 CRC16 = 0;
	static u8 sequence = 0;
	
	sendclientJudgeData[0] = SOF_FIXED;
	sendclientJudgeData[1] = 19 & 0xFF;
	sendclientJudgeData[2] = (19>>8)&0xFF;
	sendclientJudgeData[3] = sequence;
	if(sequence <= 0xFE)
		sequence++;
	else
		sequence = 0;
	sendclientJudgeData[4] = Get_CRC8_Check(sendclientJudgeData, 4);
	
	sendclientJudgeData[5] = 0x80;
	sendclientJudgeData[6] = 0xD1;
	sendclientJudgeData[7] = robot_state_judge.robot_id&0xFF;
	sendclientJudgeData[8] = (robot_state_judge.robot_id>>8)&0xFF;
	memcpy(&sendclientJudgeData[9], &custom_data_send, 19);
	CRC16 = Get_CRC16_Check(sendclientJudgeData, 27);
	
	sendclientJudgeData[27] = CRC16&0xFF;
	sendclientJudgeData[28] = (CRC16>>8)&0xFF;
	
	Judge_DMA_Send(sendclientJudgeData, 29);
}
