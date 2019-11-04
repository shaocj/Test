#include "includes.h"
#include "smt_uart.h"
#include "ci100x_suart.h"
#include "ci100x_scu.h"
#include "ci100x_uart.h"
#include "ci100x_scu.h"
#include "user_main.h"
#include "com2_task.h"
#include "nvdata_save.h"
#include "task_asr.h"


unsigned int gsUsartRxCount=0xff;
UART_USER_SETTING_Typedef gs_uar2_user;
extern int ReservedFunc(char *val);
BaseType_t send_play_by_func_index(int32_t func_index);
BaseType_t send_play_by_func_id(int32_t func_index);
extern int set_PlayVoice_time(void);
extern void start_lowpower_cout(void);
unsigned int mtype;

void uart2_Init(void)
{

    UARTInterruptConfig(UART0,UART_BaudRate9600);

}
/*
void uart1_Init(void)
{

    UARTInterruptConfig(UART1,UART_BaudRate9600);

}
*/
extern void CheckSCUWakeup(void);
extern uint32_t vad_wakeup_flag;
extern uint32_t WaitUARTTimeout;
#define UARTDATA_VALID_TIMEOUT     0xFFFF

void uart_int_rx_handle(unsigned char rxdata)
{    
    static BaseType_t xHigherPriorityTaskWoken;
    static unsigned char rx_buf[sizeof(sys_com_msg_data_t)]; 
    static int tc_state=SOP_STATE1;
    static int tc_cnt=0;
    unsigned char* pData;
    unsigned char i;
	unsigned char checksum = 0;

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
            if(UART_D1 == rxdata)
            {
                rx_buf[tc_cnt++] = rxdata;
                tc_state = DATA_STATE;
                
                #if (!USER_UART_USE_UART1)
                if(1 == vad_wakeup_flag)
                {
                    WaitUARTTimeout = UARTDATA_VALID_TIMEOUT;
                }                
                #endif
                
            }else
            {
                tc_state = SOP_STATE1;
                tc_cnt = 0;
            }
        break;
        case DATA_STATE:
            if(tc_cnt < sizeof(sys_com_msg_data_t))
            {
                rx_buf[tc_cnt] = rxdata; 


                if(tc_cnt == (sizeof(sys_com_msg_data_t)-1))
                {
                    pData = rx_buf;
                    for(i =1;i < (sizeof(sys_com_msg_data_t)-1);i++)
                    {
                        checksum += pData[i];
                    }                            
                    if(checksum == rx_buf[tc_cnt])
                    {
                        user_msg_t send_msg;
                        CheckSCUWakeup();
                        
                        send_msg.msg_type = MSG_TYPE_SYS_COM;
                        memcpy((unsigned char*)(&send_msg.msg_data.com_data),rx_buf,sizeof(sys_com_msg_data_t));
                        xQueueSendFromISR(user_task_queue,&send_msg,&xHigherPriorityTaskWoken);
                    }
                    else
                    {
                        CheckSCUWakeup();
                        mprintf("get com data err_end \r\n");
                    }
                    memset(rx_buf,0,sizeof(sys_com_msg_data_t));
                    tc_state = SOP_STATE1;
                    tc_cnt = 0;
                }
                else
                {
                    tc_cnt++;                
                }               
            }
            else
            {
                CheckSCUWakeup();
                mprintf("com2 too many data,reset recive\r\n");
                tc_state = SOP_STATE1;
                tc_cnt = 0;
            }
       break;        
    }
}

               
void uart2_send_AsrResult(int status,unsigned int index)
{
    char tc_sum = 0;
    send_com_msg_data_t send_msg;
    int totallenth = sizeof(send_com_msg_data_t);
    unsigned char* buffer;
    memset((void *)(&send_msg),0,sizeof(send_com_msg_data_t));
    send_msg.header0 = UART_HEADER0;
    send_msg.D1 = UART_D1;
    send_msg.type = status;
    if( status == 1 )
    {
        send_msg.D3= 0;
        send_msg.D4= 0;
        send_msg.D5= 0; 
        send_msg.D6= 0; 
        switch (index)
        {
            case 2:
                send_msg.cmd= 0x01;
                break;
            case 3:
                send_msg.cmd= 0x02;
                break;
            case 4:
                send_msg.cmd= 0x03;
                break;
            case 5:
                send_msg.cmd= 0x04;
                break;
            case 6:
                send_msg.cmd= 0x05;
                break;
            case 7:
                send_msg.cmd= 0x06;
                break;
            default:
                break;
        }
    }
    else
    {
        send_msg.D5= 0; 
        send_msg.D6= 0; 
        switch (index)
        {
            case 8:
                send_msg.cmd= 0x07;
                send_msg.D3= 0x14;
                send_msg.D4= 0;
                break;
            case 9:
                send_msg.cmd= 0x08;
                send_msg.D3= 0;
                send_msg.D4= 0;
                break;
            case 10:
                send_msg.cmd= 0x09;
                send_msg.D3= 0x03;
                send_msg.D4= 0xE8;
                break;
            case 11:
                send_msg.cmd= 0x09;
                send_msg.D3= 0x07;
                send_msg.D4= 0xD0;
                break;
            case 21:
                send_msg.cmd= 0x09;
                send_msg.D3= 0;
                send_msg.D4= 0xC8;
                break;
            case 24:
                send_msg.cmd= 0x09;
                send_msg.D3= 0x01;
                send_msg.D4= 0xF4;
               break;
            case 29:
                send_msg.cmd= 0x0a;
                send_msg.D3= 0;
                send_msg.D4= 0;
                break;
            case 30:
               send_msg.cmd= 0x0b;
               send_msg.D3= 0x1E;
               send_msg.D4= 0;
               break;
            case 31:
               send_msg.cmd= 0x0b;
               send_msg.D3= 0x23;
               send_msg.D4= 0;
               break;
            case 32:
               send_msg.cmd= 0x0b;
               send_msg.D3= 0x28;
               send_msg.D4= 0;
               break;
            case 33:
               send_msg.cmd= 0x0b;
               send_msg.D3= 0x2D;
               send_msg.D4= 0;
               break;
            case 34:
               send_msg.cmd= 0x0b;
               send_msg.D3= 0x32;
               send_msg.D4= 0;
               break;
            case 35:
               send_msg.cmd= 0x0b;
               send_msg.D3= 0x37;
               send_msg.D4= 0;
               break;
            case 36:
               send_msg.cmd= 0x0b;
               send_msg.D3= 0X3C;
               send_msg.D4= 0;
               break;
            case 37:
               send_msg.cmd= 0x0b;
               send_msg.D3= 0x41;
               send_msg.D4= 0;
               break;
            case 38:
               send_msg.cmd= 0x0b;
               send_msg.D3= 0x46;
               send_msg.D4= 0;
               break;
            case 39:
               send_msg.cmd= 0x0b;
               send_msg.D3= 0x4B;
               send_msg.D4= 0;
               break;
            case 40:
               send_msg.cmd= 0x0b;
               send_msg.D3= 0x50;
               send_msg.D4= 0;
               break;
            case 41:
               send_msg.cmd= 0x0b;
               send_msg.D3= 0x55;
               send_msg.D4= 0;
               break;
            case 42:
               send_msg.cmd= 0x0b;
               send_msg.D3= 0x5A;
               send_msg.D4= 0;
               break;
            case 43:
               send_msg.cmd= 0x0b;
               send_msg.D3= 0x5F;
               send_msg.D4= 0;
               break;
            case 44:
               send_msg.cmd= 0x0b;
               send_msg.D3= 0x63;
               send_msg.D4= 0;
               break;
            default:
               break;
        }
    }
    buffer = (unsigned char*)&send_msg;
    for(int i=1; i < totallenth - 1;i++)
    {
        tc_sum += buffer[i];
    }
    send_msg.chksum = tc_sum;
    buffer = (unsigned char*)&send_msg;
    mprintf("\n[");
    while(totallenth--)    
    {
        mprintf("%02x ",*buffer);
        UartPollingSenddata(UART0,*buffer++);
    }  
    mprintf("]\n");
}                
void Play_Voice_byVoiceList(playQlist_t* voiceList)
{

    xQueueReset(play_Q);
    stop_current_play();
    xQueueSend(play_Q, voiceList, 200);

}

