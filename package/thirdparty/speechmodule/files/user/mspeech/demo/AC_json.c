#ifdef LOCAL_ASR
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "local_asr.h"
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "cJSON.h"
#include <pthread.h>
#include <unistd.h>
#include "duilite.h"
#include "local_asr.h"
#include "debug.h"
#include "json2hex.h"
#include "AC_json.h"
#include "AC_play.h"
#include <time.h>
#include "msm_adapter.h"

#define MAISPEECH_in         "/tmp/maipseech_in"
#define MAISPEECH_out         "/tmp/maipseech_out"

#define VOLUME_FILE_PATH	"/mnt/app/cfg/volume_config"

#define WIND_S_A 0

extern struct localasr_proc localasr;
extern pthread_mutex_t ac_status_lock;
static timer_t localtimer=0;



struct AC_status{
	ms_enum_ac_switch power;
	ms_enum_ac_switch purifier;
	ms_enum_ac_mode mode;
	int temperature;
	float small_temperature;
	int wind_speed;
	ms_enum_ac_switch wind_swing_lr_upper;
	ms_enum_ac_switch wind_swing_lr_under;
	ms_enum_ac_switch wind_swing_ud;
	ms_enum_ac_switch power_on_timer;
	ms_enum_ac_switch power_off_timer;
	int power_off_time_value;
	int power_on_time_value;
	ms_enum_ac_switch eco;
	ms_enum_ac_switch dry;
	ms_enum_ac_switch ptc;
	ms_enum_ac_switch buzzer;
	ms_enum_ac_switch prevent_super_cool;
	ms_enum_ac_straight_wind prevent_straight_wind;
	ms_enum_ac_switch auto_prevent_straight_wind;
	ms_enum_ac_switch self_clean;
#if WIND_S_A
	ms_enum_ac_wind_s_a wind_straight_avoid;
#else
	ms_enum_ac_switch wind_straight;
	ms_enum_ac_switch wind_avoid;
#endif
	ms_enum_ac_switch intelligent_wind;
	ms_enum_ac_no_wind_sense no_wind_sense;
	ms_enum_ac_switch child_prevent_cold_wind;
	ms_enum_ac_switch strong_wind;
	ms_enum_ac_switch comfort_power_save;
	ms_enum_ac_switch screen_display;
	ms_enum_ac_switch security;
};
static struct AC_status acstatus = {
	.power = OFF,
	.purifier = OFF,
	.mode = AC_MODE_AUTO,
	.temperature = 26,
	.small_temperature = 0,
	.wind_speed = 102,
	.wind_swing_lr_upper = OFF,
	.wind_swing_lr_under = OFF,
	.wind_swing_ud = OFF,
	.power_on_timer = OFF,
	.power_off_timer = OFF,
	.power_off_time_value = OFF,
	.power_on_time_value = OFF,
	.eco = OFF,
	.dry = OFF,
	.ptc = OFF,
	.buzzer = ON,
	.prevent_super_cool = OFF,
	.prevent_straight_wind = AC_STRAIGHT_WIND_NONE,
	.auto_prevent_straight_wind = OFF,
	.self_clean = OFF,
#if WIND_S_A
	.wind_straight_avoid = AC_WIND_STRAIGHT_AVOID_OFF,
#else
	.wind_straight = OFF,
	.wind_avoid = OFF,
#endif
	.intelligent_wind = OFF,
	.no_wind_sense = AC_NO_WIND_SENSE_CLOSE,
	.child_prevent_cold_wind = OFF,
	.strong_wind = OFF,
	.comfort_power_save = OFF,
	.screen_display = OFF,
	.security = ON
};
int g_volume = VOL_MIN;

static char *ac_mode[AC_MODE_MAX] = {"heat","cool","auto","dry","fan"};

int local_cmd_send(void *data,int len)
{
	int fifo_fd;
	int ret;
	struct T_MsTransStream transtream;
	int open_mode = O_WRONLY|O_NONBLOCK;
	MS_TRACE("local cmd sned\r\n");
	if(access(MAISPEECH_out,F_OK)==-1){
		MS_TRACE("fifo file is empty\r\n");
		ret=mkfifo(MAISPEECH_out,0777);
		if(ret!=0){
			MS_TRACE("Could not create fifo %s\n",MAISPEECH_out);
		}
	}
	MS_TRACE("fifo opening\r\n");
	fifo_fd = open(MAISPEECH_out, open_mode);

	if(fifo_fd !=-1){
		memcpy(transtream.data,data,len);
		transtream.msg_id = ENUM_MS_DOWNSTREAM_NET_MSPEECH;
		transtream.size = len;
		transtream.event = MS_SYS_EVENT_TRANSDATA;
		MS_TRACE("fifo write=%d\r\n",len);
		ret = write(fifo_fd, &transtream,sizeof(struct T_MsTransStream ));
		if(ret == -1){
			MS_TRACE("loca cmd write fail\r\n");
		}
		close(fifo_fd);
	}
	else{
		MS_TRACE("fifo fd err\r\n");
	}
	return ret;
}

cJSON *create_status_json(struct AC_status ac_status)
{
	char status[1024] = {0};

	snprintf(status, 1024, "{\"power\":\"%s\","\
			"\"purifier\":\"%s\","\
			"\"mode\":\"%s\","\
			"\"temperature\":%d,"\
			"\"small_temperature\":%f,"\
			"\"wind_speed\":%d,"\
			"\"wind_swing_lr_upper\":\"%s\","\
			"\"wind_swing_lr_under\":\"%s\","\
			"\"wind_swing_ud\":\"%s\","\
			"\"power_on_timer\":\"%s\","\
			"\"power_off_timer\":\"%s\","\
			"\"power_off_time_value\":%d,"\
			"\"power_on_time_value\":%d,"\
			"\"eco\":\"%s\","\
			"\"dry\":\"%s\","\
			"\"ptc\":\"%s\","\
			"\"buzzer\":\"%s\","\
			"\"strong_wind\":\"%s\","\
			"\"comfort_power_save\":\"%s\"}", ac_status.power ? "on":"off",	\
			ac_status.purifier ? "on":"off",		\
			ac_mode[ac_status.mode],			\
			ac_status.temperature,			\
			ac_status.small_temperature,		\
			ac_status.wind_speed,			\
			ac_status.wind_swing_lr_upper ? "on":"off",	\
			ac_status.wind_swing_lr_under ? "on":"off",	\
			ac_status.wind_swing_ud ? "on":"off",	\
			ac_status.power_on_timer ? "on":"off",	\
			ac_status.power_off_timer ? "on":"off",	\
			ac_status.power_off_time_value,		\
			ac_status.power_on_time_value,		\
			ac_status.eco ? "on":"off",		\
			ac_status.dry ? "on":"off",		\
			ac_status.ptc ? "on":"off",		\
			ac_status.buzzer ? "on":"off",		\
			ac_status.strong_wind ? "on":"off",	\
			ac_status.comfort_power_save ? "on":"off"	\
			);
#if 0
	"\"prevent_super_cool\":\"%s\","\
		"\"prevent_straight_wind\":\"%s\","\
		"\"auto_prevent_straight_wind\":\"%s\","\
		"\"self_clean\":\"%s\","\
		"\"wind_straight\":\"%s\","\
		"\"wind_avoid\":\"%s\","\
		"\"intelligent_wind\":\"%s\","\
		"\"no_wind_sense\":\"%s\","\
		"\"child_prevent_cold_wind\":\"%s\","
#endif
#if 0
		ac_status.prevent_super_cool ? "on":"off",	\
		ac_status.prevent_straight_wind ? "on":"off",	\
		ac_status.auto_prevent_straight_wind ? "on":"off",	\
		ac_status.self_clean ? "on":"off",	\
		ac_status.wind_straight ? "on":"off",	\
		ac_status.wind_avoid ? "on":"off",	\
		ac_status.intelligent_wind ? "on":"off",	\
		ac_status.no_wind_sense ? "on":"off",	\
		ac_status.child_prevent_cold_wind ? "on":"off",
#endif
		MS_TRACE("%s",status);
	return cJSON_Parse(status);
}

int get_local_cmd_by_wind_speed(int speed, bool is_num)
{
	switch(speed){
		case 1:
			if(is_num)
				return LOCAL_CMD_WIND_SPEED_1;
			else
				return LOCAL_CMD_WIND_SPEED_MIN;
		case 20:
			return LOCAL_CMD_WIND_SPEED_20;
		case 40:
			return LOCAL_CMD_WIND_SPEED_40;
		case 60:
			if(is_num)
				return LOCAL_CMD_WIND_SPEED_60;
			else
				return LOCAL_CMD_WIND_SPEED_MID;
		case 80:
			return LOCAL_CMD_WIND_SPEED_80;
		case 100:
			if(is_num)
				return LOCAL_CMD_WIND_SPEED_100;
			else
				return LOCAL_CMD_WIND_SPEED_MAX;
		case 102:
			return LOCAL_CMD_WIND_SPEED_AUTO;
	}
}

int get_local_cmd_by_temp(int temp, float small_temp)
{
	if(small_temp == 0.5)
		return (LOCAL_CMD_TEMP_17_5 + temp - TEMP_MIN);
	else
		return (LOCAL_CMD_TEMP_17 + temp - TEMP_MIN);
}

int get_local_cmd_by_power_onoff_time(ms_enum_ac_switch onoff, int minute)
{
	int hour = minute / 60;

	if(minute == 0)
		return -1;
	switch(hour){
		case 0:
			if(onoff == ON)
				return LOCAL_CMD_POWER_OFF_TIMER_30M;
			else
				return LOCAL_CMD_POWER_ON_TIMER_30M;
		case 1:
			if(onoff == ON)
				return LOCAL_CMD_POWER_OFF_TIMER_1H;
			else
				return LOCAL_CMD_POWER_ON_TIMER_1H;
		case 2:
			if(onoff == ON)
				return LOCAL_CMD_POWER_OFF_TIMER_2H;
			else
				return LOCAL_CMD_POWER_ON_TIMER_2H;
		case 3:
			if(onoff == ON)
				return LOCAL_CMD_POWER_OFF_TIMER_3H;
			else
				return LOCAL_CMD_POWER_ON_TIMER_3H;
		case 4:
			if(onoff == ON)
				return LOCAL_CMD_POWER_OFF_TIMER_4H;
			else
				return LOCAL_CMD_POWER_ON_TIMER_4H;
		case 5:
			if(onoff == ON)
				return LOCAL_CMD_POWER_OFF_TIMER_5H;
			else
				return LOCAL_CMD_POWER_ON_TIMER_5H;
		case 6:
			if(onoff == ON)
				return LOCAL_CMD_POWER_OFF_TIMER_6H;
			else
				return LOCAL_CMD_POWER_ON_TIMER_6H;
		case 7:
			if(onoff == ON)
				return LOCAL_CMD_POWER_OFF_TIMER_7H;
			else
				return LOCAL_CMD_POWER_ON_TIMER_7H;
		case 8:
			if(onoff == ON)
				return LOCAL_CMD_POWER_OFF_TIMER_8H;
			else
				return LOCAL_CMD_POWER_ON_TIMER_8H;
		default:
			break;
	}
	return -1;
}

