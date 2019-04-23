#include "includes.h"
#include "smt_uart.h"
#include "ci100x_suart.h"
#include "ci100x_scu.h"
#include "ci100x_uart.h"
#include "ci100x_scu.h"
#include "user_main.h"
#include "com2_task.h"
#include "nvdata_save.h"



unsigned int gsUsartRxCount=0xff;
UART_USER_SETTING_Typedef gs_uar2_user;
static sys_com_msg_data_t g_received_msg;
unsigned char g_current_gear = 1;	
static unsigned char g_current_shake = 0;	





void Suart2_Init(void)
{
    NVIC_InitTypeDef NVIC_InitStruct = {0};
    NVIC_InitStruct.NVIC_IRQChannel = UART0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    
    Scu_SetDeviceGate((unsigned int)UART0,ENABLE);
    
    Scu_SetIOReuse(UART0_TX_PAD,FIRST_FUNCTION );
    Scu_SetIOReuse(UART0_TX_PAD,FIRST_FUNCTION );
   

    UART_EN(UART0,DISABLE);
    UART_BAUDRATEConfig(UART0,UART2_BAUDRATE_DEFAULT);
    UART_CRConfig(UART0,UART_TXE,ENABLE);
    UART_CRConfig(UART0,UART_RXE,ENABLE);
    UART_IntClear(UART0,UART_AllInt);
    UART_IntMaskConfig(UART0,UART_AllInt,ENABLE);
    UART_TXFIFOByteWordConfig(UART0,UART_Byte);
    UART_LCRConfig(UART0,UART_WordLength_8b,UART_StopBits_1,UART_Parity_No);
    UART_RXFIFOConfig(UART0,UART_FIFOLevel1);
    UART_TXFIFOConfig(UART0,UART_FIFOLevel1_8);
    UART_FIFOClear(UART0);
    UART_CRConfig(UART0,UART_NCED,ENABLE);
    UART_IntMaskConfig(UART0,UART_TXInt,ENABLE);/*if TX FIFO is empty,then TX int coming */
    UART_IntMaskConfig(UART0,UART_RXInt,DISABLE);
    UART_EN(UART0,ENABLE);
}

extern void SCUWakeupReInit(void);
extern uint32_t vad_wakeup_flag;
void com_receive_data_handle(unsigned char rxdata)
{    
    static BaseType_t xHigherPriorityTaskWoken;
    static unsigned char rx_buf[sizeof(sys_com_msg_data_t)]; 
    static int tc_state=SOP_STATE1;
    static int tc_cnt=0;
    static int dataLen = 0;
    unsigned char checksum = 0;
    unsigned char* pData;
    unsigned char i;

    switch(tc_state)
    {
        case SOP_STATE1:
            if(UART_HEADER0 == rxdata)
            {
                rx_buf[tc_cnt++] = rxdata;
                tc_state = SOP_STATE2;
            }            
            break;
      	 case SOP_STATE2:
      	    rx_buf[tc_cnt++] = rxdata;
      	    dataLen = rxdata;
      	    tc_state = DATA_STATE;
      	    break;
        case DATA_STATE:
			if(tc_cnt < dataLen + 1)
			{
				rx_buf[tc_cnt] = rxdata; 
				tc_cnt++;

				if(tc_cnt == dataLen + 1)
				{
					pData = rx_buf + 1;
					for(i =0;i < dataLen - 1;i++)
					{
					    checksum += pData[i];
					}                                        
					checksum = ~checksum +1;

					//check校验码
					if(checksum == rx_buf[dataLen])
					{
						user_msg_t send_msg;
						send_msg.msg_type = MSG_TYPE_SYS_COM;
						memcpy((unsigned char*)(&send_msg.msg_data.com_data),rx_buf, dataLen);
						xQueueSendFromISR(user_task_queue,&send_msg,&xHigherPriorityTaskWoken);
						mprintf("get com data OK!!!\r\n");
					}
					else
					{
						mprintf("get com data err_end \r\n");
					}
					memset(rx_buf,0,sizeof(sys_com_msg_data_t));
					tc_state = SOP_STATE1;
					tc_cnt = 0;	 
				}
			}
            else
            {
                mprintf("com2 too many data,reset recive\r\n");
                tc_state = SOP_STATE1;
                tc_cnt = 0;
            }
       break;        
    }
}
void uart2_send_AwakeState(int status)
{
    char tc_sum = 0;
    send_com_msg_data_t send_msg;
    int totallenth = 0;
    unsigned char* buffer;
	asr_cmd_t* pasrcmd;
	memset(&send_msg,0,sizeof(send_com_msg_data_t));
	
    send_msg.header0 = UART_HEADER0;
    send_msg.data_length = 10 +sizeof(asr_cmd_t);
	send_msg.type = UART_TYPE;
	send_msg.sync = UART_SYNC;
	send_msg.reserved = UART_RESERVE;
	send_msg.mark = 0;//?
	send_msg.version0 = UART_VERSION0;
	send_msg.version1 = UART_VERSION1;
	send_msg.cmd = UART_CMD;
	send_msg.msg_data[0] = 0xA1;
	send_msg.msg_data[1] = status;

	//计算checksum
	buffer = &send_msg.data_length;
	for(int i=0; i < send_msg.data_length - 1;i++)
	{
		tc_sum += buffer[i];
	}
	totallenth = send_msg.data_length + 1;
	
	//赋值校验值
	buffer = send_msg.msg_data + sizeof(asr_cmd_t);
	*buffer = ~tc_sum + 1;

	//发送数据
    mprintf("\n[");
	buffer = (unsigned char*)&send_msg;
    while(totallenth--)    
    {
        mprintf("%02x ",*buffer);
        UartPollingSenddata(UART0,*buffer++);
    }  
    mprintf("]\n");
}                
                  
