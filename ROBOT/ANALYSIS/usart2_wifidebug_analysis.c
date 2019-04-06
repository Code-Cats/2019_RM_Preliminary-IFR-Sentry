#include "usart2_wifidebug_analysis.h"
#include "usart2_wifidebug.h"
#include "wifi_debug.h"
#include "delay.h"
//#define SENTRY1

extern u32 time_1ms_count;
extern USART_CircularQueueTypeDef USART2_Software_FIFO;
ESP8266_Config_StateTypeDef Esp8266ConfigState=Wait_PowerOn;
WifiDebug_Connect_StateTypeDef WifidebugConnectState=Unconnected;
WifiDebug_DataSend_StateTypeDef WifidebugDataSendSatet=Config_Fomat;

/**********ESP8266配置************/
//udp监听1815专线通信,发送对象为1813
const char ESP8266_CONFIG_UDPSTART1813[] = "AT+CIPSTART=\"UDP\",\"192.168.1.255\",1813,1815,0";
const char ESP8266_CONFIG_UDPCLOSE[] = "AT+CIPCLOSE";
const char ESP8266_CONFIG_UDPSTART1815[] = "AT+CIPSTART=\"UDP\",\"192.168.1.255\",1815,1815,0";
const char ESP8266_REV_HEAD[] = "+IPD,";
/**********通信协议配置************/
/*       头尾       */
const char WFDBG_HEAD[]="#RM-DT=";
const char WFDBG_END[]="#END";
/*       内网发现       */
const char WFDBG_DCY_ROBOT[]="DCY_ROBOT:";
/*       内网发现回复       */
const char WFDBG_REP_DCY1[]="#RM-DT=REP_DCY:IM=";
const char WFDBG_REP_DCY2[]=";STA=1;#END";
//const char WFDBG_REP_DCY[]="REP_DCY:";//REP_DCY:
const char WFDBG_IM[]="IM=";
const char WFDBG_STA[]="STA=";
/*      HOST握手请求头     */
const char WFDBG_CENT_HEAD[]="#RM-DT=CNET:";//!!!!!
/*       握手开始       */
const char WFDBG_TAR[]="TAR=";
const char WFDBG_TIP[]="TIP=";
const char WFDBG_CIP[]="CIP=";
const char WFDBG_CPT[]="CPT=";
const char WFDBG_SEMICOLON[]=";";
/*       握手过程       */
const char WFDBG_RCNET[]="RCNET:";
const char WFDBG_CNET[]="CNET:";
const char WFDBG_RCNET_OK[]="RCNET:OK;";
const char WFDBG_CNET_OK[]="CNET:OK;";

/*       数据传输       */
const char WFDBG_DATA_INFO[]="DATA_INFO:";
const char WFDBG_TIM[]="TIM=";
const char WFDBG_ABYTES[]="ABYTES=";
const char WFDBG_TYPE[]="TYPE=";

const char WFDBG_INFOOK[]="#RM-DT=INFOOK:#END";	//数据配置完成
const char WFDBG_DATA[]="DATA:";	//数据类型帧
/*         心跳包         */
const char WFDBG_HTBEAT[]="HTBEAT:";	//数据类型帧

/************信息储存***********/
#ifdef SENTRY1
	const char WFDBG_MYTYPE[]="SENTRY1";
#elif defined(SENTRY2)
	const char WFDBG_MYTYPE[]="SENTRY2";
#elif defined(INFANTRY1)
	const char WFDBG_MYTYPE[]="INFANTRY1";
#elif defined(INFANTRY2)
	const char WFDBG_MYTYPE[]="INFANTRY2";
#elif defined(HERO1)
	const char WFDBG_MYTYPE[]="HERO1";
#elif defined(HERO2)
	const char WFDBG_MYTYPE[]="HERO2";
#elif defined(ENGINEER1)
	const char WFDBG_MYTYPE[]="ENGINEER1";
#elif defined(ENGINEER2)
	const char WFDBG_MYTYPE[]="ENGINEER2";
#elif defined(UAV)
	const char WFDBG_MYTYPE[]="UAV";
#elif defined(OTHER1)
	const char WFDBG_MYTYPE[]="OTHER1";
#elif defined(OTHER2)
	const char WFDBG_MYTYPE[]="OTHER2";
#elif defined(OTHER3)
	const char WFDBG_MYTYPE[]="OTHER3";
#endif

const char FrameData_Type[]= DATA_TYPE_DEF  ;
//#RM-DT=DATA_INFO:TIM=<tim_interavl>;ABYTES=<byte_numbers>;TYPE=<s2.s2.s2.s2>;#END
char WFDBUG_DataFormat_INFO[65]={0};

char WFDBG_LocalIP[]="255.255.255.255";
char WFDBG_LocalUnicastPort[]="1815";
char WFDBG_HostUnicastIP[]="255.255.255.255";
char WFDBG_HostUnicastPort[]="1815";


/**基于查询队列的方式与指定字符串比较**/
bool Compare_QueryQueue(USART_CircularQueueTypeDef const * queue, const char ch_source[])	//队列可读比传入元素数目都要小，则直接返回false
{
	u16 ch_len=strlen(ch_source);
	
	if(ch_len>GetQueueReadCount(&USART2_Software_FIFO)) return false;
	
	for(int i=0;i<ch_len;i++)
	{
		if(QueryAQueue(queue,i)!=ch_source[i])	return false;
	}
	return true;
}
/**    查询子字符串在字符串中第一次出现的位置 没有则返回-1   **/
s32 Get_Str_IndexOf(const char * str1, const char * str2)	//基于strstr()
{
	char* index=strstr(str1,str2);
	if(index!=NULL)
	{
		return (s32)(index-str1);
	}
	else
	{
		return -1;
	}
}


