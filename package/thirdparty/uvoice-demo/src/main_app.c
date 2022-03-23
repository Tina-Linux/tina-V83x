#include "main_app.h"

void eii_playback_test(int id){
    int asr_param = -1;
    asr_param = id;
    LOCK_RING_BUFFER();
    send_to_ring_buffer(&asr_param, 1);
    UNLOCK_RING_BUFFER();
}

int device_asr_cmd_process(ASR_FRAME_CMD_ID asr_cmd_id, asr_mgr_t p_mgr)
{
    int asr_param = 0;
    ASR_AC_STATUS status = ASR_AC_ERROR_NONE;
    ASR_UART_MSG_TYPE asr_msg_type;
    LOGT("device_ac_get()->local_dev_set_asr = %#x\n", device_ac_get()->local_dev_set_asr);
    LOGT("device_ac_get()->local_dev_set_play = %#x\n", device_ac_get()->local_dev_set_play);
    LOGI("recv asr_cmd_id = %d\n", asr_cmd_id);
    //switch(asr_cmd_id) {
    if((asr_cmd_id >= LOCAL_ASR_ID_MIN) && (asr_cmd_id <= LOCAL_ASR_ID_MAX)) {
        if (device_ac_get()->local_dev_set_asr) {
            switch(asr_cmd_id) {
            /*wake up*/
            case ASR_UART_CMD_WAKE_UP1: //小美小美
            case ASR_UART_CMD_WAKE_UP2: //小美同学
				asr_param = (random_value_in_range(1, 2)
					+ random_value_in_range(0, 2)
					+ random_value_in_range(0, 2));
				if (asr_param != 1)
					asr_param += 294;
				LOCK_RING_BUFFER();
				send_to_ring_buffer(&asr_param, 1);
				UNLOCK_RING_BUFFER();
				asr_msg_type = ASR_UART_MSG_TYPE_CMD;
				asr_param = ASR_UART_MSG_WAKE_UP;
                break;

            /*power on*/
            case ASR_UART_CMD_ON1: //空调开机
            case ASR_UART_CMD_ON2: //开启空调
            case ASR_UART_CMD_ON3: //打开空调
            case ASR_UART_CMD_ON4: //请开机
            case ASR_UART_CMD_ON5: //开机
                asr_msg_type = ASR_UART_MSG_TYPE_CMD;
                asr_param = ASR_UART_MSG_ON;
                break;

            /*power off*/
            case ASR_UART_CMD_OFF1: //空调关机
            case ASR_UART_CMD_OFF2: //关闭空调
            case ASR_UART_CMD_OFF3: //关掉空调
            case ASR_UART_CMD_OFF4: //请关机
            case ASR_UART_CMD_OFF5: //关机
                asr_msg_type = ASR_UART_MSG_TYPE_CMD;
                asr_param = ASR_UART_MSG_OFF;
                break;

            /*set wind mode*/
            case ASR_UART_CMD_WIND_SLOW: //最小风
                asr_msg_type = ASR_UART_MSG_TYPE_SET_WIND;
                asr_param = ASR_AC_WIND_SLOW;
                break;
            case ASR_UART_CMD_WIND_MID: //中等风
                asr_msg_type = ASR_UART_MSG_TYPE_SET_WIND;
                asr_param = ASR_AC_WIND_MID;
                break;
            case ASR_UART_CMD_WIND_HIGH: //最大风
                asr_msg_type = ASR_UART_MSG_TYPE_SET_WIND;
                asr_param = ASR_AC_WIND_HIGH;
                break;
            case ASR_UART_CMD_WIND_AUTO: //自动风
                asr_msg_type = ASR_UART_MSG_TYPE_SET_WIND;
                asr_param = ASR_AC_WIND_AUTO;
                break;

            /*set wind*/
            case ASR_UART_CMD_WIND_DEC_SPEED1: //减小风速
            case ASR_UART_CMD_WIND_DEC_SPEED2: //风速小点
            case ASR_UART_CMD_WIND_DEC_SPEED3: //调小风速
                asr_msg_type = ASR_UART_MSG_TYPE_CMD;
                asr_param = ASR_AC_WIND_DEC;
                break;
            case ASR_UART_CMD_WIND_INC_SPEED1: //增大风速
            case ASR_UART_CMD_WIND_INC_SPEED2: //风速大点
            case ASR_UART_CMD_WIND_INC_SPEED3: //调大风速
                asr_msg_type = ASR_UART_MSG_TYPE_CMD;
                asr_param = ASR_AC_WIND_INC;
                break;

            /*set temperature*/
            case ASR_UART_CMD_SET_TEMP_17: //十七度
            case ASR_UART_CMD_SET_TEMP_18: //十八度
            case ASR_UART_CMD_SET_TEMP_19: //十九度
            case ASR_UART_CMD_SET_TEMP_20: //二十度
            case ASR_UART_CMD_SET_TEMP_21: //二十一度
            case ASR_UART_CMD_SET_TEMP_22: //二十二度
            case ASR_UART_CMD_SET_TEMP_23: //二十三度
            case ASR_UART_CMD_SET_TEMP_24: //二十四度
            case ASR_UART_CMD_SET_TEMP_25: //二十五度
            case ASR_UART_CMD_SET_TEMP_26: //二十六度
            case ASR_UART_CMD_SET_TEMP_27: //二十七度
            case ASR_UART_CMD_SET_TEMP_28: //二十八度
            case ASR_UART_CMD_SET_TEMP_29: //二十九度
            case ASR_UART_CMD_SET_TEMP_30: //三十度
                asr_msg_type = ASR_UART_MSG_TYPE_SET_T;
                asr_param = (int)asr_cmd_id - 6;
                break;
            case ASR_UART_CMD_INC_T_BY_1_a: //调高1度
            case ASR_UART_CMD_INC_T_BY_1_b: //有点冷
                asr_msg_type = ASR_UART_MSG_TYPE_CMD;
                asr_param = ASR_UART_MSG_INC_T_BY_1;
                break;
            case ASR_UART_CMD_INC_T_BY_3_0: //太冷了
                //printf("%s,%d,m_ac.setT:%f, m_ac_config.maxT:%f",__func__, __LINE__, m_ac.setT, m_ac_config.maxT);
                //if (((m_ac.setT / 100) + 3) > m_ac_config.maxT) {
                    //asr_msg_type = ASR_UART_MSG_TYPE_SET_T;
                    //asr_param = 30;
                //} else {
                    asr_msg_type = ASR_UART_MSG_TYPE_CMD;
                    asr_param = ASR_UART_MSG_INC_T_BY_3;
                //}
                break;
            case ASR_UART_CMD_DEC_T_BY_1_a:    //调低1度
            case ASR_UART_CMD_DEC_T_BY_1_b:    //有点热
                asr_msg_type = ASR_UART_MSG_TYPE_CMD;
                asr_param = ASR_UART_MSG_DEC_T_BY_1;
                break;
            case ASR_UART_CMD_DEC_T_BY_3_0:    //太热了
                //printf("%s,%d,m_ac.setT:%f, m_ac_config.minT:%f",__func__, __LINE__, m_ac.setT, m_ac_config.minT);
                //if (((m_ac.setT / 100) - 3) < m_ac_config.minT) {
                //    asr_msg_type = ASR_UART_MSG_TYPE_SET_T;
                //    asr_param = 17;
                //} else {
                    asr_msg_type = ASR_UART_MSG_TYPE_CMD;
                    asr_param = ASR_UART_MSG_DEC_T_BY_3;
                //}
                break;
            case ASR_UART_CMD_INC_T_BY_0_5: //调高0.5度
                asr_msg_type = ASR_UART_MSG_TYPE_CMD;
                asr_param = ASR_UART_MSG_INC_HALFDEGREE_TEMP;
                break;
            case ASR_UART_CMD_DEC_T_BY_0_5: //调低0.5度
                asr_msg_type = ASR_UART_MSG_TYPE_CMD;
                asr_param = ASR_UART_MSG_DEC_HALFDEGREE_TEMP;
                break;

            /*set mode*/
            case ASR_UART_CMD_SET_MODE_AUTO_a: //自动模式
            case ASR_UART_CMD_SET_MODE_AUTO_b: //打开自动模式
                asr_msg_type = ASR_UART_MSG_TYPE_SET_MODE;
                asr_param = ASR_AC_MODE_AUTO;
                break;
            case ASR_UART_CMD_SET_MODE_COOL_a: //制冷模式
            case ASR_UART_CMD_SET_MODE_COOL_b: //打开制冷模式
            case ASR_UART_CMD_SET_MODE_COOL_c: //打开制冷
                asr_msg_type = ASR_UART_MSG_TYPE_SET_MODE;
                asr_param = ASR_AC_MODE_COOL;
                break;
            case ASR_UART_CMD_SET_MODE_HEAT_a: //制热模式
            case ASR_UART_CMD_SET_MODE_HEAT_b: //打开制热模式
            case ASR_UART_CMD_SET_MODE_HEAT_c: //打开制热
                asr_msg_type = ASR_UART_MSG_TYPE_SET_MODE;
                asr_param = ASR_AC_MODE_HEAT;
                break;
            case ASR_UART_CMD_SET_MODE_WIND_a: //送风模式
            case ASR_UART_CMD_SET_MODE_WIND_b: //打开送风模式
            case ASR_UART_CMD_SET_MODE_WIND_c: //打开送风
                asr_msg_type = ASR_UART_MSG_TYPE_SET_MODE;
                asr_param = ASR_AC_MODE_WIND;
                break;
            case ASR_UART_CMD_SET_MODE_DRY_a:  //抽湿模式
            case ASR_UART_CMD_SET_MODE_DRY_b:  //打开抽湿模式
            case ASR_UART_CMD_SET_MODE_DRY_c:  //打开抽湿
            case ASR_UART_CMD_SET_MODE_DRY_d:  //除湿模式
            case ASR_UART_CMD_SET_MODE_DRY_e:  //打开除湿模式
            case ASR_UART_CMD_SET_MODE_DRY_f:  //打开除湿
                asr_msg_type = ASR_UART_MSG_TYPE_SET_MODE;
                asr_param = ASR_AC_MODE_DRY;
                break;

            /*set wind swing*/
            case ASR_UART_CMD_SWING_UP_DOWN_ON_a: //上下摆风
            case ASR_UART_CMD_SWING_UP_DOWN_ON_b: //打开上下摆风
            case ASR_UART_CMD_SWING_UP_DOWN_ON_c: //上下风
                asr_msg_type = ASR_UART_MSG_TYPE_CMD;
                asr_param = ASR_UART_MSG_SWING_UP_DOWN_ON;
                break;
            case ASR_UART_CMD_SWING_UP_DOWN_OFF_a: //关闭上下摆风
            case ASR_UART_CMD_SWING_UP_DOWN_OFF_b: //关掉上下摆风
                asr_msg_type = ASR_UART_MSG_TYPE_CMD;
                asr_param = ASR_UART_MSG_SWING_UP_DOWN_OFF;
                break;
            case ASR_UART_CMD_SWING_LEFT_RIGHT_ON_a: //左右摆风
            case ASR_UART_CMD_SWING_LEFT_RIGHT_ON_b: //打开左右摆风
            case ASR_UART_CMD_SWING_LEFT_RIGHT_ON_c: //左右风
                asr_msg_type = ASR_UART_MSG_TYPE_CMD;
                asr_param = ASR_UART_MSG_SWING_LEFT_RIGHT_ON;
                break;
            case ASR_UART_CMD_SWING_LEFT_RIGHT_OFF_a: //关闭左右摆风
            case ASR_UART_CMD_SWING_LEFT_RIGHT_OFF_b: //关掉左右摆风
                asr_msg_type = ASR_UART_MSG_TYPE_CMD;
                asr_param = ASR_UART_MSG_SWING_LEFT_RIGHT_OFF;
                break;

	    /*unuse*/
            case ASR_UART_CMD_BLOW_LEFT1: //风向左吹
            //case ASR_UART_CMD_BLOW_LEFT2: //风往左吹
                asr_msg_type = ASR_UART_MSG_TYPE_CMD;
                asr_param = ASR_UART_MSG_BLOW_LEFT;
                break;
            case ASR_UART_CMD_BLOW_RIGHT1: //风向右吹
            //case ASR_UART_CMD_BLOW_RIGHT2: //风往右吹
                asr_msg_type = ASR_UART_MSG_TYPE_CMD;
                asr_param = ASR_UART_MSG_BLOW_RIGHT;
                break;
            case ASR_UART_CMD_BLOW_UP1: //风向上吹
            //case ASR_UART_CMD_BLOW_UP2: //风往上吹
                asr_msg_type = ASR_UART_MSG_TYPE_CMD;
                asr_param = ASR_UART_MSG_BLOW_UP;
                break;
            case ASR_UART_CMD_BLOW_DOWN1: //风向下吹
            //case ASR_UART_CMD_BLOW_DOWN2: //风往下吹
                asr_msg_type = ASR_UART_MSG_TYPE_CMD;
                asr_param = ASR_UART_MSG_BLOW_DOWN;
                break;

            /*timer on*/
            case ASR_UART_CMD_TIMER_30M_ON:
                asr_msg_type = ASR_UART_MSG_TYPE_SET_TIMER;
                asr_param = ASR_UART_MSG_TIMER_30M_ON;
                break;
            case ASR_UART_CMD_TIMER_1H_ON:
                asr_msg_type = ASR_UART_MSG_TYPE_SET_TIMER;
                asr_param = ASR_UART_MSG_TIMER_1H_ON;
                break;
            case ASR_UART_CMD_TIMER_2H_ON:
                asr_msg_type = ASR_UART_MSG_TYPE_SET_TIMER;
                asr_param = ASR_UART_MSG_TIMER_2H_ON;
                break;
            case ASR_UART_CMD_TIMER_3H_ON:
                asr_msg_type = ASR_UART_MSG_TYPE_SET_TIMER;
                asr_param = ASR_UART_MSG_TIMER_3H_ON;
                break;
            case ASR_UART_CMD_TIMER_4H_ON:
                asr_msg_type = ASR_UART_MSG_TYPE_SET_TIMER;
                asr_param = ASR_UART_MSG_TIMER_4H_ON;
                break;
            case ASR_UART_CMD_TIMER_5H_ON:
                asr_msg_type = ASR_UART_MSG_TYPE_SET_TIMER;
                asr_param = ASR_UART_MSG_TIMER_5H_ON;
                break;
            case ASR_UART_CMD_TIMER_6H_ON:
                asr_msg_type = ASR_UART_MSG_TYPE_SET_TIMER;
                asr_param = ASR_UART_MSG_TIMER_6H_ON;
                break;
            case ASR_UART_CMD_TIMER_7H_ON:
                asr_msg_type = ASR_UART_MSG_TYPE_SET_TIMER;
                asr_param = ASR_UART_MSG_TIMER_7H_ON;
                break;
            case ASR_UART_CMD_TIMER_8H_ON:
                asr_msg_type = ASR_UART_MSG_TYPE_SET_TIMER;
                asr_param = ASR_UART_MSG_TIMER_8H_ON;
                break;

            /*timer off*/
            case ASR_UART_CMD_TIMER_30M_OFF:
                asr_msg_type = ASR_UART_MSG_TYPE_SET_TIMER;
                asr_param = ASR_UART_MSG_TIMER_30M_OFF;
                break;
            case ASR_UART_CMD_TIMER_1H_OFF:
                asr_msg_type = ASR_UART_MSG_TYPE_SET_TIMER;
                asr_param = ASR_UART_MSG_TIMER_1H_OFF;
                break;
            case ASR_UART_CMD_TIMER_2H_OFF:
                asr_msg_type = ASR_UART_MSG_TYPE_SET_TIMER;
                asr_param = ASR_UART_MSG_TIMER_2H_OFF;
                break;
            case ASR_UART_CMD_TIMER_3H_OFF:
                asr_msg_type = ASR_UART_MSG_TYPE_SET_TIMER;
                asr_param = ASR_UART_MSG_TIMER_3H_OFF;
                break;
            case ASR_UART_CMD_TIMER_4H_OFF:
                asr_msg_type = ASR_UART_MSG_TYPE_SET_TIMER;
                asr_param = ASR_UART_MSG_TIMER_4H_OFF;
                break;
            case ASR_UART_CMD_TIMER_5H_OFF:
                asr_msg_type = ASR_UART_MSG_TYPE_SET_TIMER;
                asr_param = ASR_UART_MSG_TIMER_5H_OFF;
                break;
            case ASR_UART_CMD_TIMER_6H_OFF:
                asr_msg_type = ASR_UART_MSG_TYPE_SET_TIMER;
                asr_param = ASR_UART_MSG_TIMER_6H_OFF;
                break;
            case ASR_UART_CMD_TIMER_7H_OFF:
                asr_msg_type = ASR_UART_MSG_TYPE_SET_TIMER;
                asr_param = ASR_UART_MSG_TIMER_7H_OFF;
                break;
            case ASR_UART_CMD_TIMER_8H_OFF:
                asr_msg_type = ASR_UART_MSG_TYPE_SET_TIMER;
                asr_param = ASR_UART_MSG_TIMER_8H_OFF;
                break;

            case ASR_UART_CMD_TIMER_CANCEL:
                asr_msg_type = ASR_UART_MSG_TYPE_SET_TIMER;
                asr_param = ASR_UART_MSG_TIMER_DISABLE;
                break;

            /*comfort and eco*/
            case ASR_UART_CMD_COMFORT_ECO_ON: //打开舒省
                asr_msg_type = ASR_UART_MSG_TYPE_SET_FUNC;
                asr_param = ASR_UART_MSG_COMFORT_ECO_ON;
                break;
            case ASR_UART_CMD_COMFORT_ECO_OFF_a: //关闭舒省
            case ASR_UART_CMD_COMFORT_ECO_OFF_b: //关掉舒省
                asr_msg_type = ASR_UART_MSG_TYPE_SET_FUNC;
                asr_param = ASR_UART_MSG_COMFORT_ECO_OFF;
                break;

            /*self-clean*/
            case ASR_UART_CMD_SELF_CLEAN_ON_a: //打开自清洁
            case ASR_UART_CMD_SELF_CLEAN_ON_b: //自清洁
                asr_msg_type = ASR_UART_MSG_TYPE_SET_FUNC;
                asr_param = ASR_UART_MSG_SELF_CLEAN_ON;
                break;
            case ASR_UART_CMD_SELF_CLEAN_OFF_a: //关闭自清洁
            case ASR_UART_CMD_SELF_CLEAN_OFF_b: //关掉自清洁
                asr_msg_type = ASR_UART_MSG_TYPE_SET_FUNC;
                asr_param = ASR_UART_MSG_SELF_CLEAN_OFF;
                break;

            /*auxiliary-electric-heating*/
            case ASR_UART_CMD_AUXILIARY_ELECTRIC_HEATING_ON_a: //打开电辅热
            case ASR_UART_CMD_AUXILIARY_ELECTRIC_HEATING_ON_b: //电辅热
                asr_msg_type = ASR_UART_MSG_TYPE_SET_FUNC;
                asr_param = ASR_UART_MSG_AUXILIARY_ELECTRIC_HEATING_ON;
                break;
            case ASR_UART_CMD_AUXILIARY_ELECTRIC_HEATING_OFF_a: //关闭电辅热
            case ASR_UART_CMD_AUXILIARY_ELECTRIC_HEATING_OFF_b: //关掉电辅热
                asr_msg_type = ASR_UART_MSG_TYPE_SET_FUNC;
                asr_param = ASR_UART_MSG_AUXILIARY_ELECTRIC_HEATING_OFF;
                break;

            /*anti-blow*/
            case ASR_UART_CMD_ANTI_BLOW_ON_a: //打开防直吹
            case ASR_UART_CMD_ANTI_BLOW_ON_b: //防直吹
                asr_msg_type = ASR_UART_MSG_TYPE_SET_FUNC;
                asr_param = ASR_UART_MSG_ANTI_BLOW_ON;
                break;
            case ASR_UART_CMD_ANTI_BLOW_OFF_a: //关闭防直吹
            case ASR_UART_CMD_ANTI_BLOW_OFF_b: //关掉防直吹
                asr_msg_type = ASR_UART_MSG_TYPE_SET_FUNC;
                asr_param = ASR_UART_MSG_ANTI_BLOW_OFF;
                break;

            /*screen-display*/
            case ASR_UART_CMD_SCREEN_DISPLAY_ON: //打开屏显
                asr_msg_type = ASR_UART_MSG_TYPE_SET_FUNC;
                asr_param = ASR_UART_MSG_SCREEN_DISPLAY_ON;
                break;
            case ASR_UART_CMD_SCREEN_DISPLAY_OFF_a: //关闭屏显
            case ASR_UART_CMD_SCREEN_DISPLAY_OFF_b: //关掉屏显
                asr_msg_type = ASR_UART_MSG_TYPE_SET_FUNC;
                asr_param = ASR_UART_MSG_SCREEN_DISPLAY_OFF;
                break;

            /*anti-cold*/
            //case ASR_UART_CMD_ANTI_COLD_ON_a: //打开防过冷
            //case ASR_UART_CMD_ANTI_COLD_ON_b: //防过冷
            case ASR_UART_CMD_SMART_CTL_ON1: //打开智控温
            case ASR_UART_CMD_SMART_CTL_ON2: //智控温
                asr_msg_type = ASR_UART_MSG_TYPE_SET_FUNC;
                asr_param = ASR_UART_MSG_ANTI_COLD_ON;
                break;
            //case ASR_UART_CMD_ANTI_COLD_OFF_a: //关闭防过冷
            //case ASR_UART_CMD_ANTI_COLD_OFF_b: //关掉防过冷
            case ASR_UART_CMD_SMART_CTL_OFF1: //关闭智控温
            case ASR_UART_CMD_SMART_CTL_OFF2: //关掉智控温
                asr_msg_type = ASR_UART_MSG_TYPE_SET_FUNC;
                asr_param = ASR_UART_MSG_ANTI_COLD_OFF;
                break;

            /*strong mode*/
            case ASR_UART_CMD_STRONG_MODE_ON: //打开强劲
                asr_msg_type = ASR_UART_MSG_TYPE_SET_FUNC;
                asr_param = ASR_UART_MSG_STRONG_MODE_ON;
                break;
            case ASR_UART_CMD_STRONG_MODE_OFF_a: //关闭强劲
            case ASR_UART_CMD_STRONG_MODE_OFF_b: //关掉强劲
                asr_msg_type = ASR_UART_MSG_TYPE_SET_FUNC;
                asr_param = ASR_UART_MSG_STRONG_MODE_OFF;
                break;

            /*set volume*/
            case ASR_UART_CMD_SET_VOLUME_MAX: //最大音量
                asr_msg_type = ASR_UART_MSG_TYPE_SET_VOLUME;
                asr_param = ASR_UART_MSG_SET_VOLUME_MAX;
                break;
            case ASR_UART_CMD_SET_VOLUME_MIN: //最小音量
                asr_msg_type = ASR_UART_MSG_TYPE_SET_VOLUME;
                asr_param = ASR_UART_MSG_SET_VOLUME_MIN;
                break;
            case ASR_UART_CMD_SET_VOLUME_INC1: //增大音量
            case ASR_UART_CMD_SET_VOLUME_INC2: //调大音量
                asr_msg_type = ASR_UART_MSG_TYPE_SET_VOLUME;
                asr_param = ASR_UART_MSG_SET_VOLUME_INC;
                break;
            case ASR_UART_CMD_SET_VOLUME_DEC1: //减小音量
            case ASR_UART_CMD_SET_VOLUME_DEC2: //调小音量
                asr_msg_type = ASR_UART_MSG_TYPE_SET_VOLUME;
                asr_param = ASR_UART_MSG_SET_VOLUME_DEC;
                break;

            /*query weather*/
            case ASR_UART_CMD_QUERY_WEATHER_a: //天气查询
            case ASR_UART_CMD_QUERY_WEATHER_b: //查询天气
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                //asr_msg_type = ASR_UART_MSG_TYPE_QUERY_WEATHER;
                asr_param = ASR_UART_MSG_QUERY_WEATHER;
                break;

            /*wifi*/
            case ASR_UART_CMD_START_WIFI_SNIFFER: //空调开始配网
            case ASR_UART_CMD_RESET_WIFI_CONF:    //空调重新配网
                asr_msg_type = ASR_UART_MSG_TYPE_CMD;
                asr_param = ASR_UART_MSG_SET_WIFI;
                break;
            default:
                break;
            }
        } else {
            LOGW("local dev asr disable");
            return -300;
        }
    }
 else if((asr_cmd_id >= EII_ASR_ID_MIN) && (asr_cmd_id <= EII_ASR_ID_MAX)) {
        if (device_ac_get()->eii_dev_set_asr) {
            switch(asr_cmd_id) {
            /*wifi passthrough*/
            case ASR_UART_CMD_LIVING_ROOM_FAN_ON:    //打开客厅风扇
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_LIVING_ROOM_FAN_ON;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(358 - 42);
                #endif
                break;
            case ASR_UART_CMD_LIVING_ROOM_FAN_OFF: //关闭客厅风扇
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_LIVING_ROOM_FAN_OFF;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(360 - 42);
                #endif
                break;
            case ASR_UART_CMD_BED_ROOM_FAN_ON:    //打开卧室风扇
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_BED_ROOM_FAN_ON;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(359 - 42);
                #endif
                break;
            case ASR_UART_CMD_BED_ROOM_FAN_OFF: //关闭卧室风扇
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_BED_ROOM_FAN_OFF;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(361 - 42);
                #endif
                break;
            case ASR_UART_CMD_LIVING_ROOM_FAN_OSC_ON: //客厅风扇摇头
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_LIVING_ROOM_FAN_OSC_ON;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(362 - 42);
                #endif
                break;
            case ASR_UART_CMD_BED_ROOM_FAN_OSC_ON: //卧室风扇摇头
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_BED_ROOM_FAN_OSC_ON;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(363 - 42);
                #endif
                break;
            case ASR_UART_CMD_LIVING_ROOM_FAN_OSC_OFF://客厅风扇停止摇头
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_LIVING_ROOM_FAN_OSC_OFF;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(364 - 42);
                #endif
                break;
            case ASR_UART_CMD_BED_ROOM_FAN_OSC_OFF://卧室风扇停止摇头
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_BED_ROOM_FAN_OSC_OFF;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(365 - 42);
                #endif
                break;
            case ASR_UART_CMD_LIVING_ROOM_FAN_INC:    //客厅风扇调大一档
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_LIVING_ROOM_FAN_INC;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(366 - 42);
                #endif
                break;
            case ASR_UART_CMD_BED_ROOM_FAN_INC:     //卧室风扇调大一档
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_BED_ROOM_FAN_INC;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(367 - 42);
                #endif
                break;
            case ASR_UART_CMD_LIVING_ROOM_FAN_DEC:    //客厅风扇调小一档
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_LIVING_ROOM_FAN_DEC;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(368 - 42);
                #endif
                break;
            case ASR_UART_CMD_BED_ROOM_FAN_DEC:     //卧室风扇调小一档
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_BED_ROOM_FAN_DEC;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(369 - 42);
                #endif
                break;
            case ASR_UART_CMD_LIVING_ROOM_FAN_MAX:    //客厅风扇调到最大
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_LIVING_ROOM_FAN_MAX;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(370 - 42);
                #endif
                break;
            case ASR_UART_CMD_BED_ROOM_FAN_MAX:     //卧室风扇调到最大
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_BED_ROOM_FAN_MAX;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(371 - 42);
                #endif
                break;
            case ASR_UART_CMD_LIVING_ROOM_FAN_MIN:    //客厅风扇调到最小
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_LIVING_ROOM_FAN_MIN;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(372 - 42);
                #endif
                break;
            case ASR_UART_CMD_BED_ROOM_FAN_MIN:     //卧室风扇调到最小
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_BED_ROOM_FAN_MIN;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(373 - 42);
                #endif
                break;

            case ASR_UART_CMD_LIVING_ROOM_HUMIDIFIER_ON: //打开客厅加湿器
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_LIVING_ROOM_HUMIDIFIER_ON;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(374 - 42);
                #endif
                break;
            case ASR_UART_CMD_BED_ROOM_HUMIDIFIER_ON: //打开卧室加湿器
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_BED_ROOM_HUMIDIFIER_ON;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(375 - 42);
                #endif
                break;
            case ASR_UART_CMD_LIVING_ROOM_HUMIDIFIER_OFF: //关闭客厅加湿器
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_LIVING_ROOM_HUMIDIFIER_OFF;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(376 - 42);
                #endif
                break;
            case ASR_UART_CMD_BED_ROOM_HUMIDIFIER_OFF: //关闭卧室加湿器
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_BED_ROOM_HUMIDIFIER_OFF;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(377 - 42);
                #endif
                break;
            case ASR_UART_CMD_LIVING_ROOM_AIR_CLEANER_ON: //打开客厅净化器
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_LIVING_ROOM_AIR_CLEANER_ON;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(378 - 42);
                #endif
                break;
            case ASR_UART_CMD_BED_ROOM_AIR_CLEANER_ON: //打开卧室净化器
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_BED_ROOM_AIR_CLEANER_ON;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(379 - 42);
                #endif
                break;
            case ASR_UART_CMD_LIVING_ROOM_AIR_CLEANER_OFF: //关闭客厅净化器
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_LIVING_ROOM_AIR_CLEANER_OFF;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(380 - 42);
                #endif
                break;
            case ASR_UART_CMD_BED_ROOM_AIR_CLEANER_OFF: //关闭卧室净化器
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_BED_ROOM_AIR_CLEANER_OFF;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(381 - 42);
                #endif
                break;
            case ASR_UART_CMD_SWEEP_ROBOT_CLEAN_ALL:    //扫地机器人全屋清扫
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_SWEEP_ROBOT_CLEAN_ALL;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(382 - 42);
                #endif
                break;
            case ASR_UART_CMD_SWEEP_ROBOT_CHARGING:     //扫地机器人充电
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_SWEEP_ROBOT_CHARGING;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(383 - 42);
                #endif
                break;
            case ASR_UART_CMD_SWEEP_ROBOT_CLEAN_PAUSE://扫地机器人暂停扫地
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_SWEEP_ROBOT_CLEAN_PAUSE;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(384 - 42);
                #endif
                break;
            case ASR_UART_CMD_BED_ROOM_AC_ON: //打开卧室空调
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_BED_ROOM_AC_ON;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(385 - 42);
                #endif
                break;
            case ASR_UART_CMD_BED_ROOM_AC_OFF: //关闭卧室空调
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_BED_ROOM_AC_OFF;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(386 - 42);
                #endif
                break;
            case ASR_UART_CMD_LIVING_ROOM_LAMP_ON: //客厅开灯
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_LIVING_ROOM_LAMP_ON;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(387 - 42);
                #endif
                break;
            case ASR_UART_CMD_LIVING_ROOM_LAMP_OFF: //客厅关灯
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_LIVING_ROOM_LAMP_OFF;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(389 - 42);
                #endif
                break;
            case ASR_UART_CMD_BED_ROOM_LAMP_ON: //卧室开灯
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_BED_ROOM_LAMP_ON;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(388 - 42);
                #endif
                break;
            case ASR_UART_CMD_BED_ROOM_LAMP_OFF: //卧室关灯
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_BED_ROOM_LAMP_OFF;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(390 - 42);
                #endif
                break;
            case ASR_UART_CMD_PEOPLE_IMBACK://我回来了
            case ASR_UART_CMD_PEOPLE_IMHOME://我到家了
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_BACK_HOME;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(391 - 42);
                    eii_playback_test(392 - 42);
                #endif
                break;
            case ASR_UART_CMD_PEOPLE_IGOTOWORK://我去上班了
            case ASR_UART_CMD_PEOPLE_IGOOUT://我出门了
                asr_msg_type = ASR_UART_MSG_TYPE_PASSTHROUGH;
                asr_param = ASR_UART_MSG_LEAVE_HOME;
                #ifdef EII_TEST_ENABLE
                    eii_playback_test(393 - 42);
                    eii_playback_test(394 - 42);
                #endif
                break;
            default:
            break;
            }
        } else {
            LOGI("EII dev asr disable");
            return -300;
        }
    }
 else {
        switch(asr_cmd_id) {
        /*start-up asr id*/
        case ASR_UART_CMD_STARTUP:
#if 10
            return 0;
#else
            LOCK_RING_BUFFER();
            asr_param = 0;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
#endif
        /*wakeup-timeout asr id*/
        case ASR_UART_CMD_WAKEUP_TIMEOUT:
#if 10
            LOCK_RING_BUFFER();
            asr_param = 355 - 42;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            //return 0;
#endif
            asr_msg_type = ASR_UART_MSG_TYPE_CMD;
            asr_param = ASR_UART_MSG_TIMEOUT_RECOG;
            break;

        /*factory asr id*/
        case ASR_UART_CMD_FACTORY:
        case ASR_UART_CMD_WAKE_UP1:
        case ASR_UART_CMD_WAKE_UP2:
            sem_post(&factory_sem);
            return 0;
            break;

        default:
            LOGW("Invalid ASR Command\n");
            return -300;
        }
    }
    status = device_asr_send_msg(asr_msg_type, asr_param);
    return status;
}

