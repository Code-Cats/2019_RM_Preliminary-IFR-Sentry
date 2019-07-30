#ifndef __JUDGE_ANALYSIS_H
#define __JUDGE_ANALYSIS_H

#include "main.h"

#define  SOF_FIXED                          0xA5    //�̶�֡ͷ�ֽ�
#define  FRAMEHEADER_SIZE                  0x04      //֡ͷ����
#define  FRAMEHEADER_LEN                   0x05    //֡ͷ����


typedef enum{
	GAMEStateId 		= 0x0001, //����״̬         		 	3  Byte            1HZ
	GameResultId		= 0x0002, //�������		         	1  Byte            ������������
	RobotHPId			= 0x0003, //ȫ��������Ѫ��     			2  Byte            1Hz
	EventDataId			= 0x0101, //�����¼����ݣ�      	 	4  Byte            �¼��ı���� 
	SupplyActionId		= 0x0102, //���ز���վ������ʶ���� 		3  Byte            �����ı���� 
	SupplyBookingId		= 0x0103, //���ز���վԤԼ�ӵ����� 		2  Byte            �ɲ����ӷ��ͣ����� 10Hz���� RM �Կ�����δ���ţ�
	Referee_warningID	= 0x0104, //���о�����Ϣ				2  Byte			   �������
	RobotStateId		= 0x0201, //������״̬���ݣ�      		15 Byte            10Hz 
	RobotHeatDataId    	= 0x0202, //ʵʱ������������      		14 Byte            50HZ
	RobotPosId       	= 0x0203, //������λ������				16 Byte			   10Hz
	BuffMuskId			= 0x0204, //��������������		 		1  Byte			   ����״̬�ı���� 
	AerialRobotEnergyId	= 0x0205, //���л���������״̬����		3  Byte				10Hz ���ڷ��ͣ�ֻ�п��л��������ط��� ��
	RobotHurtId			= 0x0206, //�˺�״̬����				1  Byte			   �˺��������� 
	ShootDataId			= 0x0207, //ʵʱ�������				6  Byte			   �ӵ�������� 
	Bullet_remainingID	= 0x0208, //�ӵ�ʣ�෢����				2  Byte				1Hz
	RobotInteractiveId	= 0x0301, //�����˼佻������			n  Byte				���ͷ��������ͣ����� 10Hz 
}comIdType;

typedef __packed struct 
{   
	uint8_t game_type;              //0-3bit���������� 1��RM �Կ�����2����������3��RM ICRA   
	uint8_t game_progress;  		//4-7bit����ǰ�����׶� 0��δ��ʼ������1��׼���׶Σ�2���Լ�׶Σ�3��5s ����ʱ��4����ս�У�5������������ 
	uint16_t stage_remain_time; 	//��ǰ�׶�ʣ��ʱ�䣬��λ s 
}ext_game_state_t; 

typedef __packed struct
{ 
	uint8_t winner; //0 ƽ�� 1 �췽ʤ�� 2 ����ʤ�� 
}ext_game_result_t; 


typedef __packed struct 
{   
	uint16_t red1_robot_HP;    
	uint16_t red2_robot_HP;    
	uint16_t red3_robot_HP;    
	uint16_t red4_robot_HP;    
	uint16_t red5_robot_HP;    
	uint16_t red7_robot_HP;    
	uint16_t red_base_HP;    
	uint16_t blue1_robot_HP;    
	uint16_t blue2_robot_HP;
	uint16_t blue3_robot_HP;    
	uint16_t blue4_robot_HP;    
	uint16_t blue5_robot_HP;    
	uint16_t blue7_robot_HP;    
	uint16_t blue_base_HP; 
} ext_game_robot_HP_t;

typedef __packed struct
{ 
	uint32_t event_type; 
}ext_event_data_t;
/*
bit 0������ͣ��ƺռ��״̬ 1 Ϊ��ռ�죻 
bit 1����������վ 1 �Ų�Ѫ��ռ��״̬ 1 Ϊ��ռ�죻 
bit 2����������վ 2 �Ų�Ѫ��ռ��״̬ 1 Ϊ��ռ�죻 
bit 3����������վ 3 �Ų�Ѫ��ռ��״̬ 1 Ϊ��ռ�죻
bit 4-5����������������״̬�� 
0 Ϊ�����δռ���Ҵ���������δ��� 
1 Ϊ�����ռ���Ҵ���������δ��� 
2 Ϊ�����������Ѽ�� 
3 Ϊ�����������Ѽ����Ҵ���㱻ռ�죻 
bit 6�������ؿ�ռ��״̬ 1 Ϊ��ռ�죻 
bit 7�������ﱤռ��״̬ 1 Ϊ��ռ�죻 
bit 8��������Դ��ռ��״̬ 1 Ϊ��ռ�죻 
bit 9-10���������ط���״̬  
3 Ϊ���ذٷ�֮�ٷ��� �� 
1 Ϊ�������ڱ������� 
0 Ϊ�����޷���
*/