/**       寻找当前回传字符中是否有+IPD       **/
bool Find_ESP8266_IPD(USART_CircularQueueTypeDef * queue)	
{
	for(int i=0;i<GetQueueReadCount(queue)-3;i++)
	{
		if(RemoveAQueue(queue)==ESP8266_REV_HEAD[0])	//==? '+'
		{
			if(Compare_QueryQueue(queue, &ESP8266_REV_HEAD[1]))//Compare_QueryQueue(queue, &ESP8266_REV_HEAD[1])
			{
				DeleteQueue(&USART2_Software_FIFO,strlen(&ESP8266_REV_HEAD[1]));	//此时尾部指向IPD,后面的元素
				return true;
			}
		}
	}
	
	return false;
}

/**      基于当前队尾不断向前移动找协议帧头：#RM-DT= 若存在则将队尾移动到帧头后      **/
bool Find_RM_DT_Head(USART_CircularQueueTypeDef * queue)	//基于当前队尾找协议帧头：#RM-DT=
{
	if(RemoveAQueue(&USART2_Software_FIFO)==WFDBG_HEAD[0])	//检查OK	WFDBG_HEAD[0]='#'
	{
		if(Compare_QueryQueue(queue,&WFDBG_HEAD[1])==true)	//#RM-DT=
		{
			DeleteQueue(&USART2_Software_FIFO,strlen(&WFDBG_HEAD[1]));
			
			return true;
//			for(int j=0;j<GetQueueReadCount(&USART2_Software_FIFO);j++)	//到了这里，队尾应当是CNET:TAR=UAC;TIP=192.168.1.4;CIP=192.168.1.5;CPT=1815;#END
//			{
//				if()
//			}
		}
	}
	
	return false;
}

/**         GET贮存port数据         **/
bool Get_Port_String(char * ch_rtn,char ip_endmark)	//chrtn为返回参数，ip_endmark为Port结束的分隔符，在主机请求中查询中是"1815"结束符是"
{
	u16 count=0;//
	*ch_rtn='\0';	//清空字符串
	while(count<5)	//15是最多的 65535
	{
		count++;
		char takechar[2];
		takechar[0]=RemoveAQueue(&USART2_Software_FIFO);
		
		if(takechar[0]==ip_endmark) return true;
		
		takechar[1]='\0';
		strcat(ch_rtn, takechar); 
	}
	
	return false;
}

/**           **/
/**        从连续字符串指针获取字符串参数，存入ch_rtn中 最大100       **/
bool Get_String_FromPoint(char * source, char * ch_rtn,char ip_endmark)	//chrtn为返回参数，ip_endmark为Port结束的分隔符，在主机请求中查询中是"1815"结束符是"
{
	u16 count=0;
	
	while(count<25)	//25是最多的 65535:5 192.168.254.251:15
	{
		char takechar;
		takechar=* (source+count);
		
		if(takechar==ip_endmark)
		{
			*(ch_rtn+count)='\0';	//添加字符串结束符
			return true;	//遇到停止符-结束
		}			
		
		*(ch_rtn+count)=takechar;
		
		count++;
	}
	return false;
}

/**         HOST握手请求匹配函数，阻塞式函数         **/
bool Is_Connect_FormHost(USART_CircularQueueTypeDef * queue)//在该函数调用前应当已经收到数据大于75字节
{
	if(Find_ESP8266_IPD(queue)==true)
	{
		DeleteQueue(queue,2);	//直接跳过*，	//因为形如：+IPD,65:#RM-DT……
		for(int i=0;i<GetQueueReadCount(queue);i++)
		{
			if(Find_RM_DT_Head(queue)==true)	//寻找帧头并向前移动
			{	//到了这里，队尾应当是CNET:TAR=UAC;TIP=192.168.1.4;CIP=192.168.1.5;CPT=1815;#END
//				for(int j=0;j<GetQueueReadCount(&USART2_Software_FIFO);j++)	//到了这里，队尾应当是CNET:TAR=UAC;TIP=192.168.1.4;CIP=192.168.1.5;CPT=1815;#END
//				{
					if(Compare_QueryQueue(queue,WFDBG_CNET)==true)	//确认是否是CNET:
					{
						//Get_Str_IndexOf
						DeleteQueue(queue,strlen(WFDBG_CNET));	//队尾跳到CNET:后面一个元素
						u16 data_lengh=GetQueueReadCount(queue);
						char temp_data[data_lengh+1];
						temp_data[data_lengh]='\0';
						RemoveAllQueue(temp_data, queue);
						s32 tar_index=Get_Str_IndexOf(temp_data, WFDBG_TAR);	//检查是否有tar
						s32 tip_index=Get_Str_IndexOf(temp_data, WFDBG_TIP);	//检查是否有tip
						s32 cip_index=Get_Str_IndexOf(temp_data, WFDBG_CIP);	//检查是否有cip
						s32 cpt_index=Get_Str_IndexOf(temp_data, WFDBG_CPT);	//检查是否有cpt
						if(tar_index==-1||tip_index==-1||cip_index==-1||cpt_index==-1)
						{
							return false;
						}
						else	//都存在
						{
							char type_name[15]={0};
							if(Get_String_FromPoint(temp_data+tar_index+strlen(WFDBG_TAR), type_name, ';'))	//获取tar_type检查是否是自己
							{
								if(strcmp(type_name,WFDBG_MYTYPE)!=0)
								{
									return false;	//不等于0 不相等
								}
							}
							else
							{
								return false;
							}
							
//							if(Get_String_FromPoint(temp_data+tip_index, , ';'))	//获取tap_ip检查是否是自己
//							{
//								
//							}
//							else
//							{
//								return false;
//							}

							memset(WFDBG_HostUnicastIP,0,strlen(WFDBG_HostUnicastIP));	//内存清零
							if(Get_String_FromPoint(temp_data+cip_index+strlen(WFDBG_CIP), WFDBG_HostUnicastIP, ';'))	//获取host ip
							{
								
							}
							else
							{
								return false;
							}
							memset(WFDBG_HostUnicastPort,0,strlen(WFDBG_HostUnicastPort));	//内存清零
							if(Get_String_FromPoint(temp_data+cpt_index+strlen(WFDBG_CPT), WFDBG_HostUnicastPort, ';'))	//获取host port
							{
								
							}
							else
							{
								return false;
							}
							return true;	//只有运行到这里才说明发送的没有错
								
						}
					}
					else
					{
						return false;
					}						
//				}
			}
			
		}
	}
}	

