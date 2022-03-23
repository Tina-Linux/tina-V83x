#ifndef __ASR_CONFIG_H__
#define __ASR_CONFIG_H__

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

/*
* just for uvoice test
*/

#define UVOICE_TEST
//#define PLAY_ALL_AUDIOS_ENABLE

//#define EII_TEST_ENABLE
//#define AUTO_TEST_ENABLE

//#define WAKEUP_ONCE_ENABLE

//* config for local and eii  */
#define LOCAL_ASR_ID_MIN	(1)
#define LOCAL_ASR_ID_MAX	(125)
#define EII_ASR_ID_MIN		(126)
#define EII_ASR_ID_MAX		(150)

/*
* wakeup timeout config
* enable: std 10s;
* disable: forever 5 years
*/
#ifdef WAKEUP_ONCE_ENABLE
#undef  WAKEUP_TIMEOUT_STD_ENABLE
#else
#define WAKEUP_TIMEOUT_STD_ENABLE
#endif
/*
* for wakeup timeout config
*/

#ifdef  WAKEUP_TIMEOUT_STD_ENABLE
#define WAKEUP_TIMEOUT_STD        (15)
#define WAKEUP_TIMEOUT_CONFIG    WAKEUP_TIMEOUT_STD
#else
#define WAKEUP_TIMEOUT_FOREVER    (5 * 365 * 24 * 60 * 60)
#define WAKEUP_TIMEOUT_CONFIG    WAKEUP_TIMEOUT_FOREVER
#endif

#define PLAYBACK_COUNTS (sizeof(notice_music_name) / sizeof(const char*))

