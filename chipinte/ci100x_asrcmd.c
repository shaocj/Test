/**
  ******************************************************************************
  * @文件    ci100x_asrcmd.c
  * @作者    chipintelli软件团队
  * @版本    V1.0.0
  * @日期    2016-4-9
  * @概要    这个文件是chipintelli公司的CI100X芯片程序的命令词处理文件.
  ******************************************************************************
  * @注意
  *
  * 版权归chipintelli公司所有，未经允许不得使用或修改
  *
  ******************************************************************************
  */ 

#include "ci100x_asrcmd.h"
  
#ifdef __cplusplus
extern "C" {
#endif

/**
 *@brief Struct of asr cmd. Include cmd id and cmd and threshold score.
 */
AsrCmd_TypeDef AsrCmd_Struct[]=
{
  {0XFFFE,"<Reserved>",},//我先离开稍后回来
  {1,"小美小美",WAKEUP_SCORE_THRESHOLD},// 1
  {2,"打开风扇",WAKEUP_SCORE_THRESHOLD},// 2
  {2,"风扇开机",WAKEUP_SCORE_THRESHOLD},// 3
  {2,"开风扇",WAKEUP_SCORE_THRESHOLD},// 4
  {3,"风扇关机",WAKEUP_SCORE_THRESHOLD},// 5
  {3,"关闭风扇",WAKEUP_SCORE_THRESHOLD},// 6
  {3,"关风扇",WAKEUP_SCORE_THRESHOLD},// 7
  {4,"风扇摇头",WAKEUP_SCORE_THRESHOLD},// 8
  {5,"停止摇头",WAKEUP_SCORE_THRESHOLD},// 9
  {5,"关闭摇头",WAKEUP_SCORE_THRESHOLD},// 10
  {6,"调小风速",WAKEUP_SCORE_THRESHOLD},// 11
  {6,"减小风速",WAKEUP_SCORE_THRESHOLD},// 12
  {7,"调大风速",WAKEUP_SCORE_THRESHOLD},// 13
  {7,"增大风速",WAKEUP_SCORE_THRESHOLD},// 14
  {8,"调到低档",WAKEUP_SCORE_THRESHOLD},// 15
  {9,"调到中档",WAKEUP_SCORE_THRESHOLD},// 16
  {10,"调到高档",WAKEUP_SCORE_THRESHOLD},// 17
  {11,"定时两小时",WAKEUP_SCORE_THRESHOLD},// 18
  {12,"定时四小时",WAKEUP_SCORE_THRESHOLD},// 19
  {13,"定时六小时",WAKEUP_SCORE_THRESHOLD},// 20
  {14,"定时八小时",WAKEUP_SCORE_THRESHOLD},// 21
  {15,"取消定时",WAKEUP_SCORE_THRESHOLD},// 22
  {16,"睡眠模式",WAKEUP_SCORE_THRESHOLD},// 23
  {17,"舒适模式",WAKEUP_SCORE_THRESHOLD},// 24
  {18,"请先开机",SCORE_THRESHOLD},//25
  {19,"已调到最小风速了",SCORE_THRESHOLD},//26
  {20,"已调到最大风速了",SCORE_THRESHOLD},//27
  {-1,}/*EOF 结束标识*/
};

const AsrSpecial_TypeDef specialwords_lst[] = 
{
  {20,"开风扇"},
  {-1,}
};

#if SIMILAR_WORD_DISTINGUISH
char* unexpected_words_lst[] = 
{
  "制冷模式",
  "END"
};

char* expected_words_lst[] = 
{
  "制热模式",
  "END"
};
#endif

/**
 *@brief Wakeup words list. Be Used when USE_SEPARATE_WAKEUP is setted to 1.
 */
const char * wakewords_lst[] =
{
    "END"
};

/**
 *@brief Combo cmd first word list. Be used when USE_COMBO_CMD is setted to 1.
 */
#if USE_COMBO_CMD
const char * combo_cmd_first_word_list[]=
{
    "END"
};
#endif

#ifdef __cplusplus
}
#endif
/***************** (C) COPYRIGHT Chipintelli Technology Co., Ltd. *****END OF FILE****/