void *factory_test_thread(void *arg)
{
    struct timespec period;
    int ret = -1;
    int asr_param = 0;
    ASR_UART_MSG_TYPE asr_msg_type;
    int retry_times = FACTORY_TEST_RETRY_TIMES;
    int playback_times = FACTORY_TEST_PLAYBACK_TIMES;
    asr_mgr_t p_mgr;
    p_mgr = (struct asr_mgr_tag *)arg;
    sem_init(&factory_sem, 0, 0);
    sleep(1);

    while(1) {
        LOCK_RING_BUFFER();
        asr_param = 353 - 42;
        send_to_ring_buffer(&asr_param, 1);
        UNLOCK_RING_BUFFER();

        //long msecs = 0, add =0;
        clock_gettime(CLOCK_REALTIME, &period);
	//LOGI("1tv_sec = %ld, tv_nsec = %ld", period.tv_sec, period.tv_nsec);
	//msecs = 2 * 1000 * 1000 * 1000 + period.tv_nsec;
        //add = msecs / (1000 * 1000 * 1000);
        period.tv_sec += 2;//add;
        //period.tv_nsec = msecs % (1000 * 1000 * 1000);
        //period.tv_sec += 1;
	//LOGI("2tv_sec = %ld, tv_nsec = %ld", period.tv_sec, period.tv_nsec);

        ret = sem_timedwait(&factory_sem, &period);
        if (ret == 0) {
            LOGI("---------- 测试成功 --------------\n");
            do {
                LOCK_RING_BUFFER();
                asr_param = 281;
                send_to_ring_buffer(&asr_param, 1);
                UNLOCK_RING_BUFFER();
                playback_times--;
            } while(playback_times);
            asr_msg_type = ASR_UART_MSG_TYPE_FACTORY;
            asr_param = ASR_UART_MSG_FACTORY_TEST_PASS;
            device_asr_send_msg(asr_msg_type, asr_param);
            pthread_exit("测试完成：成功n");
        } else {
            LOGI("---------- 测试失败 --------------\n");
            retry_times--;
            if (retry_times <= 0) {
                do {
                    LOCK_RING_BUFFER();
                    asr_param = 280;
                    send_to_ring_buffer(&asr_param, 1);
                    UNLOCK_RING_BUFFER();
                    playback_times--;
                } while(playback_times);
/*
                LOCK_RING_BUFFER();
                asr_param = 280;
                send_to_ring_buffer(&asr_param, 1);
                UNLOCK_RING_BUFFER();
*/
                asr_msg_type = ASR_UART_MSG_TYPE_FACTORY;
                asr_param = ASR_UART_MSG_FACTORY_TEST_FAIL;
                device_asr_send_msg(asr_msg_type, asr_param);
                pthread_exit("测试完成：失败\n");
            }
        }
    }
}

