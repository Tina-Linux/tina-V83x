#ifndef APP_DEVICE_INCLUDE_DEVICE_ASR_UART_H_
#define APP_DEVICE_INCLUDE_DEVICE_ASR_UART_H_

#define APP_UART 			"/dev/ttyS0"
#define DEVICE_ASR_UART_BAUD_RATE 9600
#define ASR_UART_BUFFER_SIZE 		64
#define ASR_FRAME_HEADER		0xAA
#define ASR_UART_HEARTBEAT_INTERVAL	3000
#define ASR_UART_STATUS_QUERY_INTERVAL	5000
#define ASR_FRAME_REPLY_TIMEOUT		1000
#define ASR_UART_DEFAULT_RETRY		6
#define	 FALSE				0
#define	 TRUE				1

typedef struct {
	uint8_t header;
	uint8_t msgId;
	uint8_t msgLen;
	uint8_t play;
	uint8_t rsvd1;
	uint8_t data[0];
} __attribute__((packed)) cmd_uart_msg_t;

typedef enum {
	ASR_UART_RECV_HEADER,
	ASR_UART_RECV_ID,
	ASR_UART_RECV_LEN,
	ASR_UART_RECV_SPEAKER_SWITCH,
	ASR_UART_RECV_RESVED,
	ASR_UART_RECV_DATA,
	ASR_UART_RECV_SUM
}asr_uart_recv_state;

typedef struct{
	asr_uart_recv_state state;
	uint8_t recvBuffer[ASR_UART_BUFFER_SIZE];
	uint8_t recvIndex;
	uint8_t msgId;
	uint8_t dataLen;
	uint8_t sum;
}asr_uart_recv_controller_t;

typedef enum{
	ASR_UART_MSG_NONE		= 0x00,
	/*
	 * send
	 */
	ASR_UART_MSG_HEART_BEAT 	= 0xFA,
	ASR_UART_MSG_WAKE_UP 		= 0x01,
	ASR_UART_MSG_TIMEOUT_RECOG	= 0x02,
	ASR_UART_MSG_CLOSE_RECOG	= 0x02,
	ASR_UART_MSG_ON		 	= 0x03,
	ASR_UART_MSG_OFF	 	= 0x04,
	ASR_UART_MSG_SET_MODE 		= 0xE2,
	ASR_UART_MSG_SET_TEMP 		= 0x05,
	ASR_UART_MSG_INC_T_BY_1		= 0x06,
	ASR_UART_MSG_INC_T_BY_3		= 0x07,
	ASR_UART_MSG_DEC_T_BY_1		= 0x08,
	ASR_UART_MSG_DEC_T_BY_3		= 0x09,
	ASR_UART_MSG_WIND_LOW 		= 0x0A,
	ASR_UART_MSG_WIND_MID 		= 0x0B,
	ASR_UART_MSG_WIND_HIGH 		= 0x0C,
	ASR_UART_MSG_WIND_AUTO		= 0x0D,
	ASR_UART_MSG_WINDLESS_ON	= 0x0E,
	ASR_UART_MSG_WINDLESS_OFF	= 0x0F,

	ASR_UART_MSG_SWING_OPEN		= 0x32,
	ASR_UART_MSG_SWING_STOP		= 0x17,
	/*
	 * recv
	 */
	ASR_UART_MSG_QUERY_STATUS 	= 0xFA,
	ASR_UART_MSG_SET_TIMER_OFF	= 0xFB,
	ASR_UART_MSG_SET_TIMER_ON 	= 0XFC,
}ASR_FRAME_MSG_ID;

typedef enum
{
	ASR_AC_WIND_SLOW 	= 0x01,
	ASR_AC_WIND_MID 	= 0x02,
	ASR_AC_WIND_HIGH 	= 0x03,
	ASR_AC_WIND_AUTO 	= 0x04,
	ASR_AC_WIND_INC 	= 0x10,
	ASR_AC_WIND_DEC 	= 0x11,
	ASR_AC_WIND_MIN 	= 0x01,
	ASR_AC_WIND_MAX 	= 0x04,
}ASR_AC_WIND;

typedef enum
{
	ASR_AC_HUMIDITY_DRY		= 0x01,
	ASR_AC_HUMIDITY_MODERATE 	= 0x02,
	ASR_AC_HUMIDITY_WET		= 0x03,
	ASR_AC_HUMIDITY_MIN 		= 0x01,
	ASR_AC_HUMIDITY_MAX 		= 0x03,
}ASR_AC_HUMIDITY;

typedef enum
{
	ASR_AC_SWING_OFF,
	ASR_AC_SWING_LEFT_RIGHT,
	ASR_AC_SWING_UP_DOWN,
	ASR_AC_SWING_ALL,
	ASR_AC_SWING_MIN = ASR_AC_SWING_OFF,
	ASR_AC_SWING_MAX = ASR_AC_SWING_ALL,
}ASR_AC_SWING;

