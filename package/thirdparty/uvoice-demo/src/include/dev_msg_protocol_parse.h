#ifndef __DEV_MSG_PROTOCOL_PARSE_H__
#define __DEV_MSG_PROTOCOL_PARSE_H__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <semaphore.h>
#include "log.h"
#include "app_uart.h"
#include "usr_config.h"
#include "asr_config.h"

#define LOGTAG    ("ASR-UART")
#define DEBUG_ASR_UART    1    //0

#define USE_TIMER    (0)

#define ASR_UART_BUFFER_SIZE    (256) //modify from 64 to 256  for EII ctl devs by zhouhuo@20190409
#define ASR_FRAME_HEADER    (0xAA)
#define ASR_UART_STATUS_QUERY_INTERVAL    (5000)
#define ASR_FRAME_REPLY_TIMEOUT    (1000)

#define     FALSE    0
#define     TRUE    1

pthread_mutex_t ring_buffer_lock;

#define LOCK_RING_BUFFER()   {pthread_mutex_lock(&ring_buffer_lock);}//; \
    printf("ring buffer lock,%s,%d\n",__FUNCTION__,__LINE__);}
#define UNLOCK_RING_BUFFER() {pthread_mutex_unlock(&ring_buffer_lock);}//; \
    printf("ring buffer unlock,%s,%d\n",__FUNCTION__,__LINE__);}

typedef struct {
    uint8_t header;
    uint8_t msgId;
    uint8_t msgLen;
    uint8_t play;
    uint8_t rsvd1;
    uint8_t data[0]; //delete 06281350
    //uint8_t data[ASR_UART_BUFFER_SIZE]; //add 06281350
} __attribute__((packed)) cmd_uart_msg_t;

