/**
  ******************************************************************************
  * @file    task_asr.c 
  * @author  Chipintelli Technology Co., Ltd.
  * @version V1.0.0
  * @date    2016.04
  * @brief   语音识别任务文件
  ******************************************************************************
  **/

#include "includes.h" 
#include "ci100x_asr.h"
#include "task_asr.h"
#include "user_main.h"
#include "user_config.h"
#include "com2_task.h"
#include "ci100x_asrcmd.h"
#include "smt_cs42l52.h"
#include "audio_play_process.h"
#include "midea_protocol.h"
#include "flash_update.h"


#ifdef CHECK_ASR_MEMORY_LEAK/*check asr memory leak used*/
#include <iar_dlmalloc.h>

struct mallinfo alloc_meminfo;
void getMemStatus()
{
    alloc_meminfo = __iar_dlmallinfo();/*可以查看动态分配内存的总大小*/
    mprintf("alloc: %d free:%d\n",alloc_meminfo.uordblks,alloc_meminfo.fordblks);
}
#endif

/**************************************************************************
                    function prototype
****************************************************************************/
void asrmge_getResult(void);
void asrmge_ReInitasr(void);
void asrmge_asrclear(void);
void noresult_just_reset(void);
void asrmge_state_reset(void);

void vad_state_lib(int flag);

unsigned int tts_voice_index = 0;

/**************************************************************************
                    type define
****************************************************************************/
#define VADSTART_2_DNN_TIMEOUT 500/*500ms*/

 

#define SKIP_FRAME   1
#if SKIP_FRAME

    #define SKIP_FRAME_NUM 2

#if USE_ADAPTSKP
    #define MAX_FRAME_TIME (20)
    #define MAX_FRAME_TIME_NUMS (5)
    #define EXIT_MAX_FRAME_TIME_NUMS (1)  
    #define MIN_FRAME_NUMS (20)

int skip_frame_num = SKIP_FRAME_NUM;
int max_frame_time_nums = 0;
int cur_frame_nums = 0;
int max_frames_time =0;

#endif
    #define VADSTART_2_DECODE_TIMEOUT 5000/*5s*/
    #define MAX_SPNOrSIL 250//75
    #define ASR_CNT_TIME_2  16 //??¤??a?-—??–è€…??‰??a?-—????‘???¤èˉ??—??€????cnt?¤???°??????
    #define ASR_CNT_TIME 5//20  //?¨?ˉa€??￥?????￥???°?￥?¤???￥?°a€??|?????¨???¤?¤???o?¨?ˉa€??￥?????￥???°
    #define BESTCNT 1
    #define ASR_BEAM 9.0
    #define VAD_START_OFF 30
#else
    #define VADSTART_2_DECODE_TIMEOUT 5000/*5s*/    
    #define MAX_SPNOrSIL 150
    
    #define ASR_CNT_TIME_2  20 //??¤??a?-—??–è€…??‰??a?-—????‘???¤èˉ??—??€????cnt?¤???°??????
    #define ASR_CNT_TIME 10//20  //èˉ??????°?¤??°‘???è?¤??oèˉ??????°
    #define BESTCNT 2
    #define ASR_BEAM 9.0
    #define VAD_START_OFF 30
#endif


typedef enum
{
    ASRSTATE_P_IDLE = 0,
    ASRSTATE_P_VADSTART,
    ASRSTATE_P_HASDNN_RELSULT1,
    ASRSTATE_P_VADEND_EARLY,
    ASRSTATE_P_DECODE_RESULT_QUICK=4,/*early vad end*/
    ASRSTATE_P_NEWVADSTART,
    ASRSTATE_P_DNNEND_DONE,
    ASRSTATE_P_HASDNN_RELSULT2,
    ASRSTATE_P_DECODE_RESULT_SLOW=8,/*after vad end*/
    ASRSTATE_P_DNNTIMEOUT,
    ASRSTATE_P_VADEND_TIMEOUT,
    ASRSTATE_P_CHECK_VAD_TIMEOUT,
}ASR_StateType;

struct asr_manage_type
{
    uint32_t cur_asr_state;
    TickType_t cur_vad_start_ostick;
    uint32_t clear_count;
}g_asr_manage_t ;
//Add by Will for BT phone start
ASR_NumDetectStruct ASRNumDetect;
int FindNumCmdIndex(char* result,double score);

//Add by Will for BT phone end

extern void IIS0_SlaveRxSDRAMpcm(void);
extern void IIS0_SlaveRxSDRAMpcm_handler(void);

Para_Inof para_info = {0};

int decode_busy = 0;
sys_asr_msg_sentence ASR_Sentence;

