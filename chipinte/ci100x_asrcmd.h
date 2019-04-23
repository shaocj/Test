/**
  ******************************************************************************
  * @文件    ci100x_asrcmd.h
  * @作者    chipintelli软件团队
  * @版本    V1.0.0
  * @日期    2016-4-9
  * @概要    这个文件是chipintelli公司的CI100X芯片程序的命令词处理头文件。
  ******************************************************************************
  * @注意
  *
  * 版权归chipintelli公司所有，未经允许不得使用或修改
  *
  ******************************************************************************
  */
#ifndef __CI100X_ASRCMD_H
#define __CI100X_ASRCMD_H
#include "user_config.h"

#ifdef __cplusplus
extern "C" {
#endif


//---------------------------------------------------------------------------
// Typefef
//---------------------------------------------------------------------------

/**
 *@brief asr cmd struct. pVoiceAddr and VoiceSize will be setted GetUserPara function.
 */
typedef struct 
{
	int id;
	const char* const pCmd;
	float Score;
	unsigned int pVoiceAddr;
	unsigned int VoiceSize;
}AsrCmd_TypeDef;

/**
 *@brief special asr cmd word struct.
 */
typedef struct 
{
	int dtimecnt;
	const char* pCmd;
}AsrSpecial_TypeDef;


//---------------------------------------------------------------------------
// Extern statement
//---------------------------------------------------------------------------
extern AsrCmd_TypeDef AsrCmd_Struct[]; 
extern const AsrSpecial_TypeDef  specialwords_lst[];

extern const char * wakewords_lst[];
extern const char * combo_cmd_first_word_list[];

#if SIMILAR_WORD_DISTINGUISH
extern char* unexpected_words_lst[];
extern char* expected_words_lst[];
#endif

//---------------------------------------------------------------------------
// Define
//---------------------------------------------------------------------------
#define VOLUME_OPEN_COMMAND_INDEX  (84)

#define VOLUME_CLOSE_COMMAND_INDEX  (85)

#define VOLUME_UP_COMMAND_INDEX  (80)

#define VOLUME_DOWN_COMMAND_INDEX  (81)

#define VOLUME_MAX_COMMAND_INDEX  (82)

#define VOLUME_MIN_COMMAND_INDEX  (83)

#define PLAY_VOLMAX_VOICE_INDEX (82)
#define PLAY_VOLMIN_VOICE_INDEX (83)

#define NOT_CATCH_CMD_WORD_INDEX (103)

typedef enum
{
    FUNC_ID_WAKE_WORD = 1,
    FUNC_ID_LAMP_OPEN = 50,
    FUNC_ID_LAMP_CLOSE,
    FUNC_ID_LAMP_BRTNSS_MAX,
    FUNC_ID_LAMP_BRTNSS_MID,
    FUNC_ID_LAMP_BRTNSS_MIN,
    FUNC_ID_LAMP_BRTNSS_UP,
    FUNC_ID_LAMP_BRTNSS_DOWN,
    FUNC_ID_LAMP_NORMAL,
    FUNC_ID_LAMP_READING,
    FUNC_ID_LAMP_NIGHTLIGHT,
    FUNC_ID_LAMP_RED,
    FUNC_ID_LAMP_GREEN,
    FUNC_ID_LAMP_BLUE,
    FUNC_ID_LAMP_RANDOM,
}FUNCTION_ID_ENUM;

#define WAKEUP_SCORE_THRESHOLD  (0.25)
#define SCORE_THRESHOLD	(0.25)

#define OUTPUT_THRESHOLD	(0.15)

#define PERCISION		(0.0001) 

//#define MIN_SCORE ((WAKEUP_SCORE_THRESHOLD > SCORE_THRESHOLD)?SCORE_THRESHOLD:WAKEUP_SCORE_THRESHOLD)
#define MIN_SCORE ((OUTPUT_THRESHOLD > SCORE_THRESHOLD)?SCORE_THRESHOLD:OUTPUT_THRESHOLD)
#define SCORE_TAG " "
#define SCORE_LEN (strlen(SCORE_TAG))
#define SPN_TAG "<spn>"
#define SIL_TAG "<sil>"
#define MAX_SCORE (SCORE_THRESHOLD*2)

#ifdef __cplusplus
}
#endif

#endif /*__CI100X_ASRCMD_H*/