#ifdef PLAYBACK_ENABLE
static void *voice_playback_thread(void *arg)
{
    int ret;
    uint16_t playback_id = 0xFFFF;
    char playback_name[128];
    asr_mgr_t p_mgr;
    p_mgr = (struct asr_mgr_tag *)arg;
    factoryThread = NULL;
    int32_t time_period = 0;

    while(1)
    {

	/*for wakeup test0*/
/*
	printf("\n10s start ... \n");
	sleep(10);
	printf("\n10s timeout ... \n");
	printf("\nenter suspend ... \n");
	system("echo mem > /sys/power/state");
*/
	//end

        if (eii_ctl_timeout_flag == 0x01) {
            gettimeofday(&timer_end,NULL);
            time_period = (timer_end.tv_sec * 1000 + timer_end.tv_usec / 1000) - \
		(timer_start.tv_sec * 1000 + timer_start.tv_usec / 1000);
            if (time_period >= 5000) {
                cmd_all_voice_reply_function("设备控制失败:超时", 172, m_ac.eii_dev_set_play);
                gettimeofday(&timer_start,NULL);
                eii_ctl_timeout_flag = 0x0;
            }
        }
        LOCK_RING_BUFFER();
        ret = recv_from_ring_buffer(&playback_id, 1);
        UNLOCK_RING_BUFFER();
        if (ret != 0) {
            usleep(10 * 1000);
            //usleep(100 * 1000);
            continue;
        } else {
            LOGD("recv playback id = %d", playback_id);

            if((playback_id >= 0) && (playback_id <= PLAYBACK_COUNTS)) {
                asr_mgr_set_play_flag(p_mgr, 1);
                get_music_name(playback_id, &playback_name);
                asr_mgr_playback(p_mgr, &playback_name);
            } else if (playback_id == 800) {
                if (factoryThread == NULL) {
                    int8_t mode = 1;
                    asr_mgr_change_run_mode(p_mgr, mode);
                    system("amixer cset name='head phone volume' 62");
                    usleep(100 * 1000);
                    ret = pthread_create(&factoryThread, NULL, factory_test_thread, NULL);
                    if (ret != 0)
                    {
                        LOGE("factoryThread create error [ret = %d]", ret);
                        return -1;
                    }
                }
            }
        }
    }
    pthread_exit("voice_playback_thread exit\n");
}
#endif