extern int InitData(void* p,unsigned int wake_addr,unsigned int asrcmd_addr,unsigned int flag);
int sysinit_index = 0;
#if 0
void GetUserPara(unsigned int* asraddr,unsigned int*asrsize)
{
    volatile unsigned int tmp = 0,voicenums = 0,voc_tmp;
    ConfigParameterInit_Typedef codeinfo = {0}; 
    VoiceInfo vc_info = {0};
    //int tt = 0;
    int i = 0;
    int j = 0;
  
    flashtosdram(0x70000000,0x4000,4 * 1024);
    memcpy(&codeinfo,(void*)0x70000000,sizeof(codeinfo));
    memcpy(&vc_info,(void*)(0x70000000+sizeof(codeinfo) + 4 *  codeinfo.UserCodeBackupNums),sizeof(vc_info));
    *asrsize = vc_info.asrsize;
    *asraddr = 0x70000000;
    tmp = (unsigned int)(0x70000000 + sizeof(codeinfo) + 4 * codeinfo.UserCodeBackupNums) + sizeof(vc_info);
    mprintf("voicenums = %d\n",vc_info.voicenums);
    /*这里播报音数量超过256，一个字节溢出。重新计数.特别注意播报音不能超过256X2*/
    vc_info.voicenums = 256 + vc_info.voicenums;
    voc_tmp = 0x70400000;
    //int last_index = -10000,current_index = -1;

    for(i = 0, j= 0;i < vc_info.voicenums; i++,j += 2)
    {
        VoiceList_Struct[i].pVoiceAddr = *(volatile unsigned int *)(tmp + j * 4);
        VoiceList_Struct[i].VoiceSize = *(volatile unsigned int *)(tmp + j * 4 + 4);
    }
#if 0
    for(i = 0,j = 0;
      (AsrCmd_Struct[i].Index != -1); i++,j += 2)
    {
           
        current_index = AsrCmd_Struct[i].Index;
        sysinit_index = i;
          if(current_index==last_index)
        {
            last_index = current_index;
            AsrCmd_Struct[i].pVoiceAddr = AsrCmd_Struct[i-1].pVoiceAddr;
            AsrCmd_Struct[i].VoiceSize = AsrCmd_Struct[i-1].VoiceSize;
            j -= 2;
            continue;
        }
        last_index = current_index;
        
        AsrCmd_Struct[i].pVoiceAddr = *(volatile unsigned int*)(tmp + j * 4);
        AsrCmd_Struct[i].VoiceSize = *(volatile unsigned int*)(tmp + j * 4 + 4);
    }
#endif
    tts_voice_index = i - 1;
    mprintf("tts_voice_index = %d\n",tts_voice_index);
    VoiceList_Struct[tts_voice_index].pVoiceAddr = 0x704e0000;
    VoiceList_Struct[tts_voice_index].VoiceSize = 0x0;
    sysinit_index -= 4;
    para_info.asrcmdaddr = vc_info.arsaddr;
    para_info.asrcmdsize = vc_info.asrsize;
    para_info.wakeupaddr = vc_info.wakeupaddr;
    para_info.wakeupsize = vc_info.wakeupsize;
#if (VER_DECORD ==  0) 
    g_decord_ver = 0;
#elif (VER_DECORD ==  1) 
    g_decord_ver = 1;   
#endif  

    InitData(&vc_info,WAKECMD_ADDR,ASRCMD_ADDR,1);
}
#else
VoiceInfo vc_info = {0};
void GetUserPara(unsigned int* asraddr,unsigned int*asrsize)
{
    volatile unsigned int tmp = 0;
    int dPlayWordidoff= 1;
    int i = 0;
    int j = 0;
    
    FileConfig_Struct FileConfigInfo = {0};
    unsigned int FileConfigInfoLen;
    FileConfigInfoLen = sizeof(FileConfig_Struct);


    SpiFlash_ReadData(QSPIC,FILECONFIG_SPIFLASH_START_ADDR,(char*)&FileConfigInfo,FileConfigInfoLen);
	flashtosdram(0x70620000,FileConfigInfo.VoicePlayingStartAddr,8 * 1024); 
    
	tmp = (unsigned int)(0x70620000) ;
	tmp +=4;
	vc_info.voicenums= *(volatile unsigned int*)(tmp);
	tmp +=4;
    
	vc_info.arsaddr = FileConfigInfo.ASRCMDModelStartAddr;
	vc_info.asrsize = FileConfigInfo.ASRCMDModelSize;
	vc_info.dnnaddr = FileConfigInfo.DNNModelStartAddr;
	vc_info.dnnsize = FileConfigInfo.DNNModelSize;

    // vc_info.voicenums = 256 + vc_info.voicenums;

    for(i = 0, j= 0;i < vc_info.voicenums; i++,j += 2)
    {
        VoiceList_Struct[i].pVoiceAddr = *(volatile unsigned int *)(tmp + j * 4);
        // VoiceList_Struct[i].pVoiceAddr -= 0x33000;
        VoiceList_Struct[i].VoiceSize = *(volatile unsigned int *)(tmp + j * 4 + 4);
    }

#if 0
    if(vc_info.voicenums>250)
    {
          vc_info.voicenums = 250;
    }
    
   int last_index = -10000,current_index = -1;
    AsrCmd_Struct[0].pVoiceAddr = *(volatile unsigned int*)(tmp);
    AsrCmd_Struct[0].VoiceSize = *(volatile unsigned int*)(tmp + 4);
    

    for(int i = dPlayWordidoff,j = 2; (AsrCmd_Struct[i].Index != -1); i++,j += 2)
    {
           
        current_index = AsrCmd_Struct[i].Index;
         if(current_index==last_index)
        {
            last_index = current_index;
            AsrCmd_Struct[i].pVoiceAddr = AsrCmd_Struct[i-1].pVoiceAddr;
            AsrCmd_Struct[i].VoiceSize = AsrCmd_Struct[i-1].VoiceSize;
            j -= 2;
            continue;
        }
        last_index = current_index;
        AsrCmd_Struct[i].pVoiceAddr = *(volatile unsigned int*)(tmp + j * 4);
        AsrCmd_Struct[i].VoiceSize = *(volatile unsigned int*)(tmp + j * 4 + 4);
    }
#endif
    tts_voice_index = i - 1;
    mprintf("tts_voice_index = %d\n",tts_voice_index);
    VoiceList_Struct[tts_voice_index].pVoiceAddr = 0x704e0000;
    VoiceList_Struct[tts_voice_index].VoiceSize = 0x0;
    sysinit_index -= 4;
    para_info.asrcmdaddr = vc_info.arsaddr; 
    para_info.asrcmdsize = vc_info.asrsize;
    para_info.wakeupaddr = vc_info.wakeupaddr; 
    para_info.wakeupsize = vc_info.wakeupsize;
    g_decord_ver = 1;   
   InitData(&vc_info,WAKECMD_ADDR,ASRCMD_ADDR,1);   
}
#endif

/**
  * @功能: 识别模型句柄创建
  * @注意: 无      
  * @参数:无
  * @返回值:无
  */
void creatcmdmodel(void)
{
    int ret = 0;
    if(para_info.asrcmdsize>ASRCMD_MAXSIZE)
    {
       mprintf("asrcmdsize size if large than %dk,please chang ASRCMD_MAXSIZE in user_config.h",ASRCMD_MAXSIZE/1024);
       return ;
    }else
    {  
    
#if (VER_DECORD==DECORD_V1)      
        handle = asrCreate((const char*)ASRCMD_ADDR,(const char*)WAKECMD_ADDR);
#elif (VER_DECORD==DECORD_V0)        
        handle=asrCreate((const char*)ASRCMD_ADDR,para_info.asrcmdsize);       
#endif    
   
        if(handle)
        {
            mprintf("模型句柄创建成功:handle = %x\n",handle);
        }
        else
        {
            mprintf("模型句柄创建失败!\n");
        }
#if SKIP_FRAME  
        mprintf("\r\n\r\n---------1018 ASR_BEAM=%f  skip  ----------\r\n\r\n",ASR_BEAM);
        asrSetBeam(handle,ASR_BEAM);
#endif    
        asrSetWordsTag(handle,"");//设置词中字之间的间隔标志,默认无标志。
    }
    ptr_addr = 0x20400000;
#if USE_SEPARATE_WAKEUP
    ret=asrStart(handle,1);
    //0:对应asrCreate的第一个参数词组的识别，1:对应asrCreate的第二个参数词组的识别
#else
    ret=asrStart(handle,0);
#endif
    decode_busy = 0;
#if USE_ADAPTSKP
    cur_frame_nums = 0;
#endif
    
    if(ret!=-1)
    {
        mprintf("asrStart成功!\n");
    }
    else
    {
          mprintf("asrStart失败!\n");
    }
}

extern unsigned int result_count;

float score_thd=SCORE_THRESHOLD,wakeup_score_thd = WAKEUP_SCORE_THRESHOLD;

extern volatile unsigned int asr_trigger_event ;

extern  void reset_20921(void);
extern void iis1_ES8388_play_cmd();

int hasResult(void);
int ISResult(float f);

char bestmid_result[50]={0};
char final_result[80]={0};
int decode_frames =0;


void start_lowpower_cout(void)
{
    if(awaken)
    {
        
        //timer_stop(TIMER1);
    }
    else
    {

        TIMx_us(TIMER1,SUSPEND_TIME);

    }
}


void asrmge_init(void)
{
    g_asr_manage_t.cur_asr_state = ASRSTATE_P_IDLE;
    g_asr_manage_t.cur_vad_start_ostick = xTaskGetTickCount();
    g_asr_manage_t.clear_count = 0;
}
    

void asrmge_state_reset(void)
{
    /*need add read result*/
    asr_loginfo("asrmge_state_reset\n");

    g_asr_manage_t.cur_asr_state = ASRSTATE_P_IDLE;
    //clear count recount
    g_asr_manage_t.clear_count = 0; 
    start_lowpower_cout();

}


void asrmge_state_clear(void)
{
    /*need add read result*/
    asr_loginfo("asrmge_state_clear\n");

    g_asr_manage_t.cur_asr_state = ASRSTATE_P_IDLE;
    start_lowpower_cout();
}


