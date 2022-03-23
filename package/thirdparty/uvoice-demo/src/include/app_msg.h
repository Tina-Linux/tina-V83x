#ifndef __APP_MSG_H__
#define __APP_MSG_H__

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include "dev_msg_protocol_parse.h"
#include "usr_config.h"

int gs_msgid;

typedef struct{
	uint8_t *data;
	uint16_t *data_len;
	uint8_t device_type;
	uint8_t msg_id;
	uint8_t cmd;
}uart_do_package_para_t;

typedef struct{
	uint8_t head;
	uint8_t msg_length;
	uint8_t dev_type;
	uint8_t frame_crc;
	uint8_t reserve[2];
	uint8_t msg_id;
	uint8_t frame_version;
	uint8_t dev_version;
	uint8_t cmd_type;
}uart_msg_head_t;

typedef enum TASK_TYPE_T{
	TASK_TYPE_MAIN		= 0x00,
	TASK_TYPE_UART 		= 0x01,
	TASK_TYPE_UVOICE	= 0x02,
	TASK_TYPE_INVALID	= 0xFF,
}task_type_t;

typedef enum UART_EVENT_T{
	UART_EVENT_START_SUCCESS = 0x10,
	UART_EVENT_START_FAILED,
	UART_EVENT_REPORT_NOACK,
	UART_EVENT_REPORT_ACK,
	UART_EVENT_SYN_TIME,
}uart_event_t;

typedef enum SYS_EVENT_T{
	SYS_EVENT_SNIFFER_OK = 0x20,
	SYS_EVENT_SNIFFER_FAILED,
	SYS_EVENT_RESET,
	SYS_EVENT_LOGIN_ALI_CLOUD,
}sys_event_t;

//cmd_uart_msg_t msg_t;
/*
typedef struct MSG_T{
	char *data;
	int len;
	int type;
	uint32_t msg_id;
	int16_t fd;
	task_type_t src_task;
}msg_t;
*/

struct msg_buf
{
	long mtype;
	uint8_t update_flag;
	uint8_t data[ASR_UART_BUFFER_SIZE];
	uint8_t dataLen;
};

typedef enum{
	RESULT_SUCCESS = 0,
	RESULT_FAIL = -1,
}msg_result;

void uart_debug_hex_dump(char *info, uint8_t *data, uint16_t len);
void send_to_ring_buffer(uint16_t *buf, uint16_t len);
int recv_from_ring_buffer(uint16_t *buf, uint16_t len);
void msg_show_attr(int msg_id, struct msqid_ds msg_info);
int msg_queue_init();
int send_msg_to_queue(int msgid, struct msg_buf *msgbuf);
int recv_msg_from_queue(int msgid, struct msg_buf *msgbuf);
int sample_msg_demo();

#endif

