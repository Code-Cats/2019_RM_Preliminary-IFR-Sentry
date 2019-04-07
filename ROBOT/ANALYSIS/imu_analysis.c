#include "imu_analysis.h"
//#include "Hero_math.h"
#include "arm_math.h"
#include "Kalman_Filter.h"
#include "quaternion.h"
#include "protect.h"


u8 mpu6500_id = 0;

MPU_DATA_T mpu_data = {0};
IMU_T imu = {0};

u8 SPI_WriteReadByte(u8 TxData)
{
	while(SPI_I2S_GetFlagStatus(SPI5, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI5, TxData);
	while(SPI_I2S_GetFlagStatus(SPI5, SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_I2S_ReceiveData(SPI5);
}

u8 mpu_read_byte(u8 reg)
{
	u8 rx = 0;
	u8 tx = 0;
	
	MPU_NSS_LOW;
	tx = reg | 0x80;
	rx = SPI_WriteReadByte(tx);
	MPU_NSS_HIGH;
	
	return rx;
}
u8 mpu_write_byte(u8 reg, u8 data)
{
	u8 tx = 0;
	
	MPU_NSS_LOW;
	tx = reg & 0x7F;
	SPI_WriteReadByte(tx);
	tx = data;
	SPI_WriteReadByte(tx);
	MPU_NSS_HIGH;
	
	return 0;
}

void mpu_read_bytes(u8 reg, u8 *pData, u8 len)
{
	u8 i = 0;
	u8 tx = 0;
	
	MPU_NSS_LOW;
	tx = reg | 0x80;
	SPI_WriteReadByte(tx);
	for(i=0;i<len;i++)
	{
		tx = reg | 0x80;
	    *pData = SPI_WriteReadByte(tx);
		pData++;
		reg++;
	}
	
	MPU_NSS_HIGH;
}

///**
// * @brief  set imu 6500 gyroscope measure range
// * @param  fsr: range(0,±250dps;1,±500dps;2,±1000dps;3,±2000dps)
// * @retval 
// * @usage  call in MPU_device_init() function

//**/
//u8 mpu_set_gyro_fsr(u8 sfr)
//{
//	u8 tmp = 0;
//	
//	tmp = mpu_write_byte(MPU6500_GYRO_CONFIG, sfr << 3);
//	
//	return tmp;
//}

///**
//	* @brief  set imu 6050/6500 accelerate measure range
//  * @param  fsr: range(0,±2g;1,±4g;2,±8g;3,±16g)
//	* @retval 
//  * @usage  call in MPU_device_init() function
//	*/
//u8 mpu_set_accel_fsr(u8 sfr)
//{
//	u8 tmp = 0;
//	
//	tmp = mpu_write_byte(MPU6500_ACCEL_CONFIG, sfr << 3);
//	
//	return tmp;
//}

/**
	* @brief  write IST8310 register through MPU6500's I2C master
  * @param  addr: the address to be written of IST8310's register
  *         data: data to be written
	* @retval   
  * @usage  call in ist8310_init() function
	*/
static void ist_reg_write_by_mpu(uint8_t addr, uint8_t data)
{
    /* turn off slave 1 at first */
    mpu_write_byte(MPU6500_I2C_SLV1_CTRL, 0x00);
    delay_ms(2);
    mpu_write_byte(MPU6500_I2C_SLV1_REG, addr);
    delay_ms(2);
    mpu_write_byte(MPU6500_I2C_SLV1_DO, data);
    delay_ms(2);
    /* turn on slave 1 with one byte transmitting */
    mpu_write_byte(MPU6500_I2C_SLV1_CTRL, 0x80 | 0x01);
    /* wait longer to ensure the data is transmitted from slave 1 */
    delay_ms(10);
}

/**
	* @brief  write IST8310 register through MPU6500's I2C Master
	* @param  addr: the address to be read of IST8310's register
	* @retval 
  * @usage  call in ist8310_init() function
	*/
static uint8_t ist_reg_read_by_mpu(uint8_t addr)
{
    uint8_t retval;
    mpu_write_byte(MPU6500_I2C_SLV4_REG, addr);
    delay_ms(10);
    mpu_write_byte(MPU6500_I2C_SLV4_CTRL, 0x80);
    delay_ms(10);
    retval = mpu_read_byte(MPU6500_I2C_SLV4_DI);
    /* turn off slave4 after read */
    mpu_write_byte(MPU6500_I2C_SLV4_CTRL, 0x00);
    delay_ms(10);
    return retval;
}

/**
	* @brief    initialize the MPU6500 I2C Slave 0 for I2C reading.
* @param    device_address: slave device address, Address[6:0]
	* @retval   void
	* @note     
	*/
static void mpu_master_i2c_auto_read_config(uint8_t device_address, uint8_t reg_base_addr, uint8_t data_num)
{
    /* 
	   * configure the device address of the IST8310 
     * use slave1, auto transmit single measure mode 
	   */
    mpu_write_byte(MPU6500_I2C_SLV1_ADDR, device_address);
    delay_ms(2);
    mpu_write_byte(MPU6500_I2C_SLV1_REG, IST8310_R_CONFA);
    delay_ms(2);
    mpu_write_byte(MPU6500_I2C_SLV1_DO, IST8310_ODR_MODE);
    delay_ms(2);

    /* use slave0,auto read data */
    mpu_write_byte(MPU6500_I2C_SLV0_ADDR, 0x80 | device_address);
    delay_ms(2);
    mpu_write_byte(MPU6500_I2C_SLV0_REG, reg_base_addr);
    delay_ms(2);

    /* every eight mpu6500 internal samples one i2c master read */
    mpu_write_byte(MPU6500_I2C_SLV4_CTRL, 0x03);
    delay_ms(2);
    /* enable slave 0 and 1 access delay */
    mpu_write_byte(MPU6500_I2C_MST_DELAY_CTRL, 0x01 | 0x02);
    delay_ms(2);
    /* enable slave 1 auto transmit */
    mpu_write_byte(MPU6500_I2C_SLV1_CTRL, 0x80 | 0x01);
		/* Wait 6ms (minimum waiting time for 16 times internal average setup) */
    delay_ms(6); 
    /* enable slave 0 with data_num bytes reading */
    mpu_write_byte(MPU6500_I2C_SLV0_CTRL, 0x80 | data_num);
    delay_ms(2);
}

u8 ist8310_init(void)
{
	/* enable iic master mode */
    mpu_write_byte(MPU6500_USER_CTRL, 0x30);
    delay_ms(10);
	  /* enable iic 400khz */
    mpu_write_byte(MPU6500_I2C_MST_CTRL, 0x0d); 
    delay_ms(10);

    /* turn on slave 1 for ist write and slave 4 to ist read */
    mpu_write_byte(MPU6500_I2C_SLV1_ADDR, IST8310_ADDRESS);  
    delay_ms(10);
    mpu_write_byte(MPU6500_I2C_SLV4_ADDR, 0x80 | IST8310_ADDRESS);
    delay_ms(10);

    /* IST8310_R_CONFB 0x01 = device rst */
    ist_reg_write_by_mpu(IST8310_R_CONFB, 0x01);
    delay_ms(10);
    if (IST8310_DEVICE_ID_A != ist_reg_read_by_mpu(IST8310_WHO_AM_I))
        return 1;

	/* soft reset */
    ist_reg_write_by_mpu(IST8310_R_CONFB, 0x01); 
    delay_ms(10);

	/* config as ready mode to access register */
    ist_reg_write_by_mpu(IST8310_R_CONFA, 0x00);
    if (ist_reg_read_by_mpu(IST8310_R_CONFA) != 0x00)
        return 2;
    delay_ms(10);

	/* normal state, no int */
    ist_reg_write_by_mpu(IST8310_R_CONFB, 0x00);
    if (ist_reg_read_by_mpu(IST8310_R_CONFB) != 0x00)
        return 3;
    delay_ms(10);
		
    /* config low noise mode, x,y,z axis 16 time 1 avg */
    ist_reg_write_by_mpu(IST8310_AVGCNTL, 0x24); //100100
    if (ist_reg_read_by_mpu(IST8310_AVGCNTL) != 0x24)
        return 4;
    delay_ms(10);

    /* Set/Reset pulse duration setup,normal mode */
    ist_reg_write_by_mpu(IST8310_PDCNTL, 0xc0);
    if (ist_reg_read_by_mpu(IST8310_PDCNTL) != 0xc0)
        return 5;
    delay_ms(10);

    /* turn off slave1 & slave 4 */
    mpu_write_byte(MPU6500_I2C_SLV1_CTRL, 0x00);
    delay_ms(10);
    mpu_write_byte(MPU6500_I2C_SLV4_CTRL, 0x00);
    delay_ms(10);

    /* configure and turn on slave 0 */
    mpu_master_i2c_auto_read_config(IST8310_ADDRESS, IST8310_R_XL, 0x06);
    delay_ms(100);
    return 0;
}

/**
	* @brief  get the data of IST8310
  * @param  buff: the buffer to save the data of IST8310
	* @retval 
  * @usage  call in mpu_get_data() function
	*/
void ist8310_get_data(uint8_t* buff)
{
    mpu_read_bytes(MPU6500_EXT_SENS_DATA_00, buff, 6); 
}

/**
	* @brief  get the offset data of MPU6500     获取补偿信息
  * @param  
	* @retval 
  * @usage  call in main() function
	*/

void mpu_offset_call(void)
{
	int i;	
	long offset_tmp[9] = {0};
		
	delay_ms(20);
	for (i=0; i<300;i++)
	{
		offset_tmp[0] += mpu_data.acceler.x;
		offset_tmp[1] += mpu_data.acceler.y;
		offset_tmp[2] += mpu_data.acceler.z;
	
		offset_tmp[3] += mpu_data.gyro.x;
		offset_tmp[4] += mpu_data.gyro.y;
		offset_tmp[5] += mpu_data.gyro.z;;

		offset_tmp[6] += mpu_data.magnet.x;
		offset_tmp[7] += mpu_data.magnet.y;
		offset_tmp[8] += mpu_data.magnet.z;
		
		delay_ms(5);
	}
	
	mpu_data.acceler_offset.x = (s16)(offset_tmp[0] / 300);
	mpu_data.acceler_offset.y = (s16)(offset_tmp[1] / 300);
	mpu_data.acceler_offset.z = (s16)(offset_tmp[2] / 300);
	mpu_data.gyro_offset.x = (s16)(offset_tmp[3] / 300);
	mpu_data.gyro_offset.y = (s16)(offset_tmp[4] / 300);
	mpu_data.gyro_offset.z = (s16)(offset_tmp[5] / 300);
	mpu_data.magnet_offset.x = (s16)(offset_tmp[6] / 300);
	mpu_data.magnet_offset.y = (s16)(offset_tmp[7] / 300);
	mpu_data.magnet_offset.z = (s16)(offset_tmp[8] / 300);
	
	imu.norm_g = sqrt(mpu_data.acceler_offset.x*mpu_data.acceler_offset.x+mpu_data.acceler_offset.y*mpu_data.acceler_offset.y+mpu_data.acceler_offset.z*mpu_data.acceler_offset.z);
	
	imu.Gravity = imu.norm_g / 4096.0f;
}

u8 IMU_INIT_OK = 0;
u8 GET_OFFSET_OK = 0;

u8 GetTemp()
{

	if(imu.temp >= imu.control_temp && imu.heat_rate == 100)
		return 0;
	else 
		return 1;
}

void MPU_Device_Init(void)
{
	u8 i = 0;
	u8 MPU6500_Init_Data[11][2] = {
									{ MPU6500_PWR_MGMT_1, 0x80 },     /* Reset Device */ 
									{ MPU6500_SIGNAL_PATH_RESET, 0x07	},   /*陀螺仪、加速度计、温度计复位*/
									{ MPU6500_PWR_MGMT_1, 0x03 },     /* Clock Source - Gyro-Z */ 
									{ MPU6500_PWR_MGMT_2, 0x00 },     /* Enable Acc & Gyro */ 
									{ MPU6500_CONFIG, 0x04 },         /* LPF 41Hz */ 
									{ MPU6500_GYRO_CONFIG, 0x18 },    /* +-2000dps */ //量程
									{ MPU6500_ACCEL_CONFIG, 0x10 },   /* +-8G */ 
									{ MPU6500_ACCEL_CONFIG_2, 0x02 }, /* enable LowPassFilter  Set Acc LPF */ 
									{ MPU6500_USER_CTRL, 0x20 },};    /* Enable AUX  ENABLE DMP*/ 
	imu.control_temp = 45.5f;
	delay_ms(30);
	
	mpu6500_id = mpu_read_byte(MPU6500_WHO_AM_I);
	
	for (i = 0; i < 11; i++)
	{
		mpu_write_byte(MPU6500_Init_Data[i][0], MPU6500_Init_Data[i][1]);
		delay_ms(1);
	}
	
//	mpu_set_gyro_fsr(3);									//+-2000dps
//	delay_ms(2);
//	mpu_set_accel_fsr(2);									//+-8g
//	delay_ms(10);
	
	ist8310_init();
	delay_ms(10);
	IMU_INIT_OK = 1;
	
	//while(GetTemp());        //等待温度到达指定温度
	
	mpu_offset_call();
	delay_ms(5);
	GET_OFFSET_OK = 1;
		
}

void MPU_get_Data(void)
{
	extern Error_check_t Error_Check;
	extern u8 Flag_bspOK;
	u8 mpu_buff[14] = {0};
	u8 ist_buff[6] = {0};                           /* buffer to save IST8310 raw data */
	
	mpu_read_bytes(MPU6500_ACCEL_XOUT_H, mpu_buff, 14);
	
	mpu_data.acceler.x  = (mpu_buff[0] << 8 | mpu_buff[1]);
    mpu_data.acceler.y  = (mpu_buff[2] << 8 | mpu_buff[3]);
    mpu_data.acceler.z  = (mpu_buff[4] << 8 | mpu_buff[5]);
	
    mpu_data.temp = (mpu_buff[6] << 8 | mpu_buff[7]);

    mpu_data.gyro.x = ((mpu_buff[8]  << 8 | mpu_buff[9])  - mpu_data.gyro_offset.x);
    mpu_data.gyro.y = ((mpu_buff[10] << 8 | mpu_buff[11]) - mpu_data.gyro_offset.y);
    mpu_data.gyro.z = ((mpu_buff[12] << 8 | mpu_buff[13]) - mpu_data.gyro_offset.z);
	
	ist8310_get_data(ist_buff);
    memcpy(&mpu_data.magnet.x, ist_buff, 6);
    //memcpy(&imu.ax, &mpu_data.acceler.x, 6 * sizeof(int16_t));
	
	imu.acc.x = mpu_data.acceler.x / 4096.0f;
	imu.acc.y = mpu_data.acceler.y / 4096.0f;
	imu.acc.z = mpu_data.acceler.z / 4096.0f;
	
	imu.temp = 21 + mpu_data.temp / 333.87f;
	/* 2000dps -> rad/s */
	
	imu.angleV.x = mpu_data.gyro.x / 16.384f; 														
    imu.angleV.y = mpu_data.gyro.y / 16.384f; 																	 
    imu.angleV.z = mpu_data.gyro.z / 16.384f; 
	
	if(!(mpu_data.acceler.x==0&&mpu_data.acceler.y&&mpu_data.acceler.z&&mpu_data.gyro.x&&mpu_data.gyro.y&&mpu_data.gyro.z))
		LostCountFeed(&(Error_Check.count[LOST_IMU1]));
	
	if(Flag_bspOK)
		imu_attitude_update();
}

float my_Abs(float x)
{
	if(x>0)
		return x;
	else
		return -x;
}

float imu_DataAnalysis(float tmp)
{
	if(tmp >180.0f)
		return (tmp - 360.0f);
	else if(tmp < -180.0f)
		return (360.0f + tmp);
	else
		return tmp;
}


void imu_attitude_update(void)
{	
	u8 i = 0;
	float gravity_now = 0;
	Axis3f acc = {0};
	Axis3f gyro = {0};
	static float iteration_a_pit[50] = {0};
	static u8 flag_iteration_first = 1;
	
	gravity_now = sqrt(mpu_data.acceler.x*mpu_data.acceler.x+mpu_data.acceler.y*mpu_data.acceler.y+mpu_data.acceler.z*mpu_data.acceler.z);
			

	imu.euler_acc.pitch = asin(mpu_data.acceler.y / gravity_now) * RAD2DEG;
	imu.euler_acc.roll = asin(mpu_data.acceler.z / gravity_now) * RAD2DEG; 
	
	if(imu.acc.x < 0)
	{
		if(imu.acc.y<0)
			imu.euler_acc.pitch = -imu.euler_acc.pitch-180;			
		else
			imu.euler_acc.pitch = -imu.euler_acc.pitch+180;
			
		if(imu.acc.z<0)
			imu.euler_acc.roll = -imu.euler_acc.roll-180;
		else
			imu.euler_acc.roll = -imu.euler_acc.roll+180;
	}
	
	if(flag_iteration_first)
	{
		for(i=0;i<50;i++)
			iteration_a_pit[i] = imu.euler_acc.pitch;

		
		flag_iteration_first = 0;
	}
	else
	{
		for(i=0;i<49;i++)
			iteration_a_pit[i] = iteration_a_pit[i+1];

		
		iteration_a_pit[49] = imu.euler_acc.pitch;
	}
	
	for(i=0;i<50;i++)
		imu.iteration_a_pit += iteration_a_pit[i];
	
	imu.iteration_a_pit = imu.iteration_a_pit/50;
	
		
	imu.euler_gyro.pitch = imu.euler_gyro.pitch - imu.angleV.z*0.001f;
	imu.euler_gyro.yaw = imu.euler_gyro.yaw + imu.angleV.x*0.001f;
	imu.euler_gyro.roll = imu.euler_gyro.roll + imu.angleV.y*0.001f;
	
	imu.euler_gyro.yaw = imu_DataAnalysis(imu.euler_gyro.yaw);
	imu.euler_gyro.pitch = imu_DataAnalysis(imu.euler_gyro.pitch);
	imu.euler_gyro.roll = imu_DataAnalysis(imu.euler_gyro.roll);
				
	imu.Kalman.pitch = -Kalman_Filter(imu.euler_acc.pitch, imu.angleV.y);
		
	/*用四元数结算*/
	acc.z = imu.acc.x;
	acc.x = imu.acc.y;
	acc.y = imu.acc.z;
	
	gyro.z = imu.angleV.x;
	gyro.x = imu.angleV.y;
	gyro.y = imu.angleV.z;
	imuUpdate(acc, gyro, 0.001f);
}

