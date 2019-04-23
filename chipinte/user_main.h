/**
  ******************************************************************************
  * @file    task_asr.c 
  * @author  Chipintelli Technology Co., Ltd.
  * @version V1.0.0
  * @date    2017.05
  * @brief  
  ******************************************************************************
  **/

#ifndef _USER_MAIN_H_
#define _USER_MAIN_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    MSG_TYPE_SYS_ASR = 0,
    MSG_TYPE_SYS_KEY,
    MSG_TYPE_SYS_COM,
}user_msg_type_t;

typedef enum
{
    MSG_ASR_STATUS_GOOD_RESULT = 0,
}user_asr_msg_status_t;

typedef struct
{
    uint32_t asr_status;
    uint32_t asr_index;
    float asr_score;
}sys_asr_msg_data_t;

typedef struct
{
    uint32_t key;
}sys_key_msg_data_t;

typedef struct
{
    unsigned char reserved;//
    unsigned char wrongAlarm;//
    unsigned char voice;//
    unsigned char lock:2;
    unsigned char reserved1:6;//	
	unsigned char powerOnOff:1;
	unsigned char workMode:3;
	unsigned char reserved2:3;
	unsigned char powerEnable:1;
	unsigned char gear;
	unsigned char setTemperature;
	unsigned char humidity;
	unsigned char shakeSwitch:1;
	unsigned char shakeType:3;
	unsigned char angle:3;
	unsigned char shakeEnable:1;
	unsigned char anion:2;
	unsigned char anophelifuge:2;
	unsigned char humidify:4;
	unsigned char timingOffHour:5;
	unsigned char timingOffMinute:3;
	unsigned char timingOnHour:5;
	unsigned char timingOnMinute:3;
	unsigned char humidifyFeedback;
	unsigned char temperatureFeedback;
	unsigned char timingOnMinuteEx:4;
	unsigned char timingOffMinuteEx:4;
	unsigned char bodyFeelingScan;
	unsigned char scene;
	unsigned char sleepSensor;
	unsigned char versionLow:4;
	unsigned char versionHigh:4;
	unsigned char reserved3;
}control_cmd_t;

typedef struct
{
	unsigned char reserved0;
	unsigned char type;
	unsigned char model0;
	unsigned char model1;
	unsigned char reserved1:28;
}get_type_cmd_t;

typedef struct
{
	unsigned char cmd;
	unsigned char status;
	unsigned char reserved[6];
}asr_cmd_t;

typedef struct
{
	unsigned char cmd;
	unsigned char reserved:7;
}get_version_cmd_t;

typedef struct
{
	unsigned char cmd;
	unsigned char ver_length;
	unsigned char version:6;
}return_version_cmd_t;

typedef struct
{
	unsigned char cmd;
	unsigned char MD5_length;
	unsigned char adr_length;
	unsigned char MD5:3;
	unsigned char upgrade_adr:3;
}upgrade_version_cmd_t;

typedef struct
{
	unsigned char cmd;
	unsigned char reserved:7;
}upgrade_progress_inquiry_cmd_t;

typedef struct
{
	unsigned char cmd;
	unsigned char progress;
	unsigned char reserved:6;
}return_upgrade_progress_inquiry_cmd_t;

typedef struct
{
    unsigned char header0;//header
    unsigned char data_length;//ÏûÏ¢³¤¶È£¬data_length=data+10
    unsigned char type;//¼ÒµçÀàÐÍ
    unsigned char sync;//Ö¡Í¬²½Ð£Ñé,Ä¬ÈÏ0
    unsigned short reserved;//±£Áô,2b,Ä¬ÈÏ0
    unsigned char mark;//±êÖ¾
    unsigned char version0;//¿ò¼ÜÐ­Òé°æ±¾£¬Ä¬ÈÏ0
    unsigned char version1;//¼ÒµçÐ­Òé°æ±¾Ä¬ÈÏ0
    unsigned char cmd;//ÃüÁî
    unsigned char msg_data[32];//ÏûÏ¢Ìå
    unsigned char chksum;//check sum=~sum£¨data_length...data£©+1
}send_com_msg_data_t;

typedef struct
{
    unsigned char header0;//header
    unsigned char data_length;//ÏûÏ¢³¤¶È£¬data_length=data+10
    unsigned char type;//¼ÒµçÀàÐÍ
    unsigned char sync;//Ö¡Í¬²½Ð£Ñé,Ä¬ÈÏ0
    unsigned short reserved;//±£Áô,2b,Ä¬ÈÏ0
    unsigned char mark;//±êÖ¾
    unsigned char version0;//¿ò¼ÜÐ­Òé°æ±¾£¬Ä¬ÈÏ0
    unsigned char version1;//¼ÒµçÐ­Òé°æ±¾Ä¬ÈÏ0
    unsigned char cmd;//ÃüÁî
    unsigned char msg_data[32];//ÏûÏ¢Ìå
    unsigned char chksum;//check sum=~sum£¨data_length...data£©+1
}sys_com_msg_data_t;

typedef struct
{
    uint32_t msg_type;/*here will be modify use union*/
    union
    {
        sys_asr_msg_data_t asr_data;
        sys_key_msg_data_t key_data;
        sys_com_msg_data_t com_data;
    }msg_data;
}user_msg_t;

typedef struct
{
    int duty;
    int period;
}light;

typedef struct
{
    light r;
    light g;
    light b;
}color;

typedef enum
{
    UPDATE_PROCESSQUIT = 0,
    UPDATE_REQTIMEOUT = 1,
    UPDATE_UPDATING = 2,
    UPDATE_UPDATECOMPLETE = 3,
}Update_State_t;

void userapp_initial(void);
void OTAFuncProcess(void);

#ifdef __cplusplus
}
#endif

#endif