uint8_t asr_module_info[7] = {ASR_MODULE_CHIP_TYPE, ASR_MODULE_ASR_VENDOR, ASR_MODULE_HW_VERSION_HIGH, \
    ASR_MODULE_HW_VERSION_LOW, ASR_MODULE_SW_MAJOR_VERSION, ASR_MODULE_SW_MINOR_VERSION_1, ASR_MODULE_SW_MINOR_VERSION_2};

void asr_module_info_display(void)
{
    uint8_t i = 0;
    printf("\n\n*************** MIDEA ASR MODULE INFO ***************\n");
    printf("* Application build at %s %s         *\n",__DATE__, __TIME__);
    printf("* ASR version: ");
    for(i = 0; i < (sizeof(asr_module_info) / sizeof(uint8_t)); i++) {
        printf("%02x ", asr_module_info[i]);
    }
    printf("                *\n");
    printf("*****************************************************\n\n");

}

int main(int32_t argc, char** argv)
{
    int ret = -1;
    uint8_t asr_id = 0xff;

    asr_module_info_display();

#ifndef UVOICE_TEST
    device_asr_reader_init();
    device_asr_reader_start();
    //gs_msgid = msg_queue_init();
#else
    set_volume_level(4);
#endif
//    usleep(200 * 1000);
#ifdef ASR_ENABLE
    asr_mgr_t p_mgr;
    p_mgr = NULL;

    p_mgr = asr_mgr_create();
    if(p_mgr == NULL) {
        LOGE("asr_mgr_create failed.\n");
        return -1;
    }

    ret = asr_mgr_init(p_mgr);
    if(ret == -1) {
        LOGE("asr_mgr_init failed.\n");
        return -1;
    }
#else
    int count_id = 0;
#endif

    ret = pthread_mutex_init(&ring_buffer_lock, NULL);
    if (ret != 0) {
            LOGE("pthread_mutex_init ring_buffer_lock error !\n");
            return -1;
    }

#ifdef PLAYBACK_ENABLE
    ret = pthread_create(&voiceplaybackThread, NULL, voice_playback_thread, p_mgr);
    if (ret != 0)
    {
        LOGE("voice_playback_thread create error [ret = %d]\n", ret);
        return -1;
    }
#endif

    while(1) {
#ifdef ASR_ENABLE
        asr_id = asr_mgr_wait_asrid(p_mgr);
#else
        sleep(10);
        if ((count_id == 5) || (count_id == 6) || (count_id == 7)) {
            count_id = 8;
        } else if (count_id > 89) {
            count_id = 0;
        }
        asr_id = count_id;
        count_id++;
#endif

#ifdef UVOICE_TEST
        playback_test(asr_id);
#else
        ret = device_asr_cmd_process(asr_id, p_mgr);
#endif
    }
#ifdef ASR_ENABLE
    asr_mgr_release(&p_mgr);
#endif
    return 0;
}