void get_decoderesult_and_reset(void)
{
    //get result
    asrmge_getResult();
    //hard reset
    asrmge_ReInitasr();
    //software state reset
    asrmge_state_reset();
}


void noresult_just_reset(void)
{
    //hard reset
    asrmge_ReInitasr();
    //software state reset
    asrmge_state_reset();// TODO:  before reopen more better?
}


void get_decoderesult_and_clear(void)
{
    //get result
    asrmge_getResult();
    //hard clear
    asrmge_asrclear();
    //software state clear
    asrmge_state_clear();
}

int32_t  asr_frame_decode1(asr_datatype * rev_data)
{
    float * decode_start = (float*)rev_data->data_address;
    int len = rev_data->data_length;
    if(rev_data->data_type !=TYPE_HAS_DNN_EVENT)
    {
        asr_logerr("decode type error[%d]\n",rev_data->data_type);
        return -1;
    }
    int ret0=0,ret1=0;
#if SKIP_FRAME    
    
#if USE_ADAPTSKP
    for(unsigned int i=0;i<len*ASR_MODEL_LENGTH;i+= skip_frame_num*ASR_MODEL_LENGTH)
#else
    for(unsigned int i=0;i<len*ASR_MODEL_LENGTH;i+= SKIP_FRAME_NUM*ASR_MODEL_LENGTH)
#endif
      
#else        
    for(unsigned int i=0;i<len*ASR_MODEL_LENGTH;i+=ASR_MODEL_LENGTH)
#endif        
    {
      
#if USE_ADAPTSKP
        int s = OS_GetCurrentMs(0);
#endif
        decode_busy = 1;  
        asrRecognize(handle,(decode_start+i),ASR_MODEL_LENGTH);
        
#if USE_ADAPTSKP
        int e = OS_GetCurrentMs(0);
        if(e-s > MAX_FRAME_TIME)
        {
            max_frame_time_nums++;
        }
        else
        {
            max_frame_time_nums = 0;
        }
        
        if(max_frame_time_nums >= MAX_FRAME_TIME_NUMS)
        {
            skip_frame_num = SKIP_FRAME_NUM+1;
            max_frame_time_nums = 0;
        }
        else if(0==max_frame_time_nums) 
        {
            skip_frame_num = SKIP_FRAME_NUM;
        }
        else
        {
            
        }
        if(cur_frame_nums++ < MIN_FRAME_NUMS )
        {
             skip_frame_num = SKIP_FRAME_NUM;
        }
        //mprintf("skip=%d[%d ms]\n",skip_frame_num,e-s);
#endif
        ret0 = hasResult();
        if(*(volatile unsigned int*)0x40014000 & (1<<1))
        {
            asr_loginfo("HARD VAD END\n");
            *(volatile unsigned int*)0x40014008 |=(1<<1);
#if DECODE_DEBUG
            asr_loginfo("VAD END [%d,%d]\n",*(volatile unsigned int*)(0x40014000+0x60),asrtimer_counter);
#endif 
            LED_VAD_OFF;  
            ret1 =1;
        }
        if(ret0)
        {
            asr_loginfo("RESULT  \n");
            break;
        }
    }
    return ((ret1<<1)|(ret0<<0));
}

int32_t  asr_frame_decode2(asr_datatype * rev_data)
{
    float * decode_start = (float*)rev_data->data_address;
    int len = rev_data->data_length;
    
    if(rev_data->data_type !=TYPE_HAS_DNN_EVENT)
    {
        asr_logerr("decode type error[%d]\n",rev_data->data_type);
        return -1;
    }    
        
    int ret0=0 ,_dnn_end=0;
#if SKIP_FRAME    
#if USE_ADAPTSKP
    for(unsigned int i=0;i<len*ASR_MODEL_LENGTH;i+=skip_frame_num*ASR_MODEL_LENGTH)
#else
    for(unsigned int i=0;i<len*ASR_MODEL_LENGTH;i+=SKIP_FRAME_NUM*ASR_MODEL_LENGTH)
#endif
#else        
    for(unsigned int i=0;i<len*ASR_MODEL_LENGTH;i+=ASR_MODEL_LENGTH)
#endif        

    {
#if USE_ADAPTSKP
        int s = OS_GetCurrentMs(0);
#endif
        decode_busy =1;  
        asrRecognize(handle,(decode_start+i),ASR_MODEL_LENGTH);
#if USE_ADAPTSKP
        int e = OS_GetCurrentMs(0);
        
        if(e-s > MAX_FRAME_TIME)
        {
            max_frame_time_nums++;
        }
        else
        {
            max_frame_time_nums = 0;
            //skip_frame_num = SKIP_FRAME_NUM;
        }
        
        if(max_frame_time_nums >= MAX_FRAME_TIME_NUMS)
        {
            skip_frame_num = SKIP_FRAME_NUM+1;
            max_frame_time_nums =0;
        }
        else if(0==max_frame_time_nums)
        {
            skip_frame_num = SKIP_FRAME_NUM;
        }
        else
        {
            //
        }
        if(cur_frame_nums++ < MIN_FRAME_NUMS)
        {
             skip_frame_num = SKIP_FRAME_NUM;
        }
        //mprintf("skip=%d[%d ms]\n",skip_frame_num,e-s);
#endif
        ret0 = hasResult();
        if(TYPE_DNN_END_EVENT==asr_trigger_event)
        {
            //asr_trigger_event = TYPE_NULL_EVENT;
            _dnn_end =1;
        }
        if(ret0)
        {
            break;
        }
    }
    if(TYPE_DNN_END_EVENT==rev_data->data_type)
    {
        asr_loginfo("DECODE DNN\n");
        _dnn_end =1;
    }
    asr_loginfo("dnn end%d,%d\n",_dnn_end,((_dnn_end<<1)|(ret0<<0)));
    return  ((_dnn_end<<1)|(ret0<<0));
}


/*decode start and no vad_end come*/
void asrmge_dealwith_decode1(asr_datatype * rev_data)
{
    int32_t ret;

    if(TYPE_HAS_DNN_EVENT  == rev_data->data_type)
    {
        g_asr_manage_t.cur_asr_state =   ASRSTATE_P_HASDNN_RELSULT1;
    }
    else if(TYPE_DNN_END_EVENT == rev_data->data_type)
    {
        get_decoderesult_and_clear();
        asr_logdebug("dnn end when decode\n\n");
        return;
    }
    else
    {
        asr_logerr("error msg type:decode1\n\n");
        noresult_just_reset();
        return;
    }
    
    ret = asr_frame_decode1(rev_data);
    switch (ret)
    {
        case 0:
            /*no result,no vad end*/
            g_asr_manage_t.cur_asr_state =   ASRSTATE_P_HASDNN_RELSULT1;
            break;
        case 1:
            /*has result, no vad end*/
            if(ISResult(MIN_SCORE))
            {
                asr_loginfo("decode 1 good \n");
                LED_VAD_OFF;  /*VAD END LED*/
                get_decoderesult_and_reset();
            }
            else
            {
                 //result bad   
                 asr_loginfo("decode 1 bad \n");
                 asrmge_getResult();
                 g_asr_manage_t.cur_asr_state =   ASRSTATE_P_HASDNN_RELSULT1;
            }
            break;
        case 2:
            /*no result,has vad end*/
            g_asr_manage_t.cur_asr_state =   ASRSTATE_P_VADEND_EARLY;
            break;
        case 3:
            /*has result,has vad end*/
            if(ISResult(MIN_SCORE))
            {
                asr_loginfo("decode 1 good \n");
                LED_VAD_OFF;  /*VAD END LED*/
                get_decoderesult_and_reset();
            }
            else
            {
                g_asr_manage_t.cur_asr_state =   ASRSTATE_P_VADEND_EARLY;
            }
            break;
        default:
            asr_logwar("decode return error\n");
            break;
    }

}


