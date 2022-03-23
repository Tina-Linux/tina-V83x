#ifndef __LOCAL_ASR_SPEAKER_H__
#define __LOCAL_ASR_SPEAKER_H__

#define LOCAL_RADIO_PATH "/mnt/app/AC/wav%d.mp3"
#define LOCAL_RADIO_AGAININPUT "/mnt/app/AC/unknowinput.mp3"
#define LOCAL_RADIO_UNKOWNINPUTBYE "/mnt/app/AC/unknowinputbye.mp3"
#define LOCAL_RADIO_DISCONNECT "/mnt/app/tts/disconnect.mp3"

#define TEMP_MIN 17
#define TEMP_MAX 30
#define VOL_MAX 90
#define VOL_MIN 70
#define VOL_1 VOL_MIN
#define VOL_2 75
#define VOL_3 80
#define VOL_4 85
#define VOL_5 VOL_MAX
#define AUDIO_TEMP_START 34
#define AUDIO_TEMP_POINT_START 85
#define AUDIO_SMALL_TEMP 22
#define AUDIO_SET_UP 6
#define AUDIO_AC_WILL 143
#define AUDIO_NUMBER_ONE 119
#define AUDIO_AFTER_30_MIN 144
#define AUDIO_POWER_ON 141
#define AUDIO_POWER_OFF 142
#define AUDIO_HOUR 145
#define AUDIO_AUTO_WIND_SPEED 12
#define AUDIO_AUTO_MODE 133
#define AUDIO_COOL_MODE 134
#define AUDIO_HEAT_MODE 135
#define AUDIO_FAN_MODE 136
#define AUDIO_DRY_MODE 137
#define AUDIO_NUMBER(num) (num == 2? 230 : num + AUDIO_NUMBER_ONE - 1)
#define AUDIO_TEMP(temp) (temp - TEMP_MIN + AUDIO_TEMP_START)
#define AUDIO_TEMP_POINT(temp) (temp - TEMP_MIN + AUDIO_TEMP_POINT_START)


