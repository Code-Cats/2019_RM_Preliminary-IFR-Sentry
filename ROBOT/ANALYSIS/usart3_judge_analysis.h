#ifndef __JUDGE_ANALYSIS_H
#define __JUDGE_ANALYSIS_H

#include "main.h"

#define  SOF_FIXED                          0xA5    //固定帧头字节
#define  FRAMEHEADER_SIZE                  0x04      //帧头长度
#define  FRAMEHEADER_LEN                   0x05    //帧头长度

typedef enum{
	GAMEStateId 		= 0x0001, //比赛状态         		 	3  Byte            1HZ
	GameResultId		= 0x0002, //比赛结果		         	1  Byte            比赛结束发送
	RobotSurvivorsId	= 0x0003, //比赛机器人存活数据     		2  Byte            1Hz
	EventDataId			= 0x0101, //场地事件数据，      	 	4  Byte            事件改变后发送 
	SupplyActionId		= 0x0102, //场地补给站动作标识数据 		3  Byte            动作改变后发送 
	SupplyBookingId		= 0x0103, //场地补给站预约子弹数据 		2  Byte            由参赛队发送，上限 10Hz。（ RM 对抗赛尚未开放）
	RobotStateId		= 0x0201, //机器人状态数据，      		15 Byte            10Hz 
	RobotHeatDataId    	= 0x0202, //实时功率热量数据      		14 Byte            50HZ
	RobotPosId       	= 0x0203, //机器人位置数据				16 Byte			   10Hz
	BuffMuskId			= 0x0204, //机器人增益数据		 		1  Byte			   增益状态改变后发送 
	AerialRobotEnergyId	= 0x0205, //空中机器人能量状态数据		3  Byte				10Hz 周期发送（只有空中机器人主控发送 ）
	RobotHurtId			= 0x0206, //伤害状态数据				1  Byte			   伤害发生后发送 
	ShootDataId			= 0x0207, //实时射击数据				6  Byte			   子弹发射后发送 
	RobotInteractiveId	= 0x0301, //机器人间交互数据			n  Byte				发送方触发发送，上限 10Hz 
}comIdType;

typedef __packed struct 
{   
	uint8_t game_type;              //0-3bit：比赛类型 1：RM 对抗赛；2：单项赛；3：RM ICRA   
	uint8_t game_progress;  		//4-7bit：当前比赛阶段 0：未开始比赛；1：准备阶段；2：自检阶段；3：5s 倒计时；4：对战中；5：比赛结算中 
	uint16_t stage_remain_time; 	//当前阶段剩余时间，单位 s 
}ext_game_state_t; 

typedef __packed struct
{ 
	uint8_t winner; //0 平局 1 红方胜利 2 蓝方胜利 
}ext_game_result_t; 

typedef __packed struct 
{   
	uint16_t robot_legion; 

}ext_game_robot_survivors_t;
/*
bit 0：红方英雄机器人； 
bit 1：红方工程机器人； 
bit 2：红方步兵机器人 1； 
bit 3：红方步兵机器人 2； 
bit 4：红方步兵机器人 3； 
bit 5：红方空中机器人； 
bit 6：红方哨兵机器人； 
bit 7：保留 
bit 8：蓝方英雄机器人； 
bit 9：蓝方工程机器人； 
bit 10：蓝方步兵机器人 1； 
bit 11：蓝方步兵机器人 2； 
bit 12：蓝方步兵机器人 3； 
bit 13：蓝方空中机器人； 
bit 14：蓝方哨兵机器人； 
*/
typedef __packed struct
{ 
	uint32_t event_type; 
}ext_event_data_t;
/*
bit 0：己方停机坪占领状态 1 为已占领； 
bit 1：己方补给站 1 号补血点占领状态 1 为已占领； 
bit 2：己方补给站 2 号补血点占领状态 1 为已占领； 
bit 3：己方补给站 3 号补血点占领状态 1 为已占领；
bit 4-5：己方大能量机关状态： 
0 为打击点未占领且大能量机关未激活， 
1 为打击点占领且大能量机关未激活， 
2 为大能量机关已激活， 
3 为大能量机关已激活且打击点被占领； 
bit 6：己方关口占领状态 1 为已占领； 
bit 7：己方碉堡占领状态 1 为已占领； 
bit 8：己方资源岛占领状态 1 为已占领； 
bit 9-10：己方基地防御状态  
3 为基地百分之百防御 ， 
1 为基地有哨兵防御， 
0 为基地无防御
*/