void decode2_hasresult_nodnnend(void)
{
    if(ISResult(MIN_SCORE))
    {
        asr_loginfo("good result\n");
        get_decoderesult_and_reset();
    }
    else
    {
        //result bad   
        //asrmge_getResult();
        asr_loginfo("bad result\n");
        int asrmge_softvad(int frames);
        int sfvad = asrmge_softvad(5);
        
        if(1==sfvad)
        {
            mprintf("sfvad\n");
            //soft has vad
            int sfvad_timeout = 5/*30*/,has_vad = 0;
            while(sfvad_timeout--)
            {
                if(*(volatile unsigned int*)0x40014000 & (1<<0))
                {
                    has_vad = 1;
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(10)); 
            }
            if(!has_vad)
            {
                mprintf("sfot vad timeout\n"); 
                //vad time out 
                get_decoderesult_and_reset();
            }
            else
            {
                mprintf("soft vad coming\n");  
                asrmge_getResult();
                int dnn_timeout = 50;
                while(dnn_timeout--)
                {
                    if(TYPE_DNN_END_EVENT==asr_trigger_event)// TODO: clear postion  in state reset ?
                    {
                        asr_trigger_event = TYPE_NULL_EVENT;
                        break;
                    }
                    vTaskDelay(pdMS_TO_TICKS(10)); 
                }
                //等待DNN END到来
                if(dnn_timeout<=0)
                {
                    get_decoderesult_and_reset();
                }
                else
                {
                    if(g_asr_manage_t.clear_count>10)
                    {
                        get_decoderesult_and_reset();
                    }
                    else
                    {
                        asrmge_asrclear();
                        asrmge_state_clear();
                    }
                }
            }
         }
        else if (2==sfvad)
        {
            //hard  vad has  coming
            mprintf("hard vad coming\n");  
            asrmge_getResult(); // TODO:  after dnn check will get result again?
            //等待DNN END 到来
            int dnn_timeout = 50;
            while(dnn_timeout--)
            {
                if(TYPE_DNN_END_EVENT==asr_trigger_event)
                {
                    asr_trigger_event = TYPE_NULL_EVENT;
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(10)); 
            }
            //等待DNN END到来
            if(dnn_timeout<=0)
            {
                mprintf("dnn end time out\n");  
                get_decoderesult_and_reset();
            }
            else
            {
                mprintf("dnn end coming\n");  
                if(g_asr_manage_t.clear_count>10)
                {
                    get_decoderesult_and_reset();
                }
                else
                {
                    asrmge_asrclear();
                    asrmge_state_clear();
                }
            }
        }
        else
        {
            mprintf("no soft vad\n");  
            get_decoderesult_and_reset();
        }
    }

}


void decode2_noresult_nodnnend(asr_datatype * rev_data)
{
    //判读新VAD是否已经来了
    if(*(volatile unsigned int *)0x40014000 & (1<<0))
    {
        //硬件VAD 已经到来
        //SDRAM_DNN_BAK / now not cp
        int max_frames = 0;

        asr_datatype asr_receive_data1;
        unsigned int sdram_dnn_addr = SDRAM_DNN_BAK;
        int continue_decode = 0;
        
        while(max_frames--)
        {
            memset(&asr_receive_data1,0,sizeof(asr_receive_data1)); 
            if(pdPASS==xQueueReceive(ASR_xQueue,&asr_receive_data1,pdMS_TO_TICKS(0)))
            {
                if(TYPE_HAS_DNN_EVENT==asr_receive_data1.data_type)
                {
                    memcpy((void*)sdram_dnn_addr,(void*)asr_receive_data1.data_address,\
                    asr_receive_data1.data_length*4*ASR_MODEL_LENGTH);
                    sdram_dnn_addr+=asr_receive_data1.data_length*4*ASR_MODEL_LENGTH;
                }
                else if(TYPE_DNN_END_EVENT==asr_receive_data1.data_type)
                {
                    //clear asr
                    continue_decode =1;
                    asr_loginfo("dnn end event break\n");
                    break;
                }
                else
                {
                    asr_logerr("xQueueReceive type error\n");
                    continue_decode =2;
                    break;
                }
            }
            else
            {
                asr_logwar("xQueueReceive timeout error\n");
                continue_decode =3;
                asrmge_asrclear();
                break;
            }
        }

        xQueueReset(ASR_xQueue);

        asrmge_asrclear();

        switch (continue_decode)
        {
            case 0:
            case 1:
            {
                float * last_decode =(float*)SDRAM_DNN_BAK;
#if SKIP_FRAME    
                for(;(unsigned int)last_decode<sdram_dnn_addr;last_decode+=SKIP_FRAME_NUM*ASR_MODEL_LENGTH)
#else        
                for(;(unsigned int)last_decode<sdram_dnn_addr;last_decode+=ASR_MODEL_LENGTH)
#endif   
                {
                    decode_busy =1;  
                    asrRecognize(handle,last_decode,ASR_MODEL_LENGTH);
                    int ret0 = hasResult();
                    if(ret0)
                    {
                        break;
                    }
                }

                if(ISResult(MIN_SCORE))
                {
                    asr_loginfo("cpy good result\n");
                    get_decoderesult_and_reset();
                }
                else
                {
                    asrmge_getResult();
                    g_asr_manage_t.cur_asr_state = ASRSTATE_P_IDLE;
                    asr_loginfo("cpy bad result\n");
                }
                break;
            }
            case 2:
            case 3:
                get_decoderesult_and_reset();
                break;
            default:
                asr_logerr("error\n");
                break;
        }
    }
    else
    {
        //继续解码
        g_asr_manage_t.cur_asr_state =   ASRSTATE_P_HASDNN_RELSULT2;
    }
}


void decode2_hasresult_anddnnend(void)
{
    if(ISResult(MIN_SCORE))
    {
        asr_loginfo("good result\n");
        get_decoderesult_and_reset();
    }
    else
    {
        //result bad   
        //asrmge_getResult();
        asr_loginfo("bad result\n");
        int asrmge_softvad(int frames);
        int sfvad = asrmge_softvad(5);
        
        if(1==sfvad)
        {
            mprintf("sfvad\n");
            //soft has vad
            int sfvad_timeout = 5;//30;
            int has_vad = 0;
            while(sfvad_timeout--)
            {
                if(*(volatile unsigned int*)0x40014000 & (1<<0))
                {
                    has_vad = 1;
                    break;
                }
                vTaskDelay(10); 
            }
            if(!has_vad)
            {
                asr_loginfo("sfot vad timeout\n"); 
                //vad time out 
                get_decoderesult_and_reset();
            }
            else
            {
                asr_loginfo("soft vad coming\n");  
                get_decoderesult_and_clear();
            }
        }
        else if (2==sfvad)
        {
            //hard  vad has  coming
            asr_loginfo("hard vad coming\n");  
            get_decoderesult_and_clear();
        }
        else
        {
            asr_loginfo("no soft vad\n");  
            get_decoderesult_and_reset();
        }
    }
}