typedef enum{
    ASR_UART_MSG_NONE        = 0xFF,
    /*
     * msgid
    */
    ASR_UART_MSG_HEART_BEAT     = 0x00,
    ASR_UART_MSG_WAKE_UP         = 0x01,
    ASR_UART_MSG_ON                = 0x02,
    ASR_UART_MSG_SET_MODE         = 0x03,
    ASR_UART_MSG_SET_TEMP         = 0x04,
    ASR_UART_MSG_WIND_SPEED        = 0x05,
    ASR_UART_MSG_SET_SWING        = 0x06,
    ASR_UART_MSG_SET_TIMER        = 0x07,
    ASR_UART_MSG_SET_VOLUME        = 0x08,
    ASR_UART_MSG_SET_BLOW        = 0x0A,
    ASR_UART_MSG_SET_FUNC        = 0x10,
    ASR_UART_MSG_SET_FACTORY    = 0x12,
    ASR_UART_MSG_QUERY_WEATHER  = 0x13,
    ASR_UART_MSG_SET_WIFI        = 0x22,
    ASR_UART_MSG_PASSTHROUGH    = 0x41,

    /*
     * board
    */
    ASR_UART_MSG_BOARD_STARTUP_PLAY        = 0x80,
    ASR_UART_MSG_BOARD_BEEP_NOTICE      = 0x81,
    ASR_UART_MSG_BOARD_FACTORY_TEST        = 0x82,
    ASR_UART_MSG_BOARD_WIFI_STATUS        = 0x83,
    ASR_UART_MSG_BOARD_SET_CONFIGS        = 0xC1,
    ASR_UART_MSG_BOARD_WIFI_PASSTHROUGH    = 0xD4,

    ASR_UART_MSG_OFF             = 0xF000,
    ASR_UART_MSG_TIMEOUT_RECOG    = 0xF001,
    ASR_UART_MSG_INC_T_BY_1        = 0xF002,
    ASR_UART_MSG_INC_T_BY_3        = 0xF003,
    ASR_UART_MSG_DEC_T_BY_1        = 0xF004,
    ASR_UART_MSG_DEC_T_BY_3        = 0xF005,
    ASR_UART_MSG_WIND_LOW         = 0xF006,
    ASR_UART_MSG_WIND_MID         = 0xF007,
    ASR_UART_MSG_WIND_HIGH         = 0xF008,
    ASR_UART_MSG_WIND_AUTO        = 0xF009,
    ASR_UART_MSG_SWING_ON        = 0xF00A,
    ASR_UART_MSG_SWING_OFF        = 0xF00B,
    ASR_UART_MSG_SWING_UP_DOWN_ON    = 0xF00C,
    ASR_UART_MSG_SWING_UP_DOWN_OFF    = 0xF00D,
    ASR_UART_MSG_SWING_LEFT_RIGHT_ON = 0xF00E,
    ASR_UART_MSG_SWING_LEFT_RIGHT_OFF = 0xF00F,
    ASR_UART_MSG_BLOW_LEFT        = 0xF010,
    ASR_UART_MSG_BLOW_RIGHT        = 0xF011,
    ASR_UART_MSG_BLOW_UP        = 0xF012,
    ASR_UART_MSG_BLOW_DOWN        = 0xF013,
    ASR_UART_MSG_COMFORT_ECO_ON        = 0xF014,
    ASR_UART_MSG_COMFORT_ECO_OFF    = 0xF015,
    ASR_UART_MSG_SELF_CLEAN_ON    = 0xF016,
    ASR_UART_MSG_SELF_CLEAN_OFF    = 0xF017,
    ASR_UART_MSG_AUXILIARY_ELECTRIC_HEATING_ON  = 0xF018,
    ASR_UART_MSG_AUXILIARY_ELECTRIC_HEATING_OFF = 0xF019,
    ASR_UART_MSG_ANTI_BLOW_ON    = 0xF01A,
    ASR_UART_MSG_ANTI_BLOW_OFF    = 0xF01B,
    ASR_UART_MSG_SCREEN_DISPLAY_ON  = 0xF01C,
    ASR_UART_MSG_SCREEN_DISPLAY_OFF = 0xF01D,
    ASR_UART_MSG_ANTI_COLD_ON        = 0xF01E,
    ASR_UART_MSG_ANTI_COLD_OFF        = 0xF01F,
    ASR_UART_MSG_STRONG_MODE_ON        = 0xF020,
    ASR_UART_MSG_STRONG_MODE_OFF    = 0xF021,
    ASR_UART_MSG_TIMER_30M_ON    = 0xF022,
    ASR_UART_MSG_TIMER_1H_ON    = 0xF023,
    ASR_UART_MSG_TIMER_2H_ON    = 0xF024,
    ASR_UART_MSG_TIMER_3H_ON    = 0xF025,
    ASR_UART_MSG_TIMER_4H_ON    = 0xF026,
    ASR_UART_MSG_TIMER_5H_ON    = 0xF027,
    ASR_UART_MSG_TIMER_6H_ON    = 0xF028,
    ASR_UART_MSG_TIMER_7H_ON    = 0xF029,
    ASR_UART_MSG_TIMER_8H_ON    = 0xF02A,

    ASR_UART_MSG_TIMER_30M_OFF    = 0xF02B,
    ASR_UART_MSG_TIMER_1H_OFF    = 0xF02C,
    ASR_UART_MSG_TIMER_2H_OFF    = 0xF02D,
    ASR_UART_MSG_TIMER_3H_OFF    = 0xF02E,
    ASR_UART_MSG_TIMER_4H_OFF    = 0xF030,
    ASR_UART_MSG_TIMER_5H_OFF    = 0xF031,
    ASR_UART_MSG_TIMER_6H_OFF    = 0xF032,
    ASR_UART_MSG_TIMER_7H_OFF    = 0xF033,
    ASR_UART_MSG_TIMER_8H_OFF    = 0xF034,

    ASR_UART_MSG_TIMER_DISABLE        = 0xF035,
    ASR_UART_MSG_SET_VOLUME_MAX     = 0xF036,
    ASR_UART_MSG_SET_VOLUME_MIN     = 0xF037,
    ASR_UART_MSG_SET_VOLUME_INC     = 0xF038,
    ASR_UART_MSG_SET_VOLUME_DEC     = 0xF039,

    /*wifi passthrough*/
    ASR_UART_MSG_LIVING_ROOM_FAN_ON  = 0xF03A,
    ASR_UART_MSG_LIVING_ROOM_FAN_OFF = 0xF03B,
    ASR_UART_MSG_BED_ROOM_FAN_ON     = 0xF03C,
    ASR_UART_MSG_BED_ROOM_FAN_OFF    = 0xF03D,
    ASR_UART_MSG_LIVING_ROOM_HUMIDIFIER_ON  = 0xF03E,
    ASR_UART_MSG_LIVING_ROOM_HUMIDIFIER_OFF = 0xF03F,
    ASR_UART_MSG_BED_ROOM_HUMIDIFIER_ON     = 0xF040,
    ASR_UART_MSG_BED_ROOM_HUMIDIFIER_OFF    = 0xF041,
    ASR_UART_MSG_LIVING_ROOM_AIR_CLEANER_ON = 0xF042,
    ASR_UART_MSG_LIVING_ROOM_AIR_CLEANER_OFF = 0xF043,
    ASR_UART_MSG_BED_ROOM_AIR_CLEANER_ON    = 0xF044,
    ASR_UART_MSG_BED_ROOM_AIR_CLEANER_OFF   = 0xF045,
    ASR_UART_MSG_BED_ROOM_AC_ON     = 0xF046,
    ASR_UART_MSG_BED_ROOM_AC_OFF    = 0xF047,
    ASR_UART_MSG_LIVING_ROOM_LAMP_ON    = 0xF048,
    ASR_UART_MSG_LIVING_ROOM_LAMP_OFF   = 0xF049,
    ASR_UART_MSG_BED_ROOM_LAMP_ON   = 0xF04A,
    ASR_UART_MSG_BED_ROOM_LAMP_OFF  = 0xF04B,
    ASR_UART_MSG_LIVING_ROOM_FAN_OSC_ON     = 0xF04C,
    ASR_UART_MSG_BED_ROOM_FAN_OSC_ON        = 0xF04D,
    ASR_UART_MSG_LIVING_ROOM_FAN_OSC_OFF    = 0xF04E,
    ASR_UART_MSG_BED_ROOM_FAN_OSC_OFF   = 0xF04F,
    ASR_UART_MSG_LIVING_ROOM_FAN_INC    = 0xF050,
    ASR_UART_MSG_LIVING_ROOM_FAN_DEC    = 0xF051,
    ASR_UART_MSG_BED_ROOM_FAN_INC       = 0xF052,
    ASR_UART_MSG_BED_ROOM_FAN_DEC       = 0xF053,
    ASR_UART_MSG_LIVING_ROOM_FAN_MAX    = 0xF054,
    ASR_UART_MSG_BED_ROOM_FAN_MAX       = 0xF055,
    ASR_UART_MSG_LIVING_ROOM_FAN_MIN    = 0xF056,
    ASR_UART_MSG_BED_ROOM_FAN_MIN       = 0xF057,
    ASR_UART_MSG_SWEEP_ROBOT_CLEAN_ALL    = 0xF058,
    ASR_UART_MSG_SWEEP_ROBOT_CHARGING    = 0xF059,
    ASR_UART_MSG_SWEEP_ROBOT_CLEAN_PAUSE = 0xF05A,
    //ASR_UART_MSG_QUERY_WEATHER          = 0xF05B,
    ASR_UART_MSG_BACK_HOME              = 0xF05C,
    ASR_UART_MSG_LEAVE_HOME             = 0xF05D,

    /* factory test */
    ASR_UART_MSG_FACTORY_TEST_PASS      = 0xF05E,
    ASR_UART_MSG_FACTORY_TEST_FAIL      = 0xF05F,


    ASR_UART_MSG_INC_HALFDEGREE_TEMP        = 0xF060,
    ASR_UART_MSG_DEC_HALFDEGREE_TEMP        = 0xF061,

}ASR_FRAME_MSG_ID;

