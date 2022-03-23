#include "demo_show.h"

static uint8_t wind_speed = 60;
static uint8_t wind_step  = 20;

static uint8_t temp = 25;
static uint8_t temp_step_0_5  = 0.5;
static uint8_t temp_step_1  = 1;
static uint8_t temp_step_3  = 3;
static uint8_t volume_step = 1;
static uint8_t volume_level = 4;

void set_volume_level(uint8_t level) {
    if ((level > 0) && (level <= 1)) {
        system("amixer cset name='LINEOUT volume' 11");
    } else if ((level > 1) && (level <= 2)) {
        system("amixer cset name='LINEOUT volume' 13");
    } else if ((level > 2) && (level <= 3)) {
        system("amixer cset name='LINEOUT volume' 15");
    } else if ((level > 3) && (level <= 4)) {
        system("amixer cset name='LINEOUT volume' 17");
    } else if ((level > 4) && (level <= 5)) {
        system("amixer cset name='LINEOUT volume' 19");
    }
}

void playback_test(ASR_FRAME_CMD_ID asr_cmd_id)
{
    uint16_t asr_param = 0xFFFF;

    switch(asr_cmd_id) {
        /*wake up*/
        case ASR_UART_CMD_WAKE_UP1: //小美小美
        case ASR_UART_CMD_WAKE_UP2: //小美同学
        //case ASR_UART_CMD_WAKE_UP3: //嗨，小优
            LOCK_RING_BUFFER();
            asr_param = random_value_in_range(1, 6);
            if (asr_param != 1) {
                asr_param += 294;
            }
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;

        /*power on*/
        case ASR_UART_CMD_ON1: //空调开机
        case ASR_UART_CMD_ON2: //开启空调
        case ASR_UART_CMD_ON3: //打开空调
        case ASR_UART_CMD_ON4: //请开机
        case ASR_UART_CMD_ON5: //开机
            LOCK_RING_BUFFER();
            asr_param = 3;
            send_to_ring_buffer(&asr_param, 1);

            asr_param = 43;
            send_to_ring_buffer(&asr_param, 1);

            asr_param = 12;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;

        /*power off*/
        case ASR_UART_CMD_OFF1:    //空调关机
        case ASR_UART_CMD_OFF2:    //关闭空调
        case ASR_UART_CMD_OFF3:    //关掉空调
        case ASR_UART_CMD_OFF4:    //请关机
        case ASR_UART_CMD_OFF5:    //关机
            LOCK_RING_BUFFER();
            asr_param = 4;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;

        /*set wind mode*/
        case ASR_UART_CMD_WIND_SLOW: //最小风
            LOCK_RING_BUFFER();
            asr_param = 166;
            send_to_ring_buffer(&asr_param, 1);

            asr_param = 9;
            //asr_param = 199;
            send_to_ring_buffer(&asr_param, 1);

            UNLOCK_RING_BUFFER();
            wind_speed = 1;
            break;
        case ASR_UART_CMD_WIND_MID: //中等风
            LOCK_RING_BUFFER();
            asr_param = 166;
            send_to_ring_buffer(&asr_param, 1);

            asr_param = 10;
            //asr_param = 202;
            send_to_ring_buffer(&asr_param, 1);

            UNLOCK_RING_BUFFER();
            wind_speed = 60;
            break;
        case ASR_UART_CMD_WIND_HIGH: //最大风
            LOCK_RING_BUFFER();
            asr_param = 166;
            send_to_ring_buffer(&asr_param, 1);

            asr_param = 11;
            //asr_param = 198;
            send_to_ring_buffer(&asr_param, 1);

            UNLOCK_RING_BUFFER();
            wind_speed = 100;
            break;
        case ASR_UART_CMD_WIND_AUTO: //自动风
            LOCK_RING_BUFFER();
            asr_param = 166;
            send_to_ring_buffer(&asr_param, 1);

            asr_param = 12;
            send_to_ring_buffer(&asr_param, 1);

            UNLOCK_RING_BUFFER();
            break;

        /*set wind*/
        case ASR_UART_CMD_WIND_DEC_SPEED1: //减小风速
        case ASR_UART_CMD_WIND_DEC_SPEED2: //调小风速
        case ASR_UART_CMD_WIND_DEC_SPEED3: //风速小点
            LOCK_RING_BUFFER();
            if (wind_speed == 1) {
                asr_param = 14;
                send_to_ring_buffer(&asr_param, 1);

                asr_param = 9;
                send_to_ring_buffer(&asr_param, 1);
            } else {
                if (wind_speed == 20) {
                    wind_speed = 1;
                } else {
                    wind_speed -= wind_step;
                }

                asr_param = 166;
                send_to_ring_buffer(&asr_param, 1);

                switch (wind_speed) {
                case 1:
                    asr_param = 199;
                    break;
                case 20:
                    asr_param = 200;
                    break;
                case 40:
                    asr_param = 201;
                    break;
                case 60:
                    asr_param = 202;
                    break;
                case 80:
                    asr_param = 203;
                    break;
                case 100:
                    asr_param = 198;
                    break;
                default:
                    break;
                }
                send_to_ring_buffer(&asr_param, 1);
            }
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_WIND_INC_SPEED1: //增大风速
        case ASR_UART_CMD_WIND_INC_SPEED2: //调大风速
        case ASR_UART_CMD_WIND_INC_SPEED3: //风速大点
            LOCK_RING_BUFFER();
            if (wind_speed == 100) {
                asr_param = 14;
                send_to_ring_buffer(&asr_param, 1);

                asr_param = 11;
                send_to_ring_buffer(&asr_param, 1);
            } else {
                if (wind_speed == 1) {
                    wind_speed = 20;
                } else {
                    wind_speed += wind_step;
                }

                asr_param = 166;
                send_to_ring_buffer(&asr_param, 1);

                switch (wind_speed) {
                case 1:
                    asr_param = 199;
                    break;
                case 20:
                    asr_param = 200;
                    break;
                case 40:
                    asr_param = 201;
                    break;
                case 60:
                    asr_param = 202;
                    break;
                case 80:
                    asr_param = 203;
                    break;
                case 100:
                    asr_param = 198;
                    break;
                default:
                    break;
                }
                send_to_ring_buffer(&asr_param, 1);
            }
            UNLOCK_RING_BUFFER();
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
            LOCK_RING_BUFFER();
            asr_param = 6;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = (int)asr_cmd_id + 11;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_INC_T_BY_1_a:    //调高1度
        case ASR_UART_CMD_INC_T_BY_1_b:    //有点冷
            LOCK_RING_BUFFER();
            if (temp >= 30) {
                temp = 30;
                asr_param = 14;
                send_to_ring_buffer(&asr_param, 1);

                asr_param = 131;
                send_to_ring_buffer(&asr_param, 1);
            } else {
                temp += temp_step_1;

                asr_param = 6;
                send_to_ring_buffer(&asr_param, 1);

                asr_param = temp + 17;
                send_to_ring_buffer(&asr_param, 1);
            }
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_INC_T_BY_3_0:    //太冷了
            LOCK_RING_BUFFER();
            if (temp >= 30) {
                asr_param = 14;
                send_to_ring_buffer(&asr_param, 1);

                asr_param = 131;
                send_to_ring_buffer(&asr_param, 1);
            } else {
                if (temp <= 27) {
                    temp += temp_step_3;
                } else {
                    temp = 30;
                }
                asr_param = 6;
                send_to_ring_buffer(&asr_param, 1);

                asr_param = temp + 17;
                send_to_ring_buffer(&asr_param, 1);
            }
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_DEC_T_BY_1_a:    //调低1度
        case ASR_UART_CMD_DEC_T_BY_1_b:    //有点热
            LOCK_RING_BUFFER();
            if (temp <= 17) {
                asr_param = 14;
                send_to_ring_buffer(&asr_param, 1);

                asr_param = 132;
                send_to_ring_buffer(&asr_param, 1);
            } else {
                temp -= temp_step_1;

                asr_param = 6;
                send_to_ring_buffer(&asr_param, 1);

                asr_param = temp + 17;
                send_to_ring_buffer(&asr_param, 1);
            }
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_DEC_T_BY_3_0: //太热了
            LOCK_RING_BUFFER();
            if (temp <= 17) {
                asr_param = 14;
                send_to_ring_buffer(&asr_param, 1);

                asr_param = 132;
                send_to_ring_buffer(&asr_param, 1);
            } else {
                if (temp >= 20) {
                    temp -= temp_step_3;
                } else {
                    temp = 17;
                }
                asr_param = 6;
                send_to_ring_buffer(&asr_param, 1);

                asr_param = temp + 17;
                send_to_ring_buffer(&asr_param, 1);
            }
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_INC_T_BY_0_5: //调高0.5度
            LOCK_RING_BUFFER();
            asr_param = 14;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 131;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_DEC_T_BY_0_5: //调低0.5度
            LOCK_RING_BUFFER();
            asr_param = 14;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 132;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;

        /*set mode*/
        case ASR_UART_CMD_SET_MODE_AUTO_a: //自动模式
        case ASR_UART_CMD_SET_MODE_AUTO_b: //打开自动模式
            LOCK_RING_BUFFER();
            asr_param = 6;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 133;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_SET_MODE_COOL_a: //制冷模式
        case ASR_UART_CMD_SET_MODE_COOL_b: //打开制冷模式
        case ASR_UART_CMD_SET_MODE_COOL_c: //打开制冷
            LOCK_RING_BUFFER();
            asr_param = 6;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 134;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_SET_MODE_HEAT_a: //制热模式
        case ASR_UART_CMD_SET_MODE_HEAT_b: //打开制热模式
        case ASR_UART_CMD_SET_MODE_HEAT_c: //打开制热
            LOCK_RING_BUFFER();
            asr_param = 6;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 135;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_SET_MODE_WIND_a: //送风模式
        case ASR_UART_CMD_SET_MODE_WIND_b: //打开送风模式
        case ASR_UART_CMD_SET_MODE_WIND_c: //打开送风
            LOCK_RING_BUFFER();
            asr_param = 6;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 136;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_SET_MODE_DRY_a:    //抽湿模式
        case ASR_UART_CMD_SET_MODE_DRY_b:    //打开抽湿模式
        case ASR_UART_CMD_SET_MODE_DRY_c:    //打开抽湿
        case ASR_UART_CMD_SET_MODE_DRY_d:    //除湿模式
        case ASR_UART_CMD_SET_MODE_DRY_e:    //打开除湿模式
        case ASR_UART_CMD_SET_MODE_DRY_f:    //打开除湿
            LOCK_RING_BUFFER();
            asr_param = 6;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 137;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;

        /*set wind swing*/
        case ASR_UART_CMD_SWING_UP_DOWN_ON_a: //打开上下摆风
        case ASR_UART_CMD_SWING_UP_DOWN_ON_b: //上下摆风
        case ASR_UART_CMD_SWING_UP_DOWN_ON_c: //上下风    add by v3
            LOCK_RING_BUFFER();
            asr_param = 8;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 139;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_SWING_UP_DOWN_OFF_a: //关闭上下摆风 modify by v3
        case ASR_UART_CMD_SWING_UP_DOWN_OFF_b: //关掉上下摆风 add by v3
        case ASR_UART_CMD_SWING_UP_DOWN_OFF_c: //关闭上下风 add by v3
        case ASR_UART_CMD_SWING_UP_DOWN_OFF_d: //关掉上下风 add by v3
            LOCK_RING_BUFFER();
            asr_param = 7;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 139;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_SWING_LEFT_RIGHT_ON_a: //打开左右摆风
        case ASR_UART_CMD_SWING_LEFT_RIGHT_ON_b: //左右摆风
        case ASR_UART_CMD_SWING_LEFT_RIGHT_ON_c: //左右风 add by v3
            LOCK_RING_BUFFER();
            asr_param = 8;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 140;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_SWING_LEFT_RIGHT_OFF_a: //关闭左右摆风 modify by v3
        case ASR_UART_CMD_SWING_LEFT_RIGHT_OFF_b: //关掉左右摆风 add by v3
        case ASR_UART_CMD_SWING_LEFT_RIGHT_OFF_c: //关闭左右风 add by v3
        case ASR_UART_CMD_SWING_LEFT_RIGHT_OFF_d: //关掉左右风 add by v3
            LOCK_RING_BUFFER();
            asr_param = 7;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 140;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_BLOW_LEFT1: //风向左吹
        //case ASR_UART_CMD_BLOW_LEFT2: //风往左吹
            LOCK_RING_BUFFER();
            asr_param = 158;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_BLOW_RIGHT1: //风向右吹
        //case ASR_UART_CMD_BLOW_RIGHT2: //风往右吹
            LOCK_RING_BUFFER();
            asr_param = 159;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_BLOW_UP1: //风向上吹
        //case ASR_UART_CMD_BLOW_UP2: //风往上吹
            LOCK_RING_BUFFER();
            asr_param = 160;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_BLOW_DOWN1: //风向下吹
        //case ASR_UART_CMD_BLOW_DOWN2: //风往下吹
            LOCK_RING_BUFFER();
            asr_param = 161;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;

        /*timer*/ //to do
        case ASR_UART_CMD_TIMER_30M_ON:
            LOCK_RING_BUFFER();
            asr_param = 143;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 144;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 141;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_TIMER_1H_ON:
            LOCK_RING_BUFFER();
            asr_param = 143;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 119;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 145;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 141;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_TIMER_2H_ON:
            LOCK_RING_BUFFER();
            asr_param = 143;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 120;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 145;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 141;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_TIMER_3H_ON:
            LOCK_RING_BUFFER();
            asr_param = 143;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 121;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 145;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 141;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_TIMER_4H_ON:
            LOCK_RING_BUFFER();
            asr_param = 143;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 122;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 145;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 141;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_TIMER_5H_ON:
            LOCK_RING_BUFFER();
            asr_param = 143;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 123;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 145;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 141;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_TIMER_6H_ON:
            LOCK_RING_BUFFER();
            asr_param = 143;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 124;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 145;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 141;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_TIMER_7H_ON:
            LOCK_RING_BUFFER();
            asr_param = 143;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 125;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 145;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 141;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_TIMER_8H_ON:
            LOCK_RING_BUFFER();
            asr_param = 143;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 126;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 145;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 141;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        /*TIMER OFF*/
	case ASR_UART_CMD_TIMER_30M_OFF:
            LOCK_RING_BUFFER();
            asr_param = 143;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 144;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 142;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_TIMER_1H_OFF:
            LOCK_RING_BUFFER();
            asr_param = 143;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 119;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 145;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 142;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_TIMER_2H_OFF:
            LOCK_RING_BUFFER();
            asr_param = 143;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 120;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 145;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 142;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_TIMER_3H_OFF:
            LOCK_RING_BUFFER();
            asr_param = 143;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 121;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 145;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 142;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_TIMER_4H_OFF:
            LOCK_RING_BUFFER();
            asr_param = 143;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 122;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 145;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 142;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_TIMER_5H_OFF:
            LOCK_RING_BUFFER();
            asr_param = 143;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 123;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 145;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 142;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_TIMER_6H_OFF:
            LOCK_RING_BUFFER();
            asr_param = 143;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 124;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 145;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 142;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_TIMER_7H_OFF:
            LOCK_RING_BUFFER();
            asr_param = 143;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 125;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 145;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 142;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_TIMER_8H_OFF:
            LOCK_RING_BUFFER();
            asr_param = 143;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 126;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 145;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 142;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;

        case ASR_UART_CMD_TIMER_CANCEL:
            LOCK_RING_BUFFER();
            asr_param = 146;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;

        /*comfort and eco*/
        case ASR_UART_CMD_COMFORT_ECO_ON: //打开舒省
            LOCK_RING_BUFFER();
            asr_param = 8;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 189;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_COMFORT_ECO_OFF_a: //关闭舒省
        case ASR_UART_CMD_COMFORT_ECO_OFF_b: //关掉舒省
            LOCK_RING_BUFFER();
            asr_param = 7;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 189;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;

        /*self-clean*/
        case ASR_UART_CMD_SELF_CLEAN_ON_a: //打开自清洁
        case ASR_UART_CMD_SELF_CLEAN_ON_b: //自清洁
            LOCK_RING_BUFFER();
            asr_param = 272;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 273;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 302;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_SELF_CLEAN_OFF_a: //关闭自清洁
        case ASR_UART_CMD_SELF_CLEAN_OFF_b: //关掉自清洁
            LOCK_RING_BUFFER();
            asr_param = 7;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 149;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;

        /*auxiliary-electric-heating*/
        case ASR_UART_CMD_AUXILIARY_ELECTRIC_HEATING_ON_a: //打开电辅热
        case ASR_UART_CMD_AUXILIARY_ELECTRIC_HEATING_ON_b: //电辅热
            LOCK_RING_BUFFER();
            asr_param = 8;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 151;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_AUXILIARY_ELECTRIC_HEATING_OFF_a: //关闭电辅热
        case ASR_UART_CMD_AUXILIARY_ELECTRIC_HEATING_OFF_b: //关掉电辅热
            LOCK_RING_BUFFER();
            asr_param = 7;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 151;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;

        /*anti-blow*/
        case ASR_UART_CMD_ANTI_BLOW_ON_a: //打开防直吹
        case ASR_UART_CMD_ANTI_BLOW_ON_b: //防直吹
            LOCK_RING_BUFFER();
            asr_param = 8;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 153;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_ANTI_BLOW_OFF_a: //关闭防直吹
        case ASR_UART_CMD_ANTI_BLOW_OFF_b: //关掉防直吹
            LOCK_RING_BUFFER();
            asr_param = 7;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 153;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;

        /*set volume*/
        case ASR_UART_CMD_SET_VOLUME_MAX: //最大音量
            volume_level = 5;
            set_volume_level(volume_level);
            LOCK_RING_BUFFER();
            asr_param = 6;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 163;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            volume_level = 5;
            break;
        case ASR_UART_CMD_SET_VOLUME_MIN: //最小音量
            volume_level = 1;
            set_volume_level(volume_level);
            LOCK_RING_BUFFER();
            asr_param = 6;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 164;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            volume_level = 1;
            break;
        case ASR_UART_CMD_SET_VOLUME_INC1: //增大音量
        case ASR_UART_CMD_SET_VOLUME_INC2: //调大音量
            LOCK_RING_BUFFER();
            if (volume_level == 5) {
                volume_level = 5;
            } else {
                volume_level += volume_step;
            }
            set_volume_level(volume_level);
            asr_param = 155;
            send_to_ring_buffer(&asr_param, 1);

            asr_param = volume_level + 119 - 1;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_SET_VOLUME_DEC1: //减小音量
        case ASR_UART_CMD_SET_VOLUME_DEC2: //调小音量
            LOCK_RING_BUFFER();
            if (volume_level == 1) {
                volume_level = 1;
            } else {
                volume_level -= volume_step;
            }
            set_volume_level(volume_level);
            asr_param = 155;
            send_to_ring_buffer(&asr_param, 1);

            asr_param = volume_level + 119 - 1;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;

        /*screen-display*/
        case ASR_UART_CMD_SCREEN_DISPLAY_ON: //打开屏显
            LOCK_RING_BUFFER();
            asr_param = 8;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 156;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_SCREEN_DISPLAY_OFF_a: //关闭屏显
        case ASR_UART_CMD_SCREEN_DISPLAY_OFF_b: //关掉屏显
            LOCK_RING_BUFFER();
            asr_param = 7;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 156;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;

        /*anti-cold*/
        //case ASR_UART_CMD_ANTI_COLD_ON_a: //打开防过冷
        //case ASR_UART_CMD_ANTI_COLD_ON_b: //防过冷
        case ASR_UART_CMD_SMART_CTL_ON1: //打开智控温
        case ASR_UART_CMD_SMART_CTL_ON2: //智控温
            LOCK_RING_BUFFER();
            asr_param = 8;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 204;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        //case ASR_UART_CMD_ANTI_COLD_OFF_a: //关闭防过冷
        //case ASR_UART_CMD_ANTI_COLD_OFF_b: //关掉防过冷
        case ASR_UART_CMD_SMART_CTL_OFF1: //关闭智控温
        case ASR_UART_CMD_SMART_CTL_OFF2: //关掉智控温
            LOCK_RING_BUFFER();
            asr_param = 7;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 204;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;

        /*strong mode*/
        case ASR_UART_CMD_STRONG_MODE_ON: //打开强劲
            LOCK_RING_BUFFER();
            asr_param = 8;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 150;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_STRONG_MODE_OFF_a: //关闭强劲
        case ASR_UART_CMD_STRONG_MODE_OFF_b: //关掉强劲
            LOCK_RING_BUFFER();
            asr_param = 7;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 150;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;

        /*wifi*/
        case ASR_UART_CMD_START_WIFI_SNIFFER: //空调开始配网
        case ASR_UART_CMD_RESET_WIFI_CONF:    //空调重新配网
            LOCK_RING_BUFFER();
            asr_param = 162;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;

        /*weather query*/
        case ASR_UART_CMD_QUERY_WEATHER_a: //天气查询
        case ASR_UART_CMD_QUERY_WEATHER_b: //查询天气
        {
#ifdef PLAY_ALL_AUDIOS_ENABLE
            int i = 0;
            do {
                i++;
                usleep(1500 * 1000);
                asr_param = PLAYBACK_COUNTS - i;
                log_info(" ---> i = %d, total_counts = %d <---\n", i, PLAYBACK_COUNTS);
                LOCK_RING_BUFFER();
                send_to_ring_buffer(&asr_param, 1);
                UNLOCK_RING_BUFFER();
            } while((PLAYBACK_COUNTS - i) > 0);
#else
            LOCK_RING_BUFFER();
            asr_param = 264;
            send_to_ring_buffer(&asr_param, 1);
            asr_param = 206;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
#endif
        }
        break;

        /*others*/
        case ASR_UART_CMD_LIVING_ROOM_FAN_ON:    //打开客厅风扇
        case ASR_UART_CMD_BED_ROOM_FAN_ON:    //打开卧室风扇
        case ASR_UART_CMD_LIVING_ROOM_HUMIDIFIER_ON: //打开客厅加湿器
        case ASR_UART_CMD_BED_ROOM_HUMIDIFIER_ON: //打开卧室加湿器
        case ASR_UART_CMD_LIVING_ROOM_AIR_CLEANER_ON: //打开客厅净化器
        case ASR_UART_CMD_BED_ROOM_AIR_CLEANER_ON: //打开卧室净化器
        case ASR_UART_CMD_BED_ROOM_AC_ON: //打开卧室空调
        case ASR_UART_CMD_LIVING_ROOM_LAMP_ON: //客厅开灯
        case ASR_UART_CMD_BED_ROOM_LAMP_ON: //卧室开灯
            LOCK_RING_BUFFER();
            asr_param = 169;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_LIVING_ROOM_FAN_OFF: //关闭客厅风扇
        case ASR_UART_CMD_BED_ROOM_FAN_OFF: //关闭卧室风扇
        case ASR_UART_CMD_LIVING_ROOM_HUMIDIFIER_OFF: //关闭客厅加湿器
        case ASR_UART_CMD_BED_ROOM_HUMIDIFIER_OFF: //关闭卧室加湿器
        case ASR_UART_CMD_LIVING_ROOM_AIR_CLEANER_OFF: //关闭客厅净化器
        case ASR_UART_CMD_BED_ROOM_AIR_CLEANER_OFF: //关闭卧室净化器
        case ASR_UART_CMD_BED_ROOM_AC_OFF: //关闭卧室空调
        case ASR_UART_CMD_LIVING_ROOM_LAMP_OFF: //客厅关灯
        case ASR_UART_CMD_BED_ROOM_LAMP_OFF: //卧室关灯
            LOCK_RING_BUFFER();
            asr_param = 170;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_LIVING_ROOM_FAN_OSC_ON: //客厅风扇摇头
        case ASR_UART_CMD_BED_ROOM_FAN_OSC_ON: //卧室风扇摇头
        case ASR_UART_CMD_LIVING_ROOM_FAN_OSC_OFF://客厅风扇停止摇头
        case ASR_UART_CMD_BED_ROOM_FAN_OSC_OFF://卧室风扇停止摇头
        case ASR_UART_CMD_LIVING_ROOM_FAN_INC:    //客厅风扇调大一档
        case ASR_UART_CMD_BED_ROOM_FAN_INC:     //卧室风扇调大一档
        case ASR_UART_CMD_LIVING_ROOM_FAN_DEC:    //客厅风扇调小一档
        case ASR_UART_CMD_BED_ROOM_FAN_DEC:     //卧室风扇调小一档
        case ASR_UART_CMD_LIVING_ROOM_FAN_MAX:    //客厅风扇调到最大
        case ASR_UART_CMD_BED_ROOM_FAN_MAX:     //卧室风扇调到最大
        case ASR_UART_CMD_LIVING_ROOM_FAN_MIN:    //客厅风扇调到最小
        case ASR_UART_CMD_BED_ROOM_FAN_MIN:     //卧室风扇调到最小
        case ASR_UART_CMD_SWEEP_ROBOT_CLEAN_ALL:    //扫地机器人扫地
        case ASR_UART_CMD_SWEEP_ROBOT_CHARGING:     //扫地机器人充电
        case ASR_UART_CMD_SWEEP_ROBOT_CLEAN_PAUSE://扫地机器人暂停
            LOCK_RING_BUFFER();
            asr_param = 171;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_PEOPLE_IMBACK://我回来了
        case ASR_UART_CMD_PEOPLE_IMHOME://我到家了
            LOCK_RING_BUFFER();
            asr_param = 176;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;
        case ASR_UART_CMD_PEOPLE_IGOTOWORK://我去上班了
        case ASR_UART_CMD_PEOPLE_IGOOUT://我出门了
            LOCK_RING_BUFFER();
            asr_param = 175;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;

	/*start-up asr id*/
        case ASR_UART_CMD_STARTUP:
            //return 0;
            LOCK_RING_BUFFER();
            asr_param = 291;
            //asr_param = 406;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            break;

        /*wakeup-timeout asr id*/
        case ASR_UART_CMD_WAKEUP_TIMEOUT:
            LOCK_RING_BUFFER();
            asr_param = 355 - 42;
            send_to_ring_buffer(&asr_param, 1);
            UNLOCK_RING_BUFFER();
            return;
            //break;

        default:
            LOGW("Invalid ASR Command!");
            return;
    }
}