typedef enum
{
	ASR_AC_WINDLESS_OFF,
	ASR_AC_WINDLESS_ON,
}ASR_AC_WINDLESS;

typedef enum
{
	ASR_AC_WINDLESS_UP_OFF,
	ASR_AC_WINDLESS_UP_ON,
}ASR_AC_WINDLESS_UP;

typedef enum
{
	ASR_AC_WINDLESS_DOWN_OFF,
	ASR_AC_WINDLESS_DOWN_ON,
}ASR_AC_WINDLESS_DOWN;

typedef enum
{
	ASR_AC_TIMER_OFF_CANCEL,
	ASR_AC_TIMER_OFF_1,
	ASR_AC_TIMER_OFF_2,
	ASR_AC_TIMER_OFF_3,
	ASR_AC_TIMER_OFF_4,
	ASR_AC_TIMER_OFF_5,
	ASR_AC_TIMER_OFF_6,
	ASR_AC_TIMER_OFF_7,
	ASR_AC_TIMER_OFF_8
}ASR_AC_TIMER_OFF;

typedef enum
{
	ASR_AC_TIMER_ON_CANCEL,
	ASR_AC_TIMER_ON_1,
	ASR_AC_TIMER_ON_2,
	ASR_AC_TIMER_ON_3,
	ASR_AC_TIMER_ON_4
}ASR_AC_TIMER_ON;

typedef enum
{
	ASR_AC_STRONG_MODE_ON,
	ASR_AC_STRONG_MODE_OFF
}ASR_AC_STRONG_MODE;

typedef enum
{
	ASR_AC_ENERGY_SAVING_MODE_ON,
	ASR_AC_ENERGY_SAVING_MODE_OFF
}ASR_AC_ENERGY_SAVING_MODE;

typedef enum
{
	ASR_AC_SLEEP_MODE_ON,
	ASR_AC_SLEEP_MODE_OFF
}ASR_AC_SLEEP_MODE;

typedef enum
{
	ASR_AC_ELECTRIC_AUX_HEAT_MODE_ON,
	ASR_AC_ELECTRIC_AUX_HEAT_MODE_OFF
}ASR_AC_ELECTRIC_AUX_HEAT_MODE;

typedef enum
{
	ASR_AC_SELF_CLEAN_ON,
	ASR_AC_SELF_CLEAN_OFF
}ASR_AC_SELF_CLEAN;

typedef enum
{
	ASR_AC_STRONG_WIND_ON,
	ASR_AC_STRONG_WIND_OFF
}ASR_AC_STRONG_WIND;

typedef enum
{
	ASR_AC_ANTI_BLOW_ON,
	ASR_AC_ANTI_BLOW_OFF
}ASR_AC_ANTI_BLOW;

typedef enum
{
	ASR_AC_ANTI_CLOD_WIND_ON,
	ASR_AC_ANTI_CLOD_WIND_OFF
}ASR_AC_ANTI_CLOD_WIND;

typedef enum
{
	ASR_AC_MODE_AUTO	= 0x05,
	ASR_AC_MODE_COOL 	= 0x01,
	ASR_AC_MODE_HEAT 	= 0x02,
	ASR_AC_MODE_DRY 	= 0x03,
	ASR_AC_MODE_WIND	= 0x04,
	ASR_AC_MODE_MIN		= 0x01,
	ASR_AC_MODE_MAX		= 0x05,
}ASR_AC_MODE;

typedef enum{
	ASR_UART_MSG_TYPE_CMD,
	ASR_UART_MSG_TYPE_SET_MODE,
	ASR_UART_MSG_TYPE_SET_T,
	ASR_UART_MSG_TYPE_SET_WIND,
	ASR_UART_MSG_TYPE_SET_SWING,
	ASR_UART_MSG_TYPE_SET_TIMER_OFF,
	ASR_UART_MSG_TYPE_SET_TIMER_ON,
}ASR_UART_MSG_TYPE;


typedef enum{
	ASR_WAKEUP_WORD_KONGTIAO= 0x01,
	ASR_WAKEUP_WORD_XIAOMEI	= 0x02,
	ASR_WAKEUP_WORD_MASK	= 0x03,
}ASR_wakeup_word;

typedef enum{
	ASR_PLAY_VOICE_STANDARD = 0x01,
	ASR_PLAY_VOICE_SWEET	= 0x02,
	ASR_PLAY_VOICE_BEEP	= 0x04,
}ASR_play_voice_type;

typedef enum{
	PLAY_INFO_NONE		= 0x00,
	PLAY_INFO_POWER_ON_EXTRA= 0x01,
	PLAY_INFO_LOW_TEMP_ALERT,
}AC_PLAY_INFO;