typedef enum
{
    ASR_AC_WIND_SLOW     = 0x01,
    ASR_AC_WIND_20         = 0x02,
    ASR_AC_WIND_40         = 0x03,
    ASR_AC_WIND_MID     = 0x04,
    ASR_AC_WIND_80         = 0x05,
    ASR_AC_WIND_HIGH     = 0x06,

    ASR_AC_WIND_AUTO     = 0x07,
    ASR_AC_WIND_INC     = 0x10,
    ASR_AC_WIND_DEC     = 0x11,
    ASR_AC_WIND_MIN     = 0x01,
    ASR_AC_WIND_MAX     = 0x06,
}ASR_AC_WIND;

typedef enum
{
    ASR_SPEECH_VENDOR_UNISOUND    = 0x01, //云知声
    ASR_SPEECH_VENDOR_IFLYTEK     = 0x02,    //科大讯飞
    ASR_SPEECH_VENDOR_AISPEECH    = 0x03, //思必驰
    ASR_SPEECH_VENDOR_UVOICETECH    = 0x04, //声翰
    ASR_SPEECH_VENDOR_OTHERS    = 0x05, //其他

    ASR_SPEECH_VENDOR_MIN         = 0x01,
    ASR_SPEECH_VENDOR_MAX         = 0x05,
}ASR_SPEECH_VENDOR;

typedef enum
{
    ASR_AC_SWING_ON = 0x01,
    ASR_AC_SWING_OFF = 0x02,
    ASR_AC_SWING_UP_DOWN_ON = 0x03,
    ASR_AC_SWING_UP_DOWN_OFF = 0x04,
    ASR_AC_SWING_LEFT_RIGHT_ON = 0x05,
    ASR_AC_SWING_LEFT_RIGHT_OFF = 0x06,
    ASR_AC_SWING_ALL = 0x07,
    ASR_AC_SWING_MIN = ASR_AC_SWING_OFF,
    ASR_AC_SWING_MAX = ASR_AC_SWING_ALL,
}ASR_AC_SWING;

typedef enum
{
    ASR_AC_BLOW_LEFT,
    ASR_AC_BLOW_RIGHT,
    ASR_AC_BLOW_UP,
    ASR_AC_BLOW_DOWN,
    ASR_AC_BLOW_MIN = ASR_AC_BLOW_LEFT,
    ASR_AC_BLOW_MAX = ASR_AC_BLOW_DOWN,
}ASR_AC_BLOW;

