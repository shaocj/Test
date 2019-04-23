#ifndef _USER_CONFIG_H
#define _USER_CONFIG_H

/*----------------------Body Define------------------------------------*/
/*Could not change the body defines which are used by Lib*/
#define CHIP_CI1006A                0   //16M SDRAM inside
#define CHIP_CI1006B                1   //16M SDRAM inside too,cost version with CHIP_CI1006A
#define CHIP_CI1002                 2   //8M SDRAM inside,cost version

//#define CHIP_BODY                   CHIP_CI1006A     
//#define CHIP_BODY                   CHIP_CI1006B    
#define CHIP_BODY                   CHIP_CI1002

#if(CHIP_BODY == CHIP_CI1002)
#define COL_WIDTH 8    //for init 8M_SDRAM 
#else
#define COL_WIDTH 9    //for init 16M_SDRAM
#endif

/*----------------------demoboard ver------------------------------------*/
#define MODEL_STD  0 //EVBoard 
#define MODEL_V1   1 //demo board
#define MODEL_DEFINE MODEL_V1//MODEL_STD

/*----------------------bord type define -------------------------------*/
#define BOARD_TYPE      1   // 1: sigle-mic(s-mic) add double-mic(d-mic) Demo board, 0: stamp-demo board

/*----------------------new merge config---------------------------------*/
#define SUPPORT_CHIESE_ASR_ADDRESS 0x4000
#define SUPPORT_ENGLISH_ASR_ADDRESS 0x804000


/*----------------------release&produce config---------------------------*/
#define MASS_FACTOR 0 //when mass factor ,set 1;  when debug ,set 0(meaning IO not used be set Floating);
#define ENABLE_TEST_STAMP_BOARD 0 //test STAMP_board

/*----------------------adjustment dynamic threshold-------------------------*/
#define DYNAMIC_THRESHOLD (0) //0:static threshold;1:dynamic threshold

#define BACK_ENERGY_UPDATE (0) //dynamic update back ground energy

#define HIGH_SENSITIVITY_OFF            (0)
#define HIGH_SENSITIVITY_ON_CHINESE     (1)
#define HIGH_SENSITIVITY_ON_ENGLISH     (2)

#define RECOGNITION_SENSITIVITY_TYPE HIGH_SENSITIVITY_OFF
#define NOT_CATCH_CMD_WORD      (0) //I didn't catch your command word
#define SIMILAR_WORD_DISTINGUISH    (0)
/*----------------------single/double mic config-------------------------*/
#if (MODEL_DEFINE==MODEL_V1)
#define USE_ASR8388 1
#endif 

/*----------------------play flow config---------------------------------*/
//播放流程,1-旧播放流程，0-新播放流程
#define USE_ORIGIN_PLAY_FLOW    0

/*----------------------combo cmd----------------------------------------*/
#define USE_COMBO_CMD 0
#define COMBO_CMD_VALID_TIME_IN_MS 2000
#define COMBO_CMD_FIRST_WORD_CNT 16

#define COMBO_CMD_FIRST_WORD_INVALID 0
#define COMBO_CMD_FIRST_WORD_VALID   1

/*----------------------adkey config------------------------------------*/
#if (MODEL_DEFINE==MODEL_V1)
	#define ADKEY_ENABLE 0
	#define CX20921_IIC1  1 // 20921 use IIC1
#elif (MODEL_DEFINE==MODEL_STD)
	#define ADKEY_ENABLE 1
	#define CX20921_IIC1  0 // 20921 use IIC0
#endif

/*----------------------volume set--------------------------------------*/
#define SUPPORT_MAX_VOICE  250       //max play voice items
#define VOLUME_MAX 7          //voice play max volume level
#define VOLUME_MIN 1          //voice play max volume level

#if USE_ASR8388
    #define VOLUME_DEFAULT 5      //voice play default volume level
#else
    #define VOLUME_DEFAULT 3      //voice play default volume level
#endif