#define LOCAL_AUDIO_POWER_ON_RES_OK			{3}
#define LOCAL_AUDIO_POWER_OFF_RES_OK			{4}
#define LOCAL_AUDIO_PLS_TURN_ON_AC			{5}
#define LOCAL_AUDIO_WIND_SPEED_1_PCT			{166, 199}
#define LOCAL_AUDIO_WIND_SPEED_20_PCT			{166, 200}
#define LOCAL_AUDIO_WIND_SPEED_40_PCT			{166, 201}
#define LOCAL_AUDIO_WIND_SPEED_60_PCT			{166, 202}
#define LOCAL_AUDIO_WIND_SPEED_80_PCT			{166, 203}
#define LOCAL_AUDIO_WIND_SPEED_100_PCT			{166, 198}
#define LOCAL_AUDIO_SET_MIN_WIND_SPEED			{166, 9}
#define LOCAL_AUDIO_SET_MID_WIND_SPEED			{166, 10}
#define LOCAL_AUDIO_SET_MAX_WIND_SPEED			{166, 11}
#define LOCAL_AUDIO_SET_AUTO_WIND_SPEED			{166, 12}
#define LOCAL_AUDIO_SET_WIND_SPEED_IN_WRONG_MODE	{13}
#define LOCAL_AUDIO_WIND_SPEED_MAX			{14, 11}
#define LOCAL_AUDIO_WIND_SPEED_MIN			{14, 9}
#define LOCAL_AUDIO_SET_TEMP_IN_WRONG_MODE		{130}
#define LOCAL_AUDIO_TEMP_MAX				{14, 131}
#define LOCAL_AUDIO_TEMP_MIN				{14, 132}
#define LOCAL_AUDIO_AUTO_MODE				{6, 133}
#define LOCAL_AUDIO_COOL_MODE				{6, 134}
#define LOCAL_AUDIO_HEAT_MODE				{6, 135}
#define LOCAL_AUDIO_FAN_MODE				{6, 136}
#define LOCAL_AUDIO_DRY_MODE				{6, 137}
#define LOCAL_AUDIO_WIND_SWING_LR_OPEN			{8, 140}
#define LOCAL_AUDIO_WIND_SWING_LR_CLOSE			{7, 140}
#define LOCAL_AUDIO_WIND_SWING_UD_OPEN			{8, 139}
#define LOCAL_AUDIO_WIND_SWING_UD_CLOSE			{7, 139}
#define LOCAL_AUDIO_CANCLE_POWER_OFF_TIMER		{146}
#define LOCAL_AUDIO_CANCLE_POWER_ON_TIMER		{147}
#define LOCAL_AUDIO_SELF_CLEAN				{205, 206, 229}
#define LOCAL_AUDIO_CANCLE_SELF_CLEAN			{7, 149}
#define LOCAL_AUDIO_CLEANING_SELF			{207}
#define LOCAL_AUDIO_PTC_OPEN				{8, 151}
#define LOCAL_AUDIO_PTC_CLOSE				{7, 151}
#define LOCAL_AUDIO_PTC_IN_WRONG_MODE			{151, 152}
#define LOCAL_AUDIO_WIND_STRAIGHT_OPEN			{8, 216}
#define LOCAL_AUDIO_WIND_STRAIGHT_CLOSE			{7, 216}
#define LOCAL_AUDIO_WIND_AVOID_OPEN			{8, 217}
#define LOCAL_AUDIO_WIND_AVOID_CLOSE			{7, 217}
#define LOCAL_AUDIO_INTELLIGENT_WIND_OPEN		{8, 218}
#define LOCAL_AUDIO_INTELLIGENT_WIND_CLOSE		{7, 218}
#define LOCAL_AUDIO_NO_WIND_SENSE_UPPER_OPEN		{8, 219}
#define LOCAL_AUDIO_NO_WIND_SENSE_UPPER_CLOSE		{7, 219}
#define LOCAL_AUDIO_NO_WIND_SENSE_UNDER_OPEN		{8, 220}
#define LOCAL_AUDIO_NO_WIND_SENSE_UNDER_CLOSE		{7, 220}
#define LOCAL_AUDIO_NO_WIND_SENSE_OPEN			{8, 221}
#define LOCAL_AUDIO_NO_WIND_SENSE_CLOSE			{7, 221}
#define LOCAL_AUDIO_NO_WIND_SENSE_IN_WRONG_MODE		{222, 154}
#define LOCAL_AUDIO_CHILD_PREVENT_COLD_WIND_OPEN	{8, 223}
#define LOCAL_AUDIO_CHILD_PREVENT_COLD_WIND_CLOSE	{7, 223}
#define LOCAL_AUDIO_CHILD_PREVENT_IN_WRONG_MODE		{223, 154}
#define LOCAL_AUDIO_STRONG_WIND_OPEN			{8, 150}
#define LOCAL_AUDIO_STRONG_WIND_CLOSE			{7, 150}
#define LOCAL_AUDIO_STRONG_WIND_IN_WRONG_MODE		{150, 148}
#define LOCAL_AUDIO_SCREEN_DISPLAY_OPEN			{8, 156}
#define LOCAL_AUDIO_SCREEN_DISPLAY_CLOSE		{7, 156}
#define LOCAL_AUDIO_VOL_MAX				{6, 163}
#define LOCAL_AUDIO_VOL_MIN				{6, 164}
#define LOCAL_AUDIO_VOL_ALREADY_MAX			{14, 163}
#define LOCAL_AUDIO_VOL_ALREADY_MIN			{14, 164}
#define LOCAL_AUDIO_VOL_1				{155, 200}
#define LOCAL_AUDIO_VOL_2				{155, 201}
#define LOCAL_AUDIO_VOL_3				{155, 202}
#define LOCAL_AUDIO_VOL_4				{155, 203}
#define LOCAL_AUDIO_VOL_5				{155, 198}
#define LOCAL_AUDIO_CONTROL_FAILED			{172}

typedef enum MS_ENUM_AC_SWITCH{
	OFF = 0,
	ON,
}ms_enum_ac_switch;

typedef enum MS_ENUM_AC_MODE{
	AC_MODE_HEAT = 0,
	AC_MODE_COOL,
	AC_MODE_AUTO,
	AC_MODE_DRY,
	AC_MODE_FAN,
	AC_MODE_MAX,
}ms_enum_ac_mode;

typedef enum MS_ENUM_AC_STRAIGHT_WIND{
	AC_STRAIGHT_WIND_NONE = 0,
	AC_STRAIGHT_WIND_CLOSE,
	AC_STRAIGHT_WIND_OPEN,
}ms_enum_ac_straight_wind;