typedef __packed struct 
{   
	uint8_t supply_projectile_id;   //����վ�� ID��1��1 �Ų����ڣ�2 �Ų����� 
	uint8_t supply_robot_id;   		//ԤԼ������ ID��0 Ϊ��ǰ��ԤԼ��1 Ϊ�췽Ӣ��ԤԼ���Դ��������������� ID ��ԤԼ
	uint8_t supply_projectile_step; //�ӵ��ڿ���״̬��0 Ϊ�رգ�1 Ϊ�ӵ�׼���У�2 Ϊ�ӵ����� 
}ext_supply_projectile_action_t;



typedef __packed struct 
{   
	uint8_t supply_projectile_id;    //ԤԼ����վ�� ID      0�����в����ڣ����� 1��2 ˳���ѯ������������� 
	uint8_t supply_num;  			 //ԤԼ�ӵ���Ŀ�� 0-50 ΪԤԼ 50 ���ӵ���  51-100 ΪԤԼ 100 ���ӵ���101-150 ΪԤԼ150 ���ӵ��� 151-255 ΪԤԼ 200 ���ӵ������� 200 ���ӵ��� 
 
}ext_supply_projectile_booking_t; 

typedef __packed struct 
{   
	uint8_t level;   					//����ȼ�
	uint8_t foul_robot_id;  			//���������ID��1����5������ʱIDΪ0
}ext_referee_warning_t; 
 

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
������ ID�� 
1���췽Ӣ�ۻ����ˣ� 
2�� �췽���̻����ˣ� 
3/4/5���췽���������ˣ� 
6���췽���л����ˣ� 
7���췽�ڱ������ˣ� 
11������Ӣ�ۻ����ˣ� 
12���������̻����ˣ� 
13/14/15���������������ˣ� 
16���������л����ˣ� 
17�������ڱ�������
*/
typedef __packed struct 
{   
	uint16_t chassis_volt;    
	uint16_t chassis_current;    
	float chassis_power;  
	uint16_t chassis_power_buffer;   				//����ʱ250J
	uint16_t shooter_heat17;   
	uint16_t shooter_heat42;  
	uint8_t fps;									//20
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
bit 0��������Ѫ����Ѫ״̬ 
bit 1��ǹ��������ȴ���� 
bit 2�������˷����ӳ� 
bit 3�������˹����ӳ�
*/

typedef __packed struct 
{   
	uint8_t energy_point;   			//���۵���������
	uint8_t attack_time; 				//�ɹ���ʱ�� ��λS�� 30Sʱ��
}aerial_robot_energy_t;

typedef __packed struct 
{   
	uint8_t armor_id;  //��Ѫ���仯����Ϊװ���˺�������װ�� ID��������ֵΪ 0-4 �Ŵ�������˵����װ��Ƭ������Ѫ���仯���ͣ��ñ�����ֵΪ 0
	uint8_t hurt_type; //Ѫ���仯����  0x0 װ���˺���Ѫ�� 0x1 ģ����߿�Ѫ�� 0x2 �����ٿ�Ѫ�� 0x3 ��ǹ��������Ѫ�� 0x4 �����̹��ʿ�Ѫ�� 0x5 װ��ײ����Ѫ
}ext_robot_hurt_t;

typedef __packed struct 
{   
	uint8_t bullet_type;    //�ӵ�����: 1��17mm ���� 2��42mm ����
	uint8_t bullet_freq;    //�ӵ���Ƶ ��λ Hz
	float bullet_speed;  	//�ӵ����� ��λ m/s 
	uint16_t errorData_count;
	uint8_t DataNum;
}ext_shoot_data_t; 

typedef __packed struct 
{   
	uint16_t bullet_remaining_num;   		//�ӵ�ʣ�෢����
} ext_bullet_remaining_t;

typedef __packed struct 
{   
	uint16_t data_cmd_id;    
	uint16_t send_ID;   
	uint16_t receiver_ID; 
}ext_student_interactive_header_data_t; 

//���� ID:0xD180    �û��Զ������ݣ�ֻ��Ϊ�����߻����˶�Ӧ�Ŀͻ��� ��
typedef struct 
{ 
	float data1;
	float data2;
	float data3; 
	uint8_t masks; 
}client_custom_data_t; 

//���� ID:0x0201~0x02FF  �������� �����˼�ͨ��
typedef __packed struct 
{ 
	ext_student_interactive_header_data_t frame_header;
	uint8_t data[100]; 
}robot_interactive_data_t;

void judgeData_analysis(uint8_t *pata, uint8_t len);
void SendJudge_communicate_custom(uint8_t com_data);
void SendJudge_client_custom(void);

#endif
