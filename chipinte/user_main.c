/**
  ******************************************************************************
  * @file    task_asr.c 
  * @author  Chipintelli Technology Co., Ltd.
  * @version V1.0.0
  * @date    2017.05
  * @brief
  ******************************************************************************
  **/
#include <stdint.h>
#include "user_main.h"
#include "FreeRTOS.h"
#include "ci100x_global.h"
#include "user_config.h"
#include "includes.h" 
#include "../../bsp_test/src/cxdish/cxdish.h"
#include "com2_task.h"
#include "nvdata_save.h"
#include "flash_update.h"
#include "user_main.h"
#include "smt_sdram.h"

/**************************************************************************
                    type define
****************************************************************************/
typedef enum
{
    USERSTATE_WAIT_MSG = 0,
}USER_State_T;


struct user_manage_type
{
    uint32_t user_msg_state;
    uint32_t wakeup_voice_state;
}g_user_manage_t ;


/**************************************************************************
                    function prototype
****************************************************************************/
extern void userapp_deal_key_msg(sys_key_msg_data_t  *key_msg);
extern void userapp_deal_com_msg(sys_com_msg_data_t *com_data);
extern void Asr_Playback(int index);
extern void uart2_send_AwakeState(int status);
extern void uart2_send_AsrResult(int index,int status);
extern void RGBChangeColor(int index);
extern void RGBDataInit(void);
#if USE_USER_UART
extern void Suart2_Init();
#endif
void userapp_deal_asr_msg(sys_asr_msg_data_t *asr_msg);
static void play_prompt(void);
void MainCallback(void *p);
void Asr_Playback(int function_index);
BaseType_t send_play_by_func_index(int32_t func_index);


/**************************************************************************
                    global 
****************************************************************************/

void Switch_Language_play_voice(void)
{
    
    //awaken = 1;
    //Swtich_Language_ASR_Play_If();
    
}

int vol_up()
{
       int IsMax = 0;
    if(nvdata_save.volset < VOLUME_MAX)
    {
        nvdata_save.volset++;
    }
    else
    {
        nvdata_save.volset = VOLUME_MAX;
        IsMax = 1;

    }
    
    vol_set(nvdata_save.volset);
    nvdata_write((uint8_t* )&nvdata_save,sizeof(nvdata_save));   
    return IsMax;

}

int vol_dn()
{
        int IsMin = 0;
    if(nvdata_save.volset > VOLUME_MIN)
    {
        nvdata_save.volset--;
    }
    else
    {
        nvdata_save.volset = VOLUME_MIN;
          IsMin = 2;

    }
    vol_set(nvdata_save.volset);
    nvdata_write((uint8_t* )&nvdata_save,sizeof(nvdata_save));   
    return IsMin;

}
int  deal_volume_control(int function_index)
{
    int ret = 0;
    int play_index = 0;

    if(function_index == VOLUME_UP_COMMAND_INDEX)
    {
        ret = vol_up();
    }
    else if(function_index == VOLUME_DOWN_COMMAND_INDEX) 
    {
        ret = vol_dn();
    }
    else if(function_index == VOLUME_MAX_COMMAND_INDEX)
    {
        nvdata_save.volset = VOLUME_MAX;
        ret = vol_up();
    }
    else if(function_index == VOLUME_MIN_COMMAND_INDEX)
    {
        nvdata_save.volset = VOLUME_MIN;
        ret = vol_dn();
    }
    else 
    {
       return -1;
    }
    
    if(ret == 1)    //max
    {
        play_index = PLAY_VOLMAX_VOICE_INDEX;
    }
    else if(ret == 2)   //min
    {
        play_index = PLAY_VOLMIN_VOICE_INDEX;
    }
    else
    {
        play_index = function_index;
    }
    mprintf("@@@@@@@@%s,%d,%d\n",__FUNCTION__,__LINE__,play_index);
    if(pdPASS!=send_play_by_func_index(play_index))
    {
        mprintf("send err %s,%d\n",__FUNCTION__,__LINE__);
    }
    return 0;
}