typedef enum{
	ASR_AC_ERROR_NONE = 0,
	ASR_AC_ERROR_MIN_T_ALREADY = -1,
	ASR_AC_ERROR_MAX_T_ALREADY = -2,
	ASR_AC_ERROR_MIN_WIND_ALREADY = -3,
	ASR_AC_ERROR_MAX_WIND_ALREADY = -4,
	ASR_AC_ERROR_POWER_ON_FIRST = -5,
	ASR_AC_ERROR_SET_T_NOT_SUPPORT = -6,
	ASR_AC_ERROR_SET_WIND_NOT_SUPPORT = -7,
	ASR_AC_ERROR_WINDLESS_NOT_SUPPORT = -8,
	ASR_AC_ERROR_POWER_ON_ALREADY = -9,
	ASR_AC_ERROR_POWER_OFF_ALREADY = -10,
	ASR_AC_ERROR_STRONG_MODE_NOT_SUPPORT = -11,
	ASR_AC_ERROR_ELECTRIC_AUX_HEAT_MODE_NOT_SUPPORT = -12,
	ASR_AC_ERROR_SLEEP_MODE_NOT_SUPPORT = -13,
	ASR_AC_ERROR_SET_ANTI_BLOW_NOT_SUPPORT = -14,
	ASR_AC_ERROR_SET_ANTI_CLOD_WIND_NOT_SUPPORT = -15,
	ASR_AC_ERROR_STRONG_WIND_NOT_SUPPORT = -16,
	ASR_AC_ERROR_NOT_WORKING = -100,
	ASR_AC_ERROR_NETWORK = -101,
	ASR_AC_ERROR_UART_TIMEOUT = -102,
	ASR_AC_ERROR_INVALID_PARAM = -103,
	ASR_AC_ERROR_MEM_FAIL = -104,
	ASR_AC_ERROR_UART_REPLY_ERROR = -105,
	ASR_AC_ERROR_NO_PLAY = -200,
}ASR_AC_ERROR;

typedef struct{
	uint8_t can_recog;
	uint8_t can_play;
	ASR_wakeup_word wakeup_word;
	ASR_play_voice_type voice_type;
	uint32_t idle_timeout;
	uint8_t play_volumn;
}ASR_config_t;

typedef struct{
	/*
	 * parameters
	 */
	uint8_t onoff;
	ASR_AC_MODE mode;
	float setT;
	float inT;
	float outT;
	ASR_AC_WIND wind;
	ASR_AC_HUMIDITY humidrty;
	ASR_AC_SWING swing;
	ASR_AC_WINDLESS windless;
	ASR_AC_WINDLESS_UP windless_up;
	ASR_AC_WINDLESS_DOWN windless_down;
	ASR_AC_TIMER_OFF timer_off;
	ASR_AC_TIMER_ON timer_on;
	ASR_AC_STRONG_MODE strong_mode;
	ASR_AC_ENERGY_SAVING_MODE energy_saving_mode;
	ASR_AC_SLEEP_MODE sleep_mode;
	ASR_AC_ELECTRIC_AUX_HEAT_MODE electric_aux_heat_mode;
	ASR_AC_SELF_CLEAN self_clean;
	ASR_AC_STRONG_WIND strong_wind;
	ASR_AC_ANTI_BLOW anti_blow;
	ASR_AC_ANTI_CLOD_WIND anti_clod_wind;
	/*
	 * uart info
	 */
	uint8_t play;
	uint8_t play_info;
	ASR_AC_ERROR err;
}AC_t;

typedef struct{
	uint8_t onoff;
	float minT;
	float maxT;
}ASR_AC_config_t;



int device_asr_reader_init(void);
void device_asr_reader_start(void);
cmd_uart_msg_t* device_asr_alloc_frame(uint8_t msgId, \
		uint8_t* data, uint8_t len);
int device_asr_free_frame(cmd_uart_msg_t* frame);
int device_asr_send_frame_default_with_reply(cmd_uart_msg_t* frame, \
		cmd_uart_msg_t** reply);
int device_asr_send_frame_default_no_wait(cmd_uart_msg_t* frame);
int device_asr_send_frame_default(cmd_uart_msg_t* frame);
int device_asr_send_frame_no_wait(cmd_uart_msg_t* frame);
int device_asr_send_frame(cmd_uart_msg_t* frame, uint8_t retryTimes);
int device_asr_send_frame_with_reply(cmd_uart_msg_t* frame, \
		cmd_uart_msg_t** reply, uint8_t retryTimes);

int device_asr_send_msg(ASR_UART_MSG_TYPE type, int arg);

AC_t* device_ac_get(void);
ASR_config_t* const device_asr_get_config(void);

#endif /* APP_DEVICE_INCLUDE_DEVICE_ASR_UART_H_ */