typedef enum
{
    ASR_AC_TIMER_05_ON,
    ASR_AC_TIMER_05_OFF,
    ASR_AC_TIMER_10_ON,
    ASR_AC_TIMER_10_OFF,
    ASR_AC_TIMER_20_ON,
    ASR_AC_TIMER_20_OFF,
    ASR_AC_TIMER_30_ON,
    ASR_AC_TIMER_30_OFF,
    ASR_AC_TIMER_40_ON,
    ASR_AC_TIMER_40_OFF,
    ASR_AC_TIMER_50_ON,
    ASR_AC_TIMER_50_OFF,
    ASR_AC_TIMER_60_ON,
    ASR_AC_TIMER_60_OFF,
    ASR_AC_TIMER_70_ON,
    ASR_AC_TIMER_70_OFF,
    ASR_AC_TIMER_80_ON,
    ASR_AC_TIMER_80_OFF,
    ASR_AC_TIMER_DISABLE
}ASR_AC_TIMER_FUNC;

typedef enum
{
    ASR_AC_STRONG_MODE_ON,
    ASR_AC_STRONG_MODE_OFF
}ASR_AC_STRONG_MODE;

typedef enum
{
    ASR_AC_SCREEN_DISPLAY_ON,
    ASR_AC_SCREEN_DISPLAY_OFF
}ASR_AC_SCREEN_DISPLAY;

typedef enum
{
    ASR_OTA_ON,
    ASR_OTA_OFF
}ASR_OTA;

typedef enum
{
    ASR_OTA_STATUS_IDLE,
    ASR_OTA_STATUS_LOADING,
    ASR_OTA_STATUS_LOADSUCCESS,
    ASR_OTA_STATUS_LOADFAIL,
    ASR_OTA_STATUS_OTAING,
    ASR_OTA_STATUS_OTASUCCESS,
    ASR_OTA_STATUS_OTAFAIL
}ASR_OTA_STATUS;

typedef struct{
    ASR_OTA ota;
    ASR_OTA_STATUS ota_status;
    uint8_t ota_loading_progress;
}ota_info_t;

typedef enum
{
    ASR_AC_AUX_ELECTRIC_HEAT_MODE_ON,
    ASR_AC_AUX_ELECTRIC_HEAT_MODE_OFF
}ASR_AC_AUX_ELECTRIC_HEAT_MODE;

typedef enum
{
    ASR_AC_SELF_CLEAN_ON,
    ASR_AC_SELF_CLEAN_OFF,
    ASR_AC_SELF_CLEAN_FLAG,
    ASR_AC_SELF_CLEAN_DEFAULT
}ASR_AC_SELF_CLEAN;

typedef enum
{
    ASR_AC_ASR_TYPE_OFF,
    ASR_AC_ASR_TYPE_ON,
    ASR_AC_ASR_TYPE_SET,
    ASR_AC_ASR_TYPE_ROBOT_CLEAN,
    ASR_AC_ASR_TYPE_ROBOT_PAUSE,
    ASR_AC_ASR_TYPE_ROBOT_CHARGING,
    ASR_AC_ASR_TYPE_WEATHER,
    ASR_AC_ASR_TYPE_BACK_HOME,
    ASR_AC_ASR_TYPE_LEAVE_HOME,
    ASR_AC_ASR_TYPE_DEFAULT
}ASR_AC_ASR_TYPE;

typedef enum
{
    ASR_AC_CONFORT_ECO_ON,
    ASR_AC_CONFORT_ECO_OFF
}ASR_AC_CONFORT_ECO;

typedef enum
{
    ASR_AC_ANTI_BLOW_ON,
    ASR_AC_ANTI_BLOW_OFF
}ASR_AC_ANTI_BLOW;

typedef enum
{
    ASR_AC_ANTI_COLD_ON,
    ASR_AC_ANTI_COLD_OFF
}ASR_AC_ANTI_COLD;

typedef enum
{
    ASR_AC_MODE_MIN        = 0x01,
    ASR_AC_MODE_COOL     = 0x01,    //cool mode
    ASR_AC_MODE_HEAT     = 0x02,    //heat mode
    ASR_AC_MODE_DRY     = 0x03,    //dry mode
    ASR_AC_MODE_WIND    = 0x04,    //wind mode
    ASR_AC_MODE_AUTO    = 0x05,    //auto mode
    ASR_AC_MODE_MAX        = 0x05,
}ASR_AC_MODE;

typedef enum{
    ASR_UART_MSG_TYPE_CMD,
    ASR_UART_MSG_TYPE_SET_MODE,
    ASR_UART_MSG_TYPE_SET_T,
    ASR_UART_MSG_TYPE_SET_WIND,
    ASR_UART_MSG_TYPE_SET_FUNC,
    ASR_UART_MSG_TYPE_SET_TIMER,
    ASR_UART_MSG_TYPE_PASSTHROUGH,
    ASR_UART_MSG_TYPE_ACK_BOARD_STD,
    ASR_UART_MSG_TYPE_SET_VOLUME,
    ASR_UART_MSG_TYPE_QUERY_WEATHER,
    ASR_UART_MSG_TYPE_FACTORY,
}ASR_UART_MSG_TYPE;

typedef struct {
    //char info[8 * DEV_NUM_OF_CONFIG_SUPPORT];
    char info[0];
} __attribute__((packed)) device_configs_t;