void uart2_send_AsrResult(int index,int status)
{
  char tc_sum = 0;
  send_com_msg_data_t send_msg;
  int lenth = sizeof(send_com_msg_data_t);
  int totallenth = 0;
  unsigned char* buffer;
  control_cmd_t* pasrcmd;
  memset(&send_msg,0,sizeof(send_com_msg_data_t));
  
  send_msg.header0 = UART_HEADER0;
  send_msg.data_length = 10 +sizeof(control_cmd_t);
  send_msg.type = UART_TYPE;
  send_msg.sync = UART_SYNC;
  send_msg.reserved = UART_RESERVE;
  send_msg.mark = 0;
  send_msg.version0 = UART_VERSION0;
  send_msg.version1 = UART_VERSION1;
  send_msg.cmd = 0x02;
  pasrcmd = (control_cmd_t*)send_msg.msg_data;


  pasrcmd->voice = 0x04;
  pasrcmd->versionLow = 2;
  pasrcmd->sleepSensor= 0XFF;
  pasrcmd->scene= 0xFF;
   switch(index)
   {
	 case 2://风扇开机
		 pasrcmd->powerEnable = 0;
		 pasrcmd->workMode = 1;
		 pasrcmd->powerOnOff = 1;
		 nvdata_save.voice_onoff = 1;
		 if(g_current_shake == 1)
		   {
			 pasrcmd->shakeEnable = 0;
			 pasrcmd->shakeSwitch = 1;
		   }
		 break;
	 case 3://风扇关机
		 pasrcmd->powerEnable = 0;
		 pasrcmd->powerOnOff = 0;
	 	 nvdata_save.voice_onoff = 0;
		 break;
	 case 4://风扇摇头
	 	if(nvdata_save.voice_onoff == 1)
	 	{
	 	 pasrcmd->powerEnable = 0;
		 pasrcmd->powerOnOff = 1;
		 pasrcmd->shakeEnable = 0;
		 pasrcmd->shakeSwitch = 1;
		 g_current_shake = 1;
	 	}
		 break;
	 case 5://停止摇头
	 	if(nvdata_save.voice_onoff == 1)
	 	{
	 	 pasrcmd->powerEnable = 0;
		 pasrcmd->powerOnOff = 1;
		 pasrcmd->shakeEnable = 0;
		 pasrcmd->shakeSwitch = 0;
		 g_current_shake = 0;
	 	}
		 break;
	 case 6://调小风速
	 	if(nvdata_save.voice_onoff == 1)
		{  
		 pasrcmd->powerEnable = 0;
		 pasrcmd->powerOnOff = 1;
	 	 pasrcmd->gear = g_current_gear;
		 --(pasrcmd->gear);
		 if(pasrcmd->gear < 1)
			 pasrcmd->gear = 1;
		 if(g_current_shake == 1)
		   {
			 pasrcmd->shakeEnable = 0;
			 pasrcmd->shakeSwitch = 1;
		   }
		 g_current_gear = pasrcmd->gear;
	 	}
		 break;
	 case 7://调大风速
	 	if(nvdata_save.voice_onoff == 1)
	 	{
	 	 pasrcmd->powerEnable = 0;
		 pasrcmd->powerOnOff = 1;
		 pasrcmd->gear = g_current_gear;
		 ++(pasrcmd->gear);
		 if(pasrcmd->gear > 12)
			 pasrcmd->gear = 12;
		 g_current_gear = pasrcmd->gear;
		 if(g_current_shake == 1)
		   {
			 pasrcmd->shakeEnable = 0;
			 pasrcmd->shakeSwitch = 1;
		   }
	 	}
		 break;
	 case 8://调到低档
	 	if(nvdata_save.voice_onoff == 1)
	 	{
	 	pasrcmd->powerEnable = 0;
		 pasrcmd->powerOnOff = 1;
		 pasrcmd->gear = 1;
		 g_current_gear = pasrcmd->gear;
		 if(g_current_shake == 1)
		 	{
			 	pasrcmd->shakeEnable = 0;
			    pasrcmd->shakeSwitch = 1;
		 	}
	 	}
		 break;
	 case 9://调到中档
	 	if(nvdata_save.voice_onoff == 1)
	 	{
	 	 pasrcmd->powerEnable = 0;
		 pasrcmd->powerOnOff = 1;
		 pasrcmd->gear = 6;
		 g_current_gear = pasrcmd->gear;
		 if(g_current_shake == 1)
		 	{
			 	pasrcmd->shakeEnable = 0;
			    pasrcmd->shakeSwitch = 1;
		 	}
	 	}
		 break;
	 case 10://调到高档
	 	if(nvdata_save.voice_onoff == 1)
	 	{
	 	 pasrcmd->powerEnable = 0;
		 pasrcmd->powerOnOff = 1;
		 pasrcmd->gear = 12;
		 g_current_gear = pasrcmd->gear;
		  if(g_current_shake == 1)
		 	{
			 	pasrcmd->shakeEnable = 0;
			    pasrcmd->shakeSwitch = 1;
		 	}
	 	}
		 break;
	 case 11://定时两小时
	 	if(nvdata_save.voice_onoff == 1)
	 	{
		 pasrcmd->powerEnable = 0;
		 pasrcmd->powerOnOff = 1;
		 pasrcmd->timingOffHour = 2;
		  if(g_current_shake == 1)
		 	{
			 	pasrcmd->shakeEnable = 0;
			    pasrcmd->shakeSwitch = 1;
		 	}
	 	}
		 break;
	 case 12://定时四小时
	 	if(nvdata_save.voice_onoff == 1)
	 	{
		 pasrcmd->powerEnable = 0;
		 pasrcmd->powerOnOff = 1;
		 pasrcmd->timingOffHour = 4;
		  if(g_current_shake == 1)
		 	{
			 	pasrcmd->shakeEnable = 0;
			    pasrcmd->shakeSwitch = 1;
		 	}
	 	}
		 break;
	 case 13://定时六小时
	 	if (nvdata_save.voice_onoff == 1)
	 	{
	 	 pasrcmd->powerEnable = 0;
		 pasrcmd->powerOnOff = 1;
		 pasrcmd->timingOffHour = 6;
		  if(g_current_shake == 1)
		 	{
			 	pasrcmd->shakeEnable = 0;
			    pasrcmd->shakeSwitch = 1;
		 	}
	 	}
	 	 break;
	 case 14://定时八小时
	 	if(nvdata_save.voice_onoff == 1)
	 	{
	 	 pasrcmd->powerEnable = 0;
		 pasrcmd->powerOnOff = 1;
		 pasrcmd->timingOffHour = 8;
		  if(g_current_shake == 1)
		 	{
			 	pasrcmd->shakeEnable = 0;
			    pasrcmd->shakeSwitch = 1;
		 	}
	 	}
	 	 break;
	 case 15://取消定时
	 	if(nvdata_save.voice_onoff == 1)
	 	{
	 	 pasrcmd->powerEnable = 0;
		 pasrcmd->powerOnOff = 1;
		 pasrcmd->timingOffMinute = 6;
		  if(g_current_shake == 1)
		 	{
			 	pasrcmd->shakeEnable = 0;
			    pasrcmd->shakeSwitch = 1;
		 	}
	 	}
	 	 break;
	 case 16://睡眠模式
	 	if(nvdata_save.voice_onoff == 1)
	 	{
	 	 pasrcmd->powerEnable = 0;
		 pasrcmd->powerOnOff = 1;
		 pasrcmd->workMode = 3;
		  if(g_current_shake == 1)
		 	{
			 	pasrcmd->shakeEnable = 0;
			    pasrcmd->shakeSwitch = 1;
		 	}
	 	}
		 break;
	 case 17://舒适模式
	 	if(nvdata_save.voice_onoff == 1)
	 	{
		 pasrcmd->powerEnable = 0;
		 pasrcmd->powerOnOff = 1;
		 pasrcmd->workMode = 4;
		  if(g_current_shake == 1)
		 	{
			 	pasrcmd->shakeEnable = 0;
			    pasrcmd->shakeSwitch = 1;
		 	}
	 	}
	 default:
		 break;
   }

  totallenth = send_msg.data_length + 1;

  //计算校验值
  buffer = &send_msg.data_length;
  for(int i=0; i < send_msg.data_length -1;i++)
  {
	  tc_sum += buffer[i];
  }

  //赋值校验值
  buffer = send_msg.msg_data + sizeof(control_cmd_t);
  *buffer = ~tc_sum + 1;

  //发送数据
  mprintf("\n@@@@@@%d[",sizeof(control_cmd_t));
  buffer = (unsigned char*)&send_msg;
  while(totallenth--)	
  {
	  mprintf("%02x ",*buffer);
	  UartPollingSenddata(UART0,*buffer++);
  }  
  mprintf("]\n");
}

void userapp_deal_com_msg(sys_com_msg_data_t *com_data)
{  
	memcpy(&g_received_msg,com_data,sizeof(sys_com_msg_data_t));
	control_cmd_t* rPasrcmd;
	rPasrcmd = (control_cmd_t*)g_received_msg.msg_data;
	//mprintf("SCJ接收\n");
	if(rPasrcmd->powerEnable == 0 && rPasrcmd->powerOnOff == 0)
	{
		//mprintf("SCJ接收关闭\n");
		nvdata_save.voice_onoff = 0;
	}
	else if(rPasrcmd->powerEnable == 0 && rPasrcmd->powerOnOff == 1)
	{
		//mprintf("SCJ接收打开\n");
		nvdata_save.voice_onoff = 1;
	}
	if(rPasrcmd->shakeEnable == 0 && rPasrcmd->shakeSwitch == 1)
	{
		//mprintf("SCJ接收摇头\n");	
		g_current_shake = 1;
	}
	else if (rPasrcmd->shakeEnable == 0 && rPasrcmd->shakeSwitch == 0)
	{
		//mprintf("SCJ接收关摇头\n");		
		g_current_shake = 0;
	}
	g_current_gear = rPasrcmd->gear;
		

}