typedef enum MS_ENUM_AC_NO_WIND_SENSE{
	AC_NO_WIND_SENSE_CLOSE = 0,
	AC_NO_WIND_SENSE_OPEN,
	AC_NO_WIND_SENSE_UPPER_OPEN,
	AC_NO_WIND_SENSE_UNDER_OPEN,
	AC_NO_WIND_SENSE_UPPER_CLOSE,
	AC_NO_WIND_SENSE_UNDER_CLOSE,
}ms_enum_ac_no_wind_sense;

typedef enum MS_ENUM_AC_WIND_STRAIGHT_AVOID{
	AC_WIND_STRAIGHT_AVOID_OFF = 0,
	AC_WIND_STRAIGHT,
	AC_WIND_AVOID,
}ms_enum_ac_wind_s_a;
enum{
	LOCAL_AUDIO_LESSWATER_RES_OK,
	LOCAL_AUDIO_PLINPUTTEMPERATURETIME_RES_OK,
	LOCAL_AUDIO_NOINPUTTIME_RES_OK,
	LOCAL_AUDIO_PLINPUTTEMPERATURE_RES_OK,
	LOCAL_AUDIO_AGAININPUT_RES_OK,
	LOCAL_AUDIO_UNKOWNINPUTBYE_RES_OK,
	LOCAL_AUDIO_CMDTIMEOUT_RES_OK,
	LOCAL_AUDIO_NOWORK_RES_OK,
	LOCAL_AUDIO_ENDWORK_RES_OK,
	LOCAL_AUDIO_HAODE_RES_OK,
	LOCAL_AUDIO_WIFI_AP_RES_OK
};

enum {
   LOCAL_TTS_RES_COMMON,
   LOCAL_TTS_RES_DEVICE_OPEN,
   LOCAL_TTS_RES_DEVICE_CLOSE,
   LOCAL_TTS_RES_PAUSE_WORK,
   LOCAL_TTS_RES_RESUME_WORK,
   LOCAL_TTS_RES_STOP_WORK,
   LOCAL_TTS_RES_INQUERY_TIME,
   LOCAL_TTS_RES_PREHEAT_START,
   LOCAL_TTS_RES_PREHEAT_STOP,
   LOCAL_TTS_RES_BAKE_START,
   LOCAL_TTS_RES_UPDOWNBAKE_START,
   LOCAL_TTS_RES_HOTWINBAKE_START,
   LOCAL_TTS_RES_LIGHT_OPEN,
   LOCAL_TTS_RES_LIGHT_CLOSE,
   LOCAL_TTS_RES_ZYMOSIS_START,
   LOCAL_TTS_RES_ZYMOSIS_STOP,
   LOCAL_TTS_RES_VOL_ADJUST
};
enum {
	LOCAL_AUDIO_OPEN_RES_OK,
	LOCAL_AUDIO_CLOSE_RES_OK,
	LOCAL_AUDIO_PAUSE_RES_OK,
	LOCAL_AUDIO_RESUME_RES_OK,
	LOCAL_AUDIO_STOP_RES_OK,
	LOCAL_AUDIO_LIGHTOPEN_RES_OK,
	LOCAL_AUDIO_LIGHTCLOSE_RES_OK,
	LOCAL_AUDIO_CANCLEPREHEAT_RES_OK,
	LOCAL_AUDIO_ZYMOSISCLOSE_RES_OK
};
	enum{
		LOCAL_AUDIO_BEGIN_UPDOWN_BAKE_RES_WAV,
		LOCAL_AUDIO_BEGIN_HOT_BAKE_RES_WAV,
		LOCAL_AUDIO_BEGIN_PREHEAT_RES_WAV,
		LOCAL_AUDIO_BEGIN_PREHEAT_SUFFIX_RES_WAV,
		LOCAL_AUDIO_BEGIN_ZYMOSIS_RES_WAV,
		LOCAL_AUDIO_PREHEAT_PREFIX_RES_WAV,
		LOCAL_AUDIO_SWMODEERR_PREFIX_RES_WAV

		};
	enum{
		LOCAL_AUDIO_MIN_RES_WAV,
		LOCAL_AUDIO_HOUR_RES_WAV,
		LOCAL_AUDIO_DU_RES_WAV,
		LOCAL_AUDIO_SHI_RES_WAV,
		LOCAL_AUDIO_BAI_RES_WAV
		};

