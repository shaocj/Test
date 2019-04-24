#ifndef __CI100X_GLOBAL_H
#define __CI100X_GLOBAL_H

#ifdef __cplusplus
    extern "C"{
#endif
	  
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"
#include "semphr.h"
#include "event_groups.h"
		
typedef struct
{
  unsigned int data_type;	//0x55 ,normal data  0x33,end data
  unsigned int data_length;
  unsigned int data_address;
  unsigned int ostick;
}asr_datatype;

typedef struct{
	unsigned int asrcmdaddr;
	unsigned int asrcmdsize;
	unsigned int wakeupaddr;
	unsigned int wakeupsize;
}Para_Inof;	

typedef struct{
    unsigned int wakeupaddr;
    unsigned int wakeupsize;
    unsigned int arsaddr;
    unsigned int asrsize;
    unsigned int dnnaddr;
    unsigned int dnnsize;
    unsigned int rvaddr1;
    unsigned int rvszie1;
    unsigned int rvaddr2;
    unsigned int rvszie2;
    unsigned int rvaddr3;
    unsigned int rvszie3;
    unsigned int voicenums;
}VoiceInfo;

typedef struct
{
    unsigned int ProductID[4];
    unsigned int UserParaAddr;
    unsigned int UserCodeAddr;
    unsigned int UserCodeSize;
    unsigned int UserCodeBackupNums;
	//char UserBackupAddr[1];
}ConfigParameterInit_Typedef;


typedef enum
{
    idl_timeout =-1,
    clear_timeout = 0,
    nodnn_timeout =1,
    decode_timeout = 2,
    vadend_timeout = 3,
    vadstart_timeout =4,
} ASR_TimeoutType;

typedef struct
{
    unsigned char inited; 
    unsigned char volset;
    unsigned char voice_onoff;// 1:on 0:off
    unsigned char uLanguageId;
}NvData_save_Typedef;

#pragma pack(1)
typedef struct
{
    uint32_t ManufacturerID;    //32Bit
    uint32_t ProductID[2];      //64Bit (MAC Address)
//    
    uint32_t HWName[16];        //String
    uint32_t HWVersion;         //Vm.n.x.y
    uint32_t SWName[16];        //String,Exporting to Packet file name
    uint32_t SWVersion;         //Vm.n.x.y
//    
    uint32_t BootLoaderVersion; //Vm.n.x.y
//    
    uint32_t UpdateCodeVersion;
    uint32_t UpdateCodeStartAddr;
    uint32_t UpdateCodeSize;
//    
    uint16_t UpdateBlockNum;
//    
    uint32_t UserCodeMainVersion;
    uint32_t UserCodeMainStartAddr;
    uint32_t UserCodeMainSize;
    uint32_t UserCodeMainCRC;
    uint8_t UserCodeMainCompltStatus;
//    
    uint32_t UserCodeBackupVersion;
    uint32_t UserCodeBackupStartAddr;
    uint32_t UserCodeBackupSize;
    uint32_t UserCodeBackupCRC;
    uint8_t UserCodeBackupCompltStatus;
//    
    uint32_t ASRCMDModelVersion;
    uint32_t ASRCMDModelStartAddr;
    uint32_t ASRCMDModelSize;
    uint32_t ASRCMDModelCRC;
    uint8_t ASRCMDModelCompltStatus;
//    
    uint32_t DNNModelVersion;
    uint32_t DNNModelStartAddr;
    uint32_t DNNModelSize;
    uint32_t DNNModelCRC;
    uint8_t DNNModelCompltStatus;
//    
    uint32_t VoicePlayingVersion;
    uint32_t VoicePlayingStartAddr;
    uint32_t VoicePlayingSize;
    uint32_t VoicePlayingCRC;
    uint8_t VoicePlayingCompltStatus;
//
    uint32_t UserFileVersion;
    uint32_t UserFileStartAddr;
    uint32_t UserFileSize;
    uint32_t UserFileCRC;
    uint8_t UserFileCompltStatus;
//
    uint32_t NVDataStartAddr;
    uint32_t NVDataSize;
}FileConfig_Struct; 
#pragma pack()

#pragma pack(1)
typedef struct
{
    uint16_t File_ID;
    uint32_t File_Addr;
    uint32_t File_Size;
}File_Header_type;
#pragma pack()

#pragma pack(1)
typedef struct
{
    uint16_t PlayVoice_Num;
    int16_t PlayVoice_Index;
    File_Header_type *PlayVoice_Header;
}PlayVoice_Infor_Type;
#pragma pack()

typedef struct
{
    uint8_t first_word_valid;
    int first_word_valid_tick;
}combo_cmd_result_type;

extern Para_Inof para_info;  
extern void* handle;      	  
extern QueueHandle_t ASR_xQueue,Vadpcm_xQueue;
extern QueueHandle_t user_task_queue;    
extern unsigned int ptr_addr,clkgate_state[2];
extern SemaphoreHandle_t xLowPowerSemaphore,xFileMutexlockSemaphore,\
  xVadpcmSemaphore;           
extern SemaphoreHandle_t WakeupMutex;
extern SemaphoreHandle_t SpiWrMutex; 
extern SemaphoreHandle_t I2CMutex;

extern volatile unsigned int sd_transend,vad_end,vad_start,\
  asrtimer_Lcounter,asrtimer_Hcounter,asrtimer_counter;
extern volatile int vadpcm_iis_dma,iis1_rx_start;
extern volatile int g_dma_translate_ok,vad_bug;
extern volatile int g_dma_translate_ok1;
//////////////////////
extern SemaphoreHandle_t xRecSemaphore;
extern SemaphoreHandle_t xRecStopSemaphore;//其他task 通知结束录音
extern SemaphoreHandle_t xRecSemaphore_4K;
extern int start_readpcm  ;
extern unsigned int pcm_wptr,pcm_rptr;
extern QueueHandle_t play_Q;    
extern QueueHandle_t play_IT_queue;
extern QueueHandle_t play_ack_queue;
extern int sd_init_ok;
extern int sysinit_index;    
extern unsigned int vad_cnt_timer;//
extern unsigned int vad_end_timer;//
extern ASR_TimeoutType  timeout_type ;
extern unsigned int iwdg_watch;        
extern volatile unsigned int g_decord_ver;
extern unsigned int hardvad_timeout ;
typedef struct
{
    char softrst;
    char srstreq;
    char lockup;
    char wwtd;
    char iwtd;
    char jtag;
    char por;
}Reset_Type;
extern Reset_Type CPU_rstType;   
typedef enum
{
    TYPE_NULL_EVENT = 0x0,
    TYPE_VAD_START_EVENT = 0x11,
    TYPE_DNN_ALLMOSTFULL_EVENT = 0x22,
    TYPE_HAS_DNN_EVENT = 0x33,
    TYPE_DNN_END_EVENT = 0x44,
    TYPE_DNN_CALC_ERR_EVENT = 0x55,
    TYPE_DNN_BUS_REQ_ERR_EVENT = 0x66,
    TYPE_ASR_CHANGE_CMD = 0x77,
    TYPE_SWITCH_LANGUAGE_RESET_ASR = 0x88,

}ASRDNN_type_event;
extern float* dnn_sil;
extern volatile unsigned int clear ,\
      nodnn_start ,decode_start ,vadend_start ,\
        vadstart_start  ;
extern int tf_mounted_state;
//-----custem used -----------------------------  
extern NvData_save_Typedef nvdata_save;
extern unsigned int awaken;             
extern int run_mode;

extern combo_cmd_result_type combo_cmd_result;

//---------------------------------------------
//////////////////////
#ifdef __cplusplus
	}
#endif

 
#endif /*__CI100X_GLOBAL_H*/
