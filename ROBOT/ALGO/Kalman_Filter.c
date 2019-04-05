#include "Kalman_Filter.h"

float Angle, angle_dot; 	
float Q_angle=0.0001;//����������Э����
float Q_gyro=0.003;	//������Ư��������Э����
float R_angle=0.1;	// ���ٶȼƵ�Э����


float Q_bias, Angle_err;  //Q_biasΪ������Ư��

float PP[2][2] = { { 1, 0 },{ 0, 1 } };


float Kalman_Filter(float Accel,float Gyro)
{
	float PCt_0, PCt_1, E;
	float K_0, K_1, t_0, t_1;
	char  C_0 = 1;
	//float P_temp[4] ={0,0,0,0};
	float dt=0.001;   	//΢��ʱ��ϵ��
	//Gyro�����ǵĲ���ֵ��Accel���ٶȼƵĽǶȼ���ֵ
    //��Ư����˵��Ϊÿ�ζ�����ͬ��Q_bias=Q_bias��
    //�ɴ˵õ�����
     Angle += (Gyro - Q_bias) * dt; 
    
    // ����������Э����ļ����м���
//	P_temp[0]=Q_angle - PP[0][1] - PP[1][0]; 
//	P_temp[1]=-PP[1][1];
//	P_temp[2]=-PP[1][1];
//	P_temp[3]=Q_gyro;
    // ����������Э����Ļ���
//	PP[0][0] += P_temp[0] * dt;   
//	PP[0][1] += P_temp[1] * dt;   
//	PP[1][0] += P_temp[2] * dt;
//	PP[1][1] += P_temp[3] * dt;
    //���ݹ�ʽ����������Э������㹫ʽ
    PP[0][0] += Q_angle-(PP[0][1]+PP[1][0])*dt+PP[1][1]*dt*dt; 
	PP[0][1] += -PP[1][1] * dt;
	PP[1][0] += -PP[1][1] * dt;
	PP[1][1] += Q_gyro;
	//����˷����м����
	PCt_0 = C_0 * PP[0][0]; 
	PCt_1 = C_0 * PP[1][0]; 
	//��ĸ����
	E = R_angle + C_0 * PCt_0;
	//����ֵ
	K_0 = PCt_0 / E;
	K_1 = PCt_1 / E;
	//Accel�Ǽ��ٶȼƵ�ֵ��������ĽǶȵĲ���ֵ��
	Angle_err = Accel - Angle;   
    //��״̬�Ŀ��������ơ�
	Angle    += K_0 * Angle_err;  
    Q_bias   += K_1 * Angle_err;    
	//����ý��ٶ�ֵ����������ÿ�ζ�Q_bias���£��͸�׼ȷ���ȳ�ʼ�����󲻹� �϶�Ҫ�úܶࡣ
    angle_dot    = Gyro - Q_bias;      
    
	//��Ҫ�ľ�������м����    
	t_0 = PCt_0; 		
    t_1 = C_0 * PP[0][1];
	//������Ƹ������Э����PP
    PP[0][0] -= K_0 * t_0;        
    PP[0][1] -= K_0 * t_1;
    PP[1][0] -= K_1 * t_0;
    PP[1][1] -= K_1 * t_1;
	return Angle;
}


