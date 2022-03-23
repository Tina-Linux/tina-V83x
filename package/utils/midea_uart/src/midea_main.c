#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "device_asr_uart_release.h"
#include "log.h"
#include "midea_main.h"
#include <uvoice_config.h>

static const char *music_name[] = {
	"/usr/share/0.wav",
	"/usr/share/1.wav",
	"/usr/share/2.wav",
	"/usr/share/3.wav",
	"/usr/share/4.wav",
	"/usr/share/5.wav",
	"/usr/share/6.wav",
	"/usr/share/7.wav",
	"/usr/share/8.wav",
	"/usr/share/9.wav",
	"/usr/share/10.wav",
	"/usr/share/11.wav",
	"/usr/share/12.wav",
	"/usr/share/13.wav",
	"/usr/share/14.wav",
	"/usr/share/15.wav",
	"/usr/share/16.wav",
	"/usr/share/17.wav",
	"/usr/share/18.wav",
	"/usr/share/19.wav",
	"/usr/share/20.wav",
	"/usr/share/21.wav",
	"/usr/share/22.wav",
	"/usr/share/23.wav",
	"/usr/share/24.wav",
	"/usr/share/25.wav",
	"/usr/share/26.wav",
	"/usr/share/27.wav",
	"/usr/share/28.wav",
	"/usr/share/29.wav",
	"/usr/share/30.wav",
	"/usr/share/31.wav",
	"/usr/share/32.wav",
	"/usr/share/33.wav",
	"/usr/share/34.wav",
};

int device_asr_cmd_process(ASR_FRAME_CMD_ID asr_cmd_id)
{
	int asr_param = 0;
	ASR_AC_ERROR status = ASR_AC_ERROR_NONE;
	ASR_UART_MSG_TYPE asr_msg_type;

	switch(asr_cmd_id) {
		case ASR_UART_CMD_WAKE_UP:
			asr_msg_type = ASR_UART_MSG_TYPE_CMD;
			asr_param = ASR_UART_MSG_WAKE_UP;
			break;
		case ASR_UART_CMD_CLOSE_RECOG:
			asr_msg_type = ASR_UART_MSG_TYPE_CMD;
			asr_param = ASR_UART_MSG_CLOSE_RECOG;
			break;
		case ASR_UART_CMD_ON:
			asr_msg_type = ASR_UART_MSG_TYPE_CMD;
			asr_param = ASR_UART_MSG_ON;
			break;
		case ASR_UART_CMD_OFF:
			asr_msg_type = ASR_UART_MSG_TYPE_CMD;
			asr_param = ASR_UART_MSG_OFF;
			break;
		/*set mode*/
		case ASR_UART_CMD_SET_MODE_AUTO:
			asr_msg_type = ASR_UART_MSG_TYPE_SET_MODE;
			asr_param = ASR_AC_MODE_AUTO;
			break;
		case ASR_UART_CMD_SET_MODE_COOL:
			asr_msg_type = ASR_UART_MSG_TYPE_SET_MODE;
			asr_param = ASR_AC_MODE_COOL;
			break;
		case ASR_UART_CMD_SET_MODE_DRY:
			asr_msg_type = ASR_UART_MSG_TYPE_SET_MODE;
			asr_param = ASR_AC_MODE_DRY;
			break;
		case ASR_UART_CMD_SET_MODE_HEAT:
			asr_msg_type = ASR_UART_MSG_TYPE_SET_MODE;
			asr_param = ASR_AC_MODE_HEAT;
			break;
		case ASR_UART_CMD_SET_MODE_WIND:
			asr_msg_type = ASR_UART_MSG_TYPE_SET_MODE;
			asr_param = ASR_AC_MODE_WIND;
			break;
		/*set temperature*/
		case ASR_UART_CMD_SET_TEMP_17:
		case ASR_UART_CMD_SET_TEMP_18:
		case ASR_UART_CMD_SET_TEMP_19:
		case ASR_UART_CMD_SET_TEMP_20:
		case ASR_UART_CMD_SET_TEMP_21:
		case ASR_UART_CMD_SET_TEMP_22:
		case ASR_UART_CMD_SET_TEMP_23:
		case ASR_UART_CMD_SET_TEMP_24:
		case ASR_UART_CMD_SET_TEMP_25:
		case ASR_UART_CMD_SET_TEMP_26:
		case ASR_UART_CMD_SET_TEMP_27:
		case ASR_UART_CMD_SET_TEMP_28:
		case ASR_UART_CMD_SET_TEMP_29:
		case ASR_UART_CMD_SET_TEMP_30:
			asr_msg_type = ASR_UART_MSG_TYPE_SET_T;
			asr_param = (int)asr_cmd_id + 8;
			break;
		case ASR_UART_CMD_INC_T_BY_1:
			asr_msg_type = ASR_UART_MSG_TYPE_CMD;
			asr_param = ASR_UART_MSG_INC_T_BY_1;
			break;
		/*too cold*/
		case ASR_UART_CMD_INC_T_BY_3:
			asr_msg_type = ASR_UART_MSG_TYPE_CMD;
			asr_param = ASR_UART_MSG_INC_T_BY_3;
			break;
		case ASR_UART_CMD_DEC_T_BY_1:
			asr_msg_type = ASR_UART_MSG_TYPE_CMD;
			asr_param = ASR_UART_MSG_DEC_T_BY_1;
			break;
		/*too hot*/
		case ASR_UART_CMD_DEC_T_BY_3:
			asr_msg_type = ASR_UART_MSG_TYPE_CMD;
			asr_param = ASR_UART_MSG_DEC_T_BY_1;
			break;
		/*set wind */
		case ASR_UART_CMD_WIND_SLOW:
			asr_msg_type = ASR_UART_MSG_TYPE_SET_WIND;
			asr_param = ASR_AC_WIND_SLOW;
			break;
		case ASR_UART_CMD_WIND_MID:
			asr_msg_type = ASR_UART_MSG_TYPE_SET_WIND;
			asr_param = ASR_AC_WIND_MID;
			break;
		case ASR_UART_CMD_WIND_HIGH:
			asr_msg_type = ASR_UART_MSG_TYPE_SET_WIND;
			asr_param = ASR_AC_WIND_HIGH;
			break;
		case ASR_UART_CMD_WIND_AUTO:
			asr_msg_type = ASR_UART_MSG_TYPE_SET_WIND;
			asr_param = ASR_AC_WIND_AUTO;
			break;
		case ASR_UART_CMD_WINDLESS_ON:
			asr_msg_type = ASR_UART_MSG_TYPE_CMD;
			asr_param = ASR_UART_MSG_WINDLESS_ON;
			break;
		case ASR_UART_CMD_WINDLESS_OFF:
			asr_msg_type = ASR_UART_MSG_TYPE_CMD;
			asr_param = ASR_UART_MSG_WINDLESS_OFF;
			break;
		/*ASR on, play notify info*/
		case ASR_UART_CMD_FIRST_OPEN_INFO:
		/*15s idle timeout, play notify info*/
		case ASR_UART_CMD_IDLE_TIMEOUT_INFO:
			return asr_cmd_id;
		default:
			log_e("Invalid ASR Command!\n");
			return -300;
	}
	status = device_asr_send_msg(asr_msg_type, asr_param);
	return status;
}