/*----------------------ALL time define---------------------------------*/
#define VAD_ONLINE 0
#define C_VAD_SHORT_LOST (50/10)  // ms，10ms per intterupt, if short vad ,then not stop
#define C_VAD_END_DELAY  (400/10) //vad end ,then occur vad start, then combin to one vad
#define C_VAD_END_MAX    (10000/10) //10s  VAD end max time

#define SUSPEND_TIME (15*1000000) //enter lowpower time
#define SLEEP_TIME (15*1000)   //exit wakeup time
#define IWDG_TIME ((SLEEP_TIME/1000) + (SUSPEND_TIME/1000000) + 5) 

/*----------------------Language define eum-----------------------------*/
#define SUPPORT_CHINESE_LANGUAGE    1
#define SUPPORT_ENGLISH_LANGUAGE    2
#define SUPPORT_LANGUAGE            SUPPORT_CHINESE_LANGUAGE

#define ASR_SMARTCMD_ID             0   /* It's same as [n]xxxx.dat of Tool\ASR\, or 0xFF if have no such file */
#define ASR_SMARTWAKEUP_ID          1   /* It's same as [n]xxxx.dat of Tool\ASR\, or 0xFF if have no such file*/
#define DNN_ID0                     0   /* It's same as [n]xxxx.dat of Tool\DNN\ */

#define ASRWAKEUP_ID                ASR_SMARTWAKEUP_ID
#define ASRCMD_ID                   ASR_SMARTCMD_ID
#define DNN_ID                      DNN_ID0

/*----------------------Tool define------------------------------------*/
#define NEW_MERGE_TOOL          //Update to Uart_load_tool_V2.0

/*----------------------uart define ------------------------------------*/
#define USE_USER_UART  1//用户串口用于发送识别结果给上位机，同时接收上位机传来的指令
#define USER_UART_USE_UART1  0//定义uart2的情况下，实际用uart1 发送识别结果[for 方形板子]，这时的debug 串口为uart0

/*----------------------function define -------------------------------*/
#define CX_20921_UPDATA  0 //关闭20921的升级可以节省12kbyte的code size
#define CPU_RATE_PRINT 0   //查看 CPU 占用率，默认关闭
#define USE_ADAPTSKP 1      //must be set 1
#define PLAY_TASK_ENABLE 1 //用task做播放
#define USE_SEPARATE_WAKEUP 0//eque 0,in lib1108
#define USE_IWDG  1
#define AEC_FUNC_EN 0
#define VAD_CHECK 0


#define VAD_In_Data_Addr            (0x70400000)
#define VAD_IN_DATA_BUF_COUT        10


/*----------------------other function define --------------------------*/
#define RGB_ENABLE      0   // 1: RGB show enable,used at demo board; 0: RGB not be show
#define NVDATA_SAVEENABLE 1 //save nvdata_save to spi flash, default save at addr SPI_NVDATA_START_ADDR
#define OTA_FUNC_ENABLE  0 

/*----------------------20921 version define ---------------------------*/
#define USE_6_156_2_16 0 //used 20921 ver at 6.156.2.16 

/*----------------------single mic aec define --------------------------*/
#define ES8388_DAC_MICIN_AEC 0 //20921 AEC refence from 8388Linein1,IIS1 used std-iis
#if ES8388_DAC_MICIN_AEC
#define IIS0_RX___IIS_STD 1
#endif

/*----------------------debug and test ---------------------------------*/
/*use for single-mic debug, if RECTEST 1,then asr not work!! */
#define RECTEST 0       //only used at single-mic solution debug,save iis data to 0x70500000 ,detail at ci100x_it.c
#define DECODE_DEBUG 0  //used for debug

#define DEBUG_PRINTF_FUNC_EN        1 //1：other打印，0为不打印
#define AUTOTEST_PRINTF_FUNC_EN     1 //自动测试宏，1为打印send result：

#if RECTEST
#undef USE_ASR8388
#define USE_ASR8388 0
#endif
#if USE_ASR8388
#define USE_6_156_2_16 0
#endif 

//--------------
#include "gpio_config.h"
#include "platform_config.h"
#if USE_ASR8388
#define USE_S_MIC_DENOISE 1		//1: enable denoise ;0: disable denoise
#else 

#endif 
#endif