int hex_to_decimal(sys_com_msg_data_t *data,int length,int m)
{
    unsigned int a,b,c,d,result;
    unsigned int x,y,z;
    switch (m)
    {
        case Func_Data_D3:
            x = data->D3;
            break;
        case Func_Data_D4:
            x = data->D4;
            break;
        case Func_Data_D5:
            x = data->D5;
            break;
        case Func_Data_D6:
            x = data->D6;
            break;
        default:
            break;
    }
    if(length == Func_Data_One)
    {
        a=x/1000;
        b=(x%1000)/100;
        c=(x%100)/10;
        d=x%10;
    }
    else
    {
        y = data->D4;
        z = ((x<<8)+y);
        a=z/1000;
        b=(z%1000)/100;
        c=(z%100)/10;
        d=z%10;
    }
    result = a*1000+b*100+c*10+d;
	mprintf("result=%d\n",result);
    return result;
}
void play_ack_decimal(sys_com_msg_data_t *data,int rx,int status)
{
    int m,n;
    playQlist_t voiceList = {0,{0,0,0,0,0}};
    switch(data->cmd)
    {
    	mprintf("play_ack-decimal:data->cmd=%02x",data->cmd);
        case Func_Query_Filter_State:
            if((rx/100) == 1)
            {
                voiceList.wavCount = 2;
                voiceList.wavIdxs[0] = 2;
                voiceList.wavIdxs[1] = 31;
            }
            else
            {
                m = rx % 10;
                n = rx /10;
                if(n == 0)
                {
                    voiceList.wavCount = 2;
                    voiceList.wavIdxs[0] = 2;
                    voiceList.wavIdxs[1] = 12 + m;
                }
                else
                {
                    if(m == 0)
                    {
                        voiceList.wavCount = 2;
                        voiceList.wavIdxs[0] = 2;
                        voiceList.wavIdxs[1] = 21 + n;
                    }
                    else
                    {
                        voiceList.wavCount = 3;
                        voiceList.wavIdxs[0] = 2;
                        voiceList.wavIdxs[1] = 21 + n;
                        voiceList.wavIdxs[2] = 12 + m;
                    }
                }
            }
            break;
        case Func_Query_Water_TDS:
            if((rx/100) == 1)
            {
                voiceList.wavCount = 3;
                voiceList.wavIdxs[0] = 3;
                voiceList.wavIdxs[1] = 31;
                voiceList.wavIdxs[2] = 45;
            }
            else
            {
                m = rx % 10;
                n = rx /10;
                if(m == 0)
                {
                    voiceList.wavCount = 3;
                    voiceList.wavIdxs[0] = 3;
                    voiceList.wavIdxs[1] = 21 + n;
                    voiceList.wavIdxs[2] = 45;
                }
                else
                {
                    voiceList.wavCount = 4;
                    voiceList.wavIdxs[0] = 3;
                    voiceList.wavIdxs[1] = 21 + n;
                    voiceList.wavIdxs[2] = 12 + m;
                    voiceList.wavIdxs[3] = 45;
                }
                
            }
            break;
        case Func_Query_Total_Water:
            if((rx / 1000) >0)
            {
                n = rx / 1000;
                voiceList.wavCount = 3;
                voiceList.wavIdxs[0] = 4;
                voiceList.wavIdxs[1] = 12 + n;
                voiceList.wavIdxs[2] = 41;
                
            }
            break;
        case Func_Query_Water_Quality:
            if(status == Status_TDS)
                voiceList.wavIdxs[0] = 5;
            else if(status == Status_COD)
                     voiceList.wavIdxs[0] = 48;
            else if(status == Status_Chlorine)
                     voiceList.wavIdxs[0] = 49;
            else if(status == Status_Turbidity)
                     voiceList.wavIdxs[0] = 50;
            if((rx/100) > 0)
            {
                if(rx % 100 == 0)
                {
                    voiceList.wavCount = 2;
                    voiceList.wavIdxs[1] = 30+rx/100;
                }
                else if((rx % 100)/10 == 0)
                {
                    voiceList.wavCount = 4;
                    voiceList.wavIdxs[1] = 30+rx/100;
                    voiceList.wavIdxs[2] = 54;
                    voiceList.wavIdxs[3] = 12+(rx%100)%10;
                }
                else
                {
                    if((rx % 100)%10 == 0)
                    {
                        voiceList.wavCount = 3;
                        voiceList.wavIdxs[1] = 30+rx/100;
                        voiceList.wavIdxs[2] = 21 + (rx%100)/10;
                    }
                    else
                    {
                        voiceList.wavCount = 4;
                        voiceList.wavIdxs[1] = 30+rx/100;
                        voiceList.wavIdxs[2] = 21 + (rx%100)/10;
                        voiceList.wavIdxs[3] = 12 + (rx%100)%10;
                    }
                }
            }
            else
            {
            
                if(rx /10 == 0)
                {
                    voiceList.wavCount = 2;
                    voiceList.wavIdxs[1] = 12 + rx % 10;
                }
                else
                {
                    if(rx % 10 == 0)
                    {
                        voiceList.wavCount = 2;
                        voiceList.wavIdxs[1] = 21 + rx /10;
                    }
                    else
                    {
                        voiceList.wavCount = 3;
                        voiceList.wavIdxs[1] = 21 + rx /10;
                        voiceList.wavIdxs[2] = 12 + rx % 10;
                    }
                }
            }
            break;
        case Func_Query_Waste_Water_Ratio:
            if(status == 0)
            {
                voiceList.wavIdxs[0] = 6;
                if((rx/100) == 1)
                {
                    voiceList.wavCount = 2;
                    voiceList.wavIdxs[1] = 31;
                }
                else
                {
                    m = rx % 10;
                    n = rx /10;
                    if(n == 0)
                    {
                        voiceList.wavCount = 2;
                        voiceList.wavIdxs[1] = 12 + m;
                    }
                    else
                    {
                        if(m == 0)
                        {
                            voiceList.wavCount = 2;
                            voiceList.wavIdxs[1] = 21 + n;
                        }
                        else
                        {
                            voiceList.wavCount = 3;
                            voiceList.wavIdxs[1] = 21 + n;
                            voiceList.wavIdxs[2] = 12 + m;
                        }
                    }
                }
            }
            else if(status == 1)
            {
                voiceList.wavIdxs[0] = 51;
                if((rx/100) == 1)
                {
                    voiceList.wavCount = 4;
                    voiceList.wavIdxs[1] = 31;
                    voiceList.wavIdxs[2] = 52;
                    voiceList.wavIdxs[3] = 53;
                }
                else
                {
                    m = rx % 10;
                    n = rx /10;
                    if(n == 0)
                    {
                        voiceList.wavCount = 4;
                        voiceList.wavIdxs[1] = 12 + m;
                        voiceList.wavIdxs[2] = 52;
                        voiceList.wavIdxs[3] = 53;
                    }
                    else
                    {
                        if(m == 0)
                        {
                            voiceList.wavCount = 4;
                            voiceList.wavIdxs[1] = 21 + n;
                            voiceList.wavIdxs[2] = 52;
                            voiceList.wavIdxs[3] = 53;
                        }
                        else
                        {
                            voiceList.wavCount = 5;
                            voiceList.wavIdxs[1] = 21 + n;
                            voiceList.wavIdxs[2] = 12 + m;
                            voiceList.wavIdxs[3] = 52;
                            voiceList.wavIdxs[4] = 53;
                        }
                    }
                }
            }
            break;
        case Func_Query_Water_Temperature:
            if((rx/100) == 1)
            {
                voiceList.wavCount = 3;
                voiceList.wavIdxs[0] = 7;
                voiceList.wavIdxs[1] = 31;
                voiceList.wavIdxs[2] = 43;
            }
            else
            {
                m = rx % 10;
                n = rx /10;
                if(n == 0)
                {
                    voiceList.wavCount = 3;
                    voiceList.wavIdxs[0] = 7;
                    voiceList.wavIdxs[1] = 12 + m;
                    voiceList.wavIdxs[2] = 43;
                }
                else
                {
                    if(m == 0)
                    {
                        voiceList.wavCount = 3;
                        voiceList.wavIdxs[0] = 7;
                        voiceList.wavIdxs[1] = 21 + n;
                        voiceList.wavIdxs[2] = 43;
                    }
                    else
                    {
                        voiceList.wavCount = 4;
                        voiceList.wavIdxs[0] = 7;
                        voiceList.wavIdxs[1] = 21 + n;
                        voiceList.wavIdxs[2] = 12 + m;
                        voiceList.wavIdxs[3] = 43;
                    }
                }
            }
            break;
        case Func_Ctr_Stoped_Water:
            voiceList.wavIdxs[0] = 11;
            if(rx < 1000)
            {
                if((rx/100) > 0)
                {
                    if(rx % 100 == 0)
                    {
                        voiceList.wavCount = 3;
                        voiceList.wavIdxs[1] = 30+rx/100;
                        voiceList.wavIdxs[2] = 42;
                    }
                    else if((rx % 100)/10 == 0)
                    {
                        voiceList.wavCount = 5;
                        voiceList.wavIdxs[1] = 30+rx/100;
                        voiceList.wavIdxs[2] = 54;
                        voiceList.wavIdxs[3] = 12+(rx%100)%10;
                        voiceList.wavIdxs[4] = 42;
                    }
                    else
                    {
                        if((rx % 100)%10 == 0)
                        {
                            voiceList.wavCount = 4;
                            voiceList.wavIdxs[1] = 30+rx/100;
                            voiceList.wavIdxs[2] = 21 + (rx%100)/10;
                            voiceList.wavIdxs[3] = 42;
                        }
                        else
                        {
                            voiceList.wavCount = 5;
                            voiceList.wavIdxs[1] = 30+rx/100;
                            voiceList.wavIdxs[2] = 21 + (rx%100)/10;
                            voiceList.wavIdxs[3] = 12 + (rx%100)%10;
                            voiceList.wavIdxs[4] = 42;
                        }
                    }
                    
                }
                else
                {
                
                    if(rx /10 == 0)
                    {
                        voiceList.wavCount = 3;
                        voiceList.wavIdxs[1] = 12 + rx % 10;
                        voiceList.wavIdxs[2] = 42;
                    }
                    else
                    {
                        if(rx % 10 == 0)
                        {
                            voiceList.wavCount = 3;
                            voiceList.wavIdxs[1] = 21 + rx /10;
                            voiceList.wavIdxs[2] = 42;
                        }
                        else
                        {
                            voiceList.wavCount = 4;
                            voiceList.wavIdxs[1] = 21 + rx /10;
                            voiceList.wavIdxs[2] = 12 + rx % 10;
                            voiceList.wavIdxs[3] = 42;
                        }
                    }
                }
            }
            else
            {
                voiceList.wavCount = 3;
                voiceList.wavIdxs[1] = 12 + rx / 1000;
                voiceList.wavIdxs[2] = 41;
            }
            break;
        default:
            break;
        
    }
    Play_Voice_byVoiceList(&voiceList);
}