typedef enum
{
    ASR_LIVING_ROOM            = 0x00, //客厅、起居室、会客室、用户未指定地点
    ASR_BED_ROOM            = 0x01, //卧室
    ASR_BED_MASTER_ROOM        = 0x02, //主卧
    ASR_BED_AUX_ROOM        = 0x03, //次卧、客卧
    ASR_DINING_ROOM            = 0x04, //餐厅
    ASR_STUDY_ROOM            = 0x05, //书房、图书馆
    ASR_REST_ROOM            = 0x06, //卫生间、洗手间
    ASR_REST_MASTER_ROOM    = 0x07, //主卫
    ASR_REST_GUEST_ROOM        = 0x08, //客卫、次卫
    ASR_KITCHEN             = 0x09, //厨房
    ASR_KIDS_ROOM            = 0x0A, //儿童房
    ASR_OLDER_ROOM            = 0x0B, //老人房
    ASR_HOUSEKEEPER_ROOM    = 0x0C, //保姆房
    ASR_ALL_ROOM            = 0xFF, //全部房间(扫地机器人、离家、归家)
}ASR_ROOM_TYPE;
/*
typedef enum
{
    ASR_OBJ_AC              = 0x00, //空调、挂机、柜机
    ASR_OBJ_FAN             = 0x01, //风扇
    ASR_OBJ_FAS             = 0x02, //新风系统
    ASR_OBJ_HEATER          = 0x03, //暖气、热风器
    ASR_OBJ_WATER_HEATER    = 0x04, //热水器
    ASR_OBJ_WATER_DISPENSER = 0x05, //饮水机
    ASR_OBJ_BATH_HEATER     = 0x06, //浴霸
    ASR_OBJ_AIR_CLEANER     = 0x07, //空气净化器
    ASR_OBJ_HUMIDIFIER      = 0x08, //加湿器
    ASR_OBJ_DEHUMIDIFIER    = 0x09, //除湿器
    ASR_OBJ_SWEEP_ROBOT     = 0x0A, //扫地机器人
    ASR_OBJ_LAMP            = 0x0B, //电灯
}ASR_DEVICE_TYPE;
*/
typedef enum
{
    ASR_OBJ_AC              = 0xAC, //空调、挂机、柜机
    ASR_OBJ_FAN             = 0xFA, //风扇
    ASR_OBJ_AIR_CLEANER     = 0xFC, //空气净化器
    ASR_OBJ_HUMIDIFIER      = 0xFD, //加湿器
    ASR_OBJ_SWEEP_ROBOT     = 0xB8, //扫地机器人
    ASR_OBJ_SMART_LAMP      = 0x13, //智能灯具
    ASR_OBJ_RICE_COOKER     = 0xEA, //电饭煲
    ASR_OBJ_CULINARY_MACHINE = 0xEB,//烹饪机
    ASR_OBJ_PRESSURE_COOKER = 0xEC, //压力锅
    ASR_OBJ_DISH_WASHER     = 0xE1, //洗碗机
    ASR_OBJ_RIPPLE_WASHING_MACHINE  = 0xDA, //波轮洗衣机
    ASR_OBJ_CYLINDER_WASHING_MACHINE  = 0xDB, //滚筒洗衣机
    ASR_OBJ_COMPOUND_WASHING_MACHINE  = 0xD9, //复合式洗衣机
    ASR_OBJ_AIR_WATER_HEATER    = 0xCD, //空气能热水器
    ASR_OBJ_ELECTRIC_WATER_HEATER     = 0xE2, //电热风器
    ASR_OBJ_GAS_WATER_HEATER    = 0xE3, //燃热风器
    ASR_OBJ_ELECTRIC_BOILER     = 0xBE, //电热水瓶
    ASR_OBJ_BROADCAST_ALL       = 0xFF, //广播家电类型
}ASR_DEVICE_TYPE;
/*
typedef enum
{
    ASR_CMD_TYPE_SET_POWER_ON_OFF   = 0x00, //开机/关机
    ASR_CMD_TYPE_SET_FAN            = 0x01, //风扇设置
    ASR_CMD_TYPE_SET_SWEEP_ROBOT    = 0x02,//扫地机器人设定
    ASR_CMD_TYPE_SET_QUERY_WEATHER  = 0x04, //天气查询
    ASR_CMD_TYPE_SET_I              = 0x05, //我回来了/我上班去了
}ASR_CMD_TYPE;
*/
typedef enum
{
    ASR_CMD_TYPE_SET_POWER_ON_OFF   = 0x00, //开机/关机
    ASR_CMD_TYPE_SET_FAN            = 0x01, //风扇设置
    ASR_CMD_TYPE_SET_SWEEP_ROBOT    = 0x02, //扫地机器人设定
    ASR_CMD_TYPE_SET_QUERY_WEATHER  = 0x03, //天气查询
    ASR_CMD_TYPE_SET_I              = 0x04, //我回来了/我上班去了
    ASR_CMD_TYPE_SET_EII_DEV_STATUS = 0x05, //上报联动其他设备语音识别/播报开关状态
    ASR_CMD_TYPE_SET_CONFIGS        = 0x06, //上报配置表数据到云端
}ASR_CMD_TYPE;

