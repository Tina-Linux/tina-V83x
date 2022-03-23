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
#include "device_asr_uart_release.h"
/*#include "midea_tts_play.h"*/
#include "app_uart.h"

#define LOGTAG 		"ASR-UART"
#define DEBUG_ASR_UART	0
/*#define factory_test_tmp 1*/

#define USE_TIMER	0

static int device_asr_uart_fd;
struct asr_uart_processor
{
	uint8_t		readThread_PrivInitOk;
	pthread_t 	readThread;
	pthread_t 	writeThread;
	sem_t 		write_sem;
	pthread_mutex_t lock;
	pthread_cond_t 	write_signal;
	cmd_uart_msg_t 	*write_request;
	uint8_t 	requst_need_reply;
	uint8_t 	not_get_last_requst_reply;
	cmd_uart_msg_t 	*reply;
	uint64_t 	last_active_ticks;
#if USE_TIMER
	os_timer_t 	timer;
#endif
	uint64_t 	last_heartbeat_ticks;
	uint64_t 	last_query_ticks;
} m_asr_uart;

static ASR_config_t m_config =
{
	.can_recog = 1, /*default on*/
	.can_play = 1, /*default on*/
	.wakeup_word = ASR_WAKEUP_WORD_KONGTIAO,
	.voice_type = ASR_PLAY_VOICE_SWEET,
	.idle_timeout = 30, /*10s kenhe 1000*/
	.play_volumn = 1.8
};

ASR_config_t* const device_asr_get_config(void)
{
	return (ASR_config_t*)&m_config;
}

static ASR_AC_config_t m_ac_config =
{
	.onoff = 0,
	.minT = 17,
	.maxT = 30
};
static AC_t m_ac;

AC_t* device_ac_get(void)
{
	return &m_ac;
}

static asr_uart_recv_controller_t recv_controller;

#define LOCK_UART()   {pthread_mutex_lock(&m_asr_uart.lock); \
	printf("lock,%s,%d\n",__FUNCTION__,__LINE__);}
#define UNLOCK_UART() {pthread_mutex_unlock(&m_asr_uart.lock); \
	printf("unlock,%s,%d\n",__FUNCTION__,__LINE__);}
//#define LOCK_UART()   {pthread_mutex_lock(&m_asr_uart.lock);}
//#define UNLOCK_UART() {pthread_mutex_unlock(&m_asr_uart.lock);}

typedef int (*cmd_processor)(cmd_uart_msg_t* frame);

typedef struct {
	uint8_t cmd;
	cmd_processor processor;
} asr_uart_processor_t;

typedef struct {
	uint8_t network_connected;
	uint8_t ota_status;
	uint8_t vender;
	uint8_t version;
	uint8_t voice_version_H;
	uint8_t downloaded_version;
	uint8_t voice_version_L;
	uint8_t download_progress;
	uint8_t wakeup_word;
	uint8_t play_volumn;
	uint8_t voice_type;
	uint8_t idle_timeout;
	uint8_t recog_setting;
	uint8_t poweron_play_l_onoff;
	uint32_t resrvd1;
	uint32_t resrvd2;
	uint32_t resrvd3;
} __attribute__((__packed__)) heartbeat_t;


static heartbeat_t heartbeat_data =
{
	.network_connected = 0,
	.vender = 2,
	.version = 1,
	.voice_version_H = 1,
	.voice_version_L = 2,
};
static cmd_uart_msg_t* heartbeat_frame = NULL;
static cmd_uart_msg_t* ac_query_frame = NULL;
extern void set_connectting_flag(int flag);

static int send_heartbeat(void)
{
	if (heartbeat_frame == NULL) {
		heartbeat_frame = device_asr_alloc_frame(ASR_UART_MSG_HEART_BEAT,
			(uint8_t*)&heartbeat_data, sizeof(heartbeat_data));
		if (heartbeat_frame == NULL) {
			log_e("heartbeat frame alloc fail");
			return ASR_AC_ERROR_MEM_FAIL;
		}
	}

	/*fill heart beat data*/
	heartbeat_data.play_volumn = m_config.play_volumn;
	heartbeat_data.wakeup_word = m_config.wakeup_word;
	heartbeat_data.idle_timeout = m_config.idle_timeout;
	heartbeat_data.voice_type = m_config.voice_type;
	heartbeat_data.recog_setting = m_config.can_recog + (m_config.can_play << 1);
	memcpy(heartbeat_frame->data, &heartbeat_data, sizeof(heartbeat_data));
	//log_info("** heart beat **");
	if (device_asr_send_frame_default_no_wait(heartbeat_frame) != 0) {
		return ASR_AC_ERROR_UART_TIMEOUT;
	}
	return ASR_AC_ERROR_NONE;
}

static int query_ac_status(void)
{
	if (ac_query_frame == NULL) {
		uint8_t data[4];
		memset(data, 0, sizeof(data));
		ac_query_frame = device_asr_alloc_frame(ASR_UART_MSG_QUERY_STATUS, \
				data, sizeof(data));
		if (ac_query_frame == NULL) {
			log_e("ac query frame alloc fail");
			return ASR_AC_ERROR_MEM_FAIL;
		}
	}
	if (device_asr_send_frame_default_no_wait(ac_query_frame) != 0) {
		return ASR_AC_ERROR_UART_TIMEOUT;
	}
	return ASR_AC_ERROR_NONE;
}

static const char* mode_str_eng[] =
{
	"none",
	"cool",
	"heat",
	"dry",
	"wind",
	"auto"
};

static const char* wind_str_eng[] =
{
	"none",
	"min",
	"mid",
	"max",
	"auto",
};

static const char* swing_str_eng[] =
{
	"Close",
	"LeftRight",
	"UpDown",
	"All"
};