/*
* std asr id num
*/
typedef enum{
    /*wake up*/
    ASR_UART_CMD_WAKE_UP1         = 1,     //小美小美
    ASR_UART_CMD_WAKE_UP2         = 2,     //小美同学
    //ASR_UART_CMD_WAKE_UP3         = 202,   //嗨，小优

    /*power on*/
    ASR_UART_CMD_ON1         	  = 3,     //空调开机
    ASR_UART_CMD_ON2         	  = 4,     //开启空调
    ASR_UART_CMD_ON3         	  = 5,     //打开空调
    ASR_UART_CMD_ON4         	  = 6,     //请开机
    ASR_UART_CMD_ON5         	  = 7,     //开机

    /*power off*/
    ASR_UART_CMD_OFF1	          = 8,     //空调关机
    ASR_UART_CMD_OFF2         	  = 9,     //关闭空调
    ASR_UART_CMD_OFF3         	  = 10,     //关掉空调
    ASR_UART_CMD_OFF4    	  = 11,    //请关机
    ASR_UART_CMD_OFF5        	  = 12,    //关机

    /*set wind mode*/
    ASR_UART_CMD_WIND_SLOW         = 13,     //最小风
    ASR_UART_CMD_WIND_MID          = 14,     //中等风
    ASR_UART_CMD_WIND_HIGH         = 15,     //最大风
    ASR_UART_CMD_WIND_AUTO         = 16,     //自动风

    /*set wind*/
    ASR_UART_CMD_WIND_DEC_SPEED1    = 17,    //减小风速
    ASR_UART_CMD_WIND_DEC_SPEED2    = 18,    //调小风速
    ASR_UART_CMD_WIND_DEC_SPEED3    = 19,    //风速小点

    ASR_UART_CMD_WIND_INC_SPEED1    = 20,    //增大风速
    ASR_UART_CMD_WIND_INC_SPEED2    = 21,    //调大风速
    ASR_UART_CMD_WIND_INC_SPEED3    = 22,    //风速大点

    /*set temperature*/
    ASR_UART_CMD_SET_TEMP_17     = 23,    //十七度
    ASR_UART_CMD_SET_TEMP_18     = 24,    //十八度
    ASR_UART_CMD_SET_TEMP_19     = 25,    //十九度
    ASR_UART_CMD_SET_TEMP_20     = 26,    //二十度
    ASR_UART_CMD_SET_TEMP_21     = 27,    //二十一度
    ASR_UART_CMD_SET_TEMP_22     = 28,    //二十二度
    ASR_UART_CMD_SET_TEMP_23     = 29,    //二十三度
    ASR_UART_CMD_SET_TEMP_24     = 30,    //二十四度
    ASR_UART_CMD_SET_TEMP_25     = 31,    //二十五度
    ASR_UART_CMD_SET_TEMP_26     = 32,    //二十六度
    ASR_UART_CMD_SET_TEMP_27     = 33,    //二十七度
    ASR_UART_CMD_SET_TEMP_28     = 34,    //二十八度
    ASR_UART_CMD_SET_TEMP_29     = 35,    //二十九度
    ASR_UART_CMD_SET_TEMP_30     = 36,    //三十度
    ASR_UART_CMD_INC_T_BY_1_a    = 37,    //调高1度
    ASR_UART_CMD_INC_T_BY_1_b    = 38,    //有点冷
    ASR_UART_CMD_INC_T_BY_3_0    = 39,    //太冷了
    ASR_UART_CMD_DEC_T_BY_1_a    = 40,    //调低1度
    ASR_UART_CMD_DEC_T_BY_1_b    = 41,    //有点热
    ASR_UART_CMD_DEC_T_BY_3_0    = 42,    //太热了
    ASR_UART_CMD_INC_T_BY_0_5    = 43,    //调高0.5度
    ASR_UART_CMD_DEC_T_BY_0_5    = 44,    //调低0.5度

    /*set mode*/
    ASR_UART_CMD_SET_MODE_AUTO_a     = 45,   //自动模式
    ASR_UART_CMD_SET_MODE_AUTO_b     = 46,   //打开自动模式

    ASR_UART_CMD_SET_MODE_COOL_a     = 47,   //制冷模式
    ASR_UART_CMD_SET_MODE_COOL_b     = 48,   //打开制冷模式
    ASR_UART_CMD_SET_MODE_COOL_c     = 49,   //打开制冷

    ASR_UART_CMD_SET_MODE_HEAT_a     = 50,   //制热模式
    ASR_UART_CMD_SET_MODE_HEAT_b     = 51,   //打开制热模式
    ASR_UART_CMD_SET_MODE_HEAT_c     = 52,   //打开制热

    ASR_UART_CMD_SET_MODE_WIND_a     = 53,   //送风模式
    ASR_UART_CMD_SET_MODE_WIND_b     = 54,   //打开送风模式
    ASR_UART_CMD_SET_MODE_WIND_c     = 55,   //打开送风

    ASR_UART_CMD_SET_MODE_DRY_a      = 56,   //抽湿模式
    ASR_UART_CMD_SET_MODE_DRY_b      = 57,   //打开抽湿模式
    ASR_UART_CMD_SET_MODE_DRY_c      = 58,   //打开抽湿
    ASR_UART_CMD_SET_MODE_DRY_d      = 59,  //除湿模式
    ASR_UART_CMD_SET_MODE_DRY_e      = 60,  //打开除湿模式
    ASR_UART_CMD_SET_MODE_DRY_f      = 61,  //打开除湿

    /*set wind swing*/
    ASR_UART_CMD_SWING_UP_DOWN_ON_a        = 62,    //打开上下摆风
    ASR_UART_CMD_SWING_UP_DOWN_ON_b        = 63,    //上下摆风
    ASR_UART_CMD_SWING_UP_DOWN_ON_c        = 64,    //上下风 add by v3
    ASR_UART_CMD_SWING_UP_DOWN_OFF_a       = 65,    //关闭上下摆风
    ASR_UART_CMD_SWING_UP_DOWN_OFF_b       = 66,    //关掉上下摆风
    ASR_UART_CMD_SWING_UP_DOWN_OFF_c       = 151,   //关闭上下风
    ASR_UART_CMD_SWING_UP_DOWN_OFF_d       = 152,   //关掉上下风

    ASR_UART_CMD_SWING_LEFT_RIGHT_ON_a     = 67,    //打开左右摆风
    ASR_UART_CMD_SWING_LEFT_RIGHT_ON_b     = 68,    //左右摆风
    ASR_UART_CMD_SWING_LEFT_RIGHT_ON_c     = 69,    //左右风
    ASR_UART_CMD_SWING_LEFT_RIGHT_OFF_a    = 70,    //关闭左右摆风
    ASR_UART_CMD_SWING_LEFT_RIGHT_OFF_b    = 71,    //关掉左右摆风
    ASR_UART_CMD_SWING_LEFT_RIGHT_OFF_c    = 153,   //关闭左右风
    ASR_UART_CMD_SWING_LEFT_RIGHT_OFF_d    = 154,   //关掉左右风

    /*timer*/
    ASR_UART_CMD_TIMER_30M_ON        = 72,    //三十分钟后开机
    ASR_UART_CMD_TIMER_1H_ON         = 73,    //一小时后开机
    ASR_UART_CMD_TIMER_2H_ON         = 74,    //两小时后开机
    ASR_UART_CMD_TIMER_3H_ON         = 75,    //三小时后开机
    ASR_UART_CMD_TIMER_4H_ON         = 76,    //四小时后开机
    ASR_UART_CMD_TIMER_5H_ON         = 77,    //五小时后开机
    ASR_UART_CMD_TIMER_6H_ON         = 78,    //六小时后开机
    ASR_UART_CMD_TIMER_7H_ON         = 79,    //七小时后开机
    ASR_UART_CMD_TIMER_8H_ON         = 80,    //八小时后开机

    ASR_UART_CMD_TIMER_30M_OFF       = 81,    //三十分钟后关机
    ASR_UART_CMD_TIMER_1H_OFF        = 82,    //一小时后关机
    ASR_UART_CMD_TIMER_2H_OFF        = 83,    //两小时后关机
    ASR_UART_CMD_TIMER_3H_OFF        = 84,    //三小时后关机
    ASR_UART_CMD_TIMER_4H_OFF        = 85,    //四小时后关机
    ASR_UART_CMD_TIMER_5H_OFF        = 86,    //五小时后关机
    ASR_UART_CMD_TIMER_6H_OFF        = 87,    //六小时后关机
    ASR_UART_CMD_TIMER_7H_OFF        = 88,    //七小时后关机
    ASR_UART_CMD_TIMER_8H_OFF        = 89,    //八小时后关机

    ASR_UART_CMD_TIMER_CANCEL        = 90,    //取消定时
    /*comfort and eco*/
    ASR_UART_CMD_COMFORT_ECO_ON      = 91,    //打开舒省
    ASR_UART_CMD_COMFORT_ECO_OFF_a   = 92,    //关闭舒省
    ASR_UART_CMD_COMFORT_ECO_OFF_b   = 93,    //关掉舒省

    /*self-clean*/
    ASR_UART_CMD_SELF_CLEAN_ON_a     = 94,    //打开自清洁
    ASR_UART_CMD_SELF_CLEAN_ON_b     = 95,    //自清洁
    ASR_UART_CMD_SELF_CLEAN_OFF_a    = 96,    //关闭自清洁
    ASR_UART_CMD_SELF_CLEAN_OFF_b    = 97,    //关掉自清洁

    /*auxiliary-electric-heating*/
    ASR_UART_CMD_AUXILIARY_ELECTRIC_HEATING_ON_a    = 98,    //打开电辅热
    ASR_UART_CMD_AUXILIARY_ELECTRIC_HEATING_ON_b    = 99,    //电辅热
    ASR_UART_CMD_AUXILIARY_ELECTRIC_HEATING_OFF_a   = 100,    //关闭电辅热
    ASR_UART_CMD_AUXILIARY_ELECTRIC_HEATING_OFF_b   = 101,   //关掉电辅热

    /*anti-blow*/
    ASR_UART_CMD_ANTI_BLOW_ON_a     = 102,    //打开防直吹
    ASR_UART_CMD_ANTI_BLOW_ON_b     = 103,    //防直吹
    ASR_UART_CMD_ANTI_BLOW_OFF_a    = 104,    //关闭防直吹
    ASR_UART_CMD_ANTI_BLOW_OFF_b    = 105,    //关掉防直吹

    /*set-volume*/
    ASR_UART_CMD_SET_VOLUME_MAX     = 106,    //最大音量
    ASR_UART_CMD_SET_VOLUME_MIN     = 107,    //最小音量

    ASR_UART_CMD_SET_VOLUME_INC1     = 108,   //增大音量
    ASR_UART_CMD_SET_VOLUME_INC2     = 109,   //调大音量

    ASR_UART_CMD_SET_VOLUME_DEC1     = 110,   //减小音量
    ASR_UART_CMD_SET_VOLUME_DEC2     = 111,   //调小音量

    /*screen-display*/
    ASR_UART_CMD_SCREEN_DISPLAY_ON    = 112,  //打开屏显
    ASR_UART_CMD_SCREEN_DISPLAY_OFF_a = 113,  //关闭屏显
    ASR_UART_CMD_SCREEN_DISPLAY_OFF_b = 114,  //关掉屏显

    /*anti-cold*/
    //ASR_UART_CMD_ANTI_COLD_ON_a     = 73,  //打开防过冷
    //ASR_UART_CMD_ANTI_COLD_ON_b     = 74,  //防过冷
    //ASR_UART_CMD_ANTI_COLD_OFF_a    = 75,  //关闭防过冷
    //ASR_UART_CMD_ANTI_COLD_OFF_b    = 104, //关掉防过冷
    ASR_UART_CMD_SMART_CTL_ON1        = 115,  //打开智控温
    ASR_UART_CMD_SMART_CTL_ON2        = 116,  //智控温
    ASR_UART_CMD_SMART_CTL_OFF1       = 117,  //关闭智控温
    ASR_UART_CMD_SMART_CTL_OFF2       = 118,  //关掉智控温

    /*strong mode*/
    ASR_UART_CMD_STRONG_MODE_ON       = 119,  //打开强劲
    ASR_UART_CMD_STRONG_MODE_OFF_a    = 120,  //关闭强劲
    ASR_UART_CMD_STRONG_MODE_OFF_b    = 121,  //关掉强劲

    /*wifi*/
    ASR_UART_CMD_START_WIFI_SNIFFER   = 122,  //空调开始配网
    ASR_UART_CMD_RESET_WIFI_CONF      = 123,  //空调重新配网

    /*query-weather*/
    ASR_UART_CMD_QUERY_WEATHER_a      = 124,  //天气查询
    ASR_UART_CMD_QUERY_WEATHER_b      = 125,  //查询天气


    /*wifi passthrough*/
    ASR_UART_CMD_LIVING_ROOM_FAN_ON   = 126, //打开客厅风扇
    ASR_UART_CMD_BED_ROOM_FAN_ON      = 127, //打开卧室风扇
    ASR_UART_CMD_LIVING_ROOM_FAN_OFF  = 128, //关闭客厅风扇
    ASR_UART_CMD_BED_ROOM_FAN_OFF     = 129, //关闭卧室风扇

    /****/
    ASR_UART_CMD_LIVING_ROOM_HUMIDIFIER_ON    = 130,    //打开客厅加湿器
    ASR_UART_CMD_BED_ROOM_HUMIDIFIER_ON       = 131,    //打开卧室加湿器
    ASR_UART_CMD_LIVING_ROOM_HUMIDIFIER_OFF   = 132,    //关闭客厅加湿器
    ASR_UART_CMD_BED_ROOM_HUMIDIFIER_OFF      = 133,    //关闭卧室加湿器

    ASR_UART_CMD_LIVING_ROOM_AIR_CLEANER_ON   = 134,    //打开客厅净化器
    ASR_UART_CMD_BED_ROOM_AIR_CLEANER_ON      = 135,    //打开卧室净化器
    ASR_UART_CMD_LIVING_ROOM_AIR_CLEANER_OFF  = 136,    //关闭客厅净化器
    ASR_UART_CMD_BED_ROOM_AIR_CLEANER_OFF     = 137,    //关闭卧室净化器

    ASR_UART_CMD_SWEEP_ROBOT_CLEAN_ALL        = 138,    //扫地机器人扫地
    ASR_UART_CMD_SWEEP_ROBOT_CHARGING         = 139,    //扫地机器人充电
    ASR_UART_CMD_SWEEP_ROBOT_CLEAN_PAUSE      = 140,    //扫地机器人暂停

    ASR_UART_CMD_BED_ROOM_AC_ON     	      = 141,    //打开卧室空调
    ASR_UART_CMD_BED_ROOM_AC_OFF    	      = 142,    //关闭卧室空调

    ASR_UART_CMD_LIVING_ROOM_LAMP_ON          = 143,    //客厅开灯
    ASR_UART_CMD_BED_ROOM_LAMP_ON             = 144,    //卧室开灯
    ASR_UART_CMD_LIVING_ROOM_LAMP_OFF         = 145,    //客厅关灯
    ASR_UART_CMD_BED_ROOM_LAMP_OFF            = 146,    //卧室关灯

    ASR_UART_CMD_PEOPLE_IMBACK        	      = 147,    //我回来了
    ASR_UART_CMD_PEOPLE_IMHOME                = 148,    //我到家了
    ASR_UART_CMD_PEOPLE_IGOTOWORK     	      = 149,    //我去上班了
    ASR_UART_CMD_PEOPLE_IGOOUT        	      = 150,    //我出门了

    /*unuse*/
    ASR_UART_CMD_LIVING_ROOM_FAN_OSC_ON       = 1119,    //客厅风扇摇头
    ASR_UART_CMD_BED_ROOM_FAN_OSC_ON          = 1120,    //卧室风扇摇头
    ASR_UART_CMD_LIVING_ROOM_FAN_OSC_OFF      = 1121,    //客厅风扇停止摇头
    ASR_UART_CMD_BED_ROOM_FAN_OSC_OFF         = 1122,    //卧室风扇停止摇头

    ASR_UART_CMD_LIVING_ROOM_FAN_INC          = 1123,    //客厅风扇调大一档
    ASR_UART_CMD_BED_ROOM_FAN_INC             = 1124,    //卧室风扇调大一档
    ASR_UART_CMD_LIVING_ROOM_FAN_DEC          = 1125,    //客厅风扇调小一档
    ASR_UART_CMD_BED_ROOM_FAN_DEC             = 1126,    //卧室风扇调小一档

    ASR_UART_CMD_LIVING_ROOM_FAN_MAX          = 1127,    //客厅风扇调到最大
    ASR_UART_CMD_BED_ROOM_FAN_MAX             = 1128,    //卧室风扇调到最大
    ASR_UART_CMD_LIVING_ROOM_FAN_MIN          = 1129,    //客厅风扇调到最小
    ASR_UART_CMD_BED_ROOM_FAN_MIN             = 1130,    //卧室风扇调到最小

    ASR_UART_CMD_BLOW_LEFT1                   = 1141,    //风向左吹
    //ASR_UART_CMD_BLOW_LEFT2                 = 111,     //风往左吹

    ASR_UART_CMD_BLOW_RIGHT1                  = 1142,    //风向右吹
    //ASR_UART_CMD_BLOW_RIGHT2                = 112,     //风往右吹

    ASR_UART_CMD_BLOW_UP1                     = 1143,    //风向上吹
    //ASR_UART_CMD_BLOW_UP2                   = 113,     //风往上吹

    ASR_UART_CMD_BLOW_DOWN1                   = 1144,    //风向下吹
    //ASR_UART_CMD_BLOW_DOWN2                 = 114,     //风往下吹


    /*start-up asr id*/
    ASR_UART_CMD_STARTUP        = 252,

    /*wakeup-timeout asr id*/
    ASR_UART_CMD_WAKEUP_TIMEOUT    = 253,

    /*factory test asr id*/
    ASR_UART_CMD_FACTORY        = 254,

}ASR_FRAME_CMD_ID;