typedef __packed struct 
{   
	uint8_t supply_projectile_id;   //补给站口 ID：1：1 号补给口；2 号补给口 
	uint8_t supply_robot_id;   		//预约机器人 ID：0 为当前无预约，1 为红方英雄预约，以此类推其他机器人 ID 号预约
	uint8_t supply_projectile_step; //子弹口开闭状态：0 为关闭，1 为子弹准备中，2 为子弹下落 
}ext_supply_projectile_action_t;

typedef __packed struct 
{   
	uint8_t supply_projectile_id;    //预约补给站口 ID      0：空闲补给口，依照 1，2 顺序查询补给空闲情况； 
	uint8_t supply_num;  			 //预约子弹数目： 0-50 为预约 50 颗子弹，  51-100 为预约 100 颗子弹，101-150 为预约150 颗子弹， 151-255 为预约 200 颗子弹（上限 200 颗子弹） 
 
}ext_supply_projectile_booking_t; 

typedef __packed struct 
{   
	uint8_t robot_id;  
	uint8_t robot_level;   
	uint16_t remain_HP;   
	uint16_t max_HP;   
	uint16_t shooter_heat17_cooling_rate;   
	uint16_t shooter_heat17_cooling_limit;   
	uint16_t shooter_heat42_cooling_rate;   
	uint16_t shooter_heat42_cooling_limit;   
	uint8_t mains_power_gimbal_output; 
	uint8_t mains_power_chassis_output;  
	uint8_t mains_power_shooter_output;
}ext_game_robot_state_t;
/*
机器人 ID： 
1：红方英雄机器人； 
2： 红方工程机器人； 
3/4/5，红方步兵机器人； 
6，红方空中机器人； 
7，红方哨兵机器人； 
11，蓝方英雄机器人； 
12，蓝方工程机器人； 
13/14/15，蓝方步兵机器人； 
16，蓝方空中机器人； 
17，蓝方哨兵机器人
*/
typedef __packed struct 
{   
	uint16_t chassis_volt;    
	uint16_t chassis_current;    
	float chassis_power;  
	uint16_t chassis_power_buffer;   
	uint16_t shooter_heat17;   
	uint16_t shooter_heat42;  
}ext_power_heat_data_t;

typedef __packed struct 
{  
	float x;  
	float y;  
	float z;  
	float yaw; 
}ext_game_robot_pos_t;

typedef __packed struct 
{   
	uint8_t power_rune_buff; 
}ext_buff_musk_t;
/*
bit 0：机器人血量补血状态 
bit 1：枪口热量冷却加速 
bit 2：机器人防御加成 
bit 3：机器人攻击加成
*/

typedef __packed struct 
{   
	uint8_t energy_point;   
	uint8_t attack_time; 
}aerial_robot_energy_t;

typedef __packed struct 
{   
	uint8_t armor_id;  //当血量变化类型为装甲伤害，代表装甲 ID，其中数值为 0-4 号代表机器人的五个装甲片，其他血量变化类型，该变量数值为 0
	uint8_t hurt_type; //血量变化类型  0x0 装甲伤害扣血； 0x1 模块掉线扣血； 0x2 超枪口热量扣血； 0x3 超底盘功率扣血
}ext_robot_hurt_t;

typedef __packed struct 
{   
	uint8_t bullet_type;    //子弹类型: 1：17mm 弹丸 2：42mm 弹丸
	uint8_t bullet_freq;    //子弹射频 单位 Hz
	float bullet_speed;  	//子弹射速 单位 m/s 
}ext_shoot_data_t; 

typedef __packed struct 
{   
	uint16_t data_cmd_id;    
	uint16_t send_ID;   
	uint16_t receiver_ID; 
}ext_student_interactive_header_data_t; 

//内容 ID:0xD180    用户自定义数据（只能为发送者机器人对应的客户端 ）
typedef __packed struct 
{ 
	float data1;
	float data2;
	float data3; 
	uint8_t masks; 
}client_custom_data_t; 

//内容 ID:0x0201~0x02FF  交互数据 机器人间通信
typedef __packed struct 
{ 
	ext_student_interactive_header_data_t frame_header;
	uint8_t data[100]; 
}robot_interactive_data_t;

void judgeData_analysis(u8 *pata, u8 len);

#endif