PlayVoice_Infor_Type PlayVoice_Infor;
File_Header_type UserFile_Struct[USERFILE_MAX_NUM];
void UserData_Init(void)
{
    volatile unsigned int tmp = 0;
    uint16_t HeaderSize, HeaderFileNum;
    int ret,i = 0, j = 0; 
    int current_index = -1,last_index = 0x00;
    File_Header_type File_Header;
    FileConfig_Struct FileConfigInfo = {0};
    
//Get Flash Config structure 
    SpiFlash_ReadData(QSPIC,FILECONFIG_SPIFLASH_START_ADDR,(char*)&FileConfigInfo,sizeof(FileConfig_Struct));

//Get Voice Address table
    flashtosdram(SDRAM_REC_BUFFER_ADDR,FileConfigInfo.VoicePlayingStartAddr,4*1024);   //4KB first
    tmp = (unsigned int)(SDRAM_REC_BUFFER_ADDR) ;
    HeaderFileNum = *(volatile unsigned int*)(tmp);
    HeaderSize = (HeaderFileNum *sizeof(File_Header_type));
    if(HeaderSize > 4*1024)
    {
        flashtosdram(SDRAM_REC_BUFFER_ADDR,FileConfigInfo.VoicePlayingStartAddr,HeaderSize); 
    }
    tmp += sizeof(HeaderFileNum);    //Go to first ID
    
    PlayVoice_Infor.PlayVoice_Num = HeaderFileNum;
    ret = Init_SDRAM_Buffer((char **)&(PlayVoice_Infor.PlayVoice_Header), (HeaderFileNum*sizeof(File_Header_type)));
    if(RETURN_ERR == ret)
    {
        mprintf("malloc PlayVoice_Infor.PlayVoice_Header error\n");
    }
    else
    {   
        mprintf("PlayVoice_Infor.PlayVoice_Header is 0x%x\n",PlayVoice_Infor.PlayVoice_Header );
    }    
    
    memcpy(&PlayVoice_Infor.PlayVoice_Header[0].File_ID,(void *)tmp,(HeaderFileNum*sizeof(File_Header_type)));
    for(i=0;i<HeaderFileNum;i++)    
    {
        PlayVoice_Infor.PlayVoice_Header[i].File_Addr += FileConfigInfo.VoicePlayingStartAddr;
    }
    
    //Copy the Voices Addr & Size for original play structure
    for(i=0,j=0;(AsrCmd_Struct[i].id != -1);i++)
    {
        current_index = AsrCmd_Struct[i].id;
        if(current_index==last_index)
        {
            //Same Voice
            AsrCmd_Struct[i].pVoiceAddr = AsrCmd_Struct[i-1].pVoiceAddr;
            AsrCmd_Struct[i].VoiceSize = AsrCmd_Struct[i-1].VoiceSize;
        }            
        else
        {
            last_index = current_index;     //Update new voice
            AsrCmd_Struct[i].pVoiceAddr = PlayVoice_Infor.PlayVoice_Header[j].File_Addr;
            AsrCmd_Struct[i].VoiceSize = PlayVoice_Infor.PlayVoice_Header[j].File_Size; 
            j++;
        }
    }

//Get User File Address table, could be nothing if FileConfigInfo.UserFileStartAddr is 0x00
     if(0x00 != FileConfigInfo.UserFileStartAddr)
    {
        flashtosdram(SDRAM_REC_BUFFER_ADDR,FileConfigInfo.UserFileStartAddr,4*1024);   //4KB first
        tmp = (unsigned int)(SDRAM_REC_BUFFER_ADDR) ;
        HeaderFileNum = *(volatile unsigned int*)(tmp);
        HeaderSize = HeaderFileNum *sizeof(File_Header_type);
        if(HeaderSize > 4*1024)
        {
            flashtosdram(SDRAM_REC_BUFFER_ADDR,FileConfigInfo.UserFileStartAddr,HeaderSize); 
        }
        tmp += sizeof(HeaderFileNum);    //Go to first ID

        for(i=0;i<HeaderFileNum;i++)    
        {
            memcpy(&UserFile_Struct[i],(void *)tmp,sizeof(File_Header_type));
            UserFile_Struct[i].File_Addr += FileConfigInfo.UserFileStartAddr;
        }
        for(i=0;i<HeaderFileNum;i++) 
        {
            /*Get User Files by File_ID defined in path of Tool\UserFile\[n]xxx.bin  */
            /*.....*/
        }
//Get User NVData Information
        /* User could get NVData infor from following data, it's need match with SPI_NVDATA_START_ADDR define
            FileConfigInfo.NVDataStartAddr; 
            FileConfigInfo.NVDataSize;      
        */
    }
}