/*
typedef enum
{
    ASR_ACTION_POWEROFF         = 0x00, //关机
    ASR_ACTION_POWERON          = 0x01, //开机
    ASR_ACTION_SET_MODE         = 0x02, //模式设置
    ASR_ACTION_SET_TEMP         = 0x03, //温度设置
    ASR_ACTION_SET_WIND_SPEED   = 0x04, //风速设定
    ASR_ACTION_INC_TEMP         = 0x05, //调高温度
    ASR_ACTION_DEC_TEMP         = 0x06, //调低温度
    ASR_ACTION_SET_FAN          = 0x07, //风扇设置
    ASR_ACTION_SET_SWEEP_ROBOT  = 0x08, //扫地机器人设定
}ASR_ACTION_TYPE;
*/

typedef enum
{
    ASR_SET_I_TYPE_LEAVE_HOME  = 0x00, //我上班去了
    ASR_SET_I_TYPE_BACK_HOME   = 0x01, //我回来了
}ASR_CMD_TYPE_SET_I_TYPE;

typedef enum
{
    ASR_ACTION_POWEROFF         = 0x00, //关机
    ASR_ACTION_POWERON          = 0x01, //开机
}ASR_ACTION_SET_POWER_TYPE;

typedef enum
{
    ASR_SET_MODE_AUTO       = 0x01, //自动
    ASR_SET_MODE_COOL       = 0x02, //制冷
    ASR_SET_MODE_HEAT       = 0x03, //制热
    ASR_SET_MODE_WIND       = 0x04, //通风
    ASR_SET_MODE_DRY        = 0x05, //除湿
}ASR_ACTION_SET_MODE_TYPE;

typedef enum
{
    ASR_SET_WIND_SPEED_SLOW = 0x01, //最小风
    ASR_SET_WIND_SPEED_MID  = 0x02, //中等风
    ASR_SET_WIND_SPEED_HIGH = 0x03, //最大风
    ASR_SET_WIND_SPEED_AUTO = 0x04, //自动风
}ASR_ACTION_SET_WIND_SPEED_TYPE;

typedef enum
{
    ASR_SET_FAN_OSC_OFF = 0x00, //停止摇头
    ASR_SET_FAN_OSC_ON  = 0x01, //开启摇头
    ASR_SET_FAN_INC     = 0x02, //调大一档
    ASR_SET_FAN_DEC     = 0x03, //调小一档
    ASR_SET_FAN_MAX     = 0x04, //调到最大
    ASR_SET_FAN_MIN     = 0x05, //调到最小
}ASR_ACTION_SET_FAN_TYPE;

typedef enum
{
    ASR_SET_SWEEP_ROBOT_CLEAN_ALL   = 0x00, //全屋清扫
    ASR_SET_SWEEP_ROBOT_CHARGING    = 0x01, //充电
    ASR_SET_SWEEP_ROBOT_PAUSE = 0x02, //暂停扫地
}ASR_ACTION_SET_SWEEP_ROBOT_TYPE;

typedef enum
{
    ASR_BEEP_NOTICE_STARTUP     = 0x01, //上电铃声
    ASR_BEEP_NOTICE_POWERON     = 0x02, //开机铃声
    ASR_BEEP_NOTICE_POWEROFF    = 0x03, //关机铃声
    ASR_BEEP_NOTICE_DING        = 0x04, //纯“叮”铃声
    ASR_BEEP_NOTICE_SET_MODE    = 0x05, //模式切换
    ASR_BEEP_NOTICE_INC_TEMP    = 0x06, //温度上调
    ASR_BEEP_NOTICE_DEC_TEMP    = 0x07, //温度下调
}ASR_BEEP_NOTICE_TYPE;
/*
typedef enum
{
    ASR_DEV_CTL_REPLY_POWER_ON    = 0x00, //设备已打开
    ASR_DEV_CTL_REPLY_POWER_OFF   = 0x01, //设备已关闭
    ASR_DEV_CTL_REPLY_SET_SUCCESS = 0x02, //设备已设定
    ASR_DEV_CTL_REPLY_SET_FAIL    = 0x03, //设备控制失败
    ASR_DEV_QUERY_WEATHER         = 0x04, //天气查询
}ASR_DEV_CTL_REPLY_TYPE;
*/
typedef enum
{
    ASR_DEV_CLOUD_SND_CONFIGS     = 0xA0, //云端下发配置表数据
    ASR_DEV_CLOUD_RECV_CONFIGS    = 0xA1, //云端获取配置表数据
    ASR_DEV_CLOUD_QUERY_WEATHER   = 0xA2, //云端下发天气查询结果
    ASR_DEV_CLOUD_SND_EII_CONFIGS = 0xA3, //云端下发联动其他设备语音识别、播报开关状态
    ASR_DEV_CLOUD_RECV_EII_CONFIGS = 0xA4, //云端获取联动其他设备语音识别、播报开关状态
    ASR_DEV_CTL_REPLY_A5          = 0xA5, //设备控制成功(已打开、已关闭和已设定)
    ASR_DEV_CTL_WIFI_STATUS       = 0xA6, //wifi连接状态
    //ASR_DEV_CTL_REPLY_SET_SUCCESS = 0xA7, //设备已设定
    //ASR_DEV_CTL_REPLY_SET_FAIL    = 0xA8, //设备控制失败
    //ASR_DEV_NOTICE_A9             = 0xA9, //xxx空调滤网脏堵
    ASR_DEV_NOTICE_AA             = 0xAA, //设备工作完成
    //ASR_DEV_NOTICE_AB             = 0xAB, //xxx加湿器缺水
    //ASR_DEV_NOTICE_AC             = 0xAC, //xxx净化器滤网寿命到期
}ASR_DEV_CTL_REPLY_TYPE;

