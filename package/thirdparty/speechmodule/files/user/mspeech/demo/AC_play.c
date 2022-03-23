#include <time.h>

#include "mcrc_speech_user_intf.h"
#include "local_asr.h"
#include "AC_play.h"
#include "AC_json.h"
#include "errno.h"

#ifdef LOCAL_ASR
#ifdef LOCAL_ASR_AC

#define  LOCAL_END_SESSION 1
#define LOCAL_CONTINUE_SESSION   0
#define LOCAL_NO_TEMP_VOL   -1

extern int catWavFile(int argc,char *path1,char *path2,...);
extern struct localasr_proc localasr;

int my_system(const char *cmd)
{
	FILE *fp;
	int res;
	char buf[1024];
	if (cmd ==NULL)
	{
		MS_TRACE("my_system cmd isNULL!\n");
		return-1;
	}
	if ((fp = popen(cmd, "r") ) ==NULL)
	{
		perror("popen");
		MS_TRACE("popen error: %s/n",strerror(errno)); return -1;
	}
	else
	{
		while(fgets(buf, sizeof(buf),fp))
		{
			MS_TRACE("%s",buf);
		}
		if ( (res = pclose(fp)) ==-1)
		{
			MS_TRACE("close popen filepointer fp error!\n"); return res;
		}
		else if (res ==0)
		{
			return res;
		}
		else
		{
			MS_TRACE("popen res is :%d\n",res); return res;
		}
	}
}

void local_asr_tts_play_power_already_on(void)
{
	char tts[1] = LOCAL_AUDIO_POWER_ON_RES_OK;

	local_asr_tts_play(tts, 1);
}

void local_asr_tts_play_power_on(int mode, int temp, float small_temp)
{
	char tts[8] = LOCAL_AUDIO_POWER_ON_RES_OK;
	int i = 1;
	switch(mode){
		case AC_MODE_AUTO:
			tts[i++] = AUDIO_AUTO_MODE;
			break;
		case AC_MODE_COOL:
			tts[i++] = AUDIO_COOL_MODE;
			break;
		case AC_MODE_HEAT:
			tts[i++] = AUDIO_HEAT_MODE;
			break;
		case AC_MODE_FAN:
			tts[i++] = AUDIO_FAN_MODE;
			break;
		case AC_MODE_DRY:
			tts[i++] = AUDIO_DRY_MODE;
			break;
		default:
			break;
	}
	if(mode != AC_MODE_FAN){
		if(small_temp == 0.5){
			tts[i++] = AUDIO_TEMP_POINT(temp);
			tts[i++] = AUDIO_SMALL_TEMP;
		}else{
			tts[i++] = AUDIO_TEMP(temp);
		}
	}
	tts[i++] = AUDIO_AUTO_WIND_SPEED;

	local_asr_tts_play(tts, i);
}