int midea_ac_get_mode_by_str(char* str)
{
	int i;
	for (i = 1; i < 5; ++i) {
		if (strcmp(str, mode_str_eng[i]) == 0) {
			return i;
		}
	}
	return 0;
}

int midea_ac_get_wind_by_str(char* str)
{
	int i;
	for (i = 1; i < 5; ++i) {
		if (strcmp(str, wind_str_eng[i]) == 0) {
			return i;
		}
	}
	return 0;
}

int midea_ac_get_swing_by_str(char* str)
{
	if (strcmp(str, "off") == 0) {
		return ASR_AC_SWING_OFF;
	}
	else if (strcmp(str, "all") == 0) {
		return ASR_AC_SWING_ALL;
	}
	else if (strcmp(str, "lr") == 0)
	{
		return ASR_AC_SWING_LEFT_RIGHT;
	}
	else if (strcmp(str, "ud") == 0) {
		return ASR_AC_SWING_UP_DOWN;
	}
	return -1;
}

static void dump_ac_status(void)
{
	log_raw("\r\n-- ac status --\r\n");
	log_raw(">>   on:\t%d\r\n", m_ac.onoff);
	if (m_ac.onoff) {
		log_raw(">> mode:\t%s\r\n", mode_str_eng[m_ac.mode]);
		log_raw(">> setT:\t%d\r\n", (int)m_ac.setT);
		log_raw(">> wind:\t%s\r\n", wind_str_eng[m_ac.wind]);
		log_raw(">>swing:\t%s\r\n", swing_str_eng[m_ac.swing]);
		log_raw(">>windless:\t%d\r\n", m_ac.windless);
	}
	log_raw("---------------\r\n");
}

static int cmd_heartbeat(cmd_uart_msg_t* UART)
{
	if (UART->msgLen >= 0x20) {
		m_ac_config.onoff = UART->data[1];
		m_ac.onoff = UART->data[1];
		int16_t temp;
		temp = UART->data[4] + (uint16_t)((uint16_t)UART->data[5] << 8);
		m_ac_config.minT = temp / 100;
		temp = UART->data[6] + (uint16_t)((uint16_t)UART->data[7] << 8);
		m_ac_config.maxT = temp / 100;
		return ASR_AC_ERROR_NONE;
	}
	return ASR_AC_ERROR_UART_REPLY_ERROR;
}

static int cmd_wakeup(cmd_uart_msg_t* UART)
{
	return ASR_AC_ERROR_NONE;
}

static int power_on_played = 0;

static int cmd_poweron_play(cmd_uart_msg_t* UART)
{
	if (UART->msgLen >= 0x0A) {
		device_asr_send_frame_default_no_wait(UART);
		if (!power_on_played) {
			int plist[1];
			/*plist[0] = TTS_IND_POWER_ON_PLAY;*/
			/*midea_play_tts(plist, 1);*/
			power_on_played = 1;
			system("aplay -D hw:1,0 -f s32_le -c 2 -r 48000 /usr/share/kaiji.wav");
			sleep(2);
		}
		return ASR_AC_ERROR_NONE;
	}
	return ASR_AC_ERROR_UART_REPLY_ERROR;
}

static int cmd_poweron_play_l(cmd_uart_msg_t* UART)
{
	if (UART->msgLen >= 0x0A) {
		device_asr_send_frame_default_no_wait(UART);
		if (!power_on_played) {
			int plist[1];
			/*plist[0] = TTS_IND_POWER_ON_PLAY_L;*/
			/*midea_play_tts(plist, 1);*/
			power_on_played = 1;
		}
		return ASR_AC_ERROR_NONE;
	}
	return ASR_AC_ERROR_UART_REPLY_ERROR;
}

static int cmd_on(cmd_uart_msg_t* UART)
{
	if (UART->msgLen >= 0x10) {
		if (UART->data[0] >= ASR_AC_MODE_MIN
				&& UART->data[0] <= ASR_AC_MODE_MAX) {
			m_ac.mode = (ASR_AC_MODE)UART->data[0];
		} else {
			return ASR_AC_ERROR_INVALID_PARAM;
		}
		/*m_ac.onoff = 1;*/
		if (UART->data[2]&0x80)/*Negative*/
			m_ac.setT = -(0xffff-(UART->data[1] + (UART->data[2] << 8))+1) / 100.0f;
		else
			m_ac.setT = (UART->data[1] + (UART->data[2] << 8)) / 100.0f;
		if(UART->data[4]&0x80)/*Negative*/
			m_ac.inT = -(0xffff-(UART->data[3] + (UART->data[4] << 8))+1) / 100.0f;
		else
			m_ac.inT = (UART->data[3] + (UART->data[4] << 8)) / 100.0f;
		if(UART->data[6]&0x80)/*Negative*/
			m_ac.outT = -(0xffff-(UART->data[5] + (UART->data[6] << 8))+1) / 100.0f;
		else
			m_ac.outT = (UART->data[5] + (UART->data[6] << 8)) / 100.0f;

		if (UART->data[7] >= ASR_AC_WIND_MIN && UART->data[7] <= ASR_AC_WIND_MAX) {
			m_ac.wind = (ASR_AC_WIND)UART->data[7];
		} else {
			return ASR_AC_ERROR_INVALID_PARAM;
		}
		if (UART->data[8] == 0) {
			m_ac.play_info = PLAY_INFO_POWER_ON_EXTRA;
		} else {
			m_ac.play_info = PLAY_INFO_NONE;
		}
		return ASR_AC_ERROR_NONE;
	}
	return ASR_AC_ERROR_UART_REPLY_ERROR;
}

static int cmd_off(cmd_uart_msg_t* UART)
{
	if (UART->msgLen >= 0x0A) {
		m_ac.onoff = 0;
		return ASR_AC_ERROR_NONE;
	}
	return ASR_AC_ERROR_UART_REPLY_ERROR;
}