int get_volume(void)
{
	int fd;
	char buf[3] = {0};
	int volume = 0;

	fd = open(VOLUME_FILE_PATH, O_RDONLY);
	if(fd < 0){
		MS_TRACE("open failed: "VOLUME_FILE_PATH);
		return fd;
	}
	if(read(fd, buf, 3) > 0){
		volume = atoi(buf);
	}
	close(fd);
	return volume;
}

int get_device_info(ms_stored_info_t *dev_info)
{
	return msm_read_config(MS_STORED_INFO, (char *)dev_info, sizeof(ms_stored_info_t));
}

int local_fail_AC(void *user_data, int type)
{

	struct localasr_proc *proc = user_data;
    if(type == LOCAL_FAIL_NOISE){
		if(proc->cmd.device!=NULL){
			cJSON_Delete(proc->cmd.device);
			proc->cmd.device =NULL;
		}
		local_asr_tts_play_nocmdend();
	}
	else if(type == LOCAL_FAIL_VOICE){
		//local_asr_tts_play_nocmd();
	}
	else if(type == LOCAL_FAIL_RECNUMOUT){
		if(proc->cmd.device!=NULL){
			cJSON_Delete(proc->cmd.device);
			proc->cmd.device =NULL;
		}
		//local_asr_tts_play_nocmdend();
	}
	else if(type == LOCAL_FAIL_STILLNESS){
		if(proc->cmd.device!=NULL){
			//local_asr_tts_play_nocmd();
		}
	}
}

int local_cmd_get_state(struct localasr_proc *proc, cJSON *device, int cmd)
{
	cJSON *query = cJSON_CreateObject();
	if(!query){
		return LOCAL_CMD_GET_STATE_FAILED;
	}
	proc->cmd.cmdtype = cmd;
	switch(cmd){
		case LOCAL_CMD_GET_NORMAL_STATE:
			cJSON_AddStringToObject(query, "query_type", "power");
			break;
		case LOCAL_CMD_GET_SELF_CLEAN_STATE:
			cJSON_AddStringToObject(query, "query_type", "self_clean");
			break;
		case LOCAL_CMD_GET_NO_WIND_SENSE_STATE:
			cJSON_AddStringToObject(query, "query_type", "no_wind_sense");
			break;
		default:
			break;
	}
	cJSON_AddItemToObject(device, "query", query);
	char *query_str = cJSON_Print(device);
	json2hex(query_str, proc);
	pthread_mutex_unlock(&ac_status_lock);
	cJSON_DeleteItemFromObject(device, "query");
	free(query_str);
	while(proc->cmd.cmdtype == cmd){
		usleep(1000);
	}
	pthread_mutex_lock(&ac_status_lock);

	return proc->cmd.cmdtype;
}