/*decode continue after vad_end come*/
void asrmge_dealwith_decode2(asr_datatype * rev_data)
{
    int32_t ret;

    if(TYPE_HAS_DNN_EVENT == rev_data->data_type)
    {
        g_asr_manage_t.cur_asr_state =   ASRSTATE_P_HASDNN_RELSULT2;
    }
    else if(TYPE_DNN_END_EVENT == rev_data->data_type)
    {
        get_decoderesult_and_clear();
        asr_loginfo("dnn end when decode\n\n");
        return;
    }
    else
    {
        asr_logerr("error msg type:decode2\n\n");
        noresult_just_reset();
        return;
    }
    
    ret = asr_frame_decode2(rev_data);
    switch(ret)
    {
        case 0:
            //no result no dnn end
            g_asr_manage_t.cur_asr_state =   ASRSTATE_P_HASDNN_RELSULT2;
            break;
        case 1:
            //has result ,no dnn end
            decode2_hasresult_nodnnend();
            break;
        case 2:
            asr_logdebug("no result ,dnn end\n");
            decode2_noresult_nodnnend(rev_data);
            break;
        case 3:
            //has result  dnn end
            decode2_hasresult_anddnnend();
            break;
        default:
            asr_logerr("decode2 error\n");
            break;
    }

}


void asrstate_programdealwith(asr_datatype * rev_data)
{
    switch (g_asr_manage_t.cur_asr_state)
    {
        case ASRSTATE_P_IDLE:
            if(TYPE_VAD_START_EVENT== rev_data->data_type)
            {
                g_asr_manage_t.cur_asr_state =   ASRSTATE_P_VADSTART;
                g_asr_manage_t.cur_vad_start_ostick = rev_data->ostick;
            }
            else
            {
                asr_logerr("error msg type:state deal\n\n");
                noresult_just_reset();
            }
            break;
        case ASRSTATE_P_VADSTART:
        case ASRSTATE_P_HASDNN_RELSULT1:
            #if DECODE_DEBUG
            mprintf("DECODE ING [%d,%d]\n",*(volatile unsigned int*)(0x40014000+0x60),asrtimer_counter);
            #endif
            asrmge_dealwith_decode1(rev_data);
            break;
        case ASRSTATE_P_VADEND_EARLY:
        case ASRSTATE_P_HASDNN_RELSULT2:
            asrmge_dealwith_decode2(rev_data);
            break;
        default:
            break;
    }
}


/*for vad too long case*/
void deal_vad_end_timeout(void)
{
    if((ASRSTATE_P_HASDNN_RELSULT1 == g_asr_manage_t.cur_asr_state)
        ||(ASRSTATE_P_HASDNN_RELSULT2 == g_asr_manage_t.cur_asr_state))
    {
        if((xTaskGetTickCount()-g_asr_manage_t.cur_vad_start_ostick )>pdMS_TO_TICKS(VADSTART_2_DECODE_TIMEOUT))
        {
            /*also named vad too long*/
            g_asr_manage_t.cur_asr_state =  ASRSTATE_P_VADEND_TIMEOUT; 
            asr_logwar("ASRSTATE_P_VADEND_TIMEOUT\n\n\n");
            get_decoderesult_and_reset();
        }
    }
}

void asrs_timeout_dealwith(asr_datatype * rev_data)
{

    switch (g_asr_manage_t.cur_asr_state)
    {
        case ASRSTATE_P_VADSTART:
            if((xTaskGetTickCount()-g_asr_manage_t.cur_vad_start_ostick )>pdMS_TO_TICKS(VADSTART_2_DNN_TIMEOUT))
            {
                g_asr_manage_t.cur_asr_state =  ASRSTATE_P_DNNTIMEOUT; 
                asr_logerr("ASRSTATE_P_DNNTIMEOUT\n\n\n");
                noresult_just_reset();
            }
            break;
        case ASRSTATE_P_HASDNN_RELSULT1:
            deal_vad_end_timeout();
            break;
        default:
            break;
    }
    
}


void vTaskASRdecode(void *pvParameters)
{
    asr_datatype asr_receive_data;    
    
    vTaskSuspendAll();
  
    creatcmdmodel();
   
    ASR_VAD_DNN_Init();
    
#if GET_20921VER
    DelayMs(2000);
    get_fw_version();
#endif    

    vad_bug = 1;

    asrmge_init();

    if(!xTaskResumeAll())
    {
        taskYIELD();
    }
#if DECODE_DEBUG
    memset((void*)0x70500000,0,1024*1024);
    GetCurrentMs();
#endif    
    
    for( ;; )
    { 
        memset(&asr_receive_data,0,sizeof(asr_receive_data));
        
        if(pdPASS==xQueueReceive(ASR_xQueue,&asr_receive_data,pdMS_TO_TICKS(100)))
        {
            timer_stop(TIMER1);

            asr_logdebug("msg 0x%x [%#x]\n",asr_receive_data.data_type,asr_receive_data.data_address);
            asr_logdebug("state %d\n",g_asr_manage_t.cur_asr_state);

            if((TYPE_DNN_CALC_ERR_EVENT == asr_receive_data.data_type) ||
                (TYPE_DNN_BUS_REQ_ERR_EVENT == asr_receive_data.data_type) ||
                (TYPE_DNN_ALLMOSTFULL_EVENT == asr_receive_data.data_type))
            {
                /*TODO:复位操作*/
                asr_logerr("hardware error\n");
                get_decoderesult_and_reset();
            }
            else
            {
                asrstate_programdealwith(&asr_receive_data);
                deal_vad_end_timeout();
            }
        }
        else
        {
            //timeout 
            asrs_timeout_dealwith(&asr_receive_data);
        }
    }
}


#define SIL_ENG  7e7
#define SPK_ENG 7e7
 
static float last_f =-10.0;
int bestmid_result_count =0,spn_cnt=0;