static int cmd_set_mode(cmd_uart_msg_t* UART)
{
	if (UART->msgLen >= 0x0A) {
		if (UART->rsvd1 == 1) {
			return ASR_AC_ERROR_POWER_ON_FIRST;
		}
		if (UART->data[0] >= ASR_AC_MODE_MIN && UART->data[0] <= ASR_AC_MODE_MAX) {
			m_ac.mode = (ASR_AC_MODE)UART->data[0];
			return ASR_AC_ERROR_NONE;
		} else {
			return ASR_AC_ERROR_INVALID_PARAM;
		}
	}
	return ASR_AC_ERROR_UART_REPLY_ERROR;
}

static int cmd_set_temp(cmd_uart_msg_t* UART)
{
	if (UART->msgLen >= 0x0A) {
		if (UART->rsvd1 == 1) {
			return ASR_AC_ERROR_POWER_ON_FIRST;
		}
		if (UART->data[2] == 1) {
			return ASR_AC_ERROR_SET_T_NOT_SUPPORT;
		}
		/*log_raw("data[0]=0x%x,data[1]=0x%x,data[2]=0x%x,data[3]=0x%x\n",
			UART->data[0],UART->data[1],UART->data[2],UART->data[3]);*/
		if (UART->data[1]&0x80)
			m_ac.setT = -(0xffff-(UART->data[0] + (UART->data[1] << 8))+1) / 100.0f;
		else
			m_ac.setT = (UART->data[0] + (UART->data[1] << 8)) / 100.0f;
		if (UART->data[3] > 0) {
			m_ac.play_info = PLAY_INFO_LOW_TEMP_ALERT;
		}
		return ASR_AC_ERROR_NONE;
	}
	return ASR_AC_ERROR_UART_REPLY_ERROR;
}

static int cmd_inc_temp(cmd_uart_msg_t* UART)
{
	if (UART->msgLen >= 0x0A) {
		if (UART->rsvd1 == 1) {
			return ASR_AC_ERROR_POWER_ON_FIRST;
		}
		if (UART->data[2] == 1) {
			return ASR_AC_ERROR_SET_T_NOT_SUPPORT;
		} else {
		}
		if (UART->data[1]&0x80)
			m_ac.setT = -(0xffff-(UART->data[0] + (UART->data[1] << 8))+1) / 100.0f;
		else
			m_ac.setT = (UART->data[0] + (UART->data[1] << 8)) / 100.0f;
		return ASR_AC_ERROR_NONE;
	}
	return ASR_AC_ERROR_UART_REPLY_ERROR;
}

static int cmd_dec_temp(cmd_uart_msg_t* UART)
{
	if (UART->msgLen >= 0x0A) {
		if (UART->rsvd1 == 1) {
			return ASR_AC_ERROR_POWER_ON_FIRST;
		}
		if (UART->data[2] == 1) {
			return ASR_AC_ERROR_SET_T_NOT_SUPPORT;
		} else {
		}
		if (UART->data[1]&0x80)
			m_ac.setT = -(0xffff-(UART->data[0] + (UART->data[1] << 8))+1) / 100.0f;
		else
			m_ac.setT = (UART->data[0] + (UART->data[1] << 8)) / 100.0f;
		return ASR_AC_ERROR_NONE;
	}
	return ASR_AC_ERROR_UART_REPLY_ERROR;
}

static int cmd_set_wind_low(cmd_uart_msg_t* UART)
{
	if (UART->msgLen >= 0x0A) {
		if (UART->rsvd1 == 1) {
			return ASR_AC_ERROR_POWER_ON_FIRST;
		}
		if (UART->data[1] == 1) {
			return ASR_AC_ERROR_SET_WIND_NOT_SUPPORT;
		} else {
			m_ac.wind = ASR_AC_WIND_SLOW;
		}
		return ASR_AC_ERROR_NONE;
	}
	return ASR_AC_ERROR_UART_REPLY_ERROR;
}
static int cmd_set_wind_mid(cmd_uart_msg_t* UART)
{
	if (UART->msgLen >= 0x0A) {
		if (UART->rsvd1 == 1) {
			return ASR_AC_ERROR_POWER_ON_FIRST;
		}
		if (UART->data[1] == 1) {
			return ASR_AC_ERROR_SET_WIND_NOT_SUPPORT;
		} else {
			m_ac.wind = ASR_AC_WIND_MID;
		}
		return ASR_AC_ERROR_NONE;
	}
	return ASR_AC_ERROR_UART_REPLY_ERROR;
}
static int cmd_set_wind_high(cmd_uart_msg_t* UART)
{
	if (UART->msgLen >= 0x0A) {
		if (UART->rsvd1 == 1) {
			return ASR_AC_ERROR_POWER_ON_FIRST;
		}
		if (UART->data[1] == 1) {
			return ASR_AC_ERROR_SET_WIND_NOT_SUPPORT;
		} else {
			m_ac.wind = ASR_AC_WIND_HIGH;
		}
		return ASR_AC_ERROR_NONE;
	}
	return ASR_AC_ERROR_UART_REPLY_ERROR;
}
static int cmd_set_wind_auto(cmd_uart_msg_t* UART)
{
	if (UART->msgLen >= 0x0A) {
		if (UART->rsvd1 == 1) {
			return ASR_AC_ERROR_POWER_ON_FIRST;
		}
		if (UART->data[1] == 1) {
			return ASR_AC_ERROR_SET_WIND_NOT_SUPPORT;
		} else {
			m_ac.wind = ASR_AC_WIND_AUTO;
		}
		return ASR_AC_ERROR_NONE;
	}
	return ASR_AC_ERROR_UART_REPLY_ERROR;
}