void userapp_deal_com_msg(sys_com_msg_data_t *com_data)
{
    int covertdecimal;
    int i;
    switch (com_data->cmd)
    {
    	mprintf("userapp_deal_com_msg:data->cmd=%02x",com_data->cmd);
        case Func_Query_Filter_State://查询滤芯状态
            covertdecimal = hex_to_decimal(com_data,Func_Data_One,Func_Data_D3);
            play_ack_decimal(com_data,covertdecimal,0);
            break;
        case Func_Query_Water_TDS://查询净水TDS值
            playQlist_t voiceList = {0,{0,0,0,0,0}};
            covertdecimal = hex_to_decimal(com_data,Func_Data_One,Func_Data_D3);
			mprintf("covertdecimal=%d\n",covertdecimal);
            if(covertdecimal > 100)
            {
                voiceList.wavCount = 1;
                voiceList.wavIdxs[0] = 47;
                Play_Voice_byVoiceList(&voiceList);
            }
            else if(covertdecimal < 10)
            {
                voiceList.wavCount = 1;
                voiceList.wavIdxs[0] = 46;
                Play_Voice_byVoiceList(&voiceList);
            }
            else
            {
                play_ack_decimal(com_data,covertdecimal,0);
            }
            break;
        case Func_Query_Total_Water://查询总净水量
            covertdecimal = hex_to_decimal(com_data,Func_Data_Two,Func_Data_D3);
            play_ack_decimal(com_data,covertdecimal,0);
            break;
        case Func_Query_Water_Quality://查询水质
        	for(i=0;i<4;i++)
        	{
	            covertdecimal = hex_to_decimal(com_data,Func_Data_One,i);
				mtype=i;
	            play_ack_decimal(com_data,covertdecimal,mtype);
				DelayMs(4500);
        	}
            break;
        case Func_Query_Waste_Water_Ratio://查询废水比
            for(i=0;i<2;i++)
        	{
	            covertdecimal = hex_to_decimal(com_data,Func_Data_One,i);
				mtype=i;
	            play_ack_decimal(com_data,covertdecimal,mtype);
				DelayMs(4500);
        	}
            break;
        case Func_Query_Water_Temperature://查询水温
            covertdecimal = hex_to_decimal(com_data,Func_Data_One,Func_Data_D3);
            play_ack_decimal(com_data,covertdecimal,0);
            break;
        case Func_Ctr_Stoped_Water://停止出水
            covertdecimal = hex_to_decimal(com_data,Func_Data_Two,Func_Data_D3);
            play_ack_decimal(com_data,covertdecimal,0);
            break;
        default:
            break;
    }

}


