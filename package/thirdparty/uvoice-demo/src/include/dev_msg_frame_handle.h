#ifndef __DEV_MSG_FRAME_HANDLE_H__
#define __DEV_MSG_FRAME_HANDLE_H__


#include "dev_msg_protocol_parse.h"
#include "app_msg.h"
#include "usr_config.h"
/*
 * AC DEVICE UART COMM CONFIGS
*/
#define ASR_UART_DEFAULT_RETRY        (6)
#define ASR_UART_HEARTBEAT_INTERVAL    (3000)//(5000)
#define TO1_MS    (150)//(320)
#define TO2_MS  (250)//(370)
#define SUM_ERR_MS    (50)

#define READ_TIMEOUT_MS    (10)

/*
 * AC DEVICE UART COMM CONFIGS
*/
#define APP_UART                 ("/dev/ttyS0")
#define ASR_UART_SET_SPEED        (9600)
#define ASR_UART_SET_FLOWCTL    (0)
#define ASR_UART_SET_DATABITS    (8)
#define ASR_UART_SET_STOPBITS    (1)
#define ASR_UART_SET_PARITY        ('N')

int device_asr_uart_fd;
struct msg_buf msgbuf;

struct asr_uart_processor
{
    uint8_t        readThread_PrivInitOk;
    pthread_t     readThread;
    pthread_t     processframeThread;
    pthread_t     writeThread;
    sem_t         write_sem;
    pthread_mutex_t lock;
    pthread_mutex_t msg_queue_lock;
    pthread_cond_t     write_signal;
    cmd_uart_msg_t     *write_request;
    uint8_t     requst_need_reply;
    uint8_t     not_get_last_requst_reply;
    cmd_uart_msg_t     *reply;
    uint64_t     last_active_ticks;
#if USE_TIMER
    os_timer_t     timer;
#endif
    uint64_t     last_heartbeat_ticks;
    uint64_t     last_query_ticks;
} m_asr_uart;

typedef struct {
    cmd_uart_msg_t* frame;
    uint32_t timeout;
    uint8_t need_reply;
    uint8_t retry_times;
    uint32_t start_ticks;
    void (*callback)(uint8_t success, cmd_uart_msg_t* reply, void* arg);
} uart_msg_t;

#define LOCK_UART()   {pthread_mutex_lock(&m_asr_uart.lock); \
    LOGD("lock");}
#define UNLOCK_UART() {pthread_mutex_unlock(&m_asr_uart.lock); \
    LOGD("unlock");}

#define LOCK_MSG_QUEUE()   {pthread_mutex_lock(&m_asr_uart.msg_queue_lock);}//; \
    printf("msg queue lock,%s,%d\n",__FUNCTION__,__LINE__);}
#define UNLOCK_MSG_QUEUE() {pthread_mutex_unlock(&m_asr_uart.msg_queue_lock);}//; \
    printf("msg queue unlock,%s,%d\n",__FUNCTION__,__LINE__);}

typedef int (*cmd_processor)(cmd_uart_msg_t* frame);

typedef struct {
    uint8_t cmd;
    cmd_processor processor;
} asr_uart_processor_t;

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

static asr_uart_recv_controller_t recv_controller;

static asr_uart_processor_t processors[] =
{
    //asr module recv and reply msg
    {ASR_UART_MSG_HEART_BEAT,           cmd_heartbeat},
    {ASR_UART_MSG_WAKE_UP,              cmd_wakeup},
    {ASR_UART_MSG_ON,                   cmd_on},
    {ASR_UART_MSG_WIND_SPEED,             cmd_set_wind_speed},
    {ASR_UART_MSG_SET_TEMP,             cmd_set_temp},
    {ASR_UART_MSG_SET_MODE,             cmd_set_mode},
    {ASR_UART_MSG_SET_SWING,               cmd_set_swing},
    {ASR_UART_MSG_SET_BLOW,                cmd_set_blow},
    {ASR_UART_MSG_SET_FUNC,               cmd_set_func},
    {ASR_UART_MSG_SET_FACTORY,           cmd_set_factory},
    {ASR_UART_MSG_SET_TIMER,               cmd_timer_func},
    {ASR_UART_MSG_SET_VOLUME,           cmd_set_volume},
    {ASR_UART_MSG_SET_WIFI,               cmd_set_wifi},
    {ASR_UART_MSG_PASSTHROUGH,           cmd_passthrough},
    {ASR_UART_MSG_QUERY_WEATHER,         cmd_query_weather},

    //asr module ack msg
    {ASR_UART_MSG_BOARD_STARTUP_PLAY,        cmd_board_startup},
    {ASR_UART_MSG_BOARD_BEEP_NOTICE,        cmd_board_beep_notice},
    {ASR_UART_MSG_BOARD_FACTORY_TEST,        cmd_board_factory},
    {ASR_UART_MSG_BOARD_WIFI_STATUS,        cmd_board_wifi_status},
    {ASR_UART_MSG_BOARD_WIFI_PASSTHROUGH,    cmd_board_passthrough},
    {ASR_UART_MSG_BOARD_SET_CONFIGS,        cmd_board_set_configs},
};

#define PROCESSORS_COUNT (sizeof(processors) / sizeof(asr_uart_processor_t))

int device_asr_reader_init(void);
void device_asr_reader_start();
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


#endif /* __DEVICE_UART_FRAME_HANDLE_H__ end */