static int cmd_swing_on(cmd_uart_msg_t* UART)
{
	if (UART->msgLen >= 0x0A) {
		if (UART->rsvd1 == 1) {
			return ASR_AC_ERROR_POWER_ON_FIRST;
		}
		m_ac.swing = ASR_AC_SWING_ALL;
		return ASR_AC_ERROR_NONE;
	}
	return ASR_AC_ERROR_UART_REPLY_ERROR;
}

static int cmd_swing_off(cmd_uart_msg_t* UART)
{
	if (UART->msgLen >= 0x0A) {
		if (UART->rsvd1 == 1) {
			return ASR_AC_ERROR_POWER_ON_FIRST;
		}
		m_ac.swing = ASR_AC_SWING_OFF;
		return ASR_AC_ERROR_NONE;
	}
	return ASR_AC_ERROR_UART_REPLY_ERROR;
}

static int cmd_windless_on(cmd_uart_msg_t* UART)
{
	if (UART->msgLen >= 0x0A) {
		if (UART->rsvd1 == 1) {
			return ASR_AC_ERROR_POWER_ON_FIRST;
		}
		if (UART->data[1] == 1) {
			return ASR_AC_ERROR_WINDLESS_NOT_SUPPORT;
		} else {
			m_ac.windless = ASR_AC_WINDLESS_ON;
		}
		return ASR_AC_ERROR_NONE;
	}
	return ASR_AC_ERROR_UART_REPLY_ERROR;
}

static int cmd_windless_off(cmd_uart_msg_t* UART)
{
	if (UART->msgLen >= 0x0A) {
		if (UART->rsvd1 == 1) {
			return ASR_AC_ERROR_POWER_ON_FIRST;
		}
		if (UART->data[1] == 1) {
			return ASR_AC_ERROR_WINDLESS_NOT_SUPPORT;
		} else {
			m_ac.windless = ASR_AC_WINDLESS_OFF;
		}
		return ASR_AC_ERROR_NONE;
	}
	return ASR_AC_ERROR_UART_REPLY_ERROR;
}

static asr_uart_processor_t processors[] =
{
	/*
	 * reply msg
	 */
	{ASR_UART_MSG_HEART_BEAT,       cmd_heartbeat},
	{ASR_UART_MSG_WAKE_UP,          cmd_wakeup},
	{ASR_UART_MSG_ON,               cmd_on},
	{ASR_UART_MSG_OFF,              cmd_off},
	{ASR_UART_MSG_SET_MODE,         cmd_set_mode},
	{ASR_UART_MSG_SET_TEMP,         cmd_set_temp},
	{ASR_UART_MSG_INC_T_BY_1,       cmd_inc_temp},
	{ASR_UART_MSG_DEC_T_BY_1,       cmd_dec_temp},
	{ASR_UART_MSG_INC_T_BY_3,       cmd_inc_temp},
	{ASR_UART_MSG_DEC_T_BY_3,       cmd_dec_temp},
	{ASR_UART_MSG_WIND_LOW,         cmd_set_wind_low},
	{ASR_UART_MSG_WIND_MID,         cmd_set_wind_mid},
	{ASR_UART_MSG_WIND_HIGH,        cmd_set_wind_high},
	{ASR_UART_MSG_WIND_AUTO,        cmd_set_wind_auto},
	{ASR_UART_MSG_SWING_OPEN,       cmd_swing_on},
	{ASR_UART_MSG_SWING_STOP,       cmd_swing_off},
	{ASR_UART_MSG_WINDLESS_ON,      cmd_windless_on},
	{ASR_UART_MSG_WINDLESS_OFF,     cmd_windless_off},
	/*
	* push msg
	*/
};

#define PROCESSORS_COUNT (sizeof(processors) / sizeof(asr_uart_processor_t))

static void process_one_frame(uint8_t* buffer, uint8_t len)
{
	cmd_uart_msg_t* frame = (cmd_uart_msg_t*)buffer;
	int i;

	printf("asr recv:[%02X]\r\n", frame->msgId);
#if DEBUG_ASR_UART
	log_info("\r\nasr recv:[%02X]\r\n", frame->msgId);
	for (i = 0; i < frame->msgLen; ++i) {
		log_info("%02X ", ((uint8_t*)frame)[i]);
	}
	log_info("\r\n");
#endif

	int ret = ASR_AC_ERROR_NONE;
	/*process predefined callback first*/
	for (i = 0; i < PROCESSORS_COUNT; ++i) {
		if (processors[i].cmd == frame->msgId && processors[i].processor != 0) {
			ret = processors[i].processor(frame);
			m_ac.err = ret;
			break;
		}
	}

	if (frame->msgId != ASR_UART_MSG_HEART_BEAT
			&& frame->msgId != ASR_UART_MSG_QUERY_STATUS) {
		m_ac.play = frame->play;
	}

	/*then push result to user layer*/
	/*conflict with device_asr_send_frame_with_reply*/
	/*LOCK_UART()*/

	if (m_asr_uart.write_request != NULL) {
		if (m_asr_uart.write_request->msgId == frame->msgId) {
			if (m_asr_uart.reply != NULL) {
				free(m_asr_uart.reply);
				m_asr_uart.reply = NULL;
			}

			if (m_asr_uart.requst_need_reply) {
				m_asr_uart.reply = (cmd_uart_msg_t*)malloc(len);
				if (!m_asr_uart.reply) {
					log_e("mem alloc fail");
					return;
				}
				memcpy(m_asr_uart.reply, buffer, len);
			}
			sem_post(&m_asr_uart.write_sem);
			/*pthread_cond_signal(&m_asr_uart.write_signal);*/
		}
	}

	/*UNLOCK_UART()*/
}

static void reset_recv()
{
	recv_controller.state = ASR_UART_RECV_HEADER;
}

