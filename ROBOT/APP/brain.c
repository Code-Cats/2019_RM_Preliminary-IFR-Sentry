#include "brain.h"
#include "auto_aim.h"
#include "auto_move.h"

/***************************
�ڱ�����1.0
�ڱ�ģʽ���֣�
1��δ���ֵ��� ����ȫ������
2�����ֲ������������
3������Ӣ�ۣ���Ӣ�۴������
***************************/
extern s16 ViceBoard_Position;	//0-2690	//���������ٶ�һ��
u8 no_enemy_move_state=0;
AUTO_OperationStateTypeDef AutoOperationState=NO_ENEMY;

void Auto_Operation(void)
{
	Friction_Speed_Set(21);
	//if()
	
	
	
	switch(AutoOperationState)
	{
		case NO_ENEMY:
		{
			if(no_enemy_move_state==1)
			{
				Auto_Move_Task(2690-40,520);
				if(ViceBoard_Position>2690-60)
				{
					no_enemy_move_state=0;
					
				}
				
			}
			else
			{
				Auto_Move_Task(40,520);
				if(ViceBoard_Position<60)
				{
					no_enemy_move_state=1;
					
				}
			}
			break;
		}
		case FIND_INFANTRY:
		{
			break;
		}
		case FINE_HERO:
		{
			break;
		}
		default:
			break;
	}
}