int IsSpecialWords(char *result)
{
    int size = 250;//sizeof(specialwords_lst)/sizeof(*specialwords_lst);
    char words[80]={0};
    sscanf(result,"%s",words);
    for(int i=0; specialwords_lst[i].cnt!=-1 && (i< size);i++)
    {
        if(!strcmp(words,specialwords_lst[i].words))
        {
            return specialwords_lst[i].cnt;
        }
    }        
    return 0;
}
int hasResult(void)
{
    char *result=0;
    static char last_result[50]={0},cnt=0 ;
    int special_cnt = 16; 
    int ret =asrGetResult(handle,( char**)&result),r=0;
    if(NULL != result)
    {
        //mprintf("[result %s]\n",result);
    }
    memset(final_result,0,80);
	// szk deal #
 if((2==ret) &&(result) && strstr(result,"#"))
	{
		ret = 0;
        result = NULL;
        //return 0;//deal 			
	}
 
    if((2==ret)&&result)
    {
      char* pf=0;
      if((pf=strstr(result,SCORE_TAG)) && (!strstr(result,SIL_TAG))\
        &&(!strstr(result,SPN_TAG)))
      {
            if(atof(pf+SCORE_LEN)>MIN_SCORE)
            {
                bestmid_result_count++;
            }
            if(atof(pf+SCORE_LEN)>last_f)
            {
                last_f = atof(pf+SCORE_LEN);
                strcpy(bestmid_result,result);
            }
            char cur_cmd[50],best_cmd[50];
            strcpy(cur_cmd,result);
            strcpy(best_cmd,bestmid_result);
            if(pf=strstr(best_cmd,SCORE_TAG))
            {
                *pf =0;
                pf = strstr(cur_cmd,SCORE_TAG);
                if(pf)
                {
                    *pf=0;
                    if(strcmp(cur_cmd,best_cmd))
                    {
                        last_f =-10.0;
                    }
                }
            }
      }
      if(!strncmp(result,last_result,strlen(last_result))\
          &&(pf = strstr(result,SCORE_TAG))&& \
          (!strstr(result,SIL_TAG))&& \
          (!strstr(result,SPN_TAG)))
        {
            if( atof(pf+SCORE_LEN)>=MIN_SCORE)
            {
                cnt++;
                special_cnt=IsSpecialWords(result);
                if(special_cnt!= 0)
                {                                
                    if(cnt>special_cnt)// 
                    {
                        strcpy(final_result,result);
                        cnt=0;
                        r=1;
                    }                          
                }else
                {
                    if(cnt>ASR_CNT_TIME)
                    {
                        cnt=0;
                        strcpy(final_result,result);
                        r=1;
                    }
                   
                    if(atof(pf+SCORE_LEN) >= MAX_SCORE)
                    {
                        cnt=0;
                        strcpy(final_result,result);
                        r=1;
                    }
                } 
                spn_cnt =0;
            }else
            {
                cnt=0;
                spn_cnt++;
                
            }
            
        }else
        {
            cnt=0;
        }
 
        if(spn_cnt>MAX_SPNOrSIL) 
        {
              asr_loginfo("< too many spn or sil %d>\n",spn_cnt);
              r =1;
              spn_cnt =0;
              cnt =0;
        }
        strcpy(last_result,result);
        char *p=strstr(last_result,SCORE_TAG);
        if(p)*(p )=0;
    }else
    {
        cnt=0;
    }
 
    return r;
}

int ISResult(float f)
{
    char * p=0,*ret=0;
    int r=asrGetResult(handle,( char**)&ret);
//szk deal #
if((2==r) &&(ret) && strstr(ret,"#"))
	{
		ret = NULL;
        r = 0;
        //return 0;//deal 			
	}
    asr_loginfo("{%s,cnt %d}\n",bestmid_result,bestmid_result_count);
    if(ret && 2==r)
    {
        p= strstr(ret,SCORE_TAG);
        if(p)
        {
            float cur_f = atof(p+SCORE_LEN);
            if(cur_f > f )
            {
                //memset(bestmid_result,0,sizeof(bestmid_result));
                return 1;
            }
        }
        p = strstr(bestmid_result,SCORE_TAG);
        if(p)
        {
            float cur_f = atof(p+SCORE_LEN);
            if(cur_f > f )
            {
                //memset(bestmid_result,0,sizeof(bestmid_result));
                return 1;
            }
        }
    }else
    {
        p = strstr(bestmid_result,SCORE_TAG);
        if(p)
        {
            float cur_f = atof(p+SCORE_LEN);
            if(cur_f > f )
            {
                //memset(bestmid_result,0,sizeof(bestmid_result));
                return 1;
            }
        }
    }
    return 0;  

}


/*hardware clear*/
void asrmge_asrclear(void)
{
    uint32_t int_status;
    //asr_loginfo("int status = 0x%x\n",*(volatile unsigned int *)(ASR_BASE + 0));
    //asr_loginfo("flow status = 0x%x\n",*(volatile unsigned int *)(ASR_BASE + 0x54));
    //asr_loginfo("fe status = 0x%x\n",*(volatile unsigned int *)(ASR_BASE + 0x5c));
    /*clear*/
    *(volatile unsigned int *)(0x4001400c) |= ((0x1 << 8)); 
    //*(volatile unsigned int *)(0x4001400c + EXTSDRAM_SLICE_RPTR_REG) = 0x0;

    xQueueReset(ASR_xQueue);

    int_status = (*(volatile unsigned int *)(ASR_BASE + 0));
    if(0x8 == int_status&0xa)/*hardware need double check!*/
    {
        int_status = (*(volatile unsigned int *)(ASR_BASE + 0));
    }
    /*voice too short,so no dnn!bad judge method, interrupt time compare more better? */
    if((0 == *(volatile unsigned int *)(ASR_BASE + 0x5c)) && (int_status&0x2))
    {
        *(volatile unsigned int *)(ASR_BASE + 0x8) |=(1<<0);
        asr_logerr("voice too short\n");
    }

    NVIC_ClearPendingIRQ(ASR_IRQn);
    NVIC_ClearPendingIRQ(VAD_IRQn);
    NVIC_EnableIRQ(VAD_IRQn);

    //clear count count
    g_asr_manage_t.clear_count++;
    
    asr_loginfo("int status = 0x%x\n",*(volatile unsigned int *)(ASR_BASE + 0));
    asr_loginfo("flow status = 0x%x\n",*(volatile unsigned int *)(ASR_BASE + 0x54));
    asr_loginfo("fe status = 0x%x\n",*(volatile unsigned int *)(ASR_BASE + 0x5c));
}


int ASR_SentenceCheck(void)
{
    int index;
    if(ASR_Sentence.ASR_SentenceArray[ASR_Sentence.ASR_SentenceIndex] != SentenceEnd)
    {
        index = ASR_Sentence.ASR_SentenceArray[ASR_Sentence.ASR_SentenceIndex];
        ASR_Sentence.ASR_SentenceIndex++;
        //xQueueSend(play_Q, &index,200);
        insert_prompt(index,1);
        
    }        
    else
    {
        index = ASR_Sentence.ASR_SentenceArray[0];
        ASR_Sentence.ASR_SentenceArray[0] = SentenceEnd;     //Clear sentence
        ASR_Sentence.ASR_SentenceIndex = 0x00;
        
    }
    return index;
}        

void send_result_to_usertask(char *presult)
{
    if(NULL == presult)
    {
        return;
    }
    char* pscore = strstr(presult,SCORE_TAG);
    mprintf("send result:%s\n",presult);
    if(strstr(presult,"#"))
    {
	    mprintf("### not send result %s\n",presult);
        return;  
    }
    if(pscore)
    {
        pscore += SCORE_LEN;
        float score = atof(pscore);
        asr_loginfo("%0.2f\n",score);

        int index = FindCmdIndex(presult,score);//ID
        
        if(-1 != index)
        {
            user_msg_t send_msg;
            send_msg.msg_type = MSG_TYPE_SYS_ASR;
            send_msg.msg_data.asr_data.asr_status = MSG_ASR_STATUS_GOOD_RESULT;
            send_msg.msg_data.asr_data.asr_index = index;
            send_msg.msg_data.asr_data.asr_score  = score;
            xQueueSend(user_task_queue,&send_msg,200);
        }
    }
}

#if  USE_SEPARATE_WAKEUP
int IsWakeupWords(char* preuslt)
{
    if(!preuslt)return 0;
    int i=0;
    int size = 250;//sizeof(wakewords_lst)/sizeof(wakewords_lst[0])
    char words[80];
    sscanf(preuslt,"%s",words);
    for(i=0;i<size;i++)
    {
        if(!strcmp(words,wakewords_lst[i]))
        {
            return 1;
        }
        if(!strcmp("END",wakewords_lst[i]))
        {
            return 0;
        }
    }
    return 0;
}
#endif