int asr_callback(void *user_data, int type, char *msg, int len)
{
	local_asr_tts_play_networkfail();
	return 0;

	if (type == DUILITE_MSG_TYPE_JSON) {
		cJSON *device =NULL;
		cJSON *intent = NULL;
		cJSON *devinf=NULL;
		cJSON *slot =NULL;
		cJSON * deviceinfo = NULL;
		int ret = 0;

		struct localasr_proc *proc = user_data;
		//	 int n;
		//	 char data[256];
		// enum SPEECH_EVENT in = RESET;
		//  n = sprintf(data, "%d\r\n%.*s\r\n\r\n", in, 0, NULL);
		// kmsg_push(cs->msg, data, n);
		assert(proc!= NULL);

		cJSON *root = cJSON_Parse(msg);

		MS_TRACE("%s\n", msg);

		if(!root){
			MS_TRACE("is not json\r\n");
			return -1;
		}
		cJSON *conf =cJSON_GetObjectItem(root,"conf");
		if(!conf){
			MS_TRACE("sec sec device is null\r\n");
			cJSON_Delete(root);

			return -1;
		}
		if(conf->valuedouble <0.49){

			cJSON_Delete(root);
			//local_asr_tts_play_nocmdend();
			return -1;
		}
		if(conf->valuedouble <0.60){
			//local_asr_tts_play_nocmd();
			cJSON_Delete(root);
			return -1;
		}
		cJSON *rec =cJSON_GetObjectItem(root,"rec");
		if(!rec){
			MS_TRACE("rec invalid\r\n");
			//local_asr_tts_play_nocmd();
			cJSON_Delete(root);
			return -1;
		}

		device = cJSON_CreateObject();
		if(!device){
			MS_TRACE("cjson device create err\r\n");
			cJSON_Delete(root);
			return -1;
		}

		deviceinfo= cJSON_CreateObject();
		if(!deviceinfo){
			MS_TRACE("cjson deviceinfo create err\r\n");
			cJSON_Delete(device);
			cJSON_Delete(root);
			return -1;
		}
		cJSON_AddNumberToObject(deviceinfo, "deviceSubType",1);
		ms_stored_info_t dev_info;
		if(get_device_info(&dev_info) != -1)
			cJSON_AddStringToObject(deviceinfo, "deviceSN", dev_info.sn);
		cJSON_AddItemToObject(device,"deviceinfo",deviceinfo);


		cJSON *post =cJSON_GetObjectItem(root,"post");

		cJSON *sem =cJSON_GetObjectItem(post,"sem");

		if(!post || !sem){
			MS_TRACE("cjson no post sem\r\n");
			cJSON_Delete(root);
			cJSON_Delete(device);
			//local_asr_tts_play_nocmd();
			return -1;
		}
		cJSON *control = cJSON_CreateObject();
		if(!control){
			cJSON_Delete(root);
			cJSON_Delete(device);
			return -1;
		}

		proc->cmd.cmdtype = 0;
		cJSON *temperature=cJSON_GetObjectItem(sem,"temperature");
		pthread_mutex_lock(&ac_status_lock);

		if(temperature && (!strcmp(temperature->valuestring, "17度")||
					!strcmp(temperature->valuestring, "十七度"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else{
				acstatus.temperature = 17;
				acstatus.small_temperature = 0;
				proc->cmd.cmdtype = LOCAL_CMD_TEMP_17;
			}
			}
		}
		else if(temperature && (!strcmp(temperature->valuestring, "18度")||
					!strcmp(temperature->valuestring, "十八度"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else{
				acstatus.temperature = 18;
				acstatus.small_temperature = 0;
				proc->cmd.cmdtype = LOCAL_CMD_TEMP_18;
			}
			}
		}
		else if(temperature && (!strcmp(temperature->valuestring, "19度")||
					!strcmp(temperature->valuestring, "十九度"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else{
				acstatus.temperature = 19;
				acstatus.small_temperature = 0;
				proc->cmd.cmdtype = LOCAL_CMD_TEMP_19;
			}
			}
		}
		else if(temperature && (!strcmp(temperature->valuestring, "20度")||
					!strcmp(temperature->valuestring, "二十度"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else{
				acstatus.temperature = 20;
				acstatus.small_temperature = 0;
				proc->cmd.cmdtype = LOCAL_CMD_TEMP_20;
			}
			}
		}
		else if(temperature && (!strcmp(temperature->valuestring, "21度")||
					!strcmp(temperature->valuestring, "二十一度"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else{
				acstatus.temperature = 21;
				acstatus.small_temperature = 0;
				proc->cmd.cmdtype = LOCAL_CMD_TEMP_21;
			}
			}
		}
		else if(temperature && (!strcmp(temperature->valuestring, "22度")||
					!strcmp(temperature->valuestring, "二十二度"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else{
				acstatus.temperature = 22;
				acstatus.small_temperature = 0;
				proc->cmd.cmdtype = LOCAL_CMD_TEMP_22;
			}
			}
		}
		else if(temperature && (!strcmp(temperature->valuestring, "23度")||
					!strcmp(temperature->valuestring, "二十三度"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else{
				acstatus.temperature = 23;
				acstatus.small_temperature = 0;
				proc->cmd.cmdtype = LOCAL_CMD_TEMP_23;
			}
			}
		}
		else if(temperature && (!strcmp(temperature->valuestring, "24度")||
					!strcmp(temperature->valuestring, "二十四度"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else{
				acstatus.temperature = 24;
				acstatus.small_temperature = 0;
				proc->cmd.cmdtype = LOCAL_CMD_TEMP_24;
			}
			}
		}
		else if(temperature && (!strcmp(temperature->valuestring, "25度")||
					!strcmp(temperature->valuestring, "二十五度"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else{
				acstatus.temperature = 25;
				acstatus.small_temperature = 0;
				proc->cmd.cmdtype = LOCAL_CMD_TEMP_25;
			}
			}
		}
		else if(temperature && (!strcmp(temperature->valuestring, "26度")||
					!strcmp(temperature->valuestring, "二十六度"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else{
				acstatus.temperature = 26;
				acstatus.small_temperature = 0;
				proc->cmd.cmdtype = LOCAL_CMD_TEMP_26;
			}
			}
		}
		else if(temperature && (!strcmp(temperature->valuestring, "27度")||
					!strcmp(temperature->valuestring, "二十七度"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else{
				acstatus.temperature = 27;
				acstatus.small_temperature = 0;
				proc->cmd.cmdtype = LOCAL_CMD_TEMP_27;
			}
			}
		}
		else if(temperature && (!strcmp(temperature->valuestring, "28度")||
					!strcmp(temperature->valuestring, "二十八度"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else{
				acstatus.temperature = 28;
				acstatus.small_temperature = 0;
				proc->cmd.cmdtype = LOCAL_CMD_TEMP_28;
			}
			}
		}
		else if(temperature && (!strcmp(temperature->valuestring, "29度")||
					!strcmp(temperature->valuestring, "二十九度"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else{
				acstatus.temperature = 29;
				acstatus.small_temperature = 0;
				proc->cmd.cmdtype = LOCAL_CMD_TEMP_29;
			}
			}
		}
		else if(temperature && (!strcmp(temperature->valuestring, "30度")||
					!strcmp(temperature->valuestring, "三十度"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else{
				acstatus.temperature = 30;
				acstatus.small_temperature = 0;
				proc->cmd.cmdtype = LOCAL_CMD_TEMP_30;
			}
			}
		}
		else{
			temperature=NULL;
		}
		cJSON *timer_open=cJSON_GetObjectItem(sem,"timer_open");
		if(timer_open && (!strcmp(timer_open->valuestring, "三十分钟后开机")||
					!strcmp(timer_open->valuestring, "30分钟后开机"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == ON){
				local_asr_tts_play_power_already_on();
			}else if(acstatus.power == OFF){
				acstatus.power_on_timer = ON;
				acstatus.power_on_time_value = 30;
				proc->cmd.cmdtype = LOCAL_CMD_POWER_ON_TIMER_30M;
			}
			}
		}
		else if(timer_open && (!strcmp(timer_open->valuestring, "一小时后开机")||
					!strcmp(timer_open->valuestring, "1小时后开机"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == ON){
				local_asr_tts_play_power_already_on();
			}else if(acstatus.power == OFF){
				acstatus.power_on_timer = ON;
				acstatus.power_on_time_value = 60;
				proc->cmd.cmdtype = LOCAL_CMD_POWER_ON_TIMER_1H;
			}
			}
		}
		else if(timer_open && (!strcmp(timer_open->valuestring, "二小时后开机")||
					!strcmp(timer_open->valuestring, "两小时后开机")||!strcmp(timer_open->valuestring, "2小时后开机"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == ON){
				local_asr_tts_play_power_already_on();
			}else if(acstatus.power == OFF){
				acstatus.power_on_timer = ON;
				acstatus.power_on_time_value = 120;
				proc->cmd.cmdtype = LOCAL_CMD_POWER_ON_TIMER_2H;
			}
			}
		}
		else if(timer_open && (!strcmp(timer_open->valuestring, "三小时后开机")||
					!strcmp(timer_open->valuestring, "3小时后开机"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == ON){
				local_asr_tts_play_power_already_on();
			}else if(acstatus.power == OFF){
				acstatus.power_on_timer = ON;
				acstatus.power_on_time_value = 180;
				proc->cmd.cmdtype = LOCAL_CMD_POWER_ON_TIMER_3H;
			}
			}
		}
		else if(timer_open && (!strcmp(timer_open->valuestring, "四小时后开机")||
					!strcmp(timer_open->valuestring, "4小时后开机"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == ON){
				local_asr_tts_play_power_already_on();
			}else if(acstatus.power == OFF){
				acstatus.power_on_timer = ON;
				acstatus.power_on_time_value = 240;
				proc->cmd.cmdtype = LOCAL_CMD_POWER_ON_TIMER_4H;
			}
			}
		}
		else if(timer_open && (!strcmp(timer_open->valuestring, "五小时后开机")||
					!strcmp(timer_open->valuestring, "5小时后开机"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == ON){
				local_asr_tts_play_power_already_on();
			}else if(acstatus.power == OFF){
				acstatus.power_on_timer = ON;
				acstatus.power_on_time_value = 300;
				proc->cmd.cmdtype = LOCAL_CMD_POWER_ON_TIMER_5H;
			}
			}
		}
		else if(timer_open && (!strcmp(timer_open->valuestring, "六小时后开机")||
					!strcmp(timer_open->valuestring, "6小时后开机"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == ON){
				local_asr_tts_play_power_already_on();
			}else if(acstatus.power == OFF){
				acstatus.power_on_timer = ON;
				acstatus.power_on_time_value = 360;
				proc->cmd.cmdtype = LOCAL_CMD_POWER_ON_TIMER_6H;
			}
			}
		}
		else if(timer_open && (!strcmp(timer_open->valuestring, "七小时后开机")||
					!strcmp(timer_open->valuestring, "7小时后开机"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == ON){
				local_asr_tts_play_power_already_on();
			}else if(acstatus.power == OFF){
				acstatus.power_on_timer = ON;
				acstatus.power_on_time_value = 420;
				proc->cmd.cmdtype = LOCAL_CMD_POWER_ON_TIMER_7H;
			}
			}
		}
		else if(timer_open && (!strcmp(timer_open->valuestring, "八小时后开机")||
					!strcmp(timer_open->valuestring, "8小时后开机"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == ON){
				local_asr_tts_play_power_already_on();
			}else if(acstatus.power == OFF){
				acstatus.power_on_timer = ON;
				acstatus.power_on_time_value = 480;
				proc->cmd.cmdtype = LOCAL_CMD_POWER_ON_TIMER_8H;
			}
			}
		}else{
			//TODO
		}
		cJSON *timer_close=cJSON_GetObjectItem(sem,"timer_close");
		if(timer_close && (!strcmp(timer_close->valuestring, "三十分钟后关机")||
					!strcmp(timer_close->valuestring, "30分钟后关机"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == ON){
				acstatus.power_off_timer = ON;
				acstatus.power_off_time_value = 30;
				proc->cmd.cmdtype = LOCAL_CMD_POWER_OFF_TIMER_30M;
			}else if(acstatus.power == OFF){
				local_asr_tts_play_power_off();
			}
			}
		}
		else if(timer_close && (!strcmp(timer_close->valuestring, "一小时后关机")||
					!strcmp(timer_close->valuestring, "1小时后关机"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == ON){
				acstatus.power_off_timer = ON;
				acstatus.power_off_time_value = 60;
				proc->cmd.cmdtype = LOCAL_CMD_POWER_OFF_TIMER_1H;
			}else if(acstatus.power == OFF){
				local_asr_tts_play_power_off();
			}
			}
		}
		else if(timer_close && (!strcmp(timer_close->valuestring, "二小时后关机")||
					!strcmp(timer_close->valuestring, "两小时后关机")||!strcmp(timer_close->valuestring, "2小时后关机"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == ON){
				acstatus.power_off_timer = ON;
				acstatus.power_off_time_value = 120;
				proc->cmd.cmdtype = LOCAL_CMD_POWER_OFF_TIMER_2H;
			}else if(acstatus.power == OFF){
				local_asr_tts_play_power_off();
			}
			}
		}
		else if(timer_close && (!strcmp(timer_close->valuestring, "三小时后关机")||
					!strcmp(timer_close->valuestring, "3小时后关机"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == ON){
				acstatus.power_off_timer = ON;
				acstatus.power_off_time_value = 180;
				proc->cmd.cmdtype = LOCAL_CMD_POWER_OFF_TIMER_3H;
			}else if(acstatus.power == OFF){
				local_asr_tts_play_power_off();
			}
			}
		}
		else if(timer_close && (!strcmp(timer_close->valuestring, "四小时后关机")||
					!strcmp(timer_close->valuestring, "4小时后关机"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == ON){
				acstatus.power_off_timer = ON;
				acstatus.power_off_time_value = 240;
				proc->cmd.cmdtype = LOCAL_CMD_POWER_OFF_TIMER_4H;
			}else if(acstatus.power == OFF){
				local_asr_tts_play_power_off();
			}
			}
		}
		else if(timer_close && (!strcmp(timer_close->valuestring, "五小时后关机")||
					!strcmp(timer_close->valuestring, "5小时后关机"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == ON){
				acstatus.power_off_timer = ON;
				acstatus.power_off_time_value = 300;
				proc->cmd.cmdtype = LOCAL_CMD_POWER_OFF_TIMER_5H;
			}else if(acstatus.power == OFF){
				local_asr_tts_play_power_off();
			}
			}
		}
		else if(timer_close && (!strcmp(timer_close->valuestring, "六小时后关机")||
					!strcmp(timer_close->valuestring, "6小时后关机"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == ON){
				acstatus.power_off_timer = ON;
				acstatus.power_off_time_value = 360;
				proc->cmd.cmdtype = LOCAL_CMD_POWER_OFF_TIMER_6H;
			}else if(acstatus.power == OFF){
				local_asr_tts_play_power_off();
			}
			}
		}
		else if(timer_close && (!strcmp(timer_close->valuestring, "七小时后关机")||
					!strcmp(timer_close->valuestring, "7小时后关机"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == ON){
				acstatus.power_off_timer = ON;
				acstatus.power_off_time_value = 420;
				proc->cmd.cmdtype = LOCAL_CMD_POWER_OFF_TIMER_7H;
			}else if(acstatus.power == OFF){
				local_asr_tts_play_power_off();
			}
			}
		}
		else if(timer_close && (!strcmp(timer_close->valuestring, "八小时后关机")||
					!strcmp(timer_close->valuestring, "8小时后关机"))){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == ON){
				acstatus.power_off_timer = ON;
				acstatus.power_off_time_value = 480;
				proc->cmd.cmdtype = LOCAL_CMD_POWER_OFF_TIMER_8H;
			}else if(acstatus.power == OFF){
				local_asr_tts_play_power_off();
			}
			}
		}else{
			//TODO
		}

		cJSON *operation=cJSON_GetObjectItem(sem,"operation");

		if(operation &&  !strcmp(operation->valuestring, "open")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == ON){
				local_asr_tts_play_power_already_on();
			}else{
				acstatus.power = ON;
				proc->cmd.cmdtype = LOCAL_CMD_OPEN;
				acstatus.wind_speed = 102;
				if(acstatus.mode == AC_MODE_AUTO || acstatus.mode == AC_MODE_HEAT)
					acstatus.ptc = ON;
				// local_asr_tts_play_cmdfull();
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "close")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
				acstatus.power = OFF;
				acstatus.wind_speed = 102;
				proc->cmd.cmdtype = LOCAL_CMD_CLOSE;
				// local_asr_tts_play_cmdfull();
			}
		}
		else if(operation && !strcmp(operation->valuestring, "vol_add")){
			g_volume = get_volume();
			if(g_volume >= VOL_MAX){
				local_asr_tts_play_vol_already_max();
			}else{
				if(g_volume < VOL_1 && g_volume >= 0){
					midea_setSpeakerVol(true, VOL_1 - g_volume);
					g_volume = VOL_1;
				}else if(g_volume < VOL_2){
					midea_setSpeakerVol(true, VOL_2 - g_volume);
					g_volume = VOL_2;
				}else if(g_volume < VOL_3){
					midea_setSpeakerVol(true, VOL_3 - g_volume);
					g_volume = VOL_3;
				}else if(g_volume < VOL_4){
					midea_setSpeakerVol(true, VOL_4 - g_volume);
					g_volume = VOL_4;
				}else if(g_volume < VOL_5){
					midea_setSpeakerVol(true, VOL_5 - g_volume);
					g_volume = VOL_5;
				}
				local_asr_tts_play_vol(g_volume);
			}
		}
		else if(operation && !strcmp(operation->valuestring, "vol_dec")){
			g_volume = get_volume();
			if(g_volume <= VOL_MIN){
				local_asr_tts_play_vol_already_min();
			}else{
				if(g_volume > VOL_4){
					midea_setSpeakerVol(false, g_volume - VOL_4);
					g_volume = VOL_4;
				}else if(g_volume > VOL_3){
					midea_setSpeakerVol(false, g_volume - VOL_3);
					g_volume = VOL_3;
				}else if(g_volume > VOL_2){
					midea_setSpeakerVol(false, g_volume - VOL_2);
					g_volume = VOL_2;
				}else if(g_volume > VOL_1){
					midea_setSpeakerVol(false, g_volume - VOL_1);
					g_volume = VOL_1;
				}
				local_asr_tts_play_vol(g_volume);
			}
		}
		else if(operation && !strcmp(operation->valuestring, "vol_max")){
			g_volume = get_volume();
			if(g_volume < VOL_MAX)
				midea_setSpeakerVol(true, VOL_MAX - g_volume);
			local_asr_tts_play_vol_max();
		}
		else if(operation && !strcmp(operation->valuestring, "vol_min")){
			g_volume = get_volume();
			if(g_volume > VOL_MIN)
				midea_setSpeakerVol(false, g_volume - VOL_1);
			local_asr_tts_play_vol_min();
		}
		else if(operation && !strcmp(operation->valuestring, "fan_add")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_AUTO || acstatus.mode == AC_MODE_DRY){
				local_asr_tts_play_wind_speed_in_wrong_mode();
			}else if(acstatus.wind_speed == 100){
				local_asr_tts_play_wind_speed_max();
			}else{
				if(acstatus.wind_speed == 1)
					acstatus.wind_speed = 20;
				else if(acstatus.wind_speed > 1 && acstatus.wind_speed <= 20)
					acstatus.wind_speed = 40;
				else if(acstatus.wind_speed > 20 && acstatus.wind_speed <= 40)
					acstatus.wind_speed = 60;
				else if(acstatus.wind_speed > 40 && acstatus.wind_speed <= 60)
					acstatus.wind_speed = 80;
				else if((acstatus.wind_speed > 60 && acstatus.wind_speed <= 80) || acstatus.wind_speed == 102)
					acstatus.wind_speed = 100;
				proc->cmd.cmdtype = get_local_cmd_by_wind_speed(acstatus.wind_speed, true);
				//	 local_asr_tts_play_cmdfull();
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "fan_dec")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_AUTO || acstatus.mode == AC_MODE_DRY){
				local_asr_tts_play_wind_speed_in_wrong_mode();
			}else if(acstatus.wind_speed == 1){
				local_asr_tts_play_wind_speed_min();
			}else{
				if(acstatus.wind_speed < 40 || acstatus.wind_speed == 102)
					acstatus.wind_speed = 1;
				else if(acstatus.wind_speed >= 40 && acstatus.wind_speed < 60)
					acstatus.wind_speed = 20;
				else if(acstatus.wind_speed >= 60 && acstatus.wind_speed < 80)
					acstatus.wind_speed = 40;
				else if(acstatus.wind_speed >= 80 && acstatus.wind_speed < 100)
					acstatus.wind_speed = 60;
				else if(acstatus.wind_speed == 100)
					acstatus.wind_speed = 80;
				proc->cmd.cmdtype = get_local_cmd_by_wind_speed(acstatus.wind_speed, true);
				//	 local_asr_tts_play_cmdfull();
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "fan_max")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_AUTO || acstatus.mode == AC_MODE_DRY){
				local_asr_tts_play_wind_speed_in_wrong_mode();
			}else{
				acstatus.wind_speed = 100;
				proc->cmd.cmdtype = LOCAL_CMD_WIND_SPEED_MAX;
				//	 local_asr_tts_play_cmdfull();
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "fan_mid")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_AUTO || acstatus.mode == AC_MODE_DRY){
				local_asr_tts_play_wind_speed_in_wrong_mode();
			}else{
				acstatus.wind_speed = 60;
				proc->cmd.cmdtype = LOCAL_CMD_WIND_SPEED_MID;
				//	 local_asr_tts_play_cmdfull();
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "fan_min")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_AUTO || acstatus.mode == AC_MODE_DRY){
				local_asr_tts_play_wind_speed_in_wrong_mode();
			}else{
				acstatus.wind_speed = 1;
				proc->cmd.cmdtype = LOCAL_CMD_WIND_SPEED_MIN;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "fan_auto")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_AUTO || acstatus.mode == AC_MODE_DRY){
				local_asr_tts_play_wind_speed_in_wrong_mode();
			}else{
				acstatus.wind_speed = 102;
				proc->cmd.cmdtype = LOCAL_CMD_WIND_SPEED_AUTO;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "temp_add")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else if(acstatus.temperature == TEMP_MAX){
				local_asr_tts_play_temp_max();
			}else{
				acstatus.temperature++;
				if(acstatus.temperature >= TEMP_MAX){
					acstatus.temperature = TEMP_MAX;
					acstatus.small_temperature = 0;
				}
				proc->cmd.cmdtype = get_local_cmd_by_temp(acstatus.temperature, acstatus.small_temperature);
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "temp_cold_add")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else if(acstatus.temperature == TEMP_MAX){
				local_asr_tts_play_temp_max();
			}else{
				acstatus.temperature += 1;
				if(acstatus.temperature >= TEMP_MAX){
					acstatus.temperature = TEMP_MAX;
					acstatus.small_temperature = 0;
				}
				proc->cmd.cmdtype = get_local_cmd_by_temp(acstatus.temperature, acstatus.small_temperature);
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "temp_2cold_add")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else if(acstatus.temperature == TEMP_MAX){
				local_asr_tts_play_temp_max();
			}else{
				acstatus.temperature += 3;
				if(acstatus.temperature >= TEMP_MAX){
					acstatus.temperature = TEMP_MAX;
					acstatus.small_temperature = 0;
				}
				proc->cmd.cmdtype = get_local_cmd_by_temp(acstatus.temperature, acstatus.small_temperature);
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "temp_dec")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else if(acstatus.temperature == TEMP_MIN && acstatus.small_temperature == 0){
				local_asr_tts_play_temp_min();
			}else{
				acstatus.temperature --;
				if(acstatus.temperature < TEMP_MIN){
					acstatus.temperature = TEMP_MIN;
					acstatus.small_temperature = 0;
				}
				proc->cmd.cmdtype = get_local_cmd_by_temp(acstatus.temperature, acstatus.small_temperature);
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "temp_hot_dec")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else if(acstatus.temperature == TEMP_MIN && acstatus.small_temperature == 0){
				local_asr_tts_play_temp_min();
			}else{
				acstatus.temperature -= 1;
				if(acstatus.temperature < TEMP_MIN){
					acstatus.temperature = TEMP_MIN;
					acstatus.small_temperature = 0;
				}
				proc->cmd.cmdtype = get_local_cmd_by_temp(acstatus.temperature, acstatus.small_temperature);
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "temp_2hot_dec")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else if(acstatus.temperature == TEMP_MIN && acstatus.small_temperature == 0){
				local_asr_tts_play_temp_min();
			}else{
				acstatus.temperature -= 3;
				if(acstatus.temperature < TEMP_MIN){
					acstatus.temperature = TEMP_MIN;
					acstatus.small_temperature = 0;
				}
				proc->cmd.cmdtype = get_local_cmd_by_temp(acstatus.temperature, acstatus.small_temperature);
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "temp_add_half")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else if(acstatus.temperature == TEMP_MAX){
				local_asr_tts_play_temp_max();
			}else{
				if(acstatus.small_temperature == 0.5){
					acstatus.small_temperature = 0;
					acstatus.temperature ++;
				}else if(acstatus.small_temperature == 0 && acstatus.temperature < TEMP_MAX){
					acstatus.small_temperature = 0.5;
				}
				proc->cmd.cmdtype = get_local_cmd_by_temp(acstatus.temperature, acstatus.small_temperature);
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "temp_dec_half")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode == AC_MODE_FAN){
				local_asr_tts_play_set_temp_in_wrong_mode();
			}else if(acstatus.temperature == TEMP_MIN && acstatus.small_temperature == 0){
				local_asr_tts_play_temp_min();
			}else{
				if(acstatus.small_temperature == 0.5){
					acstatus.small_temperature = 0;
				}else if(acstatus.small_temperature == 0 && acstatus.temperature > TEMP_MIN){
					acstatus.small_temperature = 0.5;
					acstatus.temperature --;
				}
				proc->cmd.cmdtype = get_local_cmd_by_temp(acstatus.temperature, acstatus.small_temperature);
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "mode_auto")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else{
				acstatus.mode = AC_MODE_AUTO;
				acstatus.wind_speed = 102;
				acstatus.ptc = ON;
				proc->cmd.cmdtype = LOCAL_CMD_AUTO_MODE;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "mode_cool")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else{
				acstatus.mode = AC_MODE_COOL;
				acstatus.wind_speed = 102;
				proc->cmd.cmdtype = LOCAL_CMD_COOL_MODE;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "mode_heat")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else{
				acstatus.mode = AC_MODE_HEAT;
				acstatus.wind_speed = 102;
				acstatus.ptc = ON;
				proc->cmd.cmdtype = LOCAL_CMD_HEAT_MODE;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "mode_fan")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else{
				acstatus.mode = AC_MODE_FAN;
				acstatus.wind_speed = 102;
				proc->cmd.cmdtype = LOCAL_CMD_FAN_MODE;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "mode_dry")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else{
				acstatus.mode = AC_MODE_DRY;
				acstatus.wind_speed = 102;
				proc->cmd.cmdtype = LOCAL_CMD_DRY_MODE;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "wind_swing_ud_on")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else{
				acstatus.wind_swing_ud = ON;
				proc->cmd.cmdtype = LOCAL_CMD_WIND_SWING_UD_OPEN;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "wind_swing_lr_on")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NO_WIND_SENSE_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else{
				if(acstatus.no_wind_sense == AC_NO_WIND_SENSE_UNDER_OPEN){
					acstatus.wind_swing_lr_upper = ON;
					acstatus.wind_swing_lr_under = OFF;
				}else if(acstatus.no_wind_sense == AC_NO_WIND_SENSE_UPPER_OPEN){
					acstatus.wind_swing_lr_upper = OFF;
					acstatus.wind_swing_lr_under = ON;
				}else{
					acstatus.wind_swing_lr_upper = ON;
					acstatus.wind_swing_lr_under = ON;
				}
				proc->cmd.cmdtype = LOCAL_CMD_WIND_SWING_LR_OPEN;
			}
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "wind_swing_ud_off")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else{
				acstatus.wind_swing_ud = OFF;
				proc->cmd.cmdtype = LOCAL_CMD_WIND_SWING_UD_CLOSE;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "wind_swing_lr_off")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else{
				acstatus.wind_swing_lr_upper = OFF;
				acstatus.wind_swing_lr_under = OFF;
				proc->cmd.cmdtype = LOCAL_CMD_WIND_SWING_LR_CLOSE;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "timer_cancle")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == ON && acstatus.power_off_timer == ON){
				acstatus.power_off_timer = OFF;
				proc->cmd.cmdtype = LOCAL_CMD_CANCLE_POWER_OFF_TIMER;
			}else if(acstatus.power == OFF && acstatus.power_on_timer == ON){
				acstatus.power_on_timer = OFF;
				proc->cmd.cmdtype = LOCAL_CMD_CANCLE_POWER_ON_TIMER;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "self_clean_on")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_SELF_CLEAN_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.self_clean == ON){
				local_asr_tts_play_cleaning_self();
			}else{
				acstatus.self_clean = ON;
				cJSON_AddStringToObject(control, "self_clean", "on");
				proc->cmd.cmdtype = LOCAL_CMD_SELF_CLEAN;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "self_clean_off")){
			acstatus.self_clean = OFF;
			cJSON_AddStringToObject(control, "self_clean", "off");
			proc->cmd.cmdtype = LOCAL_CMD_CANCLE_SELF_CLEAN;
		}
		else if(operation && !strcmp(operation->valuestring, "ptc_on")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode != AC_MODE_HEAT && acstatus.mode != AC_MODE_AUTO){
				local_asr_tts_play_ptc_in_wrong_mode();
			}else{
				acstatus.ptc = ON;
				cJSON_AddStringToObject(control, "ptc", "on");
				proc->cmd.cmdtype = LOCAL_CMD_PTC_OPEN;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "ptc_off")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode != AC_MODE_HEAT && acstatus.mode != AC_MODE_AUTO){
				local_asr_tts_play_ptc_close();
			}else{
				acstatus.ptc = OFF;
				cJSON_AddStringToObject(control, "ptc", "off");
				proc->cmd.cmdtype = LOCAL_CMD_PTC_CLOSE;
			}
			}
		}