/**       ESP8266 UDP配置         **/
const char ESP8266_CIPCLOSE_CFG[]="AT+CIPCLOSE\r\n";
void ESP8266_CIPClose_Send(void)	//TEST OK
{
	USART2_DMA_Send(ESP8266_CIPCLOSE_CFG,strlen(ESP8266_CIPCLOSE_CFG));
}

const char ESP8266_UDPSTART_CFG1[]="AT+CIPSTART=\"UDP\",\"192.168.1.255\",";	//AT+CIPSTART=\"UDP\",\"192.168.1.255\",1813,1815,0\r\n
//中间插入remoteIP
const char ESP8266_UDPSTART_CFG2[]=",";
//中间插入localIP
const char ESP8266_UDPSTART_CFG3[]=",0\r\n";
bool ESP8266_UDP_Config(char * remoteport,char * localport)	//该函数的运行背景是每隔若干ms运行一次	//TEST OK 2019.4.1
{
	static u8 udp_cfg_state=0;	//定义0为未连接，应发送
	static u32 wait_esp8266_time=0;
	switch(udp_cfg_state)
	{
		case 0:
		{
			ESP8266_CIPClose_Send();
			udp_cfg_state=1;
			break;
		}
		case 1:	//这里大量使用了strlen 不知道耗时怎么样
		{
			char cipstart[strlen(ESP8266_UDPSTART_CFG1)+strlen(ESP8266_UDPSTART_CFG2)+strlen(ESP8266_UDPSTART_CFG3)+10];
			memcpy(cipstart, ESP8266_UDPSTART_CFG1, strlen(ESP8266_UDPSTART_CFG1)+1);	//!!!这里需要加1，不然最后的\0会没有拷贝进去，导致strcat每次都往后面加，越来越大
			strcat(cipstart,remoteport);
			strcat(cipstart,ESP8266_UDPSTART_CFG2);
			strcat(cipstart,localport);
			strcat(cipstart,ESP8266_UDPSTART_CFG3);
			USART2_DMA_Send(cipstart,strlen(cipstart));
			DeleteAllQueue();	//删除所有消息
			USART2_Software_FIFO.Data_state=0;	//当前有效数据已经读完了
			udp_cfg_state=2;
			break;
		}
		case 2:
		{
			wait_esp8266_time++;
			if(wait_esp8266_time>10)
			{
				DeleteAllQueue();
				udp_cfg_state=0;
				wait_esp8266_time=0;
				return false;
			}
			//这里应该做个保护
			if(USART2_Software_FIFO.Data_state==1)	//USART2_Software_FIFO.Data_state==1  有个BUG，如果传输数据正好=DMA_BUFFER_SIZE，就不会进串口中断
			for(int i=0;i<GetQueueReadCount(&USART2_Software_FIFO);i++)
			{
				if(RemoveAQueue(&USART2_Software_FIFO)=='O')	//检查OK	//方法1，没有调用Compare_QueryQueue
				{
					if(QueryAQueue(&USART2_Software_FIFO,0)=='K')
					{
						DeleteAllQueue();
						udp_cfg_state=0;
						wait_esp8266_time=0;
						return true;
					}
					
				}
			}
			break;
		}
	}
	return false;
}

/**     规定调用此参数时队列尾部应为IP地址第一位     TEST OK 2019.4.1**/
bool IP_String_Deal(char * ch_rtn,char ip_endmark)	//chrtn为返回参数，ip_endmark为IP地址结束的分隔符，在ESP8266查询中是"192.168.1.100"结束符是"
{
	u16 count=0;//
	*ch_rtn='\0';	//清空字符串
	while(count<15)	//15是最多的 192.168.254.251
	{
		count++;
		char takechar[2];
		takechar[0]=RemoveAQueue(&USART2_Software_FIFO);
		
		if(takechar[0]==ip_endmark) return true;
		
		takechar[1]='\0';
		strcat(ch_rtn, takechar); 
	}
	
	return false;
}