typedef enum
{
    ASR_DEV_QUERY_WEATHER_DISCONNECT    = 0x00, //网络未连接
    ASR_DEV_QUERY_WEATHER_SUCCESS       = 0x01, //查询天气成功
    ASR_DEV_QUERY_WEATHER_FAIL          = 0x02, //查询天气失败
}ASR_DEV_QUERY_WEATHER_TYPE;

typedef enum
{
    ASR_DEV_REPLY_WIFI_CONNECTING    = 0x00, //正在连接网络
    ASR_DEV_REPLY_WIFI_SUCCESS       = 0x01, //网络连接成功
    ASR_DEV_REPLY_WIFI_PWD_ERR       = 0x02, //wifi密码错误，请重新配网
    ASR_DEV_REPLY_WIFI_FAIL          = 0x03, //连接网络失败，请重新配网
}ASR_DEV_REPLY_WIFI_STATUS;

typedef enum
{
    ASR_DISABLE       = 0x00, //关闭
    ASR_ENABLE        = 0x01, //打开
}ASR_SET_FUNC_TYPE;

typedef enum{
    ASR_WAKEUP_WORD_KONGTIAO= 0x01,
    ASR_WAKEUP_WORD_XIAOMEI    = 0x02,
    ASR_WAKEUP_WORD_MASK    = 0x03,
}ASR_wakeup_word;

typedef struct{
    ASR_wakeup_word wakeup_word;
    uint8_t wakeup_status;
}ASR_wakeup_info_t;

typedef enum{
    ASR_PLAY_VOICE_STANDARD = 0x01,
    ASR_PLAY_VOICE_SWEET    = 0x02,
    ASR_PLAY_VOICE_BEEP    = 0x04,
}ASR_play_voice_type;

typedef enum{

    ASR_AC_ERROR_NONE               = 0,
    ASR_AC_ERROR_POWER_ON_FIRST     = -1,

    ASR_AC_ERROR_UART_TIMEOUT       = -100,
    ASR_AC_ERROR_INVALID_PARAM      = -101,
    ASR_AC_ERROR_MEM_FAIL           = -102,
    ASR_AC_ERROR_UART_REPLY_ERROR   = -103,
    ASR_AC_ERROR_NOT_ALLOWED        = -104,
    ASR_AC_ERROR_NO_DEVCONFIG       = -105,
    ASR_AC_ERROR_NO_PLAY            = -200,
}ASR_AC_STATUS;

typedef struct{
    uint8_t can_recog;
    uint8_t can_play;
    ASR_wakeup_word wakeup_word;
    ASR_play_voice_type voice_type;
    uint32_t idle_timeout;
    uint8_t play_volumn;
}ASR_config_t;

/***************** wifi info *********************/
#define WIFI_SSID_MAX_LENGTH     32
#define WIFI_PWD_MAX_LENGTH     64

/* encry type */
typedef enum{
    WIFI_ENC_TYPE_NONE     = 0x00,
    WIFI_ENC_TYPE_WEP      = 0x01,
    WIFI_ENC_TYPE_WPA    = 0x02,
    WIFI_ENC_TYPE_INVALID     = 0xff,
}WIFI_ENC_TYPE;

//AC board wifi status
typedef enum{
    WIFI_STATUS_DISCONNECT     = 0x00,
    WIFI_STATUS_CONNECTED      = 0x01,
    WIFI_STATUS_NOCONFIG    = 0x02,
    WIFI_STATUS_TYPE_INVALID = 0xff,
}WIFI_STATUS;

typedef enum{
    WIFI_MODE_MIN           = 0x00,
    WIFI_MODE_IDLE          = 0x00,
    WIFI_MODE_STA           = 0x01,
    WIFI_MODE_SET_CONFIGS   = 0x02,
    WIFI_MODE_AP            = 0x03,
    WIFI_MODE_SNIFFER       = 0x04,
    WIFI_MODE_AP_STA        = 0x05,
    WIFI_MODE_AP_MAX        = 0x05,
}WIFI_MODE;