void GetUserData(void) ININT_CALL_FUNC
{
#ifdef NEW_MERGE_TOOL
    UserData_Init();
#endif    
    nvdata_init();

    nvdata_read((uint8_t* )&nvdata_save,sizeof(nvdata_save));

    if(nvdata_save.inited != 0xa5)
    {//default init
        mprintf("--->first use spi,default init\r\n");
        nvdata_save.inited= 0xa5;//inited flag        
        nvdata_save.volset = VOLUME_DEFAULT;
        nvdata_save.voice_onoff = 1;    //默认打开播放
        nvdata_save.uLanguageId = SUPPORT_LANGUAGE;
        nvdata_write((uint8_t* )&nvdata_save,sizeof(nvdata_save));    
    }
#if ((DEBUG_PRINTF_FUNC_EN == 0) && (AUTOTEST_PRINTF_FUNC_EN == 1))
    nvdata_save.voice_onoff = 0;
#endif
    if((nvdata_save.volset>VOLUME_MAX)|| (nvdata_save.volset<VOLUME_MIN))
    {
        nvdata_save.volset = VOLUME_DEFAULT; 
    }
}

void  UserTaskManageProcess(void *p_arg)
{
    user_msg_t rev_msg;
    BaseType_t err = pdPASS;
    
    g_user_manage_t.user_msg_state = USERSTATE_WAIT_MSG;

    vol_set(nvdata_save.volset);

#if RGB_ENABLE
    RGBDataInit();
#endif

#if OTA_FUNC_ENABLE
    OTAFuncProcess();
#endif


    while(1)
    {
        err = xQueueReceive(user_task_queue, &rev_msg, portMAX_DELAY);

        if(pdPASS == err)
        {
            switch (rev_msg.msg_type)
            {
                case MSG_TYPE_SYS_ASR:
                {
                    sys_asr_msg_data_t *asr_rev_data;
                    asr_rev_data = &(rev_msg.msg_data.asr_data);
                    userapp_deal_asr_msg(asr_rev_data);
                    break;
                }
                case MSG_TYPE_SYS_KEY:
                {
                    sys_key_msg_data_t *key_rev_data;
                    key_rev_data = &rev_msg.msg_data;
                    userapp_deal_key_msg(key_rev_data);
                    break;
                }
                #if USE_USER_UART
                case MSG_TYPE_SYS_COM:
                {
                    sys_com_msg_data_t *com_rev_data;
                    com_rev_data = &rev_msg.msg_data;
                    userapp_deal_com_msg(com_rev_data);
                    break;
                }
                #endif
                default:
                    break;
            }
        
        }
        else
        {

        }
    }
}

#if USE_COMBO_CMD
int combo_combo_cmd(int index)
{
    int combo_index=index;

    if(combo_cmd_result.first_word_valid==COMBO_CMD_FIRST_WORD_VALID)
    {
        combo_cmd_result.first_word_valid = COMBO_CMD_FIRST_WORD_INVALID;
        if(pdMS_TO_TICKS(xTaskGetTickCount()-combo_cmd_result.first_word_valid_tick)
            <=COMBO_CMD_VALID_TIME_IN_MS)
        {
            //组合播报有效，返回对应的index值，自行处理
            combo_index = index+1;
        }else
        {
            mprintf("combo cmd timeout.\n");
        }
    }
    return combo_index;
}
#endif