enum{
	LOCAL_AUDIO_DOUBLE_TUBE_MODE_RES_WAV,
	LOCAL_AUDIO_HOT_WIND_MODE_RES_WAV,
	LOCAL_AUDIO_UNDERSIDE_TUDE_MODE_RES_WAV,
	LOCAL_AUDIO_CORE_BAKING_MODE_RES_WAV,
	LOCAL_AUDIO_ZYMOSIS_MODE_RES_WAV,
};

void local_asr_tts_play_power_on(int mode, int temp, float small_temp);
void local_asr_tts_play_temp(int temp, float small_temp);
void local_asr_tts_play_power_status(ms_enum_ac_switch on);
void local_asr_tts_play_pls_turn_on_ac(void);
void local_asr_tts_play_wind_speed_1_pct(void);
void local_asr_tts_play_wind_speed_20_pct(void);
void local_asr_tts_play_wind_speed_40_pct(void);
void local_asr_tts_play_wind_speed_60_pct(void);
void local_asr_tts_play_wind_speed_80_pct(void);
void local_asr_tts_play_wind_speed_100_pct(void);
void local_asr_tts_play_wind_speed(int speed);
void local_asr_tts_play_wind_speed_in_wrong_mode(void);
void local_asr_tts_play_wind_speed_max(void);
void local_asr_tts_play_wind_speed_min(void);
void local_asr_tts_play_set_temp_in_wrong_mode(void);
void local_asr_tts_play_temp_max(void);
void local_asr_tts_play_temp_min(void);
void local_asr_tts_play_cool_mode(void);
void local_asr_tts_play_set_auto_mode(void);
void local_asr_tts_play_set_cool_mode(void);
void local_asr_tts_play_set_heat_mode(void);
void local_asr_tts_play_set_fan_mode(void);
void local_asr_tts_play_set_dry_mode(void);
void local_asr_tts_play_wind_swing_lr_open(void);
void local_asr_tts_play_wind_swing_lr_close(void);
void local_asr_tts_play_wind_swing_ud_open(void);
void local_asr_tts_play_wind_swing_ud_close(void);
void local_asr_tts_play_power_onoff_time(ms_enum_ac_switch onoff, int time_value);
void local_asr_tts_play_cancle_power_off_timer(void);
void local_asr_tts_play_cancle_power_on_timer(void);
void local_asr_tts_play_self_clean(void);
void local_asr_tts_play_cancle_self_clean(void);
void local_asr_tts_play_cleaning_self(void);
void local_asr_tts_play_ptc_open(void);
void local_asr_tts_play_ptc_in_wrong_mode(void);
void local_asr_tts_play_ptc_close(void);
void local_asr_tts_play_wind_straight_open(void);
void local_asr_tts_play_wind_straight_close(void);
void local_asr_tts_play_wind_avoid_open(void);
void local_asr_tts_play_wind_avoid_close(void);
void local_asr_tts_play_intelligent_wind_open(void);
void local_asr_tts_play_intelligent_wind_close(void);
void local_asr_tts_play_no_wind_sense_upper_open(void);
void local_asr_tts_play_no_wind_sense_upper_close(void);
void local_asr_tts_play_no_wind_sense_under_open(void);
void local_asr_tts_play_no_wind_sense_under_close(void);
void local_asr_tts_play_no_wind_sense_open(void);
void local_asr_tts_play_no_wind_sense_close(void);
void local_asr_tts_play_no_wind_sense_in_wrong_mode(void);
void local_asr_tts_play_child_prevent_cold_wind_open(void);
void local_asr_tts_play_child_prevent_cold_wind_close(void);
void local_asr_tts_play_child_prevent_in_wrong_mode(void);
void local_asr_tts_play_strong_wind_open(void);
void local_asr_tts_play_strong_wind_close(void);
void local_asr_tts_play_strong_wind_in_wrong_mode(void);
void local_asr_tts_play_screen_display_open(void);
void local_asr_tts_play_screen_display_close(void);
void local_asr_tts_play_vol(int vol);
void local_asr_tts_play_control_failed(void);
void local_asr_tts_play(char *tts, int tts_num);
int local_asr_tts_play_nocmd(void);
int local_asr_tts_play_nocmdend(void);
/**
******************************************************************************************************************
**** @brief request to play network_config_fail.mp3
****
**** @param buf             - void
****
**** @retval
******************************************************************************************************************
*/
int local_asr_tts_play_networkfail(void);

#endif