void local_asr_tts_play_power_off(void)
{
	char tts[] = LOCAL_AUDIO_POWER_OFF_RES_OK;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_pls_turn_on_ac(void)
{
	char tts[] = LOCAL_AUDIO_PLS_TURN_ON_AC;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_wind_speed_1_pct(void)
{
	char tts[] = LOCAL_AUDIO_WIND_SPEED_1_PCT;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_wind_speed_20_pct(void)
{
	char tts[] = LOCAL_AUDIO_WIND_SPEED_20_PCT;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_wind_speed_40_pct(void)
{
	char tts[] = LOCAL_AUDIO_WIND_SPEED_40_PCT;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_wind_speed_60_pct(void)
{
	char tts[] = LOCAL_AUDIO_WIND_SPEED_60_PCT;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_wind_speed_80_pct(void)
{
	char tts[] = LOCAL_AUDIO_WIND_SPEED_80_PCT;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_wind_speed_100_pct(void)
{
	char tts[] = LOCAL_AUDIO_WIND_SPEED_100_PCT;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_set_min_wind_speed(void)
{
	char tts[] = LOCAL_AUDIO_SET_MIN_WIND_SPEED;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_set_mid_wind_speed(void)
{
	char tts[] = LOCAL_AUDIO_SET_MID_WIND_SPEED;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_set_max_wind_speed(void)
{
	char tts[] = LOCAL_AUDIO_SET_MAX_WIND_SPEED;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_set_auto_wind_speed(void)
{
	char tts[] = LOCAL_AUDIO_SET_AUTO_WIND_SPEED;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_auto_wind_speed(void)
{
	char tts[1] = {AUDIO_AUTO_WIND_SPEED};
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_wind_speed(int cmd)
{
	switch(cmd){
		case LOCAL_CMD_WIND_SPEED_1:
			local_asr_tts_play_wind_speed_1_pct();
			break;
		case LOCAL_CMD_WIND_SPEED_20:
			local_asr_tts_play_wind_speed_20_pct();
			break;
		case LOCAL_CMD_WIND_SPEED_40:
			local_asr_tts_play_wind_speed_40_pct();
			break;
		case LOCAL_CMD_WIND_SPEED_60:
			local_asr_tts_play_wind_speed_60_pct();
			break;
		case LOCAL_CMD_WIND_SPEED_80:
			local_asr_tts_play_wind_speed_80_pct();
			break;
		case LOCAL_CMD_WIND_SPEED_100:
			local_asr_tts_play_wind_speed_100_pct();
			break;
		case LOCAL_CMD_WIND_SPEED_MIN:
			local_asr_tts_play_set_min_wind_speed();
			break;
		case LOCAL_CMD_WIND_SPEED_MID:
			local_asr_tts_play_set_mid_wind_speed();
			break;
		case LOCAL_CMD_WIND_SPEED_MAX:
			local_asr_tts_play_set_max_wind_speed();
			break;
		case LOCAL_CMD_WIND_SPEED_AUTO:
			local_asr_tts_play_set_auto_wind_speed();
			break;
		default:
			break;
	}
}

void local_asr_tts_play_wind_speed_in_wrong_mode(void)
{
	char tts[] = LOCAL_AUDIO_SET_WIND_SPEED_IN_WRONG_MODE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_wind_speed_max(void)
{
	char tts[] = LOCAL_AUDIO_WIND_SPEED_MAX;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_wind_speed_min(void)
{
	char tts[] = LOCAL_AUDIO_WIND_SPEED_MIN;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_temp(int temp, float small_temp)
{
	char tts[3] = {0};
	int i = 0;

	tts[i++] = AUDIO_SET_UP;

	if(small_temp == 0.5){
		tts[i++] = AUDIO_TEMP_POINT(temp);
		tts[i++] = AUDIO_SMALL_TEMP;
	}else{
		tts[i++] = AUDIO_TEMP(temp);
	}
	local_asr_tts_play(tts, i);
}

void local_asr_tts_play_set_temp_in_wrong_mode(void)
{
	char tts[] = LOCAL_AUDIO_SET_TEMP_IN_WRONG_MODE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_temp_max(void)
{
	char tts[] = LOCAL_AUDIO_TEMP_MAX;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_temp_min(void)
{
	char tts[] = LOCAL_AUDIO_TEMP_MIN;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_cool_mode(void)
{
	char tts[1] = {AUDIO_COOL_MODE};
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_auto_mode(void)
{
	char tts[1] = {AUDIO_AUTO_MODE};
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_heat_mode(void)
{
	char tts[1] = {AUDIO_HEAT_MODE};
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_fan_mode(void)
{
	char tts[1] = {AUDIO_FAN_MODE};
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_dry_mode(void)
{
	char tts[1] = {AUDIO_DRY_MODE};
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_set_auto_mode(void)
{
	char tts[] = LOCAL_AUDIO_AUTO_MODE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_set_cool_mode(void)
{
	char tts[] = LOCAL_AUDIO_COOL_MODE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_set_heat_mode(void)
{
	char tts[] = LOCAL_AUDIO_HEAT_MODE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_set_fan_mode(void)
{
	char tts[] = LOCAL_AUDIO_FAN_MODE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_set_dry_mode(void)
{
	char tts[] = LOCAL_AUDIO_DRY_MODE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_wind_swing_lr_open(void)
{
	char tts[] = LOCAL_AUDIO_WIND_SWING_LR_OPEN;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_wind_swing_lr_close(void)
{
	char tts[] = LOCAL_AUDIO_WIND_SWING_LR_CLOSE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_wind_swing_ud_open(void)
{
	char tts[] = LOCAL_AUDIO_WIND_SWING_UD_OPEN;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_wind_swing_ud_close(void)
{
	char tts[] = LOCAL_AUDIO_WIND_SWING_UD_CLOSE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_power_onoff_time(ms_enum_ac_switch onoff, int time_value)
{
	char tts[4] = {AUDIO_AC_WILL};
	int hour = time_value / 60;

	if(time_value == 0)
		return ;
	if(hour == 0){
		tts[1] = AUDIO_AFTER_30_MIN;
		if(onoff == ON)
			tts[2] = AUDIO_POWER_OFF;
		else
			tts[2] = AUDIO_POWER_ON;
		local_asr_tts_play(tts, 3);
	}else{
		tts[1] = AUDIO_NUMBER(hour);
		tts[2] = AUDIO_HOUR;
		if(onoff == ON)
			tts[3] = AUDIO_POWER_OFF;
		else
			tts[3] = AUDIO_POWER_ON;
		local_asr_tts_play(tts, sizeof(tts));
	}

}

void local_asr_tts_play_cancle_power_off_timer(void)
{
	char tts[] = LOCAL_AUDIO_CANCLE_POWER_OFF_TIMER;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_cancle_power_on_timer(void)
{
	char tts[] = LOCAL_AUDIO_CANCLE_POWER_ON_TIMER;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_self_clean(void)
{
	char tts[] = LOCAL_AUDIO_SELF_CLEAN;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_cancle_self_clean(void)
{
	char tts[] = LOCAL_AUDIO_CANCLE_SELF_CLEAN;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_cleaning_self(void)
{
	char tts[] = LOCAL_AUDIO_CLEANING_SELF;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_ptc_open(void)
{
	char tts[] = LOCAL_AUDIO_PTC_OPEN;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_ptc_in_wrong_mode(void)
{
	char tts[] = LOCAL_AUDIO_PTC_IN_WRONG_MODE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_ptc_close(void)
{
	char tts[] = LOCAL_AUDIO_PTC_CLOSE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_wind_straight_open(void)
{
	char tts[] = LOCAL_AUDIO_WIND_STRAIGHT_OPEN;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_wind_straight_close(void)
{
	char tts[] = LOCAL_AUDIO_WIND_STRAIGHT_CLOSE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_wind_avoid_open(void)
{
	char tts[] = LOCAL_AUDIO_WIND_AVOID_OPEN;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_wind_avoid_close(void)
{
	char tts[] = LOCAL_AUDIO_WIND_AVOID_CLOSE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_intelligent_wind_open(void)
{
	char tts[] = LOCAL_AUDIO_INTELLIGENT_WIND_OPEN;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_intelligent_wind_close(void)
{
	char tts[] = LOCAL_AUDIO_INTELLIGENT_WIND_CLOSE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_no_wind_sense_upper_open(void)
{
	char tts[] = LOCAL_AUDIO_NO_WIND_SENSE_UPPER_OPEN;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_no_wind_sense_upper_close(void)
{
	char tts[] = LOCAL_AUDIO_NO_WIND_SENSE_UPPER_CLOSE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_no_wind_sense_under_open(void)
{
	char tts[] = LOCAL_AUDIO_NO_WIND_SENSE_UNDER_OPEN;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_no_wind_sense_under_close(void)
{
	char tts[] = LOCAL_AUDIO_NO_WIND_SENSE_UNDER_CLOSE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_no_wind_sense_open(void)
{
	char tts[] = LOCAL_AUDIO_NO_WIND_SENSE_OPEN;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_no_wind_sense_close(void)
{
	char tts[] = LOCAL_AUDIO_NO_WIND_SENSE_CLOSE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_no_wind_sense_in_wrong_mode(void)
{
	char tts[] = LOCAL_AUDIO_NO_WIND_SENSE_IN_WRONG_MODE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_child_prevent_cold_wind_open(void)
{
	char tts[] = LOCAL_AUDIO_CHILD_PREVENT_COLD_WIND_OPEN;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_child_prevent_cold_wind_close(void)
{
	char tts[] = LOCAL_AUDIO_CHILD_PREVENT_COLD_WIND_CLOSE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_child_prevent_in_wrong_mode(void)
{
	char tts[] = LOCAL_AUDIO_CHILD_PREVENT_IN_WRONG_MODE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_strong_wind_open(void)
{
	char tts[] = LOCAL_AUDIO_STRONG_WIND_OPEN;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_strong_wind_close(void)
{
	char tts[] = LOCAL_AUDIO_STRONG_WIND_CLOSE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_strong_wind_in_wrong_mode(void)
{
	char tts[] = LOCAL_AUDIO_STRONG_WIND_IN_WRONG_MODE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_screen_display_open(void)
{
	char tts[] = LOCAL_AUDIO_SCREEN_DISPLAY_OPEN;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_screen_display_close(void)
{
	char tts[] = LOCAL_AUDIO_SCREEN_DISPLAY_CLOSE;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_vol_already_max(void)
{
	char tts[] = LOCAL_AUDIO_VOL_ALREADY_MAX;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_vol_already_min(void)
{
	char tts[] = LOCAL_AUDIO_VOL_ALREADY_MIN;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_vol_max(void)
{
	char tts[] = LOCAL_AUDIO_VOL_MAX;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_vol_min(void)
{
	char tts[] = LOCAL_AUDIO_VOL_MIN;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_vol_1(void)
{
	char tts[] = LOCAL_AUDIO_VOL_1;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_vol_2(void)
{
	char tts[] = LOCAL_AUDIO_VOL_2;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_vol_3(void)
{
	char tts[] = LOCAL_AUDIO_VOL_3;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_vol_4(void)
{
	char tts[] = LOCAL_AUDIO_VOL_4;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_vol_5(void)
{
	char tts[] = LOCAL_AUDIO_VOL_5;
	local_asr_tts_play(tts, sizeof(tts));
}

void local_asr_tts_play_vol(int vol)
{
	switch(vol){
		case VOL_1:
			local_asr_tts_play_vol_1();
			break;
		case VOL_2:
			local_asr_tts_play_vol_2();
			break;
		case VOL_3:
			local_asr_tts_play_vol_3();
			break;
		case VOL_4:
			local_asr_tts_play_vol_4();
			break;
		case VOL_5:
			local_asr_tts_play_vol_5();
			break;
		default:
			if(vol > VOL_5)
				local_asr_tts_play_vol_5();
			else if(vol < VOL_1)
				local_asr_tts_play_vol_1();
			break;
	}
}

void local_asr_tts_play_control_failed(void)
{
	char tts[] = LOCAL_AUDIO_CONTROL_FAILED;
	local_asr_tts_play(tts, sizeof(tts));
}

int local_asr_tts_play_nocmd(void)
{
	return mcrc_speech_play_request(localasr.user, LOCAL_RADIO_AGAININPUT, MSUI_PLAY_LOCAL_TEST, 0, -1);
}

int local_asr_tts_play_nocmdend(void)
{
	time_t time_now;
    time(&time_now);
	static time_t prev_call_time = 0;
    if(time_now - prev_call_time > 60){
		prev_call_time = time_now;
		//return mcrc_speech_play_request(localasr.user, LOCAL_RADIO_UNKOWNINPUTBYE, MSUI_PLAY_LOCAL_TEST, LOCAL_END_SESSION, LOCAL_NO_TEMP_VOL);
		return 0;
	}
	else {
	     prev_call_time = time_now;
        return mcrc_speech_play_request(localasr.user, LOCAL_RADIO_DISCONNECT, MSUI_PLAY_LOCAL_TEST, LOCAL_END_SESSION, LOCAL_NO_TEMP_VOL);
	}
}

int local_asr_tts_play_networkfail(void)
{
	return mcrc_speech_play_request(localasr.user, "/mnt/app/tts/network_config_fail.mp3", MSUI_PLAY_LOCAL_TEST, 1, -1);
}

int local_asr_tts_tempfile_play()
{
	my_system("aplay /tmp/asr.wav;rm /tmp/asr.wav");
	local_asr_send_reset(&localasr);
	return 0;
}

#define ARGC_MAX 6
void local_asr_tts_play(char *tts, int tts_num)
{
	int i;
	char tts_path[ARGC_MAX][256] = {0};
	int argc = tts_num > ARGC_MAX?ARGC_MAX:tts_num;

	if(argc == 1){
		snprintf(tts_path[0], 256, LOCAL_RADIO_PATH, tts[0]);
		//my_system(tts_path[0]);
		mcrc_speech_play_request(localasr.user, tts_path[0], MSUI_PLAY_LOCAL_TEST, 1, -1);
		return ;
	}

	for(i = 0; i < argc; i++){
		snprintf(tts_path[i], 256, LOCAL_RADIO_PATH, tts[i]);
		//mcrc_speech_play_request(localasr.user, tts_path, MSUI_PLAY_LOCAL_TEST, (i == tts_num-1)?1:0);
		MS_TRACE("tts play %s\n", tts_path[i]);
	}
	my_system("rm /tmp/asr.mp3");
	//catWavFile(argc, tts_path[0], tts_path[1], tts_path[2], tts_path[3]);
	catmp3File(argc, tts_path[0], tts_path[1], tts_path[2], tts_path[3], tts_path[4], tts_path[5]);
	mcrc_speech_play_request(localasr.user, "/tmp/asr.mp3", MSUI_PLAY_LOCAL_TEST, 1, -1);
	//local_asr_tts_tempfile_play();
}

#endif
#endif