static const char *notice_music_name[] = {
    "/usr/share/std_resources/aw_[0]主人再见.wav",
    "/usr/share/std_resources/aw_[1]请吩咐.wav",
    "/usr/share/std_resources/aw_[2]请开机.wav",
    "/usr/share/std_resources/aw_[3]空调已开机.wav",
    "/usr/share/std_resources/aw_[4]空调已关机.wav",
    "/usr/share/std_resources/aw_[5]请先开机.wav",
    "/usr/share/std_resources/aw_[6]已设为.wav",
    "/usr/share/std_resources/aw_[7]已关闭.wav",
    "/usr/share/std_resources/aw_[8]已打开.wav",
    "/usr/share/std_resources/aw_[9]最小风.wav",
    "/usr/share/std_resources/aw_[10]中等风.wav",
    "/usr/share/std_resources/aw_[11]最大风.wav",
    "/usr/share/std_resources/aw_[12]自动风.wav",
    "/usr/share/std_resources/aw_[13]自动和抽湿模式不能调风速.wav",
    "/usr/share/std_resources/aw_[14]当前已经是.wav",
    "/usr/share/std_resources/aw_[15]零下.wav",
    "/usr/share/std_resources/aw_[16]点.wav",
    "/usr/share/std_resources/aw_[17]零度.wav",
    "/usr/share/std_resources/aw_[18]一度.wav",
    "/usr/share/std_resources/aw_[19]二度.wav",
    "/usr/share/std_resources/aw_[20]三度.wav",
    "/usr/share/std_resources/aw_[21]四度.wav",
    "/usr/share/std_resources/aw_[22]五度.wav",
    "/usr/share/std_resources/aw_[23]六度.wav",
    "/usr/share/std_resources/aw_[24]七度.wav",
    "/usr/share/std_resources/aw_[25]八度.wav",
    "/usr/share/std_resources/aw_[26]九度.wav",
    "/usr/share/std_resources/aw_[27]十度.wav",
    "/usr/share/std_resources/aw_[28]十一度.wav",
    "/usr/share/std_resources/aw_[29]十二度.wav",
    "/usr/share/std_resources/aw_[30]十三度.wav",
    "/usr/share/std_resources/aw_[31]十四度.wav",
    "/usr/share/std_resources/aw_[32]十五度.wav",
    "/usr/share/std_resources/aw_[33]十六度.wav",
    "/usr/share/std_resources/aw_[34]十七度.wav",
    "/usr/share/std_resources/aw_[35]十八度.wav",
    "/usr/share/std_resources/aw_[36]十九度.wav",
    "/usr/share/std_resources/aw_[37]二十度.wav",
    "/usr/share/std_resources/aw_[38]二十一度.wav",
    "/usr/share/std_resources/aw_[39]二十二度.wav",
    "/usr/share/std_resources/aw_[40]二十三度.wav",
    "/usr/share/std_resources/aw_[41]二十四度.wav",
    "/usr/share/std_resources/aw_[42]二十五度.wav",
    "/usr/share/std_resources/aw_[43]二十六度.wav",
    "/usr/share/std_resources/aw_[44]二十七度.wav",
    "/usr/share/std_resources/aw_[45]二十八度.wav",
    "/usr/share/std_resources/aw_[46]二十九度.wav",
    "/usr/share/std_resources/aw_[47]三十度.wav",
    "/usr/share/std_resources/aw_[48]三十一度.wav",
    "/usr/share/std_resources/aw_[49]三十二度.wav",
    "/usr/share/std_resources/aw_[50]三十三度.wav",
    "/usr/share/std_resources/aw_[51]三十四度.wav",
    "/usr/share/std_resources/aw_[52]三十五度.wav",
    "/usr/share/std_resources/aw_[53]三十六度.wav",
    "/usr/share/std_resources/aw_[54]三十七度.wav",
    "/usr/share/std_resources/aw_[55]三十八度.wav",
    "/usr/share/std_resources/aw_[56]三十九度.wav",
    "/usr/share/std_resources/aw_[57]四十度.wav",
    "/usr/share/std_resources/aw_[58]四十一度.wav",
    "/usr/share/std_resources/aw_[59]四十二度.wav",
    "/usr/share/std_resources/aw_[60]四十三度.wav",
    "/usr/share/std_resources/aw_[61]四十四度.wav",
    "/usr/share/std_resources/aw_[62]四十五度.wav",
    "/usr/share/std_resources/aw_[63]四十六度.wav",
    "/usr/share/std_resources/aw_[64]四十七度.wav",
    "/usr/share/std_resources/aw_[65]四十八度.wav",
    "/usr/share/std_resources/aw_[66]四十九度.wav",
    "/usr/share/std_resources/aw_[67]五十度.wav",
    "/usr/share/std_resources/aw_[68]零点.wav",
    "/usr/share/std_resources/aw_[69]一点.wav",
    "/usr/share/std_resources/aw_[70]二点.wav",
    "/usr/share/std_resources/aw_[71]三点.wav",
    "/usr/share/std_resources/aw_[72]四点.wav",
    "/usr/share/std_resources/aw_[73]五点.wav",
    "/usr/share/std_resources/aw_[74]六点.wav",
    "/usr/share/std_resources/aw_[75]七点.wav",
    "/usr/share/std_resources/aw_[76]八点.wav",
    "/usr/share/std_resources/aw_[77]九点.wav",
    "/usr/share/std_resources/aw_[78]十点.wav",
    "/usr/share/std_resources/aw_[79]十一点.wav",
    "/usr/share/std_resources/aw_[80]十二点.wav",
    "/usr/share/std_resources/aw_[81]十三点.wav",
    "/usr/share/std_resources/aw_[82]十四点.wav",
    "/usr/share/std_resources/aw_[83]十五点.wav",
    "/usr/share/std_resources/aw_[84]十六点.wav",
    "/usr/share/std_resources/aw_[85]十七点.wav",
    "/usr/share/std_resources/aw_[86]十八点.wav",
    "/usr/share/std_resources/aw_[87]十九点.wav",
    "/usr/share/std_resources/aw_[88]二十点.wav",
    "/usr/share/std_resources/aw_[89]二十一点.wav",
    "/usr/share/std_resources/aw_[90]二十二点.wav",
    "/usr/share/std_resources/aw_[91]二十三点.wav",
    "/usr/share/std_resources/aw_[92]二十四点.wav",
    "/usr/share/std_resources/aw_[93]二十五点.wav",
    "/usr/share/std_resources/aw_[94]二十六点.wav",
    "/usr/share/std_resources/aw_[95]二十七点.wav",
    "/usr/share/std_resources/aw_[96]二十八点.wav",
    "/usr/share/std_resources/aw_[97]二十九点.wav",
    "/usr/share/std_resources/aw_[98]三十点.wav",
    "/usr/share/std_resources/aw_[99]三十一点.wav",
    "/usr/share/std_resources/aw_[100]三十二点.wav",
    "/usr/share/std_resources/aw_[101]三十三点.wav",
    "/usr/share/std_resources/aw_[102]三十四点.wav",
    "/usr/share/std_resources/aw_[103]三十五点.wav",
    "/usr/share/std_resources/aw_[104]三十六点.wav",
    "/usr/share/std_resources/aw_[105]三十七点.wav",
    "/usr/share/std_resources/aw_[106]三十八点.wav",
    "/usr/share/std_resources/aw_[107]三十九点.wav",
    "/usr/share/std_resources/aw_[108]四十点.wav",
    "/usr/share/std_resources/aw_[109]四十一点.wav",
    "/usr/share/std_resources/aw_[110]四十二点.wav",
    "/usr/share/std_resources/aw_[111]四十三点.wav",
    "/usr/share/std_resources/aw_[112]四十四点.wav",
    "/usr/share/std_resources/aw_[113]四十五点.wav",
    "/usr/share/std_resources/aw_[114]四十六点.wav",
    "/usr/share/std_resources/aw_[115]四十七点.wav",
    "/usr/share/std_resources/aw_[116]四十八点.wav",
    "/usr/share/std_resources/aw_[117]四十九点.wav",
    "/usr/share/std_resources/aw_[118]零.wav",
    "/usr/share/std_resources/aw_[119]一.wav",
    "/usr/share/std_resources/aw_[120]二.wav",
    "/usr/share/std_resources/aw_[121]三.wav",
    "/usr/share/std_resources/aw_[122]四.wav",
    "/usr/share/std_resources/aw_[123]五.wav",
    "/usr/share/std_resources/aw_[124]六.wav",
    "/usr/share/std_resources/aw_[125]七.wav",
    "/usr/share/std_resources/aw_[126]八.wav",
    "/usr/share/std_resources/aw_[127]九.wav",
    "/usr/share/std_resources/aw_[128]十.wav",
    "/usr/share/std_resources/aw_[129]百.wav",
    "/usr/share/std_resources/aw_[130]送风模式不能设置温度.wav",
    "/usr/share/std_resources/aw_[131]最高温度.wav",
    "/usr/share/std_resources/aw_[132]最低温度.wav",
    "/usr/share/std_resources/aw_[133]自动模式.wav",
    "/usr/share/std_resources/aw_[134]制冷模式.wav",
    "/usr/share/std_resources/aw_[135]制热模式.wav",
    "/usr/share/std_resources/aw_[136]送风模式.wav",
    "/usr/share/std_resources/aw_[137]抽湿模式.wav",
    "/usr/share/std_resources/aw_[138]摆风功能.wav",
    "/usr/share/std_resources/aw_[139]上下摆风.wav",
    "/usr/share/std_resources/aw_[140]左右摆风.wav",
    "/usr/share/std_resources/aw_[141]开机.wav",
    "/usr/share/std_resources/aw_[142]关机.wav",
    "/usr/share/std_resources/aw_[143]空调将在.wav",
    "/usr/share/std_resources/aw_[144]三十分钟后.wav",
    "/usr/share/std_resources/aw_[145]小时后.wav",
    "/usr/share/std_resources/aw_[146]定时关机功能已取消.wav",
    "/usr/share/std_resources/aw_[147]定时开机功能已取消.wav",
    "/usr/share/std_resources/aw_[148]只能在制冷模式或者制热模式下运行.wav",
    "/usr/share/std_resources/aw_[149]自清洁.wav",
    "/usr/share/std_resources/aw_[150]强劲.wav",
    "/usr/share/std_resources/aw_[151]电辅热.wav",
    "/usr/share/std_resources/aw_[152]只能在制热模式下或者自动模式下运行.wav",
    "/usr/share/std_resources/aw_[153]防直吹.wav",
    "/usr/share/std_resources/aw_[154]只能在制冷模式下运行.wav",
    "/usr/share/std_resources/aw_[155]音量.wav",
    "/usr/share/std_resources/aw_[156]屏显.wav",
    "/usr/share/std_resources/aw_[157]欢迎使用美的语音空调.wav",
    "/usr/share/std_resources/aw_[158]风已向左吹.wav",
    "/usr/share/std_resources/aw_[159]风已向右吹.wav",
    "/usr/share/std_resources/aw_[160]风已向上吹.wav",
    "/usr/share/std_resources/aw_[161]风已向下吹.wav",
    "/usr/share/std_resources/aw_[162]正在等待配网，请打开美居APP，点击添加设备，并按指示操作.wav",
    "/usr/share/std_resources/aw_[163]最大音量.wav",
    "/usr/share/std_resources/aw_[164]最小音量.wav",
    "/usr/share/std_resources/aw_[165]防过冷.wav",
    "/usr/share/std_resources/aw_[166]风速已设为.wav",
    "/usr/share/std_resources/aw_[167]百分之.wav",
    "/usr/share/std_resources/aw_[168]好的主人.wav",
    "/usr/share/std_resources/aw_[169]设备已打开.wav",
    "/usr/share/std_resources/aw_[170]设备已关闭.wav",
    "/usr/share/std_resources/aw_[171]设备已设定.wav",
    "/usr/share/std_resources/aw_[172]设备控制失败.wav",
    "/usr/share/std_resources/aw_[173]主人.wav",
    "/usr/share/std_resources/aw_[174]您回来了.wav",
    "/usr/share/std_resources/aw_[175]主人再见.wav",
    "/usr/share/std_resources/aw_[176]主人，您回来了.wav",
    "/usr/share/std_resources/aw_[177]欢迎您回来.wav",
    "/usr/share/std_resources/aw_[178]检测到客厅空调滤网很脏，建议马上清洗.wav",
    "/usr/share/std_resources/aw_[179]电饭煲已完成工作.wav",
    "/usr/share/std_resources/aw_[180]烹饪机已完成工作.wav",
    "/usr/share/std_resources/aw_[181]破壁机已完成工作.wav",
    "/usr/share/std_resources/aw_[182]压力锅烹饪完成.wav",
    "/usr/share/std_resources/aw_[183]碗洗好了.wav",
    "/usr/share/std_resources/aw_[184]水烧好了.wav",
    "/usr/share/std_resources/aw_[185]衣服已洗好.wav",
    "/usr/share/std_resources/aw_[186]热水器加热已完成.wav",
    "/usr/share/std_resources/aw_[187]加湿器缺水，请加水.wav",
    "/usr/share/std_resources/aw_[188]净化器滤网寿命已到期，请及时更换.wav",
    "/usr/share/std_resources/aw_[189]舒省.wav",
    "/usr/share/std_resources/aw_[190]二十.wav",
    "/usr/share/std_resources/aw_[191]三十.wav",
    "/usr/share/std_resources/aw_[192]四十.wav",
    "/usr/share/std_resources/aw_[193]五十.wav",
    "/usr/share/std_resources/aw_[194]六十.wav",
    "/usr/share/std_resources/aw_[195]七十.wav",
    "/usr/share/std_resources/aw_[196]八十.wav",
    "/usr/share/std_resources/aw_[197]九十.wav",
    "/usr/share/std_resources/aw_[198]百分之百.wav",
    "/usr/share/std_resources/aw_[199]百分之一.wav",
    "/usr/share/std_resources/aw_[200]百分之二十.wav",
    "/usr/share/std_resources/aw_[201]百分之四十.wav",
    "/usr/share/std_resources/aw_[202]百分之六十.wav",
    "/usr/share/std_resources/aw_[203]百分之八十.wav",
    "/usr/share/std_resources/aw_[204]智控温.wav",
    "/usr/share/std_resources/aw_[205]晴.wav",
    "/usr/share/std_resources/aw_[206]多云.wav",
    "/usr/share/std_resources/aw_[207]阴.wav",
    "/usr/share/std_resources/aw_[208]阵雨.wav",
    "/usr/share/std_resources/aw_[209]雷阵雨.wav",
    "/usr/share/std_resources/aw_[210]雷阵雨伴有冰雹.wav",
    "/usr/share/std_resources/aw_[211]雨夹雪.wav",
    "/usr/share/std_resources/aw_[212]小雨.wav",
    "/usr/share/std_resources/aw_[213]中雨.wav",
    "/usr/share/std_resources/aw_[214]大雨.wav",
    "/usr/share/std_resources/aw_[215]暴雨.wav",
    "/usr/share/std_resources/aw_[216]大暴雨.wav",
    "/usr/share/std_resources/aw_[217]特大暴雨.wav",
    "/usr/share/std_resources/aw_[218]阵雪.wav",
    "/usr/share/std_resources/aw_[219]小雪.wav",
    "/usr/share/std_resources/aw_[220]中雪.wav",
    "/usr/share/std_resources/aw_[221]大雪.wav",
    "/usr/share/std_resources/aw_[222]暴雪.wav",
    "/usr/share/std_resources/aw_[223]雾.wav",
    "/usr/share/std_resources/aw_[224]冻雨.wav",
    "/usr/share/std_resources/aw_[225]沙尘暴.wav",
    "/usr/share/std_resources/aw_[226]小到大雨.wav",
    "/usr/share/std_resources/aw_[227]中到大雨.wav",
    "/usr/share/std_resources/aw_[228]大到暴雨.wav",
    "/usr/share/std_resources/aw_[229]暴雨到特大暴雨.wav",
    "/usr/share/std_resources/aw_[230]大暴雨到特大暴雨.wav",
    "/usr/share/std_resources/aw_[231]小到中雪.wav",
    "/usr/share/std_resources/aw_[232]中到大雪.wav",
    "/usr/share/std_resources/aw_[233]大到暴雪.wav",
    "/usr/share/std_resources/aw_[234]浮尘.wav",
    "/usr/share/std_resources/aw_[235]扬沙.wav",
    "/usr/share/std_resources/aw_[236]强沙尘暴.wav",
    "/usr/share/std_resources/aw_[237]北风.wav",
    "/usr/share/std_resources/aw_[238]东风.wav",
    "/usr/share/std_resources/aw_[239]南风.wav",
    "/usr/share/std_resources/aw_[240]西风.wav",
    "/usr/share/std_resources/aw_[241]东北风.wav",
    "/usr/share/std_resources/aw_[242]东南风.wav",
    "/usr/share/std_resources/aw_[243]西北风.wav",
    "/usr/share/std_resources/aw_[244]西南风.wav",
    "/usr/share/std_resources/aw_[245]北东北风.wav",
    "/usr/share/std_resources/aw_[246]北西北风.wav",
    "/usr/share/std_resources/aw_[247]东东北风.wav",
    "/usr/share/std_resources/aw_[248]东东南风.wav",
    "/usr/share/std_resources/aw_[249]南东南风.wav",
    "/usr/share/std_resources/aw_[250]南西南风.wav",
    "/usr/share/std_resources/aw_[251]西西南风.wav",
    "/usr/share/std_resources/aw_[252]西西北风.wav",
    "/usr/share/std_resources/aw_[253]无持续风.wav",
    "/usr/share/std_resources/aw_[254]温度.wav",
    "/usr/share/std_resources/aw_[255]摄氏度.wav",
    "/usr/share/std_resources/aw_[256]空气质量.wav",
    "/usr/share/std_resources/aw_[257]优.wav",
    "/usr/share/std_resources/aw_[258]良.wav",
    "/usr/share/std_resources/aw_[259]轻度污染.wav",
    "/usr/share/std_resources/aw_[260]中度污染.wav",
    "/usr/share/std_resources/aw_[261]重度污染.wav",
    "/usr/share/std_resources/aw_[262]严重污染.wav",
    "/usr/share/std_resources/aw_[263]极度污染.wav",
    "/usr/share/std_resources/aw_[264]今天天气.wav",
    "/usr/share/std_resources/aw_[265]转.wav",
    "/usr/share/std_resources/aw_[266]到.wav",
    "/usr/share/std_resources/aw_[267]湿度.wav",
    "/usr/share/std_resources/aw_[268]湿度偏干.wav",
    "/usr/share/std_resources/aw_[269]湿度适宜.wav",
    "/usr/share/std_resources/aw_[270]湿度潮湿.wav",
    "/usr/share/std_resources/aw_[271]只能在制冷模式、抽湿模式或者送风模式下运行.wav",
    "/usr/share/std_resources/aw_[272]空调进入自清洁.wav",
    "/usr/share/std_resources/aw_[273]预计四十五分钟完成.wav",
    "/usr/share/std_resources/aw_[274]空调正在自清洁.wav",
    "/usr/share/std_resources/aw_[275]小美小美.wav",
    "/usr/share/std_resources/aw_[276]正在连接网络.wav",
    "/usr/share/std_resources/aw_[277]网络连接已成功.wav",
    "/usr/share/std_resources/aw_[278]WIFI密码错误，请重新配网.wav",
    "/usr/share/std_resources/aw_[279]连接网络失败，请重新配网.wav",
    "/usr/share/std_resources/aw_[280]测试失败.wav",
    "/usr/share/std_resources/aw_[281]测试成功.wav",
    "/usr/share/std_resources/aw_[282]风吹人.wav",
    "/usr/share/std_resources/aw_[283]风避人.wav",
    "/usr/share/std_resources/aw_[284]智慧风.wav",
    "/usr/share/std_resources/aw_[285]上无风感.wav",
    "/usr/share/std_resources/aw_[286]下无风感.wav",
    "/usr/share/std_resources/aw_[287]全无风感.wav",
    "/usr/share/std_resources/aw_[288]无风感.wav",
    "/usr/share/std_resources/aw_[289]儿童防冷风.wav",
    "/usr/share/std_resources/aw_[290]模式.wav",
    "/usr/share/std_resources/aw_[291]欢迎使用美的语音空调.wav",
    "/usr/share/std_resources/aw_[292]欢迎使用华菱语音空调.wav",
    "/usr/share/std_resources/aw_[293]空调空调.wav",
    "/usr/share/std_resources/aw_[294]网络未连接，请开始配网.wav",
    "/usr/share/std_resources/aw_[295]网络出现异常，请稍候再试一下吧.wav",
    "/usr/share/std_resources/aw_[296]我在.wav",
    "/usr/share/std_resources/aw_[297]在呢.wav",
    "/usr/share/std_resources/aw_[298]唉.wav",
    "/usr/share/std_resources/aw_[299]你好.wav",
    "/usr/share/std_resources/aw_[300]来啦.wav",
    "/usr/share/std_resources/aw_[301]定时已取消.wav",
    "/usr/share/std_resources/aw_[302]完成后将自动关机.wav",
    "/usr/share/std_resources/aw_[303]已开始扫地.wav",
    "/usr/share/std_resources/aw_[304]已暂停扫地.wav",
    "/usr/share/std_resources/aw_[305]马上回去充电.wav",
    "/usr/share/std_resources/aw_[306]风扇.wav",
    "/usr/share/std_resources/aw_[307]加湿器.wav",
    "/usr/share/std_resources/aw_[308]净化器.wav",
    "/usr/share/std_resources/aw_[309]卧室空调.wav",
    "/usr/share/std_resources/aw_[310]灯.wav",

    "/usr/share/std_resources/aw_[353]xiaomei.wav",//xiaomeixiaomei wakeup for factory test
    "/usr/share/std_resources/aw_[354]do2.wav",//beep
    "/usr/share/std_resources/aw_[355]do3.wav",//asr timeout notice
#ifdef EII_TEST_ENABLE
    "/usr/share/std_resources/aw_356.wav",//定时开机功能
    "/usr/share/std_resources/aw_357.wav",//定时关机功能
    "/usr/share/std_resources/aw_358.wav",//打开客厅风扇
    "/usr/share/std_resources/aw_359.wav",//打开卧室风扇
    "/usr/share/std_resources/aw_360.wav",//关闭客厅风扇
    "/usr/share/std_resources/aw_361.wav",//关闭卧室风扇
    "/usr/share/std_resources/aw_362.wav",//客厅风扇摇头
    "/usr/share/std_resources/aw_363.wav",//卧室风扇摇头
    "/usr/share/std_resources/aw_364.wav",//客厅风扇停止摇头
    "/usr/share/std_resources/aw_365.wav",//卧室风扇停止摇头
    "/usr/share/std_resources/aw_366.wav",//客厅风扇调大一档
    "/usr/share/std_resources/aw_367.wav",//卧室风扇调大一档
    "/usr/share/std_resources/aw_368.wav",//客厅风扇调小一档
    "/usr/share/std_resources/aw_369.wav",//卧室风扇调小一档
    "/usr/share/std_resources/aw_370.wav",//客厅风扇调到最大
    "/usr/share/std_resources/aw_371.wav",//卧室风扇调到最大
    "/usr/share/std_resources/aw_372.wav",//客厅风扇调到最小
    "/usr/share/std_resources/aw_373.wav",//卧室风扇调到最小
    "/usr/share/std_resources/aw_374.wav",//打开客厅加湿器
    "/usr/share/std_resources/aw_375.wav",//打开卧室加湿器
    "/usr/share/std_resources/aw_376.wav",//关闭客厅加湿器
    "/usr/share/std_resources/aw_377.wav",//关闭卧室加湿器
    "/usr/share/std_resources/aw_378.wav",//打开客厅净化器
    "/usr/share/std_resources/aw_379.wav",//打开卧室净化器
    "/usr/share/std_resources/aw_380.wav",//关闭客厅净化器
    "/usr/share/std_resources/aw_381.wav",//关闭卧室净化器
    "/usr/share/std_resources/aw_382.wav",//扫地机器人全屋清扫
    "/usr/share/std_resources/aw_383.wav",//扫地机器人充电
    "/usr/share/std_resources/aw_384.wav",//扫地机器人暂停扫地
    "/usr/share/std_resources/aw_385.wav",//打开卧室空调
    "/usr/share/std_resources/aw_386.wav",//关闭卧室空调
    "/usr/share/std_resources/aw_387.wav",//客厅开灯
    "/usr/share/std_resources/aw_388.wav",//卧室开灯
    "/usr/share/std_resources/aw_389.wav",//客厅关灯
    "/usr/share/std_resources/aw_390.wav",//卧室关灯
    "/usr/share/std_resources/aw_391.wav",//我回来了
    "/usr/share/std_resources/aw_392.wav",//我到家了
    "/usr/share/std_resources/aw_393.wav",//我上班去了
    "/usr/share/std_resources/aw_394.wav",//我出门了
#endif
};

#endif