static void *reader_thread(void *arg)
{
	static int test_flag=0;
	char reveive_buf[10]={0};
	log_raw("Device reader start ...\n");
	int i=0;
	int ret;
	char  ch;
	int64_t last_read_milisec = 0;
	int64_t milisec;
	struct timeval value;
	#define READ_TIMEOUT	200
	gettimeofday(&value,NULL);
	m_asr_uart.last_heartbeat_ticks = (uint64_t)((value.tv_sec*1000 + value.tv_usec/1000)-ASR_UART_HEARTBEAT_INTERVAL);
	while (1) {
		ret = app_uart_read(device_asr_uart_fd, &ch, 1, READ_TIMEOUT);
		gettimeofday(&value,NULL);
		milisec = (uint64_t)(value.tv_sec*1000 + value.tv_usec/1000);
		test_flag++;
		if (test_flag>=10) {
			test_flag=0;
			/*log_raw("ret===========%d\n",ret);*/
		}
		if (ret == 1) {
			if (milisec - last_read_milisec > 50) {
				reset_recv();
				last_read_milisec = milisec;
			}
			//log_info("%02X ", ((uint8_t*)ch));
			/*update last active ticks when any char recved*/
			m_asr_uart.last_active_ticks = milisec;
			switch (recv_controller.state) {
			case ASR_UART_RECV_HEADER:
				if (ch == ASR_FRAME_HEADER) {
					recv_controller.state = ASR_UART_RECV_ID;
					recv_controller.recvIndex = 0;
					recv_controller.dataLen = 0;
					recv_controller.recvBuffer[recv_controller.recvIndex++] = ch;
					recv_controller.sum = ch;
				}
				break;
			case ASR_UART_RECV_ID:
				recv_controller.msgId = ch;
				recv_controller.recvBuffer[recv_controller.recvIndex++] = ch;
				recv_controller.state = ASR_UART_RECV_LEN;
				recv_controller.sum += ch;
				break;
			case ASR_UART_RECV_LEN:
				recv_controller.recvBuffer[recv_controller.recvIndex++] = ch;
				recv_controller.dataLen = ch;
				recv_controller.state = ASR_UART_RECV_SPEAKER_SWITCH;
				recv_controller.sum += ch;
				if (ch + 6 >= ASR_UART_BUFFER_SIZE) {
					log_e("recv len exceeded");
					reset_recv();
				}
				break;
			case ASR_UART_RECV_SPEAKER_SWITCH:
				recv_controller.recvBuffer[recv_controller.recvIndex++] = ch;
				recv_controller.state = ASR_UART_RECV_RESVED;
				recv_controller.sum += ch;
				break;
			case ASR_UART_RECV_RESVED:
				recv_controller.recvBuffer[recv_controller.recvIndex++] = ch;
				recv_controller.state = ASR_UART_RECV_DATA;
				recv_controller.sum += ch;
				break;
			case ASR_UART_RECV_DATA:
				recv_controller.recvBuffer[recv_controller.recvIndex++] = ch;
				recv_controller.sum += ch;
				if (recv_controller.recvIndex == recv_controller.dataLen - 1) {
					recv_controller.state = ASR_UART_RECV_SUM;
				}
				break;
			case ASR_UART_RECV_SUM:
				recv_controller.recvBuffer[recv_controller.recvIndex++] = ch;
				if (recv_controller.sum == ch) {
					process_one_frame(recv_controller.recvBuffer, recv_controller.dataLen);
				} else {
					log_e("sum error,%d,%d", recv_controller.sum, ch);
				}
			default:
				reset_recv();
				break;
			}
		}
		#if 1
		else {
			if (milisec - m_asr_uart.last_heartbeat_ticks >= ASR_UART_HEARTBEAT_INTERVAL) {
				if (!m_asr_uart.not_get_last_requst_reply) {
					if (send_heartbeat() == ASR_AC_ERROR_NONE) {
						m_asr_uart.last_heartbeat_ticks = milisec;
					} else {
						/*try next 200ms*/
						m_asr_uart.last_heartbeat_ticks = milisec - ASR_UART_HEARTBEAT_INTERVAL + 200;
					}
				} else {
					/*try next 200ms*/
					/*m_asr_uart.last_heartbeat_ticks = milisec - ASR_UART_HEARTBEAT_INTERVAL + 200;*/
					continue;
				}
			}
			/*else if (ticks - m_asr_uart.last_query_ticks >= ASR_UART_STATUS_QUERY_INTERVAL)
			if (milisec - m_asr_uart.last_query_ticks >= ASR_UART_STATUS_QUERY_INTERVAL) {
				if (query_ac_status() == ASR_AC_ERROR_NONE) {
					m_asr_uart.last_query_ticks = milisec;
				} else {
					m_asr_uart.last_query_ticks = milisec - ASR_UART_HEARTBEAT_INTERVAL + 500;
				}
			}*/
		}
		#endif
	}
	pthread_exit(NULL);
	log_raw("Device reader end ...\n");
	return NULL;
}

typedef struct {
	cmd_uart_msg_t* frame;
	uint32_t timeout;
	uint8_t need_reply;
	uint8_t retry_times;
	uint32_t start_ticks;
	void (*callback)(uint8_t success, cmd_uart_msg_t* reply, void* arg);
} uart_msg_t;


int device_asr_reader_init(void)
{
	memset(&recv_controller, 0, sizeof(recv_controller));
	reset_recv();
	m_asr_uart.readThread_PrivInitOk=0;
	return 0;
}