const char ESP8266_GETLOCALIP_ASK[]="AT+CIFSR\r\n";
const char ESP8266_GETLOCALIP_REP1[]="+CIFSR:STAIP,\"";
bool ESP8266_GetLocalIP(void)	//获取本地IP	//放在每10ms运行一次的函数里  WFDBG_LocalIP TEST OK 2019.4.1
{
	static bool getIpState=false;
	static u8 getlocalip_state=0;
	if(getIpState==false)
	{
		switch(getlocalip_state)
		{
			case 0:
			{
				USART2_DMA_Send(ESP8266_GETLOCALIP_ASK,strlen(ESP8266_GETLOCALIP_ASK));	//回复+CIFSR:STAIP,"192.168.1.100"
				getlocalip_state=1;
				break;
			}
			case 1:
			{
				
				if(USART2_Software_FIFO.Data_state==1)
				{
					if(GetQueueReadCount(&USART2_Software_FIFO)>=strlen(ESP8266_GETLOCALIP_REP1)+12)
					{
						for(int i=0;i<GetQueueReadCount(&USART2_Software_FIFO)-strlen(ESP8266_GETLOCALIP_REP1);i++)	//这里不能-12，因为for循环会remove.或者直接在检测OK时直接return、break
						{
							if(RemoveAQueue(&USART2_Software_FIFO)=='+')	//Remove语句保证每次向前推进
							{
								if(Compare_QueryQueue(&USART2_Software_FIFO,&ESP8266_GETLOCALIP_REP1[1]))	//上一个if已经检测了ESP8266_CWJAP_OK[0]
								{//检测完成
									DeleteQueue(&USART2_Software_FIFO, strlen(&ESP8266_GETLOCALIP_REP1[1]));	//执行完此句此时指针应该在192.168.1.100首部
									USART2_Software_FIFO.Data_state=0;	//当前有效数据已经读完了
									getlocalip_state=2;
									return false;
								}
								
							}
						}

					}
					else
					{
						
					}
				}
				
				break;
			}
			case 2:
			{
				if(IP_String_Deal(WFDBG_LocalIP,'"')==true)	//成功获取到LocalIP
				{
					getlocalip_state=0;
					getIpState=true;
					DeleteAllQueue();	//结束时清空队列
				}
				break;
			}
		}
		
		
		
	}
	else
	{
		return true;
	}
	return false;
}


/**     开机对ESP8266进行检查     **/
const char ESP8266_CWJAP_ASK[]="AT+CWJAP?\r\n";
const char ESP8266_CWJAP_OK[]="+CWJAP:\"IFR_ROBOT\"";
void ESP8266_ConfigCheck(void)
{
	static u32 time_last=0;
	
	if(time_1ms_count-time_last>=22)	//保证执行周期至少10ms  沙雕ESP8266 10ms反应不过来
	{
		time_last=time_1ms_count;
		switch(Esp8266ConfigState)
		{
			case Wait_PowerOn:
			{
				//u16 time_count=0;
				USART2_DMA_Send("AT\r\n",4);
				delay_ms(10);
				if(USART2_Software_FIFO.Data_state==1)	//USART2_Software_FIFO.Data_state==1  有个BUG，如果传输数据正好=DMA_BUFFER_SIZE，就不会进串口中断
				{
					for(int i=1;i<GetQueueReadCount(&USART2_Software_FIFO);)	//!!!!!!!!!!!!!!!BUG!!!!!!!!!!!!BUG!!!!!!!!!!BUG!!!!!!!!!!BUG!!!!!!!!for(int i=0;i<GetQueueReadCount(&USART2_Software_FIFO);i++) 中如果有RemoveAQueue，因为for每次循环都会计算，所以只会循环一半的次数
					{
						if(RemoveAQueue(&USART2_Software_FIFO)=='O'&&QueryAQueue(&USART2_Software_FIFO,0)=='K')	//检查OK	//方法1，没有调用Compare_QueryQueue
						{
							DeleteAllQueue();
							USART2_Software_FIFO.Data_state=0;	//当前有效数据已经读完了
							Esp8266ConfigState=Wait_ConnectWIFI;
						}
					}
				}
				break;
			}
			case Wait_ConnectWIFI:	//strlen大量使用会不会造成速度慢？
			{
				USART2_DMA_Send(ESP8266_CWJAP_ASK,strlen(ESP8266_CWJAP_ASK));	//有两种情况："No AP" "+CWJAP:"IFR_ROBOT","50:fa:84:1e:1f:4c",13,-30"
				if(USART2_Software_FIFO.Data_state==1)
				{
					if(GetQueueReadCount(&USART2_Software_FIFO)>=strlen(ESP8266_CWJAP_OK))
					{
						u16 queuereadcount=GetQueueReadCount(&USART2_Software_FIFO);
						for(int i=0;i<queuereadcount-strlen(ESP8266_CWJAP_OK);i++)
						{
							if(RemoveAQueue(&USART2_Software_FIFO)=='+')	//Remove语句保证每次向前推进
							{
								if(Compare_QueryQueue(&USART2_Software_FIFO,&ESP8266_CWJAP_OK[1]))	//上一个if已经检测了ESP8266_CWJAP_OK[0]
								{//检测完成
									DeleteAllQueue();
									USART2_Software_FIFO.Data_state=0;	//当前有效数据已经读完了
									Esp8266ConfigState=Wait_GetLocalIP;
								}
								
							}
						}

					}
					else
					{
						
					}
				}
				break;
			}
			case Wait_GetLocalIP:
			{
				if(ESP8266_GetLocalIP()==true)
				{
					DeleteAllQueue();
					USART2_Software_FIFO.Data_state=0;	//当前有效数据已经读完了
					Esp8266ConfigState=Wait_MonitorUDP;
				}
				break;
			}
			case Wait_MonitorUDP:
			{
				if(ESP8266_UDP_Config("1813","1815"))	//向广播信道发送信息
				{
					Esp8266ConfigState=ESP8266_OK;
					USART2_Software_FIFO.Data_state=0;	//当前有效数据已经读完了
				}
				break;
			}
			
			case ESP8266_OK:	//后面这里需要优化  对不同网段***.***.***.255
			{
				
				break;
			}
		}
	}
	
}


