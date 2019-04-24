
/**
  ******************************************************************************
  * @文件    ci100x_global.c
  * @作者    chipintelli软件团队
  * @版本    V1.0.0
  * @日期    2016-4-9
  * @概要    这个文件用于chipintelli公司的CI100X芯片程序用到的部分全局变量定义.
  ******************************************************************************
  * @注意
  *
  * 版权归chipintelli公司所有，未经允许不得使用或修改
  *
  ******************************************************************************
  */ 
#include "ci100x_global.h"

//===================================================
QueueHandle_t ASR_xQueue = NULL;  
QueueHandle_t Vadpcm_xQueue = NULL;
QueueHandle_t user_task_queue= NULL; 

SemaphoreHandle_t xLowPowerSemaphore = NULL;
SemaphoreHandle_t xFileMutexlockSemaphore = NULL;
SemaphoreHandle_t xVadpcmSemaphore = NULL,xAgcalgoSemaphore= NULL;
SemaphoreHandle_t WakeupMutex = NULL;
SemaphoreHandle_t SpiWrMutex = NULL;
SemaphoreHandle_t I2CMutex = NULL;

void* handle = NULL;           //used at lib
unsigned int ptr_addr = 0x20400000;
unsigned int clkgate_state[2] = {0};
volatile unsigned int ulHighFrequencyTimerTicks;
volatile unsigned int sd_transend = 0;      

volatile unsigned int vad_end = 0;
volatile unsigned int vad_start = 0;
unsigned int vad_cnt_timer;
unsigned int vad_end_timer; 

volatile unsigned int asrtimer_Lcounter = 0;
volatile unsigned int asrtimer_Hcounter = 0;
volatile unsigned int asrtimer_counter =0;
//--iis dma 
volatile int vadpcm_iis_dma = 0;
volatile int iis1_rx_start = 0;
volatile int g_dma_translate_ok = 0; 
volatile int g_dma_translate_ok1 = 0;
volatile int vad_bug = 0;
        
//--record pcm to tf used
SemaphoreHandle_t xRecSemaphore = NULL;
SemaphoreHandle_t xRecStopSemaphore = NULL;//其他task 通知结束录音
SemaphoreHandle_t xRecSemaphore_4K = NULL;
int start_readpcm =0 ;
unsigned int pcm_wptr=0,pcm_rptr=0;
QueueHandle_t play_Q= NULL;
QueueHandle_t airuart_Q= NULL;
QueueHandle_t play_IT_queue = NULL;
QueueHandle_t play_ack_queue = NULL;
int sd_init_ok = -1;
int tf_mounted_state = -1;   //when use tf,this flag meaning tf mount state
//---
ASR_TimeoutType  timeout_type = clear_timeout;
unsigned int iwdg_watch = 0;
unsigned int hardvad_timeout =0;
volatile unsigned int g_decord_ver=0;    //used at lib
float* dnn_sil =NULL;
Reset_Type CPU_rstType = {0};

volatile unsigned int clear = 1;
volatile unsigned int nodnn_start = 0;
volatile unsigned int decode_start = 0;
volatile unsigned int vadend_start = 0;
volatile unsigned int vadstart_start = 0;
//-----custem used -----------------------------  

NvData_save_Typedef nvdata_save;//save some data to spi-flash
unsigned int awaken = 0;
int run_mode = 0;

combo_cmd_result_type combo_cmd_result;

//-----------------------------------------------
/***************** (C) COPYRIGHT Chipintelli Technology Co., Ltd. *****END OF FILE****/