void device_asr_reader_start(void)
{
	if (m_asr_uart.readThread_PrivInitOk == 0) {
		log_raw("Enter\n");
		int ret = -1;

		if (pthread_mutex_init(&m_asr_uart.lock, NULL) != 0) {
			log_e("pthread_mutex_init m_asr_uart.lock error !\n");
			return;
		}
		#if 0
		if (pthread_cond_init(&m_asr_uart.write_signal,NULL) != 0) {
			log_e("pthread_cond_init m_asr_uart.write_signal error !\n");
			goto err_delete_lock;
		}
		#endif
		sem_init(&m_asr_uart.write_sem, 0, 0);
		m_asr_uart.write_request = NULL;
		m_asr_uart.requst_need_reply = 0;
		m_asr_uart.not_get_last_requst_reply = 0;
		m_asr_uart.reply = NULL;
		m_asr_uart.last_active_ticks = 0;
		m_asr_uart.last_heartbeat_ticks = 0;
		m_asr_uart.last_query_ticks = 0;
		device_asr_uart_fd=app_uart_open(device_asr_uart_fd,APP_UART);
		assert(device_asr_uart_fd);
		app_uart_Init(device_asr_uart_fd,DEVICE_ASR_UART_BAUD_RATE,0,8,1,'N');
		#ifdef factory_test_tmp
		char send_buf[]={0xAA,0x03,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0xB7};
		app_uart_write(device_asr_uart_fd, send_buf, sizeof(send_buf));
		#endif
		ret = pthread_create(&m_asr_uart.readThread, NULL, reader_thread, NULL);
		if (ret != 0)
		{
			log_e("reader_thread create error(%d), FIXME!\n", ret);
			goto err_delete_write_signal;
		}

		log_raw("Leave\n");
		m_asr_uart.readThread_PrivInitOk=1;

err_delete_write_signal:
		pthread_cond_destroy(&m_asr_uart.write_signal);
err_delete_lock:
		pthread_mutex_destroy(&m_asr_uart.lock);
	}
}

cmd_uart_msg_t* device_asr_alloc_frame(uint8_t msgId, uint8_t* data, uint8_t len)
{
	cmd_uart_msg_t* frame = (cmd_uart_msg_t*)malloc(sizeof(cmd_uart_msg_t) + len + 1);
	if (frame == NULL)
		return NULL;
	frame->header = ASR_FRAME_HEADER;
	frame->msgId = msgId;
	frame->msgLen = sizeof(cmd_uart_msg_t) + len + 1;
	frame->rsvd1 = 0;
	frame->play = 0;
	memcpy(frame->data, data, len);
	return frame;
}

int device_asr_free_frame(cmd_uart_msg_t* frame)
{
	if (frame != NULL)
		free(frame);

	return 0;
}

int device_asr_send_frame_default(cmd_uart_msg_t* frame)
{
	int ret;
	m_asr_uart.not_get_last_requst_reply = 1;
	ret = device_asr_send_frame_with_reply(frame, NULL, ASR_UART_DEFAULT_RETRY);
	return ret;
}

int device_asr_send_frame_default_with_reply(cmd_uart_msg_t* frame, cmd_uart_msg_t** reply)
{
	int ret;
	ret = device_asr_send_frame_with_reply(frame, reply, ASR_UART_DEFAULT_RETRY);
	return ret;
}

int device_asr_send_frame_default_no_wait(cmd_uart_msg_t* frame)
{
	int ret;
	ret = device_asr_send_frame_no_wait(frame);
	return ret;
}

int device_asr_send_frame(cmd_uart_msg_t* frame, uint8_t retryTimes)
{
	int ret;
	ret = device_asr_send_frame_with_reply(frame, NULL, retryTimes);
	return ret;
}

int device_asr_send_frame_no_wait(cmd_uart_msg_t* frame)
{
	uint8_t sum = 0;
	int i = 0;
	struct timeval value;
	int ret = 0;
	LOCK_UART()
	if (m_asr_uart.write_request != NULL) {
		UNLOCK_UART()
		return -1;
	}
	m_asr_uart.write_request = frame;
	m_asr_uart.requst_need_reply = 0;
	/*m_asr_uart.not_get_last_requst_reply = 0;*/
	/*calculate sum in case frame data is modified outside*/
	for (i = 0; i < frame->msgLen - 1; ++i) {
		sum += ((uint8_t*)frame)[i];
	}
	((uint8_t*)frame)[frame->msgLen - 1] = sum;
	/*send request*/
#if DEBUG_ASR_UART
	log_raw("\r\nasr send:[%02X]\r\n", frame->msgId);
	for (i = 0; i < frame->msgLen; ++i) {
		log_raw("%02X ", ((uint8_t*)frame)[i]);
	}
	log_raw("\r\n");
#endif
	/*check last active time*/
	uint64_t sleep_ticks = 0;
	if (m_asr_uart.last_active_ticks) {
		gettimeofday(&value,NULL);
		sleep_ticks = (uint64_t)(value.tv_sec*1000 + value.tv_usec/1000)
			- m_asr_uart.last_active_ticks;
		if (sleep_ticks < 50)
			usleep((50 - sleep_ticks)*1000);
	}
	ret = app_uart_write(device_asr_uart_fd, (uint8_t*)frame, frame->msgLen);
	if (ret<0) {
		UNLOCK_UART()
		return -1;
	}
	m_asr_uart.write_request = NULL;

	UNLOCK_UART()

	return 0;
}