/**        内网发现回复的发送函数          **/
void WFDBG_ReplyDiscover_SendDex(void)	//后面加上/0
{
	u16 rep_dcy1_len=strlen(WFDBG_REP_DCY1);
	u16 type_len=strlen(WFDBG_MYTYPE);
	u16 rep_dcy2_len=strlen(WFDBG_REP_DCY2);
	char temp_send[rep_dcy1_len+type_len+rep_dcy2_len+1];
	memcpy(temp_send,WFDBG_REP_DCY1,rep_dcy1_len);
	memcpy(temp_send+rep_dcy1_len,WFDBG_MYTYPE,type_len);
	memcpy(temp_send+rep_dcy1_len+type_len,WFDBG_REP_DCY2,rep_dcy2_len+1);	//+1把末尾的\0也复制进去
//	for(int i=0;i<strlen(WFDBG_HEAD))
	USART2_DMA_Send(temp_send,rep_dcy1_len+type_len+rep_dcy2_len+1);	//长度36字节+1字节 '\0'
}
/**     内网发现的自动回复  暂未启用！   **/
const char WFDBG_DCYROBOT[]="#RM-DT=DCY_ROBOT";	//主机的问询
void WFDBG_AutoReply_Discover(void)	//暂未启用！！！
{
	if(USART2_Software_FIFO.Data_state==1)
	{
		if(GetQueueReadCount(&USART2_Software_FIFO)>=strlen(WFDBG_DCYROBOT)+3)
		{
			for(int i=0;i<GetQueueReadCount(&USART2_Software_FIFO)-strlen(WFDBG_DCYROBOT);i++)	//这里不能-12，因为for循环会remove.或者直接在检测OK时直接return、break
			{
				if(RemoveAQueue(&USART2_Software_FIFO)=='#')	//Remove语句保证每次向前推进
				{
					if(Compare_QueryQueue(&USART2_Software_FIFO,&WFDBG_DCYROBOT[1]))	//上一个if已经检测了ESP8266_CWJAP_OK[0]
					{//检测完成
						DeleteQueue(&USART2_Software_FIFO, strlen(&WFDBG_DCYROBOT[1]));	//执行完此句此时指针应该在192.168.1.100首部
						USART2_Software_FIFO.Data_state=0;	//当前有效数据已经读完了
						//getlocalip_state=2;
						return;
					}
					
				}
			}

		}
		else
		{
			
		}
	}
}


/**         ESP8266 UPD非透传模式发送字节        **/
const char ESP8266_STARTSENDBYTES_DEX1[]="AT+CIPSENDEX=";	//小于100	//esp8266这个功能就是沙雕，发送的包写几个就有几个，而不是遇到\0结束
//中间插入发送数量
const char ESP8266_STARTSENDBYTES_DEX2[]="\r\n";
bool ESP8266_UDPSendBytesDEX(char len[])	//该函数调用处不应太快 len形如"12"
{
	static u8 send_state=0;
	static u32 time_count=0;
	switch(send_state)
	{
		case 0:
		{
			char strsend[20]={0};
			DeleteAllQueue();	//先清空，不留历史痕迹
			USART2_Software_FIFO.Data_state=0;	//先清空，不留历史痕迹
			strcat(strsend,ESP8266_STARTSENDBYTES_DEX1);
			strcat(strsend,len);
			strcat(strsend,ESP8266_STARTSENDBYTES_DEX2);
			USART2_DMA_Send(strsend,strlen(strsend));
			send_state=1;
		}
		case 1:
		{
			time_count++;
			if(time_count>10)
			{
				time_count=0;
				send_state=0;
			}
			if(USART2_Software_FIFO.Data_state==1)
			{
				//if(GetQueueReadCount(&USART2_Software_FIFO)>=strlen(ESP8266_GETLOCALIP_REP1)+12)//因为只检测一个'>'所以不设置这个if判断
				//{
					for(int i=0;i<GetQueueReadCount(&USART2_Software_FIFO);i++)	//这里不能-12，因为for循环会remove.或者直接在检测OK时直接return、break
					{
						if(RemoveAQueue(&USART2_Software_FIFO)=='>')	//Remove语句保证每次向前推进
						{
							send_state=0;	//复位
							time_count=0;
							//DeleteAllQueue();	//清空，不留历史痕迹 这里不能清空，因为此时有可能主机此时发起握手连接
							USART2_Software_FIFO.Data_state=0;	//清空，不留历史痕迹
							return true;
						}
					}

				//}
			}
		}
	}
	return false;
}