void userapp_deal_asr_msg(sys_asr_msg_data_t *asr_msg)
{
    int32_t index;
    int32_t function_index;
    if(MSG_ASR_STATUS_GOOD_RESULT == asr_msg->asr_status)
    {
        index = asr_msg->asr_index;
#if USE_COMBO_CMD
        index = combo_combo_cmd(index);
#endif
        function_index = AsrCmd_Struct[index].id;

        if(function_index >= 0)
        {
            Asr_Playback(function_index);
        }  

#if RGB_ENABLE
        if(function_index >= 0 && awaken)
        {
            RGBChangeColor(function_index);
        }
#endif

        #if USE_USER_UART
        if(awaken)
        {
        	if(function_index > 1)
            	uart2_send_AsrResult(function_index,0);
        }
        #endif
    }

}


void userapp_initial(void) ININT_CALL_FUNC
{
    #if CX_20921_UPDATA    
    cxdish_init();
    #endif    

    #if CPU_RATE_PRINT
    init_timer3_getresource();
    #endif

    #if USE_USER_UART
    Suart2_Init();
    #endif
}


/**
  * @功能: 退出识别模式，播放提示音
  * @注意: 无
  * @参数: 无
  * @返回值: 无
  */
static void play_prompt(void)
{
    int index;
    index = 0;

    if(nvdata_save.voice_onoff  ==1)
    {
        xQueueSend(play_Q, &index,200);
    }
    
    #if USE_USER_UART// quit awake state
    uart2_send_AwakeState(3);
    #endif
}


/**
  * @功能: 软timer 回调函数 
  * @注意: 无
  * @参数: 无
  * @返回值: 无
  */
void MainCallback(void)
{   
    if(awaken)
    {
        awaken = 0;
        extern void start_lowpower_cout(void);
        start_lowpower_cout();

    #if USE_SEPARATE_WAKEUP
        int ret = asrSwitchWordsFst(handle,1);
        if(1 == ret)
        {
            mprintf("switch to wakeup\n");
        }
    #endif
        mprintf("go away from wakeup\n");
        play_prompt();  
        
    }
}

extern int get_PlayVoice_EndTime(void);
extern void set_PlayVoice_time(void);
extern void Reset_PlayVoice_EndTime(void);

void Check_Asr_Enter_Sleep(void)
{
    int dLasttime=get_PlayVoice_EndTime();
    int dCurrentTime = xTaskGetTickCount();
     if(awaken&&(dLasttime!=-1))
     {
        if((dCurrentTime - dLasttime)>pdMS_TO_TICKS(SLEEP_TIME))
        {
            if(1 == asr_decode_busy)
            {
                if((dCurrentTime - dLasttime)>pdMS_TO_TICKS(SLEEP_TIME+800))
                {
                    MainCallback();
                }
            }
            else
            {
                MainCallback();
            }
        }
     }
    

}


bool check_wakeup(int function_index)
{
	if(function_index == 1)
	{
		return true;
	}

	return false;
}


/**
* @功能: 播放对应命令词的语音
  * @注意: 无
  * @参数: 无
  * @返回值: 无
  */