void asrmge_getResult(void)
{
    char * ret =0,*presult=0;
    int r=asrGetResult(handle,( char**)&ret);

    char *pf = NULL;
    asr_loginfo("VAD <%d>\n",*(volatile unsigned int*)(0x40014000+0x60));
    float fs=-10.0,midfs=-20.0;
    #if USE_SEPARATE_WAKEUP
    int iswakeup = 0;
    #endif
    strcpy(bestmid_result,final_result);
    if((2==r) &&(ret) && strstr(ret,"#"))
	{
        ret = NULL;
        r =0;
 		//return  ;//deal 			
	}
    if(2==r)
    {
      asr_loginfo("result =%s\n",ret);
      pf = strstr(ret,SCORE_TAG);
      if(pf)fs = atof(pf+SCORE_LEN);
    }
    pf = strstr(bestmid_result,SCORE_TAG);
    if(pf)midfs = atof(pf+SCORE_LEN);
    asr_loginfo("bestmid_result =%s %d\n",bestmid_result,bestmid_result_count);
    if((fs>WAKEUP_SCORE_THRESHOLD||midfs> WAKEUP_SCORE_THRESHOLD)&&\
      fs < 10&& midfs<10)
    {
        presult = (midfs>fs)?bestmid_result:ret;
        if(presult==bestmid_result)
        {
            if(bestmid_result_count>BESTCNT) 
            {
                #if  USE_SEPARATE_WAKEUP
                iswakeup = IsWakeupWords(presult);
                #endif
                send_result_to_usertask(presult);
            }
            else
            {
                asr_loginfo("good mid_result too  little\n");
            }
        }
        else
        {
            #if  USE_SEPARATE_WAKEUP
            iswakeup = IsWakeupWords(presult);
            #endif
            send_result_to_usertask(presult);
        }
    }
    else
    {

    }
    last_f = -10.0;
    bestmid_result_count =0;
    spn_cnt=0;
    memset(bestmid_result,0,sizeof(bestmid_result));
#if USE_SEPARATE_WAKEUP
    if( awaken ||(1==iswakeup))
    {
        asrStart(handle,0);
        mprintf("asring..\n");
        //waking  enter asr...
    }else
    {
        asrStart(handle,1);
        mprintf("waking..\n");
    }
#else
    asrStart(handle,0);
#endif
    decode_busy = 0;  
#if USE_ADAPTSKP
    cur_frame_nums = 0;
#endif
    
}


/*hardware reset*/
void asrmge_ReInitasr(void)
{
    asr_Cancelprocessing();
    xQueueReset(ASR_xQueue);
    ASR_VAD_DNN_Init();
}


int asrmge_softvad(int frames)
{
    extern unsigned  int vad_engrecord_idx ,vad_engrecord[500];
    int idx = vad_engrecord_idx;
    int count =40,spncnt = 0,silcnt=0,ret= 0;
    for(int i=0;i<count;i++)
    {
        if(vad_engrecord[idx]>(unsigned int)SPK_ENG)
        {
            spncnt++;
            silcnt=0;
            if(spncnt>5)
            {
                ret =1;
                break;
            }
        }else
        {
            silcnt++;
            if(silcnt>3)
            {
                spncnt=0;
                silcnt =0;
            }
        }
        if(idx)
        {
            idx--;
        }else
        {
            idx = 500-1;
        }
    }
    if(*(volatile unsigned int*)0x40014000 & (1<<0))
    {
        //hard vad coming
        ret =2;
    }
    return ret;
}



static unsigned char alarm_clock_run_times[8] = {0};
static unsigned alarm_clock_flag = 0;


void set_alarm_clock(unsigned char alarm_num)
{
    unsigned char mask_data = 0x1;
    mask_data = 1<<(alarm_num - 1);
    alarm_clock_flag |= mask_data;
}

void clear_all_alarm(void)
{
    alarm_clock_flag = 0;
    memset(alarm_clock_run_times,0,8);
}


void alarm_clock_process(void)
{
    unsigned char mask_data = 0x1;
    if(alarm_clock_flag)
    {
        if(alarm_clock_flag & mask_data)
        {
            // insert_prompt(alarm_clock_num[0],1);
            insert_prompt(231,1);
            vTaskDelay(1000);
            alarm_clock_run_times[0]++;
            mprintf("alarm_clock_run_times = %d\n",alarm_clock_run_times[0]);
            if(alarm_clock_run_times[0] == 5)
            {
                insert_prompt(224,1);
                clear_all_alarm();
            }
        }
    }
}

/**
  * @功能: FreeRTos 的测试函数
  * @注意: 无      
  * @参数:无
  * @返回值:无
  */
//uint8_t pcWriteBuffer[500]={0};

void vTask_Test(void *pvParameters)
{
    static unsigned int times = 0;
    static unsigned char bt_state = 0;
    
    clear_all_alarm();
    for(;;)
    {
        mprintf("%#x:RTOS\n",times);
        
       
        if(bt_state != get_bt_connect_state())
        {
            bt_state = get_bt_connect_state();

            if(1 == bt_state)
            {
                mprintf("bt connect!\n");
                insert_prompt(222,1);//蓝牙已连接
            }
            else if(0 == bt_state)
            {
                mprintf("bt disconnect!\n");
                insert_prompt(203,1);//蓝牙已断开
            }
        }

        alarm_clock_process();

        times++;
        if((times == 2))
        {
            GPIO_Output(GPIO1,(GPIO_Pinx)GPIO_Pin6,0);
        }
        #if USE_IWDG
        iwdg_watch |= (1<<0); //喂狗
        #endif
        vTaskDelay(500); 
    }
}

/**
  * @功能: ASR_VAD 初始化配置
  * @注意: 无      
  * @参数:无
  * @返回值:无
  */
void ASR_VAD_DNN_Init(void)
{
    NVIC_InitTypeDef NVIC_InitStruct = {0};
    ASR_InitStruct InitStruct = {0};
    static char asr_inits = 0;
    if(!asr_inits)
    {
        Scu_Setdiv_Parameter(ASR_BASE,0x12);
        Scu_SetClkDiv_Enable(ASR_BASE);
    
        Scu_SetDeviceGate(ASR_BASE,ENABLE);
        Scu_Setdiv_Parameter(DNN_DIV_BASE,0x6);
        Scu_SetClkDiv_Enable(DNN_DIV_BASE);

        Scu_SetDeviceGate(VAD_GATE,1);
        Scu_SetDeviceGate(FE_GATE,1);
        Scu_SetDeviceGate(DNN_GATE,1);
                
        asr_inits = 1;
    }    
    Scu_Setdevice_Reset(ASR_AHBBASE);
    Scu_Setdevice_Reset(ASR_BASE);
    
    Scu_Setdevice_ResetRelease(ASR_AHBBASE);
    Scu_Setdevice_ResetRelease(ASR_BASE);
#if USE_ASR8388 ||USE_6_152_16_2    
    extern unsigned int vad_backengrg;
    InitStruct.VAD_MAX_SILLEN = (25 << 16); //25 
    InitStruct.VAD_MIN_BACK_ENG =  vad_backengrg;//20000000; 
    InitStruct.VAD_MIN_SILLEN =  (10<< 8) ; 
    InitStruct.VAD_PRSP = 0x3F000000;//  0x3F000000 - 0.5 0x3F7D70A4;//
    InitStruct.VAD_START_OFFSET =  (40 << 24); 
#else
    InitStruct.VAD_MAX_SILLEN = (25 << 16); //25 
    InitStruct.VAD_MIN_BACK_ENG =  40000000;//5000000;
    InitStruct.VAD_MIN_SILLEN =  (10<< 8) ; 
    InitStruct.VAD_PRSP = 0x3E4CCCCD;//0x3f266666;
    InitStruct.VAD_START_OFFSET =  (35 << 24); 
#endif 
    asr_init(&InitStruct);

    NVIC_ClearPendingIRQ(ASR_IRQn);
    NVIC_ClearPendingIRQ(VAD_IRQn);
    
    NVIC_InitStruct.NVIC_IRQChannel = ASR_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 6;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStruct);
    
    NVIC_InitStruct.NVIC_IRQChannel = VAD_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 6;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @功能: 词条搜索
  * @注意: 无      
  * @参数:1.result 输入词条
  *       2.score 词条的得分
  * @返回值:词条的index 
  */