/***        内网发现的主动式发送在线  回值的意思是当前是否处于安全期     ***/
bool WFDBG_ActiveReply_Discover(void)	//外部调用周期20ms  回值的意思是当前是否处于安全期
{
	static u8 send_state=0;
	static u32 time_count=0;
	switch(send_state)
	{
		case 0:
		{
			if(ESP8266_UDPSendBytesDEX("36")==true)
			{
				send_state=1;
//				WFDBG_ReplyDiscover_SendDex();
//				WifidebugConnectState=Wait_connect;
			}
			break;
		}
		case 1:
		{
			WFDBG_ReplyDiscover_SendDex();
			send_state=2;
			break;
		}
		case 2:
		{
			time_count++;
			if(time_count>85)
			{
				send_state=0;
				time_count=0;
			}
			else if(time_count>5)
			{
				return true;	//此时是安全的，已经执行了一遍内网回复
			}
			
			break;
		}
	}
	return false;
}
/**          ESP8266进入/退出透传模式           **/
const char ESP8266_OSPF_ASK[]="AT+CIPMODE?\r\n";
const char ESP8266_OSPF_CFG0[]="AT+CIPMODE=0\r\n";
const char ESP8266_OSPF_CFG1[]="AT+CIPMODE=1\r\n";
const char ESP8266_OSPF_REPLY[]="+CIPMODE:";	//完整的是+CIPMODE:1
bool ESP8266_OSPF_Config(u8 mode)	//设置  OSPF模式为0 外部调用周期应为20ms
{
	static u8 ospf_state=0;
	switch(ospf_state)
	{
		case 0:	//查询当前是什么状态
		{
			USART2_DMA_Send(ESP8266_OSPF_ASK,strlen(ESP8266_OSPF_ASK));
			ospf_state=1;
			break;
		}
		case 1:
		{
			if(USART2_Software_FIFO.Data_state==1)	//USART2_Software_FIFO.Data_state==1  有个BUG，如果传输数据正好=DMA_BUFFER_SIZE，就不会进串口中断
			{
				for(int i=0;i<GetQueueReadCount(&USART2_Software_FIFO)-strlen(ESP8266_OSPF_REPLY);i++)
				{
					if(RemoveAQueue(&USART2_Software_FIFO)==ESP8266_OSPF_REPLY[0])	//Remove语句保证每次向前推进
					{
						if(Compare_QueryQueue(&USART2_Software_FIFO,&ESP8266_OSPF_REPLY[1]))	//上一个if已经检测了ESP8266_CWJAP_OK[0]
						{//检测完成
							DeleteQueue(&USART2_Software_FIFO, strlen(&ESP8266_OSPF_REPLY[1]));	//执行完此句此时指针应该在192.168.1.100首部
							USART2_Software_FIFO.Data_state=0;	//当前有效数据已经读完了
							if(RemoveAQueue(&USART2_Software_FIFO)==mode)	//在这里队尾应该指向mode
							{
								ospf_state=0;
								DeleteAllQueue();
								return true;
							}
							ospf_state=2;	//模式不是透传模式，应做配置
							return false;
						}
						
					}
				}
			}
			break;
		}
		case 2:
		{
			if(mode==0)
			{
				USART2_DMA_Send(ESP8266_OSPF_CFG0,strlen(ESP8266_OSPF_CFG0));
				ospf_state=3;
			}
			else if(mode==1)
			{
				USART2_DMA_Send(ESP8266_OSPF_CFG1,strlen(ESP8266_OSPF_CFG1));
				ospf_state=3;
			}
			else
			{
				return false;
			}
			
			break;
		}
		case 3:
		{
			if(USART2_Software_FIFO.Data_state==1)	//USART2_Software_FIFO.Data_state==1  有个BUG，如果传输数据正好=DMA_BUFFER_SIZE，就不会进串口中断
			{
				for(int i=0;i<GetQueueReadCount(&USART2_Software_FIFO);i++)
				{
					if(RemoveAQueue(&USART2_Software_FIFO)=='O'&&QueryAQueue(&USART2_Software_FIFO,0)=='K')	//检查OK	//方法1，没有调用Compare_QueryQueue
					{
						USART2_Software_FIFO.Data_state=0;	//当前有效数据已经读完了
						ospf_state=0;
						DeleteAllQueue();
						return true;
					}
				}
				USART2_Software_FIFO.Data_state=0;	//当前有效数据已经读完了
			}
		}
	}
	return false;
}