int device_asr_send_frame_with_reply(cmd_uart_msg_t* frame,
		cmd_uart_msg_t** reply, uint8_t retryTimes)
{
	int ret;
	uint8_t sum = 0;
	int i = 0;
	struct timeval value;
	struct timespec outtime;

	LOCK_UART()
	if (m_asr_uart.write_request != NULL) {
		UNLOCK_UART()
		return -1;
	}
	m_asr_uart.write_request = frame;
	if (reply != NULL) {
		m_asr_uart.requst_need_reply = 1;
		m_asr_uart.not_get_last_requst_reply = 1;
	} else {
		m_asr_uart.requst_need_reply = 0;
		/*m_asr_uart.not_get_last_requst_reply = 0;*/
	}
	/*calculate sum in case frame data is modified outside*/
	for (i = 0; i < frame->msgLen - 1; ++i) {
		sum += ((uint8_t*)frame)[i];
	}
	((uint8_t*)frame)[frame->msgLen - 1] = sum;

	/*send request*/
	printf("asr send:[%02X]\r\n", frame->msgId);
#if DEBUG_ASR_UART
	log_info("\r\nasr send:[%02X]\r\n", frame->msgId);
	for (i = 0; i < frame->msgLen; ++i) {
		log_info("%02X ", ((uint8_t*)frame)[i]);
	}
	log_info("\r\n");
	/*log_raw("m_asr_uart.last_active_ticks==%d\r\n",m_asr_uart.last_active_ticks);*/
#endif
	/*check last active time*/
	uint64_t sleep_ticks = 0;
	if (m_asr_uart.last_active_ticks) {
		gettimeofday(&value,NULL);
		sleep_ticks = (uint64_t)(value.tv_sec*1000 + value.tv_usec/1000)
			- m_asr_uart.last_active_ticks;
		if (sleep_ticks < 50)
			usleep((50 - sleep_ticks)*1000);
	}
	/*set reply msgid timeout 2s, TBD*/
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_nsec += 2000*1000*1000;

	while (retryTimes-- > 0) {
		ret = app_uart_write(device_asr_uart_fd,
				(uint8_t*)frame, frame->msgLen);
		if (ret<0) {
			UNLOCK_UART()
			return -1;
		}
		/* modify from sem_wait to sem_timedwait */
		/*ret = sem_wait(&m_asr_uart.write_sem);*/
		ret = sem_timedwait(&m_asr_uart.write_sem, &ts);
		/*gettimeofday(&value, NULL);
		outtime.tv_sec = value.tv_sec + 1;
		outtime.tv_nsec = value.tv_usec * 1000;
		ret = pthread_cond_timedwait(&m_asr_uart.write_signal, &m_asr_uart.lock,&outtime);*/
		if (ret == 0) {
			log_raw("recv ASR reply OK\n");
			if (reply != NULL) {
				*reply = m_asr_uart.reply;
				m_asr_uart.reply = NULL;
				ret = 0;
			}
			break;
		} else {
			sem_post(&m_asr_uart.write_sem);
			/*log_e("recv ASR reply timeout");*/
			continue;
		}
	}

	m_asr_uart.write_request = NULL;
	m_asr_uart.requst_need_reply = 0;
	UNLOCK_UART()
	m_asr_uart.not_get_last_requst_reply = 0;
	/*log_raw("not_get_last_requst_reply==%d,%s,%d\r\n",
	 * m_asr_uart.not_get_last_requst_reply,__FUNCTION__,__LINE__);
	 * log_e("ret=====%d\n",ret);
	 */
	return ret;
}