#if WIND_S_A
		else if(operation && !strcmp(operation->valuestring, "wind_straight_on")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else{
				acstatus.wind_straight_avoid = AC_WIND_STRAIGHT;
				cJSON_AddNumberToObject(control, "wind_straight_avoid", 1);
				proc->cmd.cmdtype = LOCAL_CMD_WIND_STRAIGHT_OPEN;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "wind_straight_off")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else{
				acstatus.wind_straight_avoid = AC_WIND_STRAIGHT_AVOID_OFF;
				cJSON_AddNumberToObject(control, "wind_straight_avoid", 0);
				proc->cmd.cmdtype = LOCAL_CMD_WIND_STRAIGHT_CLOSE;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "wind_avoid_on")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else{
				acstatus.wind_straight_avoid = AC_WIND_AVOID;
				cJSON_AddNumberToObject(control, "wind_straight_avoid", 2);
				proc->cmd.cmdtype = LOCAL_CMD_WIND_AVOID_OPEN;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "wind_avoid_off")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else{
				acstatus.wind_straight_avoid = AC_WIND_STRAIGHT_AVOID_OFF;
				cJSON_AddNumberToObject(control, "wind_straight_avoid", 0);
				proc->cmd.cmdtype = LOCAL_CMD_WIND_AVOID_CLOSE;
			}
			}
		}