int main(int32_t argc, char** argv)
{
	int8_t *record_buf;
	int asr_id = 0x1fff;
	int ret;
	FILE *file_dsc;
	char *filename;
	char playback_name[128];
	int all_buffers_size = 32000*2*16*3;
	int8_t data_ctl = 1; /*1:uvoice kws detect 0:direct record*/

	struct data_config *set_config;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s {file.wav | --}\n\n"
			"Use -- for filename to send raw PCM to stdout\n", argv[0]);
		return 1;
	}

	if (strcmp(argv[1], "--") == 0) {
		file_dsc = NULL;
	} else {
		file_dsc= fopen(argv[1], "wb");
		if (!file_dsc) {
			fprintf(stderr, "Unable to create file '%s'\n", argv[1]);
			return 1;
		}
	}

	log_info(">====Welcome to Media voice asistant=====<");

	set_config = malloc(sizeof(struct data_config));
	set_config->card = 1;
	set_config->device = 0;
	set_config->channels = 2;
	set_config->rate = 16000;
	set_config->period_size = 1024;
	set_config->period_count = 4;
	set_config->capture_time = UINT_MAX;
	set_config->format = PCM_FORMAT_S16_LE;
	set_config->play_flag = 0;
	set_config->file = file_dsc;

	device_asr_reader_init();
	device_asr_reader_start();

	uvoice_init(set_config, all_buffers_size);
	creat_record(set_config);
	if (data_ctl == 1) {
		creat_uvoice_handle(set_config);
	} else {
		record_buf = get_data(set_config);
	}

	while(1) {
		pthread_mutex_lock(&set_config->player_queue->play_mutex);
		pthread_cond_wait(&set_config->player_queue->player,
			&set_config->player_queue->play_mutex);
		asr_id = set_config->player_queue->data[0];
		pthread_mutex_unlock(&set_config->player_queue->play_mutex);

		log_info("==> recognaze flag = %d\n", asr_id);
		ret = device_asr_cmd_process(asr_id);
		switch(ret) {
			case ASR_AC_ERROR_NONE:
			case ASR_AC_ERROR_MIN_T_ALREADY:
			case ASR_AC_ERROR_MAX_T_ALREADY:
			case ASR_AC_ERROR_MIN_WIND_ALREADY:
			case ASR_AC_ERROR_MAX_WIND_ALREADY:
			case ASR_AC_ERROR_POWER_ON_FIRST:
			case ASR_AC_ERROR_SET_T_NOT_SUPPORT:
			case ASR_AC_ERROR_SET_WIND_NOT_SUPPORT:
			case ASR_AC_ERROR_WINDLESS_NOT_SUPPORT:
			case ASR_AC_ERROR_POWER_ON_ALREADY:
			case ASR_AC_ERROR_POWER_OFF_ALREADY:
			case ASR_AC_ERROR_STRONG_MODE_NOT_SUPPORT:
			case ASR_AC_ERROR_ELECTRIC_AUX_HEAT_MODE_NOT_SUPPORT:
			case ASR_AC_ERROR_SLEEP_MODE_NOT_SUPPORT:
			case ASR_AC_ERROR_SET_ANTI_BLOW_NOT_SUPPORT:
			case ASR_AC_ERROR_SET_ANTI_CLOD_WIND_NOT_SUPPORT:
			case ASR_AC_ERROR_STRONG_WIND_NOT_SUPPORT:
			case ASR_AC_ERROR_NOT_WORKING:
			case ASR_AC_ERROR_NETWORK:
			case ASR_AC_ERROR_UART_TIMEOUT:
			case ASR_AC_ERROR_INVALID_PARAM:
			case ASR_AC_ERROR_MEM_FAIL:
			case ASR_AC_ERROR_UART_REPLY_ERROR:
			case ASR_AC_ERROR_NO_PLAY:
				break;
			default:
				break;
		}

		if(asr_id <= 34) {
			set_config->play_flag = 1;
			strcpy(playback_name, (char*)music_name[asr_id]);
			playback(&playback_name, set_config);
		}
	};

	destory_all();

	return 0;
}