int device_asr_send_msg(ASR_UART_MSG_TYPE type, int arg)
{
	#ifdef test_asr_mode
	return ASR_AC_ERROR_NONE;
	#endif
	cmd_uart_msg_t* f = NULL;
	/*cmd_uart_msg_t* reply = NULL;*/
	int ret = ASR_AC_ERROR_NONE;
	m_ac.play_info = PLAY_INFO_NONE;
	if (type == ASR_UART_MSG_TYPE_CMD) {
		#if 0//just test
		if (((int)arg == ASR_UART_MSG_WIND_FOLLOW)||((int)arg == ASR_UART_MSG_WIND_NOT_FOLLOW)
			||((int)arg == ASR_UART_MSG_OPEN_DISPLAY)||((int)arg == ASR_UART_MSG_CLOSE_DISPLAY)
			||((int)arg == ASR_UART_MSG_OPEN_AUTO_CLEAN)||((int)arg == ASR_UART_MSG_CLOSE_AUTO_CLEAN)
			)
			return ASR_AC_ERROR_NONE;
		#endif
		if (((int)arg == ASR_UART_MSG_INC_T_BY_1)
				||((int)arg == ASR_UART_MSG_INC_T_BY_3)) {
			if (m_ac.setT == m_ac_config.maxT) {
				return ASR_AC_ERROR_MAX_T_ALREADY;
			}
		}
		else if (((int)arg == ASR_UART_MSG_DEC_T_BY_1)
				||((int)arg == ASR_UART_MSG_DEC_T_BY_3)) {
			if (m_ac.setT == m_ac_config.minT) {
				return ASR_AC_ERROR_MIN_T_ALREADY;
			}
		}
		else if ((int)arg == ASR_UART_MSG_SET_TIMER_OFF) {
			if (device_ac_get()->onoff == 0) {
				return ASR_AC_ERROR_POWER_OFF_ALREADY;
			}
		}
		else if ((int)arg == ASR_UART_MSG_SET_TIMER_ON) {
			if (device_ac_get()->onoff == 1) {
				return ASR_AC_ERROR_POWER_ON_ALREADY;
			}
		}
		uint8_t data[4];
		memset(data, 0, sizeof(data));
		f = device_asr_alloc_frame((int)arg, data, sizeof(data));
		if (!f) {
			ret = ASR_AC_ERROR_MEM_FAIL;
			goto fail;
		}
		if (device_asr_send_frame_default(f) != 0) {
			ret = ASR_AC_ERROR_UART_TIMEOUT;
			goto fail;
		}
	}
	else if (type == ASR_UART_MSG_TYPE_SET_TIMER_OFF) {
		if (device_ac_get()->onoff == 0) {
			ret = ASR_AC_ERROR_POWER_OFF_ALREADY;
			goto fail;
		}
		uint8_t data[4];
		memset(data, 0, sizeof(data));
		data[0] = 1;
		data[1] = (uint8_t)arg;
		f = device_asr_alloc_frame(ASR_UART_MSG_SET_TIMER_OFF, data, sizeof(data));
		if (!f) {
			ret = ASR_AC_ERROR_MEM_FAIL;
			goto fail;
		}
		if (device_asr_send_frame_default(f) != 0) {
			ret = ASR_AC_ERROR_UART_TIMEOUT;
			goto fail;
		}
		return ASR_AC_ERROR_NONE;
	}
	else if (type == ASR_UART_MSG_TYPE_SET_TIMER_ON) {
		if (device_ac_get()->onoff == 1) {
			ret = ASR_AC_ERROR_POWER_ON_ALREADY;
			goto fail;
		}
		uint8_t data[4];
		memset(data, 0, sizeof(data));
		data[1] = (uint8_t)arg;
		f = device_asr_alloc_frame(ASR_UART_MSG_SET_TIMER_ON, data, sizeof(data));
		if (!f) {
			ret = ASR_AC_ERROR_MEM_FAIL;
			goto fail;
		}
		if (device_asr_send_frame_default(f) != 0) {
			ret = ASR_AC_ERROR_UART_TIMEOUT;
			goto fail;
		}
	}
	else if (type == ASR_UART_MSG_TYPE_SET_T) {
		/*if (m_ac.onoff == 0) {
			ret = ASR_AC_ERROR_POWER_ON_FIRST;
			goto fail;
		}
		if (m_ac.mode == ASR_AC_MODE_WIND) {
			ret = ASR_AC_ERROR_SET_T_NOT_SUPPORT;
			goto fail;
		}*/
		uint8_t data[4];
		memset(data, 0, sizeof(data));
		int t = (int)arg;
		if (t >= m_ac_config.minT && t <= m_ac_config.maxT) {
			t *= 100;
			data[0] = (uint8_t)(t & 0xFF);
			data[1] = (uint8_t)((t >> 8) & 0xFF);
			f = device_asr_alloc_frame(ASR_UART_MSG_SET_TEMP, data, sizeof(data));
			if (!f) {
				ret = ASR_AC_ERROR_MEM_FAIL;
				goto fail;
			}
			if (device_asr_send_frame_default(f) != 0) {
				ret = ASR_AC_ERROR_UART_TIMEOUT;
				goto fail;
			}
		} else {
			ret = ASR_AC_ERROR_INVALID_PARAM;
			goto fail;
		}
	}
	else if (type == ASR_UART_MSG_TYPE_SET_MODE) {
		uint8_t data[4];
		memset(data, 0, sizeof(data));
		int mode = (int)arg;
		if (mode >= ASR_AC_MODE_MIN && mode <= ASR_AC_MODE_MAX) {
			data[0] = mode;
			f = device_asr_alloc_frame(ASR_UART_MSG_SET_MODE, data, sizeof(data));
			if (!f) {
				ret = ASR_AC_ERROR_MEM_FAIL;
				goto fail;
			}
			if (device_asr_send_frame_default(f) != 0) {
				ret = ASR_AC_ERROR_UART_TIMEOUT;
				goto fail;
			}
		} else {
			ret = ASR_AC_ERROR_INVALID_PARAM;
			goto fail;
		}
	}
	else if (type == ASR_UART_MSG_TYPE_SET_WIND) {
		if (m_ac.onoff == 0) {
			ret = ASR_AC_ERROR_POWER_ON_FIRST;
			goto fail;
		}
		if (m_ac.mode == ASR_AC_MODE_AUTO || m_ac.mode == ASR_AC_MODE_DRY) {
			ret = ASR_AC_ERROR_SET_WIND_NOT_SUPPORT;
			goto fail;
		}
		uint8_t data[4];
		memset(data, 0, sizeof(data));
		int wind = (int)arg;
		int msgId = ASR_UART_MSG_NONE;
		if (wind == ASR_AC_WIND_SLOW) {
			msgId = ASR_UART_MSG_WIND_LOW;
		} else if (wind == ASR_AC_WIND_MID) {
			msgId = ASR_UART_MSG_WIND_MID;
		} else if (wind == ASR_AC_WIND_HIGH) {
			msgId = ASR_UART_MSG_WIND_HIGH;
		} else if (wind == ASR_AC_WIND_AUTO) {
			msgId = ASR_UART_MSG_WIND_AUTO;
		} else if (wind == ASR_AC_WIND_INC) {
			if (m_ac.wind == ASR_AC_WIND_SLOW) {
				msgId = ASR_UART_MSG_WIND_MID;
			} else if (m_ac.wind == ASR_AC_WIND_MID) {
				msgId = ASR_UART_MSG_WIND_HIGH;
			} else if(m_ac.wind == ASR_AC_WIND_HIGH) {
				return ASR_AC_ERROR_MAX_WIND_ALREADY;
			}
		}
		else if (wind == ASR_AC_WIND_DEC) {
			if (m_ac.wind == ASR_AC_WIND_MID) {
				msgId = ASR_UART_MSG_WIND_LOW;
			} else if(m_ac.wind == ASR_AC_WIND_HIGH) {
				msgId = ASR_UART_MSG_WIND_MID;
			} else if(m_ac.wind == ASR_AC_WIND_SLOW) {
				return ASR_AC_ERROR_MIN_WIND_ALREADY;
			}
		}
		if (msgId != ASR_UART_MSG_NONE) {
			f = device_asr_alloc_frame(msgId, data, sizeof(data));
			if (!f) {
				ret = ASR_AC_ERROR_MEM_FAIL;
				goto fail;
			}
			if (device_asr_send_frame_default(f) != 0) {
				ret = ASR_AC_ERROR_UART_TIMEOUT;
				goto fail;
			}
		} else {
			ret = ASR_AC_ERROR_INVALID_PARAM;
			goto fail;
		}
	}
fail:
	if (f)
	{
		device_asr_free_frame(f);
	}

	if (ret == ASR_AC_ERROR_NONE) {
		if (!m_ac.play) {
			ret = ASR_AC_ERROR_NO_PLAY;
		} else {
			ret = m_ac.err;
			m_ac.err = ASR_AC_ERROR_NONE;
		}
	}

	return ret;
}