#else
		else if(operation && !strcmp(operation->valuestring, "wind_straight_on")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else{
				acstatus.wind_straight = ON;
				cJSON_AddStringToObject(control, "wind_straight", "on");
				proc->cmd.cmdtype = LOCAL_CMD_WIND_STRAIGHT_OPEN;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "wind_straight_off")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else{
				acstatus.wind_straight = OFF;
				cJSON_AddStringToObject(control, "wind_straight", "off");
				proc->cmd.cmdtype = LOCAL_CMD_WIND_STRAIGHT_CLOSE;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "wind_avoid_on")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else{
				acstatus.wind_avoid = ON;
				cJSON_AddStringToObject(control, "wind_avoid", "on");
				proc->cmd.cmdtype = LOCAL_CMD_WIND_AVOID_OPEN;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "wind_avoid_off")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else{
				acstatus.wind_avoid = OFF;
				cJSON_AddStringToObject(control, "wind_avoid", "off");
				proc->cmd.cmdtype = LOCAL_CMD_WIND_AVOID_CLOSE;
			}
			}
		}
#endif
		else if(operation && !strcmp(operation->valuestring, "intelligent_wind_on")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else{
				acstatus.intelligent_wind = ON;
				cJSON_AddStringToObject(control, "intelligent_wind", "on");
				proc->cmd.cmdtype = LOCAL_CMD_INTELLIGENT_WIND_OPEN;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "intelligent_wind_off")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else{
				acstatus.intelligent_wind = OFF;
				cJSON_AddStringToObject(control, "intelligent_wind", "off");
				proc->cmd.cmdtype = LOCAL_CMD_INTELLIGENT_WIND_CLOSE;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "no_wind_sense_upper_on")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode != AC_MODE_COOL){
				local_asr_tts_play_no_wind_sense_in_wrong_mode();
			}else{
				ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NO_WIND_SENSE_STATE);
				if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
				if(acstatus.no_wind_sense == AC_NO_WIND_SENSE_UNDER_OPEN)
					acstatus.no_wind_sense = AC_NO_WIND_SENSE_OPEN;
				else if(acstatus.no_wind_sense != AC_NO_WIND_SENSE_OPEN)
					acstatus.no_wind_sense = AC_NO_WIND_SENSE_UPPER_OPEN;
				cJSON_AddNumberToObject(control, "no_wind_sense", acstatus.no_wind_sense);
				proc->cmd.cmdtype = LOCAL_CMD_NO_WIND_SENSE_UPPER_OPEN;
				}
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "no_wind_sense_upper_off")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode != AC_MODE_COOL){
				local_asr_tts_play_no_wind_sense_upper_close();
			}else{
				ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NO_WIND_SENSE_STATE);
				if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
				if(acstatus.no_wind_sense == AC_NO_WIND_SENSE_OPEN)
					acstatus.no_wind_sense = AC_NO_WIND_SENSE_UNDER_OPEN;
				else if(acstatus.no_wind_sense == AC_NO_WIND_SENSE_UPPER_OPEN)
					acstatus.no_wind_sense = AC_NO_WIND_SENSE_CLOSE;
				cJSON_AddNumberToObject(control, "no_wind_sense", acstatus.no_wind_sense);
				proc->cmd.cmdtype = LOCAL_CMD_NO_WIND_SENSE_UPPER_CLOSE;
				}
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "no_wind_sense_under_on")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode != AC_MODE_COOL){
				local_asr_tts_play_no_wind_sense_in_wrong_mode();
			}else{
				ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NO_WIND_SENSE_STATE);
				if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
				if(acstatus.no_wind_sense == AC_NO_WIND_SENSE_UPPER_OPEN)
					acstatus.no_wind_sense = AC_NO_WIND_SENSE_OPEN;
				else if(acstatus.no_wind_sense != AC_NO_WIND_SENSE_OPEN)
					acstatus.no_wind_sense = AC_NO_WIND_SENSE_UNDER_OPEN;
				cJSON_AddNumberToObject(control, "no_wind_sense", acstatus.no_wind_sense);
				proc->cmd.cmdtype = LOCAL_CMD_NO_WIND_SENSE_UNDER_OPEN;
				}
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "no_wind_sense_under_off")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode != AC_MODE_COOL){
				local_asr_tts_play_no_wind_sense_under_close();
			}else{
				ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NO_WIND_SENSE_STATE);
				if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
				if(acstatus.no_wind_sense == AC_NO_WIND_SENSE_OPEN)
					acstatus.no_wind_sense = AC_NO_WIND_SENSE_UPPER_OPEN;
				else if(acstatus.no_wind_sense == AC_NO_WIND_SENSE_UNDER_OPEN)
					acstatus.no_wind_sense = AC_NO_WIND_SENSE_CLOSE;
				cJSON_AddNumberToObject(control, "no_wind_sense", acstatus.no_wind_sense);
				proc->cmd.cmdtype = LOCAL_CMD_NO_WIND_SENSE_UNDER_CLOSE;
				}
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "no_wind_sense_on")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode != AC_MODE_COOL){
				local_asr_tts_play_no_wind_sense_in_wrong_mode();
			}else{
				acstatus.no_wind_sense = AC_NO_WIND_SENSE_OPEN;
				cJSON_AddNumberToObject(control, "no_wind_sense", AC_NO_WIND_SENSE_OPEN);
				proc->cmd.cmdtype = LOCAL_CMD_NO_WIND_SENSE_OPEN;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "no_wind_sense_off")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode != AC_MODE_COOL){
				local_asr_tts_play_no_wind_sense_close();
			}else{
				acstatus.no_wind_sense = AC_NO_WIND_SENSE_CLOSE;
				cJSON_AddNumberToObject(control, "no_wind_sense", AC_NO_WIND_SENSE_CLOSE);
				proc->cmd.cmdtype = LOCAL_CMD_NO_WIND_SENSE_CLOSE;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "child_prevent_cold_wind_on")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode != AC_MODE_COOL){
				local_asr_tts_play_child_prevent_in_wrong_mode();
			}else{
				acstatus.child_prevent_cold_wind = ON;
				cJSON_AddStringToObject(control, "child_prevent_cold_wind", "on");
				proc->cmd.cmdtype = LOCAL_CMD_CHILD_PREVENT_COLD_WIND_OPEN;
			}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "child_prevent_cold_wind_off")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode != AC_MODE_COOL){
				local_asr_tts_play_child_prevent_cold_wind_close();
			}else{
				acstatus.child_prevent_cold_wind = OFF;
				cJSON_AddStringToObject(control, "child_prevent_cold_wind", "off");
				proc->cmd.cmdtype = LOCAL_CMD_CHILD_PREVENT_COLD_WIND_CLOSE;
			}
			}
		}
		/*
		else if(operation && !strcmp(operation->valuestring, "strong_wind_on")){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode != AC_MODE_COOL && acstatus.mode != AC_MODE_HEAT){
				local_asr_tts_play_strong_wind_in_wrong_mode();
			}else{
				acstatus.strong_wind = ON;
				cJSON_AddStringToObject(control, "strong_wind", "on");
				proc->cmd.cmdtype = LOCAL_CMD_STRONG_WIND_OPEN;
			}
		}
		else if(operation && !strcmp(operation->valuestring, "strong_wind_off")){
			if(acstatus.power == OFF){
				local_asr_tts_play_pls_turn_on_ac();
			}else if(acstatus.mode != AC_MODE_COOL && acstatus.mode != AC_MODE_HEAT){
				local_asr_tts_play_strong_wind_close();
			}else{
				acstatus.strong_wind = OFF;
				cJSON_AddStringToObject(control, "strong_wind", "off");
				proc->cmd.cmdtype = LOCAL_CMD_STRONG_WIND_CLOSE;
			}
		}
			   else if(operation && !strcmp(operation->valuestring, "monitor")){
			   acstatus.security = ON;
			   proc->cmd.cmdtype = LOCAL_CMD_MONITOR;
			   cJSON_AddStringToObject(control, "security", "on");
			   }	*/
		else if(operation && !strcmp(operation->valuestring, "screen_display_on")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
				ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_SELF_CLEAN_STATE);
				if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
				if(acstatus.power == OFF && acstatus.self_clean == OFF){
					local_asr_tts_play_pls_turn_on_ac();
				}else{
					acstatus.screen_display = ON;
					cJSON_AddStringToObject(control, "screen_display", "on");
					proc->cmd.cmdtype = LOCAL_CMD_SCREEN_DISPLAY_OPEN;
				}
				}
			}
		}
		else if(operation && !strcmp(operation->valuestring, "screen_display_off")){
			ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_NORMAL_STATE);
			if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
				ret = local_cmd_get_state(proc, device, LOCAL_CMD_GET_SELF_CLEAN_STATE);
				if(ret != LOCAL_CMD_GET_STATE_TIMEOUT && ret != LOCAL_CMD_GET_STATE_FAILED){
				if(acstatus.power == OFF && acstatus.self_clean == OFF){
					local_asr_tts_play_pls_turn_on_ac();
				}else{
					acstatus.screen_display = OFF;
					cJSON_AddStringToObject(control, "screen_display", "off");
					proc->cmd.cmdtype = LOCAL_CMD_SCREEN_DISPLAY_CLOSE;
				}
				}
			}
		}
		else{
#if 0
			local_asr_tts_play_firstnocmd();
			cJSON_Delete(root);
			cJSON_Delete(device);
			MS_TRACE("nocmd\r\n");
			return -1;
#endif
		}

		cJSON *status = create_status_json(acstatus);
		if(!status){
			cJSON_Delete(root);
			cJSON_Delete(device);
			MS_TRACE("create_status_json error!");
			return -1;
		}
		cJSON_AddItemToObject(device, "status", status);
		cJSON_AddItemToObject(device, "control", control);
		char *out = cJSON_Print(device);
		MS_TRACE("local:\n%s\n", out);


		if(proc->cmd.device!=NULL){
			cJSON_Delete(proc->cmd.device);
			proc->cmd.device=NULL;

		}
		if(proc->cmd.cmdtype == LOCAL_CMD_GET_STATE_TIMEOUT || proc->cmd.cmdtype == LOCAL_CMD_GET_STATE_FAILED)
			proc->cmd.cmdtype = 0;
		if(proc->cmd.cmdtype != 0){
			MS_TRACE("cmdtype=%d\r\n",proc->cmd.cmdtype);
			json2hex(out,proc);
			proc->cmd.hasreceived=false;
			cJSON_Delete(device);

		}
		else{
			proc->cmd.device = device;
		}
		pthread_mutex_unlock(&ac_status_lock);
		free(out);
		cJSON_Delete(root);
	}
	return 0;
}