int FindCmdIndex(char* result,double score)
{
    int ret = -1;
    int res_length=0;
    char* pscore;
//    int AsrCmd_lenth = 0;
      
    if(!result)
    {
        return -1; 
    }
    
    pscore = strstr(result,SCORE_TAG);
    res_length = pscore - result;
    for(int i=0;(AsrCmd_Struct[i].Index != -1) && (i < 250);i++)
    {
        if(res_length != strlen(AsrCmd_Struct[i].pCmd))
        {
            continue;
        }
        if(!strncmp(result,AsrCmd_Struct[i].pCmd,res_length))
        {
            #if (VER_DECORD==DECORD_V0)         
            if(fabs(score - PERCISION) <= AsrCmd_Struct[i].Score)
            #elif (VER_DECORD==DECORD_V1)                     
            if(score - PERCISION > AsrCmd_Struct[i].Score)
            #endif            
            {
                //ret = i;
                ret = AsrCmd_Struct[i].Index;
            }
#if  REPEAT_TRYAGAIN            
            if(((score - PERCISION) <= AsrCmd_Struct[i].Score) && (score - PERCISION )>0.05)
//           else
                {
                ret = SPEAKCLEARLY;
                }

#endif               
           return ret;
         
           //break;
        }
        
    }
#if  REPEAT_TRYAGAIN     
    ret = SPEAKCLEARLY;
#endif    
    return ret;
}

int Get_NextInitnum(char *filename_head)
{
    char filename[50]={0};
    int i,result;
    FIL filehandle;
    
    for(i=0;i<512;i++)
    {        
        sprintf(filename,"%s%d.pcm",filename_head,i);
        result =f_open(&filehandle, filename, FA_READ);
        if(result != FR_OK)
        {
            mprintf("open %s fail,so Fixnum %d ",filename,i);
            break;
        }else
        {
            result = f_close(&filehandle);
            if(result != FR_OK)
            {
                mprintf( "!!! close file somthing err:please reset!\r\n");
            }
        }
    }
    return i;
}

//FIL PCM_voiceFile,VAD_logfile,CorpusFile,NoiseFile;

int tf_mounted_state = FR_DISK_ERR;

void vad_time2_init(void)
{
    static int ustime_init = 0;
    if(!ustime_init)
    {
              vad_cnt_timer = 0xffffffff;
              vad_end_timer = 0xffffffff;
              ustime_init = 1;        
        //delay unit 10ms
        NVIC_InitTypeDef NVIC_InitStructure={0};
          Scu_SetDeviceGate(TIMER2,ENABLE);
        TIMx_us(TIMER2,10000);
        NVIC_InitStructure.NVIC_IRQChannel = TIMER2_IRQn;
           NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
           NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
           NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
           NVIC_Init(&NVIC_InitStructure);
    } 
}
void vad_time0_init(void)
{
    static int ustime_init = 0;
    if(!ustime_init)
    {
        vad_cnt_timer = 0xffffffff;
        vad_end_timer = 0xffffffff;
        ustime_init = 1;        
        //delay unit 5s
        NVIC_InitTypeDef NVIC_InitStructure={0};
        Scu_SetDeviceGate(TIMER0,ENABLE);
        TIMx_us(TIMER0,5000000);
        timer_stop(TIMER0);
        NVIC_InitStructure.NVIC_IRQChannel = TIMER0_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    } 
}

extern unsigned char vad_start_flag;
void vad_state_lib(int flag)
{
    vad_time2_init();
    vad_time0_init();
    if(flag != 5)
    {
        new_printf("VAD[%d]\n",flag);
        if(0 == flag)
        {
            vad_start_flag = 1;
        }
        if(2 == flag)
        {
            vad_start_flag = 0;
        }
    }
    static int vad_decode_timeflag=0;
    switch(flag)
    {
        case ASR_DECODE_START:
            mprintf("vad_decode_timeflag %d\n",vad_decode_timeflag);
            if(0x0==vad_decode_timeflag)
            {
                hardvad_timeout = 0X0;
                TIMx_us(TIMER0,5000000);
                //timer_restart(TIMER0);
                vad_decode_timeflag = 1;
            }
            break;
   
        case VAD_STATE_VADEND:
            hardvad_timeout = 0X0;
            vad_decode_timeflag =0;
            timer_stop(TIMER0);
            break;
        case VAD_STATE_RSTEND:
            hardvad_timeout = 0X0;
            vad_decode_timeflag =0x0;
            timer_stop(TIMER0);
            break;
        case VAD_STATE_DNNEND:
            hardvad_timeout = 0X0;
            vad_decode_timeflag =0x0;
            timer_stop(TIMER0);
            break;
        case VAD_STATE_RECEND:
            hardvad_timeout = 0X0;
            vad_decode_timeflag =0x0;
            timer_stop(TIMER0);
            break;
        }
    switch(flag)
    {
        case VAD_STATE_START:
#if VAD_ONLINE                
            if(vad_cnt_timer == 0xffffffff)//first vad_start
            {
                vad_cnt_timer = C_VAD_END_MAX;
            //      LED_VAD_ON;
            }else
            {
                vad_end_timer = 0xffffffff;
            }
#else

#endif
                
        break;
          
        case VAD_STATE_VADEND:
        case VAD_STATE_DNNEND:
        case VAD_STATE_RECEND:
        case VAD_STATE_RSTEND:
#if VAD_ONLINE
            if(vad_cnt_timer!=0xffffffff)
            {//vad start 已经开始计数
                vad_end_timer = C_VAD_END_DELAY;
            }else
            {
                //mprintf("no vad start,just end, not care========\r\n");
            }
#else                
       //         LED_VAD_OFF;
#endif                
        break;
    }
    static int dnn_start=0,asr_start=0;//asr_end=0;  dnn_end=0,
    switch (flag)
    {
        case DNN_STATE_START:
            if(!dnn_start)
            {
                dnn_start = GetCurrentMs();
            }
            break;
        case VAD_STATE_DNNEND:
            //dnn_end = GetCurrentMs(); 
            //mprintf("DNN[%u:ms]\n",(dnn_end-dnn_start)*10);
            dnn_start =0;
            break;
        case ASR_DECODE_START:
            if(!asr_start)
            {
                asr_start = GetCurrentMs(); 
            }
        break;
        case VAD_STATE_RECEND:
        case VAD_STATE_RETEND:
            //asr_end = GetCurrentMs(); 
            //mprintf("DEC[%u:ms]\n",(asr_end - asr_start)*10);
            asr_start=0;
            dnn_start =0;
        break;
    }
}
