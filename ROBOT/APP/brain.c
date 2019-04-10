#include "brain.h"
#include "auto_aim.h"
#include "auto_move.h"

/***************************
哨兵大脑1.0
哨兵模式划分：
1、未发现敌人 缓速全程运行
2、发现步兵，最近点打击
3、发现英雄，至英雄打击死角
***************************/
extern s16 ViceBoard_Position;	//0-2690	//正方向与速度一致
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