#if 0
void B1_set_msg_has_received(void)
{
	struct localasr_proc *plocalasr=&localasr;

	int cmdtype= plocalasr->cmd.cmdtype;
	switch(cmdtype)
	{
		case LOCAL_CMD_OPEN:
			if(acstatus.work_status == B1_WORK_STATUS_STANDBY){
				local_asr_tts_basectrl_play(LOCAL_AUDIO_OPEN_RES_OK);
				plocalasr->cmd.cmdtype=0;
			}
			break;
		case LOCAL_CMD_CLOSE:
			if(acstatus.work_status == B1_WORK_STATUS_SAVE_POWER){
				local_asr_tts_basectrl_play(LOCAL_AUDIO_CLOSE_RES_OK);
				plocalasr->cmd.cmdtype=0;

			}
			break;
		case LOCAL_CMD_PREHEAT:
			break;
		case LOCAL_CMD_CANCEL_PREHEAT:
			if(acstatus.work_status == B1_WORK_STATUS_STANDBY||
					acstatus.work_status == B1_WORK_STATUS_SAVE_POWER||
					acstatus.work_status == B1_WORK_STATUS_STANDBY||
					acstatus.tips_code !=8){
				local_asr_tts_basectrl_play(LOCAL_AUDIO_CANCLEPREHEAT_RES_OK);
				plocalasr->cmd.cmdtype=0;

			}

			break;
		case LOCAL_CMD_ZYMOSIS_CLOSE:
			if(acstatus.work_status == B1_WORK_STATUS_STANDBY||
					acstatus.work_status == B1_WORK_STATUS_SAVE_POWER||
					acstatus.work_mode != B1_WORK_MODE_ZYMOSIS){
				local_asr_tts_basectrl_play(LOCAL_AUDIO_ZYMOSISCLOSE_RES_OK);
				plocalasr->cmd.cmdtype=0;
			}

			break;
		case LOCAL_CMD_ZYMOSIS:
			break;
		case LOCAL_CMD_PAUSE:
			if(acstatus.work_status != B1_WORK_STATUS_WORK){
				local_asr_tts_basectrl_play(LOCAL_AUDIO_PAUSE_RES_OK);
				plocalasr->cmd.cmdtype=0;

			}

			break;
		case LOCAL_CMD_RESUME:
			if(acstatus.work_status != B1_WORK_STATUS_PAUSE){
				local_asr_tts_basectrl_play(LOCAL_AUDIO_RESUME_RES_OK);
				plocalasr->cmd.cmdtype=0;

			}

			break;
		case LOCAL_CMD_STOP:
			if(acstatus.work_status != B1_WORK_STATUS_WORK||
					acstatus.work_status != B1_WORK_STATUS_PAUSE){
				local_asr_tts_basectrl_play(LOCAL_AUDIO_STOP_RES_OK);
				plocalasr->cmd.cmdtype=0;

			}

			break;

		default:
			break;



	}










	localasr.cmd.hasreceived =true;

}
#endif
void result_parse(char *data,unsigned char cmdtype)
{
	struct localasr_proc *plocalasr=&localasr;
	int old_cmd = plocalasr->cmd.cmdtype;
#if 0
	if(cmdtype == 0x03&&plocalasr->cmd.cmdtype ==0){
		result_03parse(data);
		return;
	}
#endif
	cJSON *root = cJSON_Parse(data);

	if(!root){
		MS_TRACE("result parse err\r\n");
		return ;
	}

	cJSON *status =cJSON_GetObjectItem(root,"status");
	if(!status){
		MS_TRACE("result status err\r\n");
		cJSON_Delete(root);
		return ;
	}
	MS_TRACE("status:--------------\n%s\n",cJSON_Print(status));
	cJSON * power =cJSON_GetObjectItem(status,"power");
	cJSON * purifier =cJSON_GetObjectItem(status,"purifier");
	cJSON * mode =cJSON_GetObjectItem(status,"mode");
	cJSON * temperature =cJSON_GetObjectItem(status,"temperature");
	cJSON * small_temperature =cJSON_GetObjectItem(status,"small_temperature");
	cJSON * wind_speed =cJSON_GetObjectItem(status,"wind_speed");
	cJSON * wind_swing_lr_upper =cJSON_GetObjectItem(status,"wind_swing_lr_upper");
	cJSON * wind_swing_lr_under =cJSON_GetObjectItem(status,"wind_swing_lr_under");
	cJSON * wind_swing_ud =cJSON_GetObjectItem(status,"wind_swing_ud");
	cJSON * power_on_timer =cJSON_GetObjectItem(status,"power_on_timer");
	cJSON * power_off_timer =cJSON_GetObjectItem(status,"power_off_timer");
	cJSON * power_off_time_value =cJSON_GetObjectItem(status,"power_off_time_value");
	cJSON * power_on_time_value =cJSON_GetObjectItem(status,"power_on_time_value");
	cJSON * eco =cJSON_GetObjectItem(status,"eco");
	cJSON * dry =cJSON_GetObjectItem(status,"dry");
	cJSON * ptc =cJSON_GetObjectItem(status,"ptc");
	cJSON * buzzer =cJSON_GetObjectItem(status,"buzzer");
	cJSON * prevent_super_cool =cJSON_GetObjectItem(status,"prevent_super_cool");
	cJSON * prevent_straight_wind =cJSON_GetObjectItem(status,"prevent_straight_wind");
	cJSON * auto_prevent_straight_wind =cJSON_GetObjectItem(status,"auto_prevent_straight_wind");
	cJSON * self_clean =cJSON_GetObjectItem(status,"self_clean");
#if WIND_S_A
	cJSON * wind_straight_avoid =cJSON_GetObjectItem(status,"wind_straight_avoid");
#else
	cJSON * wind_straight =cJSON_GetObjectItem(status,"wind_straight");
	cJSON * wind_avoid =cJSON_GetObjectItem(status,"wind_avoid");
#endif
	cJSON * intelligent_wind =cJSON_GetObjectItem(status,"intelligent_wind");
	cJSON * no_wind_sense =cJSON_GetObjectItem(status,"no_wind_sense");
	cJSON * child_prevent_cold_wind =cJSON_GetObjectItem(status,"child_prevent_cold_wind");
	cJSON * strong_wind =cJSON_GetObjectItem(status,"strong_wind");
	cJSON * comfort_power_save =cJSON_GetObjectItem(status,"comfort_power_save");
	cJSON * screen_display =cJSON_GetObjectItem(status,"screen_display");

	pthread_mutex_lock(&ac_status_lock);
	if(power && !strcmp(power->valuestring, "on")){
		acstatus.power = ON;
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_OPEN){
			local_asr_tts_play_power_on(acstatus.mode, acstatus.temperature, acstatus.small_temperature);
			plocalasr->cmd.cmdtype=0;
		}else if(plocalasr->cmd.cmdtype==LOCAL_CMD_GET_NORMAL_STATE){
			plocalasr->cmd.cmdtype=0;
		}
	}
	else if(power && !strcmp(power->valuestring, "off")){
		acstatus.power = OFF;
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_CLOSE){
			local_asr_tts_play_power_off();
			plocalasr->cmd.cmdtype=0;
		}else if(plocalasr->cmd.cmdtype==LOCAL_CMD_GET_NORMAL_STATE){
			plocalasr->cmd.cmdtype=0;
		}
	}

	if(purifier && !strcmp(purifier->valuestring, "on")){
		/*
		   if(plocalasr->cmd.cmdtype==LOCAL_CMD_OPEN_LIGHT){
		   local_asr_tts_basectrl_play(LOCAL_AUDIO_LIGHTOPEN_RES_OK);
		   plocalasr->cmd.cmdtype=0;
		   }
		 */
		acstatus.purifier = ON;
	}
	else if(purifier && !strcmp(purifier->valuestring, "off")){
		/*
		   if(plocalasr->cmd.cmdtype==LOCAL_CMD_OPEN_LIGHT){
		   local_asr_tts_basectrl_play(LOCAL_AUDIO_LIGHTOPEN_RES_OK);
		   plocalasr->cmd.cmdtype=0;
		   }
		 */
		acstatus.purifier = OFF;
	}

	if(mode && !strcmp(mode->valuestring, "heat")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_HEAT_MODE){
			local_asr_tts_play_set_heat_mode();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.mode = AC_MODE_HEAT;
	}
	else if(mode && !strcmp(mode->valuestring, "cool")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_COOL_MODE){
			local_asr_tts_play_set_cool_mode();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.mode = AC_MODE_COOL;
	}
	else if(mode && !strcmp(mode->valuestring, "auto")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_AUTO_MODE){
			local_asr_tts_play_set_auto_mode();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.mode = AC_MODE_AUTO;
	}
	else if(mode && !strcmp(mode->valuestring, "dry")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_DRY_MODE){
			local_asr_tts_play_set_dry_mode();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.mode = AC_MODE_DRY;
	}
	else if(mode && !strcmp(mode->valuestring, "fan")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_FAN_MODE){
			local_asr_tts_play_set_fan_mode();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.mode = AC_MODE_FAN;
	}

	if(temperature){
		/*
		   if(plocalasr->cmd.cmdtype==LOCAL_CMD_OPEN_LIGHT){
		   local_asr_tts_basectrl_play(LOCAL_AUDIO_LIGHTOPEN_RES_OK);
		   plocalasr->cmd.cmdtype=0;
		   }
		 */
		if(temperature->valueint <= 30 && temperature->valueint >= 17)
			acstatus.temperature = temperature->valueint;
	}

	if(small_temperature){
		if(small_temperature->valuedouble == 0.5 || small_temperature->valuedouble == 0)
			acstatus.small_temperature = small_temperature->valuedouble;
		if(plocalasr->cmd.cmdtype==get_local_cmd_by_temp(acstatus.temperature, acstatus.small_temperature)){
			local_asr_tts_play_temp(acstatus.temperature, acstatus.small_temperature);
			plocalasr->cmd.cmdtype=0;
		}
	}

	if(wind_speed){
		if(wind_speed->valueint <= 102 && wind_speed->valueint >= 1)
			acstatus.wind_speed = wind_speed->valueint;
		if(plocalasr->cmd.cmdtype==get_local_cmd_by_wind_speed(acstatus.wind_speed, true) || plocalasr->cmd.cmdtype==get_local_cmd_by_wind_speed(acstatus.wind_speed, false)){
			local_asr_tts_play_wind_speed(plocalasr->cmd.cmdtype);
			plocalasr->cmd.cmdtype=0;
		}
	}

	if(wind_swing_lr_upper && !strcmp(wind_swing_lr_upper->valuestring, "on")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_WIND_SWING_LR_OPEN){
			local_asr_tts_play_wind_swing_lr_open();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.wind_swing_lr_upper = ON;
	}
	else if(wind_swing_lr_upper && !strcmp(wind_swing_lr_upper->valuestring, "off")){
		acstatus.wind_swing_lr_upper = OFF;
	}

	if(wind_swing_lr_under && !strcmp(wind_swing_lr_under->valuestring, "on")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_WIND_SWING_LR_OPEN){
			local_asr_tts_play_wind_swing_lr_open();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.wind_swing_lr_under = ON;
	}
	else if(wind_swing_lr_under && !strcmp(wind_swing_lr_under->valuestring, "off")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_WIND_SWING_LR_CLOSE && acstatus.wind_swing_lr_upper == OFF){
			local_asr_tts_play_wind_swing_lr_close();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.wind_swing_lr_under = OFF;
	}

	if(wind_swing_ud && !strcmp(wind_swing_ud->valuestring, "on")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_WIND_SWING_UD_OPEN){
			local_asr_tts_play_wind_swing_ud_open();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.wind_swing_ud = ON;
	}
	else if(wind_swing_ud && !strcmp(wind_swing_ud->valuestring, "off")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_WIND_SWING_UD_CLOSE){
			local_asr_tts_play_wind_swing_ud_close();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.wind_swing_ud = OFF;
	}

	if(power_off_time_value){
		if(power_off_time_value->valueint >= 0)
			acstatus.power_off_time_value = power_off_time_value->valueint;
	}

	if(power_on_time_value){
		if(power_on_time_value->valueint >= 0)
			acstatus.power_on_time_value = power_on_time_value->valueint;
	}

	if(power_on_timer && !strcmp(power_on_timer->valuestring, "on")){
		acstatus.power_on_timer = ON;
		if(plocalasr->cmd.cmdtype==get_local_cmd_by_power_onoff_time(acstatus.power, acstatus.power_on_time_value)){
			local_asr_tts_play_power_onoff_time(acstatus.power, acstatus.power_on_time_value);
			plocalasr->cmd.cmdtype=0;
		}
	}
	else if(power_on_timer && !strcmp(power_on_timer->valuestring, "off")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_CANCLE_POWER_ON_TIMER){
			local_asr_tts_play_cancle_power_on_timer();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.power_on_timer = OFF;
	}

	if(power_off_timer && !strcmp(power_off_timer->valuestring, "on")){
		acstatus.power_off_timer = ON;
		if(plocalasr->cmd.cmdtype==get_local_cmd_by_power_onoff_time(acstatus.power, acstatus.power_off_time_value)){
			local_asr_tts_play_power_onoff_time(acstatus.power, acstatus.power_off_time_value);
			plocalasr->cmd.cmdtype=0;
		}
	}
	else if(power_off_timer && !strcmp(power_off_timer->valuestring, "off")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_CANCLE_POWER_OFF_TIMER){
			local_asr_tts_play_cancle_power_off_timer();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.power_off_timer = OFF;
	}

	if(eco && !strcmp(eco->valuestring, "on")){/*
						      if(plocalasr->cmd.cmdtype==LOCAL_CMD_OPEN_LIGHT){
						      local_asr_tts_basectrl_play(LOCAL_AUDIO_LIGHTOPEN_RES_OK);
						      plocalasr->cmd.cmdtype=0;
						      }
						    */
		acstatus.eco = ON;
	}
	else if(eco && !strcmp(eco->valuestring, "off")){
		/*
		   if(plocalasr->cmd.cmdtype==LOCAL_CMD_OPEN_LIGHT){
		   local_asr_tts_basectrl_play(LOCAL_AUDIO_LIGHTOPEN_RES_OK);
		   plocalasr->cmd.cmdtype=0;
		   }
		 */
		acstatus.eco = OFF;
	}

	if(dry && !strcmp(dry->valuestring, "on")){
		/*
		   if(plocalasr->cmd.cmdtype==LOCAL_CMD_OPEN_LIGHT){
		   local_asr_tts_basectrl_play(LOCAL_AUDIO_LIGHTOPEN_RES_OK);
		   plocalasr->cmd.cmdtype=0;
		   }
		 */
		acstatus.dry = ON;
	}
	else if(dry && !strcmp(dry->valuestring, "off")){
		/*
		   if(plocalasr->cmd.cmdtype==LOCAL_CMD_OPEN_LIGHT){
		   local_asr_tts_basectrl_play(LOCAL_AUDIO_LIGHTOPEN_RES_OK);
		   plocalasr->cmd.cmdtype=0;
		   }
		 */
		acstatus.dry = OFF;
	}

	if(ptc && !strcmp(ptc->valuestring, "on")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_PTC_OPEN){
			local_asr_tts_play_ptc_open();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.ptc = ON;
	}
	else if(ptc && !strcmp(ptc->valuestring, "off")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_PTC_CLOSE){
			local_asr_tts_play_ptc_close();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.ptc = OFF;
	}

	if(buzzer && !strcmp(buzzer->valuestring, "on")){
		/*
		   if(plocalasr->cmd.cmdtype==LOCAL_CMD_OPEN_LIGHT){
		   local_asr_tts_basectrl_play(LOCAL_AUDIO_LIGHTOPEN_RES_OK);
		   plocalasr->cmd.cmdtype=0;
		   }
		 */
		acstatus.buzzer = ON;
	}
	else if(buzzer && !strcmp(buzzer->valuestring, "off")){
		/*
		   if(plocalasr->cmd.cmdtype==LOCAL_CMD_OPEN_LIGHT){
		   local_asr_tts_basectrl_play(LOCAL_AUDIO_LIGHTOPEN_RES_OK);
		   plocalasr->cmd.cmdtype=0;
		   }
		 */
		acstatus.buzzer = OFF;
	}

	if(prevent_super_cool && !strcmp(prevent_super_cool->valuestring, "on")){
		/*
		   if(plocalasr->cmd.cmdtype==LOCAL_CMD_OPEN_LIGHT){
		   local_asr_tts_basectrl_play(LOCAL_AUDIO_LIGHTOPEN_RES_OK);
		   plocalasr->cmd.cmdtype=0;
		   }
		 */
		acstatus.prevent_super_cool = ON;
	}
	else if(prevent_super_cool && !strcmp(prevent_super_cool->valuestring, "off")){
		/*
		   if(plocalasr->cmd.cmdtype==LOCAL_CMD_OPEN_LIGHT){
		   local_asr_tts_basectrl_play(LOCAL_AUDIO_LIGHTOPEN_RES_OK);
		   plocalasr->cmd.cmdtype=0;
		   }
		 */
		acstatus.prevent_super_cool = OFF;
	}

	if(prevent_straight_wind && prevent_straight_wind->valueint >= AC_STRAIGHT_WIND_NONE \
			&& prevent_straight_wind->valueint <= AC_STRAIGHT_WIND_OPEN){
		/*
		   if(plocalasr->cmd.cmdtype==LOCAL_CMD_OPEN_LIGHT){
		   local_asr_tts_basectrl_play(LOCAL_AUDIO_LIGHTOPEN_RES_OK);
		   plocalasr->cmd.cmdtype=0;
		   }
		 */
		acstatus.prevent_straight_wind = prevent_straight_wind->valueint;
	}

	if(auto_prevent_straight_wind && !strcmp(auto_prevent_straight_wind->valuestring, "on")){
		/*
		   if(plocalasr->cmd.cmdtype==LOCAL_CMD_OPEN_LIGHT){
		   local_asr_tts_basectrl_play(LOCAL_AUDIO_LIGHTOPEN_RES_OK);
		   plocalasr->cmd.cmdtype=0;
		   }
		 */
		acstatus.auto_prevent_straight_wind = ON;
	}
	else if(auto_prevent_straight_wind && !strcmp(auto_prevent_straight_wind->valuestring, "off")){
		/*
		   if(plocalasr->cmd.cmdtype==LOCAL_CMD_OPEN_LIGHT){
		   local_asr_tts_basectrl_play(LOCAL_AUDIO_LIGHTOPEN_RES_OK);
		   plocalasr->cmd.cmdtype=0;
		   }
		 */
		acstatus.auto_prevent_straight_wind = OFF;
	}

	if(self_clean && !strcmp(self_clean->valuestring, "on")){
		acstatus.self_clean = ON;
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_SELF_CLEAN){
			local_asr_tts_play_self_clean();
			plocalasr->cmd.cmdtype=0;
		}else if(plocalasr->cmd.cmdtype==LOCAL_CMD_GET_SELF_CLEAN_STATE){
			plocalasr->cmd.cmdtype=0;
		}
	}
	else if(self_clean && !strcmp(self_clean->valuestring, "off")){
		acstatus.self_clean = OFF;
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_CANCLE_SELF_CLEAN){
			local_asr_tts_play_cancle_self_clean();
			plocalasr->cmd.cmdtype=0;
		}else if(plocalasr->cmd.cmdtype==LOCAL_CMD_GET_SELF_CLEAN_STATE){
			plocalasr->cmd.cmdtype=0;
		}
	}
