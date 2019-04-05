#include "Kalman_Filter.h"

float Angle, angle_dot; 	
float Q_angle=0.0001;//陀螺仪噪声协方差
float Q_gyro=0.003;	//陀螺仪漂移噪声的协方差
float R_angle=0.1;	// 加速度计的协方差


float Q_bias, Angle_err;  //Q_bias为陀螺仪漂移

float PP[2][2] = { { 1, 0 },{ 0, 1 } };


float Kalman_Filter(float Accel,float Gyro)
{
	float PCt_0, PCt_1, E;
	float K_0, K_1, t_0, t_1;
	char  C_0 = 1;
	//float P_temp[4] ={0,0,0,0};
	float dt=0.001;   	//微分时间系数
	//Gyro陀螺仪的测量值，Accel加速度计的角度计算值
    //就漂移来说认为每次都是相同的Q_bias=Q_bias；
    //由此得到矩阵
     Angle += (Gyro - Q_bias) * dt; 
    
    // 先验估计误差协方差的计算中间量
//	P_temp[0]=Q_angle - PP[0][1] - PP[1][0]; 
//	P_temp[1]=-PP[1][1];
//	P_temp[2]=-PP[1][1];
//	P_temp[3]=Q_gyro;
    // 先验估计误差协方差的积分
//	PP[0][0] += P_temp[0] * dt;   
//	PP[0][1] += P_temp[1] * dt;   
//	PP[1][0] += P_temp[2] * dt;
//	PP[1][1] += P_temp[3] * dt;
    //根据公式更正后的误差协方差计算公式
    PP[0][0] += Q_angle-(PP[0][1]+PP[1][0])*dt+PP[1][1]*dt*dt; 
	PP[0][1] += -PP[1][1] * dt;
	PP[1][0] += -PP[1][1] * dt;
	PP[1][1] += Q_gyro;
	//矩阵乘法的中间变量
	PCt_0 = C_0 * PP[0][0]; 
	PCt_1 = C_0 * PP[1][0]; 
	//分母计算
	E = R_angle + C_0 * PCt_0;
	//增益值
	K_0 = PCt_0 / E;
	K_1 = PCt_1 / E;
	//Accel是加速度计的值，算出来的角度的测量值。
	Angle_err = Accel - Angle;   
    //对状态的卡尔曼估计。
	Angle    += K_0 * Angle_err;  
    Q_bias   += K_1 * Angle_err;    
	//计算得角速度值，这里由于每次对Q_bias更新，就更准确，比初始矫正后不管 肯定要好很多。
    angle_dot    = Gyro - Q_bias;      
    
	//需要的矩阵计算中间变量    
	t_0 = PCt_0; 		
    t_1 = C_0 * PP[0][1];
	//后验估计更新误差协方差PP
    PP[0][0] -= K_0 * t_0;        
    PP[0][1] -= K_0 * t_1;
    PP[1][0] -= K_1 * t_0;
    PP[1][1] -= K_1 * t_1;
	return Angle;
}