/**       Wait_Host_OK         **/
const char WFDBG_RCNET_OK_REPLY[]="#RM-DT=RCNET:OK;#END";
const char WFDBG_CNET_REPLY_FORM_HOST[]="#RM-DT=CNET:OK;#END";
bool WFDBG_RCNET_Wait_HostOK(void)	//外部调用至少20ms周期！
{
	static u8 rcnet_state=0;
	static u32 overtime_count=0;
	switch(rcnet_state)	//WFDBG_HEAD+WFDBG_RCNET+ "TIP=192.168.1.74;CIP=192.168.1.25;CPT=18"  +WFDBG_SEMICOLON
	{
		case 0:
		{
			char send_str[70];
			send_str[0]='\0';
			strcat(send_str, WFDBG_HEAD);	//	#RM-DT=
			strcat(send_str, WFDBG_RCNET);	//	RCNET:
			strcat(send_str, WFDBG_TIP);	//	TIP=
			strcat(send_str, WFDBG_HostUnicastIP);	//char WFDBG_LocalUnicastPort[]="1815";  char WFDBG_HostUnicastIP[]="255.255.255.255";  char WFDBG_HostUnicastPort[]="1815";
			strcat(send_str, WFDBG_SEMICOLON);//	;
			strcat(send_str, WFDBG_CIP);	//	CIP=
			strcat(send_str, WFDBG_LocalIP);
			strcat(send_str, WFDBG_SEMICOLON);//	;
			strcat(send_str, WFDBG_CPT);	//	CPT==
			strcat(send_str, WFDBG_LocalUnicastPort);
			strcat(send_str, WFDBG_SEMICOLON);	//	;
			strcat(send_str, WFDBG_END);	//#END
			USART2_DMA_Send(send_str,strlen(send_str));
			rcnet_state=1;
			break;
		}
		case 1:	//#RM-DT=RCNET:OK;#END  WFDBG_RCNET_OK_REPLY
		{
			USART2_DMA_Send(WFDBG_RCNET_OK_REPLY,strlen(WFDBG_RCNET_OK_REPLY));
			DeleteAllQueue();	//清空接受数据，其实如果一切正常，是不用清空的
			USART2_Software_FIFO.Data_state=0;	//清空有效数据
			rcnet_state=2;
			break;
		}
		case 2:	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////这里待添加超时重传
		{	//检测主机的回应
			if(USART2_Software_FIFO.Data_state==1)	//USART2_Software_FIFO.Data_state==1  有个BUG，如果传输数据正好=DMA_BUFFER_SIZE，就不会进串口中断
			{
				for(int i=1;i<GetQueueReadCount(&USART2_Software_FIFO);)
				{
					if(RemoveAQueue(&USART2_Software_FIFO)==WFDBG_CNET_REPLY_FORM_HOST[0])	//Remove语句保证每次向前推进
					{
						if(Compare_QueryQueue(&USART2_Software_FIFO,&WFDBG_CNET_REPLY_FORM_HOST[1]))	//上一个if已经检测了ESP8266_CWJAP_OK[0]
						{//检测完成
							DeleteQueue(&USART2_Software_FIFO, strlen(&WFDBG_CNET_REPLY_FORM_HOST[1]));	
							DeleteAllQueue();
							USART2_Software_FIFO.Data_state=0;	//当前有效数据已经读完了
							rcnet_state=0;
							overtime_count=0;
							return true;
						}
						
					}
				}
				USART2_Software_FIFO.Data_state=0;	//当前有效数据已经读完了
			}
			overtime_count++;
			if(overtime_count>150)	//2s
			{
				overtime_count=0;
				rcnet_state=0;
			}
			break;
		}
	}
	return false;
}
////////////////////////////////////////////////////////
extern u16 copy_index;
extern char Apack_Data[FRAME_BYTES*FRAMENUM_IN_PACK+20+1];	//FRAME_BYTES+20+1
////////////////////////////////////////////////////////

