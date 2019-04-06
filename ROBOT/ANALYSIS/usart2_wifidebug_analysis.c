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

/**********ESP8266����************/
//udp����1815ר��ͨ��,���Ͷ���Ϊ1813
const char ESP8266_CONFIG_UDPSTART1813[] = "AT+CIPSTART=\"UDP\",\"192.168.1.255\",1813,1815,0";
const char ESP8266_CONFIG_UDPCLOSE[] = "AT+CIPCLOSE";
const char ESP8266_CONFIG_UDPSTART1815[] = "AT+CIPSTART=\"UDP\",\"192.168.1.255\",1815,1815,0";
const char ESP8266_REV_HEAD[] = "+IPD,";
/**********ͨ��Э������************/
/*       ͷβ       */
const char WFDBG_HEAD[]="#RM-DT=";
const char WFDBG_END[]="#END";
/*       ��������       */
const char WFDBG_DCY_ROBOT[]="DCY_ROBOT:";
/*       �������ֻظ�       */
const char WFDBG_REP_DCY1[]="#RM-DT=REP_DCY:IM=";
const char WFDBG_REP_DCY2[]=";STA=1;#END";
//const char WFDBG_REP_DCY[]="REP_DCY:";//REP_DCY:
const char WFDBG_IM[]="IM=";
const char WFDBG_STA[]="STA=";
/*      HOST��������ͷ     */
const char WFDBG_CENT_HEAD[]="#RM-DT=CNET:";//!!!!!
/*       ���ֿ�ʼ       */
const char WFDBG_TAR[]="TAR=";
const char WFDBG_TIP[]="TIP=";
const char WFDBG_CIP[]="CIP=";
const char WFDBG_CPT[]="CPT=";
const char WFDBG_SEMICOLON[]=";";
/*       ���ֹ���       */
const char WFDBG_RCNET[]="RCNET:";
const char WFDBG_CNET[]="CNET:";
const char WFDBG_RCNET_OK[]="RCNET:OK;";
const char WFDBG_CNET_OK[]="CNET:OK;";

/*       ���ݴ���       */
const char WFDBG_DATA_INFO[]="DATA_INFO:";
const char WFDBG_TIM[]="TIM=";
const char WFDBG_ABYTES[]="ABYTES=";
const char WFDBG_TYPE[]="TYPE=";

const char WFDBG_INFOOK[]="#RM-DT=INFOOK:#END";	//�����������
const char WFDBG_DATA[]="DATA:";	//��������֡
/*         ������         */
const char WFDBG_HTBEAT[]="HTBEAT:";	//��������֡

/************��Ϣ����***********/
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


/**���ڲ�ѯ���еķ�ʽ��ָ���ַ����Ƚ�**/
bool Compare_QueryQueue(USART_CircularQueueTypeDef const * queue, const char ch_source[])	//���пɶ��ȴ���Ԫ����Ŀ��ҪС����ֱ�ӷ���false
{
	u16 ch_len=strlen(ch_source);
	
	if(ch_len>GetQueueReadCount(&USART2_Software_FIFO)) return false;
	
	for(int i=0;i<ch_len;i++)
	{
		if(QueryAQueue(queue,i)!=ch_source[i])	return false;
	}
	return true;
}
/**    ��ѯ���ַ������ַ����е�һ�γ��ֵ�λ�� û���򷵻�-1   **/
s32 Get_Str_IndexOf(const char * str1, const char * str2)	//����strstr()
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


/**       Ѱ�ҵ�ǰ�ش��ַ����Ƿ���+IPD       **/
bool Find_ESP8266_IPD(USART_CircularQueueTypeDef * queue)	
{
	for(int i=0;i<GetQueueReadCount(queue)-3;i++)
	{
		if(RemoveAQueue(queue)==ESP8266_REV_HEAD[0])	//==? '+'
		{
			if(Compare_QueryQueue(queue, &ESP8266_REV_HEAD[1]))//Compare_QueryQueue(queue, &ESP8266_REV_HEAD[1])
			{
				DeleteQueue(&USART2_Software_FIFO,strlen(&ESP8266_REV_HEAD[1]));	//��ʱβ��ָ��IPD,�����Ԫ��
				return true;
			}
		}
	}
	
	return false;
}