void Asr_Playback(int function_index)
{
	int index[3]={25,26,27};
    xSemaphoreTake(WakeupMutex, portMAX_DELAY);
    set_PlayVoice_time();
    if((check_wakeup(function_index) == true) && (!awaken))
    {
        
        awaken = 1;
        mprintf("进入识别模式...\n");
		uart2_send_AwakeState(2);
        send_play_by_func_index(function_index);
    }
    else
    {
        if(1==awaken)
        {     
            //唤醒状态下再次唤醒，也发数据。可能会删除
            
            if(check_wakeup(function_index) == true)
            {
            	uart2_send_AwakeState(2);
            }
           
            if((nvdata_save.voice_onoff ==0)&&(function_index == VOLUME_OPEN_COMMAND_INDEX))
            {
                nvdata_save.voice_onoff  = 1;

            }
            else if((function_index == VOLUME_CLOSE_COMMAND_INDEX))
            {
				if(nvdata_save.voice_onoff  == 1)
                {
                    if(pdPASS != send_play_by_func_index(function_index))
                    {
                        mprintf("send err %s,%d\n",__FUNCTION__,__LINE__);
                    }
                }
                nvdata_save.voice_onoff  = 0;
            }
            
            if(nvdata_save.voice_onoff  == 1)
            {
				
				if(deal_volume_control(function_index) == -1)
                {
					
                		if((g_current_gear == 1) && (function_index == 6))
                		{
							xQueueSend(play_Q, index+1,200);
                		}
						else if((g_current_gear == 12) && (function_index == 7))
						{
							xQueueSend(play_Q, index+2,200);
						}
						else if(pdPASS != send_play_by_func_index(function_index))
	                    {
	                        mprintf("send err %s,%d\n",__FUNCTION__,__LINE__);
	                    }
                	
						
                }
                
            }
            else
            {
            //Play Wakeup Word even in silence mode
                if(function_index == FUNC_ID_WAKE_WORD)
                {
                    if(pdPASS != send_play_by_func_index(function_index))
                    {
                        mprintf("send err %s,%d\n",__FUNCTION__,__LINE__);
                    }                    
                }
				else
					xQueueSend(play_Q, index,200);//关机状态下，控制命令播报请先开机
            }    
        }
        else
        {
#if NOT_CATCH_CMD_WORD
            if(function_index == AsrCmd_Struct[NOT_CATCH_CMD_WORD_INDEX].id)
            {
                send_play_by_func_index(function_index);
            }
            else
#endif                
            {
                mprintf("如需进入识别模式,请说唤醒词\n");
            }
        }
    }
    xSemaphoreGive(WakeupMutex);
}

int32_t GetIndexbyAsrIndex(int32_t index)
{
    int i = 0;

    while(1)
    {
        if(AsrCmd_Struct[i].id == -1)
        {
            break;
        }
        
        if(AsrCmd_Struct[i].id == index)
        {
            return i;
        }
        
        i++;
    }

    return -1;
}

BaseType_t send_play_by_func_index(int32_t func_index)
{
    int index;
    BaseType_t ret;
    #if AEC_FUNC_EN
    extern unsigned int WavFileSize;
    WavFileSize = 0;
    #endif
    index = GetIndexbyAsrIndex(func_index);
    ret = xQueueSend(play_Q, &index,200);
    return ret;
}


/*--------------OTA Function Process start--------------------------------*/
#if OTA_FUNC_ENABLE

#define TIMEOUT_TIMER_NAME TIMER0
#define TIMEOUT_TIMER_IRQ TIMER0_IRQn
#define TIMEOUT_TIMER_IRQ_PRIORITY_PREEMPTION 3
#define TIMEOUT_TIMER_IRQ_PRIORITY_SUB 0
#define TIMER0_ONEUS_COUNT (25)/*FPCLK/FEQDIV*/

#define UPDATE_REQ_CMD_MIN_TIMES 2
#define UPDATE_REQ_RECV_MIN_TIMES 2

uint32_t Update_state = 0x00;
static uint32_t update_req_count = 0;
uint32_t timeout_flag = 0;
extern Data_p Recever_Box;

/*if timer is timeout return ok,and clear flag*/
int32_t check_req_timeout(void)
{
    if(0 != timeout_flag)
    {
        timeout_flag = 0;
        return RETURN_OK;
    }
    else
    {
        return RETURN_ERR;
    }
}

void JmpAddr(unsigned int addr)
{
	asm("LDR R1,[R0]");
	asm("MSR MSP,R1");
	asm("LDR PC,[R0,#4]");
}

