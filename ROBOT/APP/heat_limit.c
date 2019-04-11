#include "heat_limit.h"
#include "protect.h"
#include "usart3_judge_analysis.h"
#include "shoot.h"
/////*********************************
////Shoot_Frequency_Limit	//老版的V^2已弃用
////用于自动打击
////*********************************/
////#define UPPER_LIMIT_OF_HEAT 4500	//热量上限
////#define COOLING_PER_SECOND 1500	//每秒冷却
////void Shoot_Frequency_Limit(int* ferquency,u16 rate,u16 heat)	//m/s为单位
////{
////	u16 heating=rate*rate;
////	s16 ferquency_safe=(s16)(COOLING_PER_SECOND/heating);
////	if(*ferquency!=0)
////	{
////		if(heat<5*heating&&heat>=2*heating)	//4倍余量时开始缓冲，以防超出
////		{
////			*ferquency=(u16)ferquency_safe+1;
////		}
////		else if(heat<=heating)	//单发余量触发保护
////		{
////			*ferquency=0;
////		}
////		else if(heat>=heating&&heat<2*heating)
////		{
////			*ferquency=(u16)((ferquency_safe-1)>0?(ferquency_safe-1):0);
////		}
////	}

////}

////u8 Shoot_Heat_Limit(u16 heat,u16 maxheat)	//还应当限制射频
////{
////	if((maxheat-heat)>=44&&time_1ms_count-shoot_Data_Down.last_time>170)	//testPowerHeatData.shooterHeat1
////	{
////		return 1;
////	}
////	else
////	{
////		return 0;
////	}
////}

////u8 Shoot_Heat_Lost_Fre_Limit(void)	//裁判lost情况对射频的限制，反返回1是OK
////{
////	u8 limit_state=0;
////	if(Error_Check.statu[LOST_REFEREE]==1)	//裁判lost
////	{
////		if(time_1ms_count-shoot_Data_Down.last_time>1500)	//大于1s	//临时测试，1秒4发	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
////		{
////			limit_state=1;
////		}
////	}
////	else
////	{
////		limit_state=1;
////	}
////	return limit_state;
////}

extern ext_shoot_data_t shoot_data_judge;
extern u32 time_1ms_count;

RobotHeatDataSimuTypeDef RobotHeatDataSimu17={0};	//高尔夫
//哨兵没有等级
	
void Heat_Simulating(void)	//热量仿真//运行频率10-100HZ
{
	static u16 bulletnum_last=0;
	
	if(RobotHeatDataSimu17.bullet_num!=bulletnum_last)	//我不认为10ms内能射出两发
	{
		RobotHeatDataSimu17.heat+=(s16)shoot_data_judge.bullet_speed;
	}
//	if(level>0&&level<4)
//	{
		RobotHeatDataSimu17.maxheat=SENTRY_HEAT_MAX;
//	}
	
	
	if(time_1ms_count%100==0)	//结算频率10HZ
	{
		if(RobotHeatDataSimu17.heat>2*RobotHeatDataSimu17.maxheat)
		{
			RobotHeatDataSimu17.heat=2*RobotHeatDataSimu17.maxheat;
		}
		
		RobotHeatDataSimu17.heat-=SENTRY_HEAT_COOLING/10;
		
		RobotHeatDataSimu17.heat=RobotHeatDataSimu17.heat<0?0:RobotHeatDataSimu17.heat;
		
//		switch(level)
//		{
//			case 1:
//			{
//				if(RobotHeatDataSimu42.heat>2*MaxHeat[level-1])
//				{
//					RobotHeatDataSimu42.heat=2*MaxHeat[level-1];
//				}
//				
//				RobotHeatDataSimu42.heat-=CoolHeat[level-1]/10;
//				
//				RobotHeatDataSimu42.heat=RobotHeatDataSimu42.heat<0?0:RobotHeatDataSimu42.heat;
//				break;
//			}
//			case 2:
//			{
//				if(RobotHeatDataSimu42.heat>2*MaxHeat[level-1])
//				{
//					RobotHeatDataSimu42.heat=2*MaxHeat[level-1];
//				}
//				
//				RobotHeatDataSimu42.heat-=CoolHeat[level-1]/10;
//				
//				RobotHeatDataSimu42.heat=RobotHeatDataSimu42.heat<0?0:RobotHeatDataSimu42.heat;
//				break;
//			}
//			case 3:
//			{
//				if(RobotHeatDataSimu42.heat>2*MaxHeat[level-1])
//				{
//					RobotHeatDataSimu42.heat=2*MaxHeat[level-1];
//				}
//				
//				RobotHeatDataSimu42.heat-=CoolHeat[level-1]/10;
//				
//				RobotHeatDataSimu42.heat=RobotHeatDataSimu42.heat<0?0:RobotHeatDataSimu42.heat;
//				break;
//			}
//			default:
//			{
//				if(RobotHeatDataSimu42.heat>2*MaxHeat[0])
//				{
//					RobotHeatDataSimu42.heat=2*MaxHeat[0];
//				}
//				
//				RobotHeatDataSimu42.heat-=CoolHeat[0]/10;
//				
//				RobotHeatDataSimu42.heat=RobotHeatDataSimu42.heat<0?0:RobotHeatDataSimu42.heat;
//				break;
//			}
//		}
	}
	
	bulletnum_last=RobotHeatDataSimu17.bullet_num;
}


void BulletNum_Simu_ADD(void)
{
	RobotHeatDataSimu17.bullet_num++;
}

/*****************************
热量限制，包含：
射频热量正相关
根据模拟热量限制当前能否能发弹
裁判丢失则限制射频至绝对安全区	//留5-6发余量
*****************************/
extern s16 bulletSpeedSet;//bulletSpeedSet
extern SHOOT_DATA shoot_Data_Down;
u8 Shoot_Heat_Limit(void)
{
	if(Error_Check.statu[LOST_REFEREE]==0)	//裁判未丢失
	{
		s16 feq_low=(s16)(SENTRY_HEAT_COOLING/bulletSpeedSet)-1;	//-1为了保护
		s16 feq_high=12;//(s16)(SENTRY_HEAT_MAX/bulletSpeedSet)
		s16 feq_now=RobotHeatDataSimu17.heat*(feq_high-feq_low)/SENTRY_HEAT_MAX+feq_low;
		if(RobotHeatDataSimu17.heat<6*bulletSpeedSet)
		{
			feq_now=0;
		}
		s16 tick_now=(s16)(1000/feq_now);
		if(time_1ms_count-shoot_Data_Down.last_time>=tick_now)
		{
			return 1;
		}
	}
	else	//裁判丢失
	{
		s16 feq_low=(s16)(SENTRY_HEAT_COOLING/bulletSpeedSet)-1;	//-1为了保护
		s16 tick_now=(s16)(1000/feq_low);
		if(time_1ms_count-shoot_Data_Down.last_time>=tick_now)
		{
			return 1;
		}
	}
	return 0;
}