const char ESP8266_START_OSPF_CFG[]="AT+CIPSEND\r\n";
/**    ESP8266基础配置好后     **/
void Wifi_Debug_Main(void)//主运行状态机 放在while中
{
	static u32 time_record=0;
	if(Esp8266ConfigState==ESP8266_OK)	//ESP8266已经配置好	//下面应该发送在线包
	{
		switch(WifidebugConnectState)
		{
			case Unconnected:
			{
				if(time_1ms_count-time_record>=24)
				{
					time_record=time_1ms_count;
					
					if(WFDBG_ActiveReply_Discover()==true)	//如果是安全的，不在内网回复中。就可以开始检测#RM-DT了
					{
						if(GetQueueReadCount(&USART2_Software_FIFO)>70)	//如果接收数据大于75，符合握手特征
						{
							if(Is_Connect_FormHost(&USART2_Software_FIFO)==true)
							{
								WifidebugConnectState=Wait_connect;
							}
						}
					}
					
				}
				break;
			}
			case Wait_connect:	//这一步该连接远端1815
			{
				if(time_1ms_count-time_record>=22)
				{
					time_record=time_1ms_count;
					if(ESP8266_UDP_Config(WFDBG_HostUnicastPort, WFDBG_LocalUnicastPort))	//连接到单播端口
					{
						WifidebugConnectState=Wait_OSPF1;
					}
				}
				
				break;
			}
			case Wait_OSPF1:
			{
				if(time_1ms_count-time_record>=22)
				{
					time_record=time_1ms_count;
					if(ESP8266_OSPF_Config(1)==true)
					{
						WifidebugConnectState=Wait_OSPF2;
						USART2_DMA_Send(ESP8266_START_OSPF_CFG,strlen(ESP8266_START_OSPF_CFG));
					}
				}
				break;
			}
			case Wait_OSPF2:
			{
				if(time_1ms_count-time_record>=22)
				{
					time_record=time_1ms_count;
					//555
					if(USART2_Software_FIFO.Data_state==1)	//USART2_Software_FIFO.Data_state==1  有个BUG，如果传输数据正好=DMA_BUFFER_SIZE，就不会进串口中断
					{
						for(int i=1;i<GetQueueReadCount(&USART2_Software_FIFO);)	//!!!!!!!!!!!!!!!BUG!!!!!!!!!!!!BUG!!!!!!!!!!BUG!!!!!!!!!!BUG!!!!!!!!for(int i=0;i<GetQueueReadCount(&USART2_Software_FIFO);i++) 中如果有RemoveAQueue，因为for每次循环都会计算，所以只会循环一半的次数
						{
							if(RemoveAQueue(&USART2_Software_FIFO)=='O'&&QueryAQueue(&USART2_Software_FIFO,0)=='K')	//检查OK	//方法1，没有调用Compare_QueryQueue
							{
								USART2_Software_FIFO.Data_state=0;	//当前有效数据已经读完了
								WifidebugConnectState=Wait_HostOK;
								DeleteAllQueue();
								return;
							}
						}
						USART2_Software_FIFO.Data_state=0;	//当前有效数据已经读完了
					}
				}
				break;
			}
			case Wait_HostOK:
			{
				if(time_1ms_count-time_record>=50)
				{
					time_record=time_1ms_count;
					if(WFDBG_RCNET_Wait_HostOK()==true)
					{
						WifidebugConnectState=ConnectOK;
					}	
				}
				
				break;
			}
			case ConnectOK:
			{
//				if(time_1ms_count-time_record>=21)	//24 270  25 380推测22 50
//				{
//					time_record=time_1ms_count;
//						USART2_DMA_Send("test1test2test3test4test5tes6test7test8test9test0test1test2test3test4test5tes6test7test8test9formtest1test2test3test4test5tes6test7test8test9hostform1form2form1form2form1form2form1form2form1form2form1form2form1form2form1form2form1form2form1form2form1form2gets1gets2gets3gets4gets51form2form1form2form1form2form1form2form1form2form1form2form1form2form1form2form1form2gets1gets2gets3gets4gets5orm2form1form2gets1gets2gets3gets4gets5rm2form1form2form1form2form1form2form1form2form1form2form1form2form1form2form1form2gets1gets2gets3gets4gets5orm2form1form2gets1gets2gets3gets4gets5",20);
//				}
				if(time_1ms_count-time_record>=22)	//只有当time_record被记录，才会被触发
				{
					switch(WifidebugDataSendSatet)
					{
						case Config_Fomat:	//const char FrameData_Type[]= DATA_TYPE_DEF;  //#RM-DT=DATA_INFO:TIM=<tim_interavl>;ABYTES=<byte_numbers>;TYPE=<s2.s2.s2.s2>;#END
						{
							strcat(WFDBUG_DataFormat_INFO, WFDBG_HEAD);	//	#RM-DT=
							strcat(WFDBUG_DataFormat_INFO, WFDBG_DATA_INFO);	//	DATA_INFO:
							strcat(WFDBUG_DataFormat_INFO, WFDBG_TIM);	//	TIM=
							char temp_inter_time[2]={0};
							temp_inter_time[0]=INTER_FRAME_TIME+48;
							strcat(WFDBUG_DataFormat_INFO, temp_inter_time);	//48为数字0的index
							strcat(WFDBUG_DataFormat_INFO, WFDBG_SEMICOLON);//	;
							strcat(WFDBUG_DataFormat_INFO, WFDBG_ABYTES);	//	ABYTES=
							char temp_abytes[3]={0};
							if(FRAME_BYTES>=10)
							{
								temp_abytes[0]=(u16)FRAME_BYTES/10+48;
								temp_abytes[1]=(u16)FRAME_BYTES%10+48;
							}
							else
							{
								temp_abytes[0]=FRAME_BYTES+48;
							}
							strcat(WFDBUG_DataFormat_INFO, temp_abytes);	//FRAME_BYTES
							strcat(WFDBUG_DataFormat_INFO, WFDBG_SEMICOLON);//	;
							strcat(WFDBUG_DataFormat_INFO, WFDBG_TYPE);	//	TYPE=
							strcat(WFDBUG_DataFormat_INFO, FrameData_Type);	//FrameData_Type
							strcat(WFDBUG_DataFormat_INFO, WFDBG_SEMICOLON);	//	;
							strcat(WFDBUG_DataFormat_INFO, WFDBG_END);	//#END
							USART2_DMA_Send(WFDBUG_DataFormat_INFO,strlen(WFDBUG_DataFormat_INFO));
							WifidebugDataSendSatet=Wait_Host_INFOOK;
							
							USART2_Software_FIFO.Data_state=0;	//当前有效数据已经读完了
							DeleteAllQueue();
							
							time_record=time_1ms_count;
							break;
						}
						case Wait_Host_INFOOK://WFDBG_INFOOK
						{
							time_record=time_1ms_count;
							if(USART2_Software_FIFO.Data_state==1)	//USART2_Software_FIFO.Data_state==1  有个BUG，如果传输数据正好=DMA_BUFFER_SIZE，就不会进串口中断
							{
								for(int i=1;i<GetQueueReadCount(&USART2_Software_FIFO);)	//!!!!!!!!!!!!!!!BUG!!!!!!!!!!!!BUG!!!!!!!!!!BUG!!!!!!!!!!BUG!!!!!!!!for(int i=0;i<GetQueueReadCount(&USART2_Software_FIFO);i++) 中如果有RemoveAQueue，因为for每次循环都会计算，所以只会循环一半的次数
								{
									if(RemoveAQueue(&USART2_Software_FIFO)==WFDBG_INFOOK[0])	//Remove语句保证每次向前推进
									{
										if(Compare_QueryQueue(&USART2_Software_FIFO,&WFDBG_INFOOK[1]))	//上一个if已经检测了ESP8266_CWJAP_OK[0]
										{//检测完成
											DeleteQueue(&USART2_Software_FIFO, strlen(&WFDBG_INFOOK[1]));	
											DeleteAllQueue();
											USART2_Software_FIFO.Data_state=0;	//当前有效数据已经读完了
											////////////////////////////////////////////////////////OKOKOKOKOKOKOKOK
											WifidebugDataSendSatet=DebugDataSend;
										}
										
									}
								}
								USART2_Software_FIFO.Data_state=0;	//当前有效数据已经读完了
							}
							break;
						}
						case DebugDataSend:	//移到函数内部
						{
							//if(copy_index>=FRAMENUM_IN_PACK-1)
							//{
//								static char sn='0';
//								Apack_Data[13]=sn;
//								sn++;
//								if(sn>'9')	sn='0';
//								memcpy((Apack_Data+15+FRAMENUM_IN_PACK*FRAME_BYTES), ";#END", 5);
//								USART2_DMA_Send(Apack_Data,FRAME_BYTES*FRAMENUM_IN_PACK+20); 
//								copy_index=0;
							//}
							
							
							break;
						}
					}
				}
				
				break;
			}
		}
	}
	

}