void OTAFuncProcess(void)
{
    TIM_InitTypeDef timer0_init;
    NVIC_InitTypeDef NVIC_InitStructure={0};

    UARTInterruptConfig(UART1,UART_BaudRate115200);
    
    /*timer0 initial*/
    Scu_SetDeviceGate(TIMEOUT_TIMER_NAME,ENABLE);
    
    timer0_init.period= TIMER0_ONEUS_COUNT*5000000;/*5s timeout,no used, just init*/
    timer0_init.duty = 50;    
    timer0_init.mode  = COUNTER_MODE_SINGLE; /*one shot mode*/   
    timer0_init.divFreq   = TIMER_CLK_DIVIDED_2;/*1us = 50/2 period*/
    timer0_init.sourceClk    = TIMER_CLK_PCLK;
    timer_init(TIMEOUT_TIMER_NAME,&timer0_init);
    timer_stop(TIMEOUT_TIMER_NAME);
    
    Update_state = UPDATE_REQTIMEOUT;    
    
    NVIC_InitStructure.NVIC_IRQChannel = TIMEOUT_TIMER_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIMEOUT_TIMER_IRQ_PRIORITY_PREEMPTION;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = TIMEOUT_TIMER_IRQ_PRIORITY_SUB;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    if(RETURN_OK != flash_update_buf_init())
    {
        /*alway shoud be ok*/
        bootloader_assert(0);
    }
    
    //SPIFlash初始化
//    QSPI_Init(); 
    
	update_req_count = 0x00;
	send_req_update_req_packet();  
    update_req_count++;

    /*here test,PC ACK 0.012s. 128K falsh read to RAM 0.3s,so here will send at least 3 req packet*/
    timer_setcount(TIMEOUT_TIMER_NAME,TIMER0_ONEUS_COUNT*50000);/*every 50ms, which could last 40ms; timer++*/
    timer_restart(TIMEOUT_TIMER_NAME);
    
    while(1)
    {
        if(RETURN_OK == have_a_new_message())// TODO: need modify
        {
            Resolution_func(Recever_Box); 
        }
        else
        {
            switch(Update_state)
            {
                case UPDATE_REQTIMEOUT:
                    if(RETURN_OK == check_req_ack())/*if req ack get, state to update,don't copy app code*/
                    {   
                        if(check_req_recv() >= UPDATE_REQ_RECV_MIN_TIMES)
                            Update_state = UPDATE_UPDATING;
                        else
                        {
                            send_req_update_req_packet();
                            update_req_count++;
                            timer_setcount(TIMEOUT_TIMER_NAME,TIMER0_ONEUS_COUNT*50000);/*every 50ms, which could last 40ms; timer++*/
                            timer_restart(TIMEOUT_TIMER_NAME);
                        }
                        break;
                    }

                    if(RETURN_OK == check_req_timeout())/*when timeout,and no ack receive,retry*/
                    {
                        if(update_req_count > UPDATE_REQ_CMD_MIN_TIMES)
                        {                                    
                            //JmpAddr(USERCODE_SRAM_START_ADDR);    
                            
                            UARTInterruptConfig(UART1,UART_BaudRate9600);                   
                            timer_stop(TIMEOUT_TIMER_NAME);
                            Update_state = UPDATE_PROCESSQUIT;
                            return;
                        }
                        else
                        {
                            send_req_update_req_packet();
                            update_req_count++;
                            timer_setcount(TIMEOUT_TIMER_NAME,TIMER0_ONEUS_COUNT*50000);/*every 50ms, which could last 40ms; timer++*/
                            timer_restart(TIMEOUT_TIMER_NAME);
                        }
                    }
                    break;

                case UPDATE_UPDATING:
                    if(RETURN_OK == GetUpdateState())                                    
                        Update_state = UPDATE_UPDATECOMPLETE;
                    break;

                case UPDATE_UPDATECOMPLETE:
                    /*
                    if(RETURN_OK == GetUserCodeUpdate())
                    {
                        BootUserCodeAgain();
                    }
                    JmpAddr(USERCODE_SRAM_START_ADDR);                            
                    */
                    /*Initial as normal mode while quit OTA*/
                    UARTInterruptConfig(UART1,UART_BaudRate9600);                   
                    timer_stop(TIMEOUT_TIMER_NAME);
                    Update_state = UPDATE_PROCESSQUIT;
                    return;
                    //break;
                    
                default:
                    break;

            }
        }  
    }

}
#endif
/*--------------OTA Function Process end--------------------------------*/