#if WIND_S_A
	if(wind_straight_avoid){
		acstatus.wind_straight_avoid = wind_straight_avoid->valueint;
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_WIND_STRAIGHT_OPEN && acstatus.wind_straight_avoid == AC_WIND_STRAIGHT){
			local_asr_tts_play_wind_straight_open();
			plocalasr->cmd.cmdtype=0;
		}else if(plocalasr->cmd.cmdtype==LOCAL_CMD_WIND_STRAIGHT_CLOSE && acstatus.wind_straight_avoid == AC_WIND_STRAIGHT_AVOID_OFF){
			local_asr_tts_play_wind_straight_close();
			plocalasr->cmd.cmdtype=0;
		}else if(plocalasr->cmd.cmdtype==LOCAL_CMD_WIND_AVOID_OPEN && acstatus.wind_straight_avoid == AC_WIND_AVOID){
			local_asr_tts_play_wind_avoid_open();
			plocalasr->cmd.cmdtype=0;
		}else if(plocalasr->cmd.cmdtype==LOCAL_CMD_WIND_AVOID_CLOSE && acstatus.wind_straight_avoid == AC_WIND_STRAIGHT_AVOID_OFF){
			local_asr_tts_play_wind_avoid_close();
			plocalasr->cmd.cmdtype=0;
		}
	}
#else
	if(wind_straight && !strcmp(wind_straight->valuestring, "on")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_WIND_STRAIGHT_OPEN){
			local_asr_tts_play_wind_straight_open();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.wind_straight = ON;
	}
	else if(wind_straight && !strcmp(wind_straight->valuestring, "off")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_WIND_STRAIGHT_CLOSE){
			local_asr_tts_play_wind_straight_close();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.wind_straight = OFF;
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_WIND_AVOID_CLOSE){
			local_asr_tts_play_wind_avoid_close();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.wind_avoid = OFF;
	}
	if(wind_avoid && !strcmp(wind_avoid->valuestring, "on")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_WIND_AVOID_OPEN){
			local_asr_tts_play_wind_avoid_open();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.wind_avoid = ON;
	}
	/*
	else if(wind_avoid && !strcmp(wind_avoid->valuestring, "off")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_WIND_AVOID_CLOSE){
			local_asr_tts_play_wind_avoid_close();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.wind_avoid = OFF;
	}
	*/