typedef enum{
    ROUTER_STATUS_CONNECTED     = 0x00,
    ROUTER_STATUS_DISCONNECTED  = 0x01,
    ROUTER_STATUS_CONNECTING    = 0x02,
    ROUTER_STATUS_PWD_ERROR     = 0x03,
    ROUTER_STATUS_NO_ROUTER     = 0x04,
    ROUTER_STATUS_GETIP_FAIL    = 0x05,
    ROUTER_STATUS_NOT_STABLE    = 0x06,
    ROUTER_STATUS_WIFI_FAULT    = 0xF1,
    ROUTER_STATUS_SN_ABNORMAL   = 0xFF,
}ROUTER_STATUS;
/************************************************/

typedef struct{
    uint8_t ssid[WIFI_SSID_MAX_LENGTH + 1];//+ '\0'
    uint8_t ssid_len;
    uint8_t pwd[WIFI_PWD_MAX_LENGTH + 1];//+ '\0'
    uint8_t pwd_len;
    WIFI_ENC_TYPE encry;
    WIFI_STATUS ac_board_wifi_status;
    WIFI_STATUS voice_board_wifi_status;
    WIFI_MODE   ac_board_wifi_mode;
}wifi_request_info_t;

typedef struct{
    uint16_t running_version;
    uint16_t ota_version;
}voice_version_info_t;

typedef struct{
    /*
    * parameters
    */
    uint8_t onoff_status;
    //uint8_t factory_status;
    uint8_t volume;
    uint8_t poweron_play_long_onoff;
    float setT;
    float indoorT;
    float outdoorT;
    uint8_t wind_speed;

    ota_info_t ota_info;
    voice_version_info_t voice_version_info;
    ASR_AC_MODE mode;
    ASR_AC_WIND wind;
    uint8_t wind_play_type;
    ASR_AC_SWING swing;
    ASR_AC_TIMER_FUNC timer_status;
    ASR_AC_STRONG_MODE strong_mode;
    ASR_AC_CONFORT_ECO confort_eco;

    ASR_AC_SCREEN_DISPLAY screen_display;
    ASR_AC_AUX_ELECTRIC_HEAT_MODE aux_electric_heat_mode;
    ASR_AC_SELF_CLEAN self_clean;
    ASR_AC_ASR_TYPE asr_type;
    ASR_AC_BLOW blow;
    ASR_AC_ANTI_BLOW anti_blow;
    ASR_AC_ANTI_COLD anti_cold;
    ASR_SET_FUNC_TYPE local_dev_set_asr;
    ASR_SET_FUNC_TYPE local_dev_set_play;
    ASR_SET_FUNC_TYPE eii_dev_set_asr;
    ASR_SET_FUNC_TYPE eii_dev_set_play;

    wifi_request_info_t wifi_info;
    ASR_play_voice_type voice_type;
    //ASR_wakeup_info_t wakeup_info;

    uint8_t play;
    ASR_AC_STATUS err;
}AC_t;

typedef struct{
    uint8_t onoff;
    float minT;
    float maxT;
}ASR_AC_config_t;

static ASR_AC_config_t m_ac_config =
{
    .onoff = 0,
    .minT = 17,
    .maxT = 30
};

AC_t m_ac;
AC_t* device_ac_get(void);
ASR_config_t* const device_asr_get_config(void);

int send_heartbeat(void);

void cmd_all_voice_reply_function(char *buf, uint16_t asr_id, char play_enable);
int cmd_heartbeat(cmd_uart_msg_t* UART);
int cmd_wakeup(cmd_uart_msg_t* UART);
int cmd_on(cmd_uart_msg_t* UART);
int cmd_set_wind_speed(cmd_uart_msg_t* UART);
int cmd_set_temp(cmd_uart_msg_t* UART);
int cmd_set_mode(cmd_uart_msg_t* UART);
int cmd_set_swing(cmd_uart_msg_t* UART);
int cmd_set_blow(cmd_uart_msg_t* UART);
int cmd_set_func(cmd_uart_msg_t* UART);
int cmd_set_factory(cmd_uart_msg_t* UART);
int cmd_timer_func(cmd_uart_msg_t* UART);
int cmd_set_volume(cmd_uart_msg_t* UART);
int cmd_set_wifi(cmd_uart_msg_t* UART);
int cmd_passthrough(cmd_uart_msg_t* UART);
int cmd_query_weather(cmd_uart_msg_t* UART);

int cmd_board_startup(cmd_uart_msg_t* UART);
int cmd_board_beep_notice(cmd_uart_msg_t* UART);
int cmd_board_factory(cmd_uart_msg_t* UART);
int cmd_board_wifi_status(cmd_uart_msg_t* UART);
int cmd_board_set_configs(cmd_uart_msg_t* UART);
int cmd_board_passthrough(cmd_uart_msg_t* UART);

#endif /* __DEVICE_ASR_UART_RELEASE_H__ end */