/**      ���ڵ�ǰ��β������ǰ�ƶ���Э��֡ͷ��#RM-DT= �������򽫶�β�ƶ���֡ͷ��      **/
bool Find_RM_DT_Head(USART_CircularQueueTypeDef * queue)	//���ڵ�ǰ��β��Э��֡ͷ��#RM-DT=
{
	if(RemoveAQueue(&USART2_Software_FIFO)==WFDBG_HEAD[0])	//���OK	WFDBG_HEAD[0]='#'
	{
		if(Compare_QueryQueue(queue,&WFDBG_HEAD[1])==true)	//#RM-DT=
		{
			DeleteQueue(&USART2_Software_FIFO,strlen(&WFDBG_HEAD[1]));
			
			return true;
//			for(int j=0;j<GetQueueReadCount(&USART2_Software_FIFO);j++)	//���������βӦ����CNET:TAR=UAC;TIP=192.168.1.4;CIP=192.168.1.5;CPT=1815;#END
//			{
//				if()
//			}
		}
	}
	
	return false;
}

/**         GET����port����         **/
bool Get_Port_String(char * ch_rtn,char ip_endmark)	//chrtnΪ���ز�����ip_endmarkΪPort�����ķָ����������������в�ѯ����"1815"��������"
{
	u16 count=0;//
	*ch_rtn='\0';	//����ַ���
	while(count<5)	//15������ 65535
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
/**        �������ַ���ָ���ȡ�ַ�������������ch_rtn�� ���100       **/
bool Get_String_FromPoint(char * source, char * ch_rtn,char ip_endmark)	//chrtnΪ���ز�����ip_endmarkΪPort�����ķָ����������������в�ѯ����"1815"��������"
{
	u16 count=0;
	
	while(count<25)	//25������ 65535:5 192.168.254.251:15
	{
		char takechar;
		takechar=* (source+count);
		
		if(takechar==ip_endmark)
		{
			*(ch_rtn+count)='\0';	//����ַ���������
			return true;	//����ֹͣ��-����
		}			
		
		*(ch_rtn+count)=takechar;
		
		count++;
	}
	return false;
}

/**         HOST��������ƥ�亯��������ʽ����         **/
bool Is_Connect_FormHost(USART_CircularQueueTypeDef * queue)//�ڸú�������ǰӦ���Ѿ��յ����ݴ���75�ֽ�
{
	if(Find_ESP8266_IPD(queue)==true)
	{
		DeleteQueue(queue,2);	//ֱ������*��	//��Ϊ���磺+IPD,65:#RM-DT����
		for(int i=0;i<GetQueueReadCount(queue);i++)
		{
			if(Find_RM_DT_Head(queue)==true)	//Ѱ��֡ͷ����ǰ�ƶ�
			{	//���������βӦ����CNET:TAR=UAC;TIP=192.168.1.4;CIP=192.168.1.5;CPT=1815;#END
//				for(int j=0;j<GetQueueReadCount(&USART2_Software_FIFO);j++)	//���������βӦ����CNET:TAR=UAC;TIP=192.168.1.4;CIP=192.168.1.5;CPT=1815;#END
//				{
					if(Compare_QueryQueue(queue,WFDBG_CNET)==true)	//ȷ���Ƿ���CNET:
					{
						//Get_Str_IndexOf
						DeleteQueue(queue,strlen(WFDBG_CNET));	//��β����CNET:����һ��Ԫ��
						u16 data_lengh=GetQueueReadCount(queue);
						char temp_data[data_lengh+1];
						temp_data[data_lengh]='\0';
						RemoveAllQueue(temp_data, queue);
						s32 tar_index=Get_Str_IndexOf(temp_data, WFDBG_TAR);	//����Ƿ���tar
						s32 tip_index=Get_Str_IndexOf(temp_data, WFDBG_TIP);	//����Ƿ���tip
						s32 cip_index=Get_Str_IndexOf(temp_data, WFDBG_CIP);	//����Ƿ���cip
						s32 cpt_index=Get_Str_IndexOf(temp_data, WFDBG_CPT);	//����Ƿ���cpt
						if(tar_index==-1||tip_index==-1||cip_index==-1||cpt_index==-1)
						{
							return false;
						}
						else	//������
						{
							char type_name[15]={0};
							if(Get_String_FromPoint(temp_data+tar_index+strlen(WFDBG_TAR), type_name, ';'))	//��ȡtar_type����Ƿ����Լ�
							{
								if(strcmp(type_name,WFDBG_MYTYPE)!=0)
								{
									return false;	//������0 �����
								}
							}
							else
							{
								return false;
							}
							
//							if(Get_String_FromPoint(temp_data+tip_index, , ';'))	//��ȡtap_ip����Ƿ����Լ�
//							{
//								
//							}
//							else
//							{
//								return false;
//							}

							memset(WFDBG_HostUnicastIP,0,strlen(WFDBG_HostUnicastIP));	//�ڴ�����
							if(Get_String_FromPoint(temp_data+cip_index+strlen(WFDBG_CIP), WFDBG_HostUnicastIP, ';'))	//��ȡhost ip
							{
								
							}
							else
							{
								return false;
							}
							memset(WFDBG_HostUnicastPort,0,strlen(WFDBG_HostUnicastPort));	//�ڴ�����
							if(Get_String_FromPoint(temp_data+cpt_index+strlen(WFDBG_CPT), WFDBG_HostUnicastPort, ';'))	//��ȡhost port
							{
								
							}
							else
							{
								return false;
							}
							return true;	//ֻ�����е������˵�����͵�û�д�
								
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

/**       ESP8266 UDP����         **/
const char ESP8266_CIPCLOSE_CFG[]="AT+CIPCLOSE\r\n";
void ESP8266_CIPClose_Send(void)	//TEST OK
{
	USART2_DMA_Send(ESP8266_CIPCLOSE_CFG,strlen(ESP8266_CIPCLOSE_CFG));
}

const char ESP8266_UDPSTART_CFG1[]="AT+CIPSTART=\"UDP\",\"192.168.1.255\",";	//AT+CIPSTART=\"UDP\",\"192.168.1.255\",1813,1815,0\r\n
//�м����remoteIP
const char ESP8266_UDPSTART_CFG2[]=",";
//�м����localIP
const char ESP8266_UDPSTART_CFG3[]=",0\r\n";
bool ESP8266_UDP_Config(char * remoteport,char * localport)	//�ú��������б�����ÿ������ms����һ��	//TEST OK 2019.4.1
{
	static u8 udp_cfg_state=0;	//����0Ϊδ���ӣ�Ӧ����
	static u32 wait_esp8266_time=0;
	switch(udp_cfg_state)
	{
		case 0:
		{
			ESP8266_CIPClose_Send();
			udp_cfg_state=1;
			break;
		}
		case 1:	//�������ʹ����strlen ��֪����ʱ��ô��
		{
			char cipstart[strlen(ESP8266_UDPSTART_CFG1)+strlen(ESP8266_UDPSTART_CFG2)+strlen(ESP8266_UDPSTART_CFG3)+10];
			memcpy(cipstart, ESP8266_UDPSTART_CFG1, strlen(ESP8266_UDPSTART_CFG1)+1);	//!!!������Ҫ��1����Ȼ����\0��û�п�����ȥ������strcatÿ�ζ�������ӣ�Խ��Խ��
			strcat(cipstart,remoteport);
			strcat(cipstart,ESP8266_UDPSTART_CFG2);
			strcat(cipstart,localport);
			strcat(cipstart,ESP8266_UDPSTART_CFG3);
			USART2_DMA_Send(cipstart,strlen(cipstart));
			DeleteAllQueue();	//ɾ��������Ϣ
			USART2_Software_FIFO.Data_state=0;	//��ǰ��Ч�����Ѿ�������
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
			//����Ӧ����������
			if(USART2_Software_FIFO.Data_state==1)	//USART2_Software_FIFO.Data_state==1  �и�BUG�����������������=DMA_BUFFER_SIZE���Ͳ���������ж�
			for(int i=0;i<GetQueueReadCount(&USART2_Software_FIFO);i++)
			{
				if(RemoveAQueue(&USART2_Software_FIFO)=='O')	//���OK	//����1��û�е���Compare_QueryQueue
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

/**     �涨���ô˲���ʱ����β��ӦΪIP��ַ��һλ     TEST OK 2019.4.1**/
bool IP_String_Deal(char * ch_rtn,char ip_endmark)	//chrtnΪ���ز�����ip_endmarkΪIP��ַ�����ķָ�������ESP8266��ѯ����"192.168.1.100"��������"
{
	u16 count=0;//
	*ch_rtn='\0';	//����ַ���
	while(count<15)	//15������ 192.168.254.251
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
bool ESP8266_GetLocalIP(void)	//��ȡ����IP	//����ÿ10ms����һ�εĺ�����  WFDBG_LocalIP TEST OK 2019.4.1
{
	static bool getIpState=false;
	static u8 getlocalip_state=0;
	if(getIpState==false)
	{
		switch(getlocalip_state)
		{
			case 0:
			{
				USART2_DMA_Send(ESP8266_GETLOCALIP_ASK,strlen(ESP8266_GETLOCALIP_ASK));	//�ظ�+CIFSR:STAIP,"192.168.1.100"
				getlocalip_state=1;
				break;
			}
			case 1:
			{
				
				if(USART2_Software_FIFO.Data_state==1)
				{
					if(GetQueueReadCount(&USART2_Software_FIFO)>=strlen(ESP8266_GETLOCALIP_REP1)+12)
					{
						for(int i=0;i<GetQueueReadCount(&USART2_Software_FIFO)-strlen(ESP8266_GETLOCALIP_REP1);i++)	//���ﲻ��-12����Ϊforѭ����remove.����ֱ���ڼ��OKʱֱ��return��break
						{
							if(RemoveAQueue(&USART2_Software_FIFO)=='+')	//Remove��䱣֤ÿ����ǰ�ƽ�
							{
								if(Compare_QueryQueue(&USART2_Software_FIFO,&ESP8266_GETLOCALIP_REP1[1]))	//��һ��if�Ѿ������ESP8266_CWJAP_OK[0]
								{//������
									DeleteQueue(&USART2_Software_FIFO, strlen(&ESP8266_GETLOCALIP_REP1[1]));	//ִ����˾��ʱָ��Ӧ����192.168.1.100�ײ�
									USART2_Software_FIFO.Data_state=0;	//��ǰ��Ч�����Ѿ�������
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
				if(IP_String_Deal(WFDBG_LocalIP,'"')==true)	//�ɹ���ȡ��LocalIP
				{
					getlocalip_state=0;
					getIpState=true;
					DeleteAllQueue();	//����ʱ��ն���
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


/**     ������ESP8266���м��     **/
const char ESP8266_CWJAP_ASK[]="AT+CWJAP?\r\n";
const char ESP8266_CWJAP_OK[]="+CWJAP:\"IFR_ROBOT\"";
void ESP8266_ConfigCheck(void)
{
	static u32 time_last=0;
	
	if(time_1ms_count-time_last>=22)	//��ִ֤����������10ms  ɳ��ESP8266 10ms��Ӧ������
	{
		time_last=time_1ms_count;
		switch(Esp8266ConfigState)
		{
			case Wait_PowerOn:
			{
				//u16 time_count=0;
				USART2_DMA_Send("AT\r\n",4);
				delay_ms(10);
				if(USART2_Software_FIFO.Data_state==1)	//USART2_Software_FIFO.Data_state==1  �и�BUG�����������������=DMA_BUFFER_SIZE���Ͳ���������ж�
				{
					for(int i=1;i<GetQueueReadCount(&USART2_Software_FIFO);)	//!!!!!!!!!!!!!!!BUG!!!!!!!!!!!!BUG!!!!!!!!!!BUG!!!!!!!!!!BUG!!!!!!!!for(int i=0;i<GetQueueReadCount(&USART2_Software_FIFO);i++) �������RemoveAQueue����Ϊforÿ��ѭ��������㣬����ֻ��ѭ��һ��Ĵ���
					{
						if(RemoveAQueue(&USART2_Software_FIFO)=='O'&&QueryAQueue(&USART2_Software_FIFO,0)=='K')	//���OK	//����1��û�е���Compare_QueryQueue
						{
							DeleteAllQueue();
							USART2_Software_FIFO.Data_state=0;	//��ǰ��Ч�����Ѿ�������
							Esp8266ConfigState=Wait_ConnectWIFI;
						}
					}
				}
				break;
			}
			case Wait_ConnectWIFI:	//strlen����ʹ�û᲻������ٶ�����
			{
				USART2_DMA_Send(ESP8266_CWJAP_ASK,strlen(ESP8266_CWJAP_ASK));	//�����������"No AP" "+CWJAP:"IFR_ROBOT","50:fa:84:1e:1f:4c",13,-30"
				if(USART2_Software_FIFO.Data_state==1)
				{
					if(GetQueueReadCount(&USART2_Software_FIFO)>=strlen(ESP8266_CWJAP_OK))
					{
						u16 queuereadcount=GetQueueReadCount(&USART2_Software_FIFO);
						for(int i=0;i<queuereadcount-strlen(ESP8266_CWJAP_OK);i++)
						{
							if(RemoveAQueue(&USART2_Software_FIFO)=='+')	//Remove��䱣֤ÿ����ǰ�ƽ�
							{
								if(Compare_QueryQueue(&USART2_Software_FIFO,&ESP8266_CWJAP_OK[1]))	//��һ��if�Ѿ������ESP8266_CWJAP_OK[0]
								{//������
									DeleteAllQueue();
									USART2_Software_FIFO.Data_state=0;	//��ǰ��Ч�����Ѿ�������
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
					USART2_Software_FIFO.Data_state=0;	//��ǰ��Ч�����Ѿ�������
					Esp8266ConfigState=Wait_MonitorUDP;
				}
				break;
			}
			case Wait_MonitorUDP:
			{
				if(ESP8266_UDP_Config("1813","1815"))	//��㲥�ŵ�������Ϣ
				{
					Esp8266ConfigState=ESP8266_OK;
					USART2_Software_FIFO.Data_state=0;	//��ǰ��Ч�����Ѿ�������
				}
				break;
			}
			
			case ESP8266_OK:	//����������Ҫ�Ż�  �Բ�ͬ����***.***.***.255
			{
				
				break;
			}
		}
	}
	
}


/**        �������ֻظ��ķ��ͺ���          **/
void WFDBG_ReplyDiscover_SendDex(void)	//�������/0
{
	u16 rep_dcy1_len=strlen(WFDBG_REP_DCY1);
	u16 type_len=strlen(WFDBG_MYTYPE);
	u16 rep_dcy2_len=strlen(WFDBG_REP_DCY2);
	char temp_send[rep_dcy1_len+type_len+rep_dcy2_len+1];
	memcpy(temp_send,WFDBG_REP_DCY1,rep_dcy1_len);
	memcpy(temp_send+rep_dcy1_len,WFDBG_MYTYPE,type_len);
	memcpy(temp_send+rep_dcy1_len+type_len,WFDBG_REP_DCY2,rep_dcy2_len+1);	//+1��ĩβ��\0Ҳ���ƽ�ȥ
//	for(int i=0;i<strlen(WFDBG_HEAD))
	USART2_DMA_Send(temp_send,rep_dcy1_len+type_len+rep_dcy2_len+1);	//����36�ֽ�+1�ֽ� '\0'
}
/**     �������ֵ��Զ��ظ�  ��δ���ã�   **/
const char WFDBG_DCYROBOT[]="#RM-DT=DCY_ROBOT";	//��������ѯ
void WFDBG_AutoReply_Discover(void)	//��δ���ã�����
{
	if(USART2_Software_FIFO.Data_state==1)
	{
		if(GetQueueReadCount(&USART2_Software_FIFO)>=strlen(WFDBG_DCYROBOT)+3)
		{
			for(int i=0;i<GetQueueReadCount(&USART2_Software_FIFO)-strlen(WFDBG_DCYROBOT);i++)	//���ﲻ��-12����Ϊforѭ����remove.����ֱ���ڼ��OKʱֱ��return��break
			{
				if(RemoveAQueue(&USART2_Software_FIFO)=='#')	//Remove��䱣֤ÿ����ǰ�ƽ�
				{
					if(Compare_QueryQueue(&USART2_Software_FIFO,&WFDBG_DCYROBOT[1]))	//��һ��if�Ѿ������ESP8266_CWJAP_OK[0]
					{//������
						DeleteQueue(&USART2_Software_FIFO, strlen(&WFDBG_DCYROBOT[1]));	//ִ����˾��ʱָ��Ӧ����192.168.1.100�ײ�
						USART2_Software_FIFO.Data_state=0;	//��ǰ��Ч�����Ѿ�������
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


/**         ESP8266 UPD��͸��ģʽ�����ֽ�        **/
const char ESP8266_STARTSENDBYTES_DEX1[]="AT+CIPSENDEX=";	//С��100	//esp8266������ܾ���ɳ�񣬷��͵İ�д�������м���������������\0����
//�м���뷢������
const char ESP8266_STARTSENDBYTES_DEX2[]="\r\n";
bool ESP8266_UDPSendBytesDEX(char len[])	//�ú������ô���Ӧ̫�� len����"12"
{
	static u8 send_state=0;
	static u32 time_count=0;
	switch(send_state)
	{
		case 0:
		{
			char strsend[20]={0};
			DeleteAllQueue();	//����գ�������ʷ�ۼ�
			USART2_Software_FIFO.Data_state=0;	//����գ�������ʷ�ۼ�
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
				//if(GetQueueReadCount(&USART2_Software_FIFO)>=strlen(ESP8266_GETLOCALIP_REP1)+12)//��Ϊֻ���һ��'>'���Բ��������if�ж�
				//{
					for(int i=0;i<GetQueueReadCount(&USART2_Software_FIFO);i++)	//���ﲻ��-12����Ϊforѭ����remove.����ֱ���ڼ��OKʱֱ��return��break
					{
						if(RemoveAQueue(&USART2_Software_FIFO)=='>')	//Remove��䱣֤ÿ����ǰ�ƽ�
						{
							send_state=0;	//��λ
							time_count=0;
							//DeleteAllQueue();	//��գ�������ʷ�ۼ� ���ﲻ����գ���Ϊ��ʱ�п���������ʱ������������
							USART2_Software_FIFO.Data_state=0;	//��գ�������ʷ�ۼ�
							return true;
						}
					}

				//}
			}
		}
	}
	return false;
}

/***        �������ֵ�����ʽ��������  ��ֵ����˼�ǵ�ǰ�Ƿ��ڰ�ȫ��     ***/
bool WFDBG_ActiveReply_Discover(void)	//�ⲿ��������20ms  ��ֵ����˼�ǵ�ǰ�Ƿ��ڰ�ȫ��
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
				return true;	//��ʱ�ǰ�ȫ�ģ��Ѿ�ִ����һ�������ظ�
			}
			
			break;
		}
	}
	return false;
}
/**          ESP8266����/�˳�͸��ģʽ           **/
const char ESP8266_OSPF_ASK[]="AT+CIPMODE?\r\n";
const char ESP8266_OSPF_CFG0[]="AT+CIPMODE=0\r\n";
const char ESP8266_OSPF_CFG1[]="AT+CIPMODE=1\r\n";
const char ESP8266_OSPF_REPLY[]="+CIPMODE:";	//��������+CIPMODE:1
bool ESP8266_OSPF_Config(u8 mode)	//����  OSPFģʽΪ0 �ⲿ��������ӦΪ20ms
{
	static u8 ospf_state=0;
	switch(ospf_state)
	{
		case 0:	//��ѯ��ǰ��ʲô״̬
		{
			USART2_DMA_Send(ESP8266_OSPF_ASK,strlen(ESP8266_OSPF_ASK));
			ospf_state=1;
			break;
		}
		case 1:
		{
			if(USART2_Software_FIFO.Data_state==1)	//USART2_Software_FIFO.Data_state==1  �и�BUG�����������������=DMA_BUFFER_SIZE���Ͳ���������ж�
			{
				for(int i=0;i<GetQueueReadCount(&USART2_Software_FIFO)-strlen(ESP8266_OSPF_REPLY);i++)
				{
					if(RemoveAQueue(&USART2_Software_FIFO)==ESP8266_OSPF_REPLY[0])	//Remove��䱣֤ÿ����ǰ�ƽ�
					{
						if(Compare_QueryQueue(&USART2_Software_FIFO,&ESP8266_OSPF_REPLY[1]))	//��һ��if�Ѿ������ESP8266_CWJAP_OK[0]
						{//������
							DeleteQueue(&USART2_Software_FIFO, strlen(&ESP8266_OSPF_REPLY[1]));	//ִ����˾��ʱָ��Ӧ����192.168.1.100�ײ�
							USART2_Software_FIFO.Data_state=0;	//��ǰ��Ч�����Ѿ�������
							if(RemoveAQueue(&USART2_Software_FIFO)==mode)	//�������βӦ��ָ��mode
							{
								ospf_state=0;
								DeleteAllQueue();
								return true;
							}
							ospf_state=2;	//ģʽ����͸��ģʽ��Ӧ������
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
			if(USART2_Software_FIFO.Data_state==1)	//USART2_Software_FIFO.Data_state==1  �и�BUG�����������������=DMA_BUFFER_SIZE���Ͳ���������ж�
			{
				for(int i=0;i<GetQueueReadCount(&USART2_Software_FIFO);i++)
				{
					if(RemoveAQueue(&USART2_Software_FIFO)=='O'&&QueryAQueue(&USART2_Software_FIFO,0)=='K')	//���OK	//����1��û�е���Compare_QueryQueue
					{
						USART2_Software_FIFO.Data_state=0;	//��ǰ��Ч�����Ѿ�������
						ospf_state=0;
						DeleteAllQueue();
						return true;
					}
				}
				USART2_Software_FIFO.Data_state=0;	//��ǰ��Ч�����Ѿ�������
			}
		}
	}
	return false;
}

/**       Wait_Host_OK         **/
const char WFDBG_RCNET_OK_REPLY[]="#RM-DT=RCNET:OK;#END";
const char WFDBG_CNET_REPLY_FORM_HOST[]="#RM-DT=CNET:OK;#END";
bool WFDBG_RCNET_Wait_HostOK(void)	//�ⲿ��������20ms���ڣ�
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
			DeleteAllQueue();	//��ս������ݣ���ʵ���һ���������ǲ�����յ�
			USART2_Software_FIFO.Data_state=0;	//�����Ч����
			rcnet_state=2;
			break;
		}
		case 2:	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////�������ӳ�ʱ�ش�
		{	//��������Ļ�Ӧ
			if(USART2_Software_FIFO.Data_state==1)	//USART2_Software_FIFO.Data_state==1  �и�BUG�����������������=DMA_BUFFER_SIZE���Ͳ���������ж�
			{
				for(int i=1;i<GetQueueReadCount(&USART2_Software_FIFO);)
				{
					if(RemoveAQueue(&USART2_Software_FIFO)==WFDBG_CNET_REPLY_FORM_HOST[0])	//Remove��䱣֤ÿ����ǰ�ƽ�
					{
						if(Compare_QueryQueue(&USART2_Software_FIFO,&WFDBG_CNET_REPLY_FORM_HOST[1]))	//��һ��if�Ѿ������ESP8266_CWJAP_OK[0]
						{//������
							DeleteQueue(&USART2_Software_FIFO, strlen(&WFDBG_CNET_REPLY_FORM_HOST[1]));	
							DeleteAllQueue();
							USART2_Software_FIFO.Data_state=0;	//��ǰ��Ч�����Ѿ�������
							rcnet_state=0;
							overtime_count=0;
							return true;
						}
						
					}
				}
				USART2_Software_FIFO.Data_state=0;	//��ǰ��Ч�����Ѿ�������
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
/**    ESP8266�������úú�     **/
void Wifi_Debug_Main(void)//������״̬�� ����while��
{
	static u32 time_record=0;
	if(Esp8266ConfigState==ESP8266_OK)	//ESP8266�Ѿ����ú�	//����Ӧ�÷������߰�
	{
		switch(WifidebugConnectState)
		{
			case Unconnected:
			{
				if(time_1ms_count-time_record>=24)
				{
					time_record=time_1ms_count;
					
					if(WFDBG_ActiveReply_Discover()==true)	//����ǰ�ȫ�ģ����������ظ��С��Ϳ��Կ�ʼ���#RM-DT��
					{
						if(GetQueueReadCount(&USART2_Software_FIFO)>70)	//����������ݴ���75��������������
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
			case Wait_connect:	//��һ��������Զ��1815
			{
				if(time_1ms_count-time_record>=22)
				{
					time_record=time_1ms_count;
					if(ESP8266_UDP_Config(WFDBG_HostUnicastPort, WFDBG_LocalUnicastPort))	//���ӵ������˿�
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
					if(USART2_Software_FIFO.Data_state==1)	//USART2_Software_FIFO.Data_state==1  �и�BUG�����������������=DMA_BUFFER_SIZE���Ͳ���������ж�
					{
						for(int i=1;i<GetQueueReadCount(&USART2_Software_FIFO);)	//!!!!!!!!!!!!!!!BUG!!!!!!!!!!!!BUG!!!!!!!!!!BUG!!!!!!!!!!BUG!!!!!!!!for(int i=0;i<GetQueueReadCount(&USART2_Software_FIFO);i++) �������RemoveAQueue����Ϊforÿ��ѭ��������㣬����ֻ��ѭ��һ��Ĵ���
						{
							if(RemoveAQueue(&USART2_Software_FIFO)=='O'&&QueryAQueue(&USART2_Software_FIFO,0)=='K')	//���OK	//����1��û�е���Compare_QueryQueue
							{
								USART2_Software_FIFO.Data_state=0;	//��ǰ��Ч�����Ѿ�������
								WifidebugConnectState=Wait_HostOK;
								DeleteAllQueue();
								return;
							}
						}
						USART2_Software_FIFO.Data_state=0;	//��ǰ��Ч�����Ѿ�������
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
//				if(time_1ms_count-time_record>=21)	//24 270  25 380�Ʋ�22 50
//				{
//					time_record=time_1ms_count;
//						USART2_DMA_Send("test1test2test3test4test5tes6test7test8test9test0test1test2test3test4test5tes6test7test8test9formtest1test2test3test4test5tes6test7test8test9hostform1form2form1form2form1form2form1form2form1form2form1form2form1form2form1form2form1form2form1form2form1form2gets1gets2gets3gets4gets51form2form1form2form1form2form1form2form1form2form1form2form1form2form1form2form1form2gets1gets2gets3gets4gets5orm2form1form2gets1gets2gets3gets4gets5rm2form1form2form1form2form1form2form1form2form1form2form1form2form1form2form1form2gets1gets2gets3gets4gets5orm2form1form2gets1gets2gets3gets4gets5",20);
//				}
				if(time_1ms_count-time_record>=22)	//ֻ�е�time_record����¼���Żᱻ����
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
							strcat(WFDBUG_DataFormat_INFO, temp_inter_time);	//48Ϊ����0��index
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
							
							USART2_Software_FIFO.Data_state=0;	//��ǰ��Ч�����Ѿ�������
							DeleteAllQueue();
							
							time_record=time_1ms_count;
							break;
						}
						case Wait_Host_INFOOK://WFDBG_INFOOK
						{
							time_record=time_1ms_count;
							if(USART2_Software_FIFO.Data_state==1)	//USART2_Software_FIFO.Data_state==1  �и�BUG�����������������=DMA_BUFFER_SIZE���Ͳ���������ж�
							{
								for(int i=1;i<GetQueueReadCount(&USART2_Software_FIFO);)	//!!!!!!!!!!!!!!!BUG!!!!!!!!!!!!BUG!!!!!!!!!!BUG!!!!!!!!!!BUG!!!!!!!!for(int i=0;i<GetQueueReadCount(&USART2_Software_FIFO);i++) �������RemoveAQueue����Ϊforÿ��ѭ��������㣬����ֻ��ѭ��һ��Ĵ���
								{
									if(RemoveAQueue(&USART2_Software_FIFO)==WFDBG_INFOOK[0])	//Remove��䱣֤ÿ����ǰ�ƽ�
									{
										if(Compare_QueryQueue(&USART2_Software_FIFO,&WFDBG_INFOOK[1]))	//��һ��if�Ѿ������ESP8266_CWJAP_OK[0]
										{//������
											DeleteQueue(&USART2_Software_FIFO, strlen(&WFDBG_INFOOK[1]));	
											DeleteAllQueue();
											USART2_Software_FIFO.Data_state=0;	//��ǰ��Ч�����Ѿ�������
											////////////////////////////////////////////////////////OKOKOKOKOKOKOKOK
											WifidebugDataSendSatet=DebugDataSend;
										}
										
									}
								}
								USART2_Software_FIFO.Data_state=0;	//��ǰ��Ч�����Ѿ�������
							}
							break;
						}
						case DebugDataSend:	//�Ƶ������ڲ�
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