#endif
	if(intelligent_wind && !strcmp(intelligent_wind->valuestring, "on")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_INTELLIGENT_WIND_OPEN){
			local_asr_tts_play_intelligent_wind_open();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.intelligent_wind = ON;
	}
	else if(intelligent_wind && !strcmp(intelligent_wind->valuestring, "off")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_INTELLIGENT_WIND_CLOSE){
			local_asr_tts_play_intelligent_wind_close();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.intelligent_wind = OFF;
	}

	if(no_wind_sense && no_wind_sense->valueint >= AC_NO_WIND_SENSE_CLOSE \
			&& no_wind_sense->valueint <= AC_NO_WIND_SENSE_UNDER_CLOSE){
		acstatus.no_wind_sense = no_wind_sense->valueint;
		switch(plocalasr->cmd.cmdtype){
			case LOCAL_CMD_NO_WIND_SENSE_UPPER_OPEN:
				if(acstatus.no_wind_sense == AC_NO_WIND_SENSE_OPEN ||
						acstatus.no_wind_sense == AC_NO_WIND_SENSE_UPPER_OPEN){
					local_asr_tts_play_no_wind_sense_upper_open();
					plocalasr->cmd.cmdtype=0;
				}
				break;
			case LOCAL_CMD_NO_WIND_SENSE_UPPER_CLOSE:
				if(acstatus.no_wind_sense == AC_NO_WIND_SENSE_CLOSE ||
						acstatus.no_wind_sense == AC_NO_WIND_SENSE_UNDER_OPEN ||
						acstatus.no_wind_sense == AC_NO_WIND_SENSE_UPPER_CLOSE){
					local_asr_tts_play_no_wind_sense_upper_close();
					plocalasr->cmd.cmdtype=0;
				}
				break;
			case LOCAL_CMD_NO_WIND_SENSE_UNDER_OPEN:
				if(acstatus.no_wind_sense == AC_NO_WIND_SENSE_OPEN ||
						acstatus.no_wind_sense == AC_NO_WIND_SENSE_UNDER_OPEN){
					local_asr_tts_play_no_wind_sense_under_open();
					plocalasr->cmd.cmdtype=0;
				}
				break;
			case LOCAL_CMD_NO_WIND_SENSE_UNDER_CLOSE:
				if(acstatus.no_wind_sense == AC_NO_WIND_SENSE_CLOSE ||
						acstatus.no_wind_sense == AC_NO_WIND_SENSE_UPPER_OPEN ||
						acstatus.no_wind_sense == AC_NO_WIND_SENSE_UNDER_CLOSE){
					local_asr_tts_play_no_wind_sense_under_close();
					plocalasr->cmd.cmdtype=0;
				}
				break;
			case LOCAL_CMD_NO_WIND_SENSE_OPEN:
				if(acstatus.no_wind_sense == AC_NO_WIND_SENSE_OPEN){
					local_asr_tts_play_no_wind_sense_open();
					plocalasr->cmd.cmdtype=0;
				}
				break;
			case LOCAL_CMD_NO_WIND_SENSE_CLOSE:
				if(acstatus.no_wind_sense == AC_NO_WIND_SENSE_CLOSE){
					local_asr_tts_play_no_wind_sense_close();
					plocalasr->cmd.cmdtype=0;
				}
				break;
		}
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_GET_NO_WIND_SENSE_STATE){
			plocalasr->cmd.cmdtype=0;
		}
	}

	if(child_prevent_cold_wind && !strcmp(child_prevent_cold_wind->valuestring, "on")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_CHILD_PREVENT_COLD_WIND_OPEN){
			local_asr_tts_play_child_prevent_cold_wind_open();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.child_prevent_cold_wind = ON;
	}
	else if(child_prevent_cold_wind && !strcmp(child_prevent_cold_wind->valuestring, "off")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_CHILD_PREVENT_COLD_WIND_CLOSE){
			local_asr_tts_play_child_prevent_cold_wind_close();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.child_prevent_cold_wind = OFF;
	}

	if(strong_wind && !strcmp(strong_wind->valuestring, "on")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_STRONG_WIND_OPEN){
			local_asr_tts_play_strong_wind_open();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.strong_wind = ON;
	}
	else if(strong_wind && !strcmp(strong_wind->valuestring, "off")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_STRONG_WIND_CLOSE){
			local_asr_tts_play_strong_wind_close();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.strong_wind = OFF;
	}

	if(comfort_power_save && !strcmp(comfort_power_save->valuestring, "on")){
		/*
		   if(plocalasr->cmd.cmdtype==LOCAL_CMD_OPEN_LIGHT){
		   local_asr_tts_basectrl_play(LOCAL_AUDIO_LIGHTOPEN_RES_OK);
		   plocalasr->cmd.cmdtype=0;
		   }
		 */
		acstatus.comfort_power_save = ON;
	}
	else if(comfort_power_save && !strcmp(comfort_power_save->valuestring, "off")){
		/*
		   if(plocalasr->cmd.cmdtype==LOCAL_CMD_OPEN_LIGHT){
		   local_asr_tts_basectrl_play(LOCAL_AUDIO_LIGHTOPEN_RES_OK);
		   plocalasr->cmd.cmdtype=0;
		   }
		 */
		acstatus.comfort_power_save = OFF;
	}

	if(screen_display && !strcmp(screen_display->valuestring, "on")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_SCREEN_DISPLAY_OPEN){
			local_asr_tts_play_screen_display_open();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.screen_display = ON;
	}
	else if(screen_display && !strcmp(screen_display->valuestring, "off")){
		if(plocalasr->cmd.cmdtype==LOCAL_CMD_SCREEN_DISPLAY_CLOSE){
			local_asr_tts_play_screen_display_close();
			plocalasr->cmd.cmdtype=0;
		}
		acstatus.screen_display = OFF;
	}

	if(plocalasr->cmd.cmdtype != old_cmd){
		local_timer_stop();
	}
	pthread_mutex_unlock(&ac_status_lock);
	//cJSON * query_type =cJSON_GetObjectItem(status,"query_type");

	cJSON_Delete(root);

}
void result_03parse(char *data)
{
#if 0
	int workstatus=0;

	int hourvalue=0;


	int minvalue =0;
	int temperaturevlue=0;

	cJSON *root = cJSON_Parse(data);
	struct localasr_proc *plocalasr=&localasr;
	if(!root){
		MS_TRACE("result parse err\r\n");
		return ;
	}

	cJSON *status =cJSON_GetObjectItem(root,"status");
	if(!status){
		MS_TRACE("result status err\r\n");
		cJSON_Delete(root);
		return ;
	}
	acstatus.tips_code=0;
	acstatus.work_mode =0;
	acstatus.work_status =0;


	cJSON * work_mode =cJSON_GetObjectItem(status,"work_mode");

	cJSON * work_status =cJSON_GetObjectItem(status,"work_status");

	cJSON * tips_code = cJSON_GetObjectItem(status,"tips_code");

	if(tips_code){
		acstatus.tips_code = tips_code->valueint;
	}
	if(work_status && !strcmp(work_status->valuestring, "standby")){

		acstatus.work_status = B1_WORK_STATUS_STANDBY;
	}
	else if(work_status && !strcmp(work_status->valuestring, "work")){
		acstatus.work_status = B1_WORK_STATUS_WORK;
	}
	else if(work_status && !strcmp(work_status->valuestring, "save_power")){
		acstatus.work_status = B1_WORK_STATUS_SAVE_POWER;
	}
	else if(work_status && !strcmp(work_status->valuestring, "order")){
		acstatus.work_status = B1_WORK_STATUS_ORDER;
	}
	else if(work_status && !strcmp(work_status->valuestring, "work_finish")){
		acstatus.work_status = B1_WORK_STATUS_WORK_FINISH;
	}
	else if(work_status && !strcmp(work_status->valuestring, "three")){
		acstatus.work_status = B1_WORK_STATUS_THREE;
	}
	else if(work_status && !strcmp(work_status->valuestring, "pause")){
		acstatus.work_status = B1_WORK_STATUS_PAUSE;
	}
	else if(work_status && !strcmp(work_status->valuestring, "pause_c")){
		acstatus.work_status = B1_WORK_STATUS_PAUSE_C;
	}


	if(work_mode && !strcmp(work_mode->valuestring, "double_tube")){
		acstatus.work_mode = B1_WORK_MODE_DOUBLE_TUBE;
	}
	else if(work_mode && !strcmp(work_mode->valuestring, "hot_wind_bake")){

		acstatus.work_mode =B1_WORK_MODE_HOT_WIND_BAKE;
	}
	else if(work_mode && !strcmp(work_mode->valuestring, "double_tube_fan")){
		acstatus.work_mode =B1_WORK_MODE_DOUBLE_TUBE_FAN;
	}
	else if(work_mode && !strcmp(work_mode->valuestring, "underside_tube")){
		acstatus.work_mode =B1_WORK_MODE_UNDERSIDE_TUBE;
	}
	else if(work_mode && !strcmp(work_mode->valuestring, "double_upside_tube_fan")){
		acstatus.work_mode =B1_WORK_MODE_DOUBLE_UPSIDE_TUBE_FAN;
	}
	else if(work_mode && !strcmp(work_mode->valuestring, "core_baking")){
		acstatus.work_mode =B1_WORK_MODE_CORE_BAKING;
	}
	else if(work_mode && !strcmp(work_mode->valuestring, "total_baking")){
		acstatus.work_mode =B1_WORK_MODE_TOTAL_BAKING;
	}
	else if(work_mode && !strcmp(work_mode->valuestring, "underside_tube_hot_wind_bake")){
		acstatus.work_mode =B1_WORK_MODE_UNDERSIDE_TUBE_HOT_WIND;
	}
	else if(work_mode && !strcmp(work_mode->valuestring, "zymosis")){
		acstatus.work_mode =B1_WORK_MODE_ZYMOSIS;

	}
	else if(work_mode && !strcmp(work_mode->valuestring, "double_tube_fan")){
		acstatus.work_mode =B1_WORK_MODE_DOUBLE_TUBE_FAN;
	}





	cJSON_Delete(root);

#endif
}

void AC_timeout_handler (int signum)
{
	local_timer_stop();
	debug_local(LOG_LEVEL_DEBUG,"AC_timeout_handler\r\n");
	//if(localasr.cmd.cmdtype>0)
		//local_asr_tts_play_control_failed();
	if(localasr.cmd.cmdtype >= LOCAL_CMD_GET_NORMAL_STATE)
		localasr.cmd.cmdtype=LOCAL_CMD_GET_STATE_TIMEOUT;
	else
		localasr.cmd.cmdtype=0;
}




void local_timer_init(void)
{
	struct sigevent evp;
	if(localtimer >0)
		local_timer_stop();
	//debug_local(LOG_LEVEL_DEBUG,"local_timer_init\r\n");
	int ret;
	//evp.sigev_value.sival_ptr = ptimer;
	evp.sigev_value.sival_int = 0;
	evp.sigev_notify = SIGEV_SIGNAL;
	evp.sigev_signo = SIGUSR1;
	signal(SIGUSR1, AC_timeout_handler);
	ret = timer_create(CLOCK_REALTIME, &evp, &localtimer);
	if( ret ){
		MS_TRACE("timer_create");

	}
}
void local_timer_start(void)
{
	int ret;
	struct itimerspec ts;
	debug_local(LOG_LEVEL_DEBUG,"local_timer_start\r\n");
	ts.it_interval.tv_sec = 0;
	ts.it_interval.tv_nsec = 0;
	ts.it_value.tv_sec = 9;
	ts.it_value.tv_nsec = 0;
	ret = timer_settime(localtimer, 0, &ts, NULL);
	if( ret )
		MS_TRACE("timer_settime");

}
void local_timer_stop(void)
{
	int ret;
	struct itimerspec ts;

	ts.it_interval.tv_sec = 0;
	ts.it_interval.tv_nsec = 0;
	ts.it_value.tv_sec = 0;
	ts.it_value.tv_nsec = 0;
	ret = timer_settime(localtimer, 0, &ts, NULL);
	if( ret )
		MS_TRACE("timer_settime");

	timer_delete(localtimer);
	localtimer =0;

}
#endif
