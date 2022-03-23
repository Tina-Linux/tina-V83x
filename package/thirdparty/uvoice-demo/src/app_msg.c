#include "app_msg.h"

/* ring buffer */
static uint16_t     uart_recv_total_len = 0;
static uint16_t  uart_ring_buffer[128];

#define UART_BUFFER_LEN		300
#define READ_TIMEOUT	200

void uart_debug_hex_dump(char *info, uint8_t *data, uint16_t len)
{
	int i;

	printf("%s", info);
	for (i = 0; i < len; i++) {
		printf("%02x ", data[i]);
	}
	printf("]\r\n");
}

void msg_show_attr(int msg_id, struct msqid_ds msg_info)
{
	int ret = -1;

	ret = msgctl(msg_id, IPC_STAT, &msg_info);
	if (ret == -1) {
		LOGW("fail to get msg info!");
		return;
	}

	//printf("the byetes num of the queue:%d\n", msg_info.msg_cbytes);
	//printf("the msg num of the queue:%d\n", msg_info.msg_qnum);
	//printf("the max num bytes of the queue:%d\n", msg_info.msg_qbytes);
	//printf("the last process to send the msg:%d\n",msg_info.msg_lspid);
	//printf("the last process to rcv the msg:%d\n",msg_info.msg_lrpid);
	//printf("the last time to send the msg:%d\n",msg_info.msg_stime);
	//printf("the last time to rcv the msg:%d\n",msg_info.msg_rtime);
	//printf("the last time to change the msg:%d\n",msg_info.msg_ctime);
	//printf("the UID of the msg:%d\n", msg_info.msg_perm.uid);
	//printf("the GID of the msg:%d\n", msg_info.msg_perm.gid);
	//printf("\n");
}

int msg_queue_init()
{
	int msgid;
	struct msqid_ds msg_info;

	LOGT("msg queue init");
	memset(&msg_info, 0, sizeof(msg_info));
	msgid = msgget((key_t)1234, IPC_CREAT|0666);
	if (msgid == -1) {
		LOGE("create msg error!");
		return -1;
	}
	LOGD("msgid = %d",msgid);
	msg_show_attr(msgid, msg_info);
	return msgid;
}

int send_msg_to_queue(int msgid, struct msg_buf *msgbuf)
{
	int ret;
	struct msqid_ds msg_info;

	LOGT("smd msg to queue");
	memset(&msg_info, 0, sizeof(msg_info));
	ret = msgsnd(msgid, msgbuf,
		sizeof(msgbuf->data) + sizeof(msgbuf->dataLen),
		IPC_NOWAIT);
	if (ret == -1) {
		LOGE("snd msg error!");
		return RESULT_FAIL;
	}
	msg_show_attr(msgid, msg_info);
	return RESULT_SUCCESS;
}

int recv_msg_from_queue(int msgid, struct msg_buf *msgbuf)
{
	int ret;
	struct msqid_ds msg_info;

	LOGT("recv msg from queue");
	memset(&msg_info, 0, sizeof(msg_info));
	ret = msgrcv(msgid, msgbuf,
		sizeof(msgbuf->data) + sizeof(msgbuf->dataLen),
		0, IPC_NOWAIT);
	if (ret == -1) {
		//printf("rcv msg error!\n");
		return RESULT_FAIL;
	}
	//printf("rcv msg ok!\n");
	//printf("msgbuf->data[0] = %#x\n",msgbuf->data[0]);
	//printf("msgbuf->data[1] = %#x\n",msgbuf->data[1]);
	//printf("msgbuf->dataLen = %d\n",msgbuf->dataLen);
	//printf("rcv msg :%s\n", msgbuf->data);
	uart_debug_hex_dump("rcv msg = [", msgbuf->data, msgbuf->dataLen);
	msg_show_attr(msgid, msg_info);
	return RESULT_SUCCESS;
}

int sample_msg_demo()
{
	int msgid;
	int ret;
	struct msg_buf msgbuf1;
	struct msg_buf msgbuf2;
	struct msqid_ds msg_info;

	msgid = msg_queue_init();
	msgbuf1.mtype = getpid();
	//printf("mtype=%d\n",msgbuf1.mtype);
	//strcpy(msgbuf1.data, "this is a msg test demo!");
	msgbuf1.data[0] = 0xAA;
	msgbuf1.data[1] = 0xFA;
	msgbuf1.data[2] = 0x20;
	msgbuf1.data[3] = 0x00;
	msgbuf1.data[4] = 0x01;
	msgbuf1.data[5] = 0x02;
	msgbuf1.data[6] = 0x03;
	msgbuf1.data[7] = 0xFA;
	msgbuf1.data[8] = 0x00;
	msgbuf1.data[9] = 0xA4;
	msgbuf1.data[10] = 0x06;
	msgbuf1.data[11] = 0xB8;
	msgbuf1.data[12] = 0x12;
	msgbuf1.data[13] = 0xFA;
	msgbuf1.data[14] = 0x00;
	msgbuf1.data[15] = 0x00;
	msgbuf1.data[16] = 0x00;
	msgbuf1.data[17] = 0x31;
	msgbuf1.data[18] = '\0';
	printf("msgbuf1.data = %s\n",msgbuf1.data);
	uart_debug_hex_dump("msgbuf1.data = [", msgbuf1.data, sizeof(msgbuf1.data));

	strcpy(msgbuf2.data, "123456789!");
	printf("msgbuf2.data = %s\n",msgbuf2.data);

	send_msg_to_queue(msgid, &msgbuf1);
	//2nd
	strcpy(msgbuf1.data, "this is second msg test demo!");
	printf("msgbuf1.data = %s\n",msgbuf1.data);

	send_msg_to_queue(msgid, &msgbuf1);
	recv_msg_from_queue(msgid, &msgbuf2);

	//2nd
	recv_msg_from_queue(msgid, &msgbuf2);

	ret = msgctl(msgid, IPC_RMID, NULL);
	if (ret != -1) {
		printf("rm msg success!\n");
	}
	return 0;
}

void send_to_ring_buffer(uint16_t *buf, uint16_t len)
{
	static uint16_t offset = 0;
	uint16_t len_tp = 0;

	//LOGT("snd to ring buffer");
	uart_recv_total_len += len;
	if ((offset + len) > (sizeof(uart_ring_buffer))/sizeof(uint16_t)) {
		len_tp = (sizeof(uart_ring_buffer)/sizeof(uint16_t)) - offset;
		memcpy(uart_ring_buffer + offset, buf, len_tp * sizeof(uint16_t));
		offset = 0;
		memcpy(uart_ring_buffer + offset, buf + len_tp, (len - len_tp) * sizeof(uint16_t));
		offset += (len - len_tp);
	} else {
		memcpy(uart_ring_buffer + offset, buf, len * sizeof(uint16_t));
		offset += len;
	}
}

int recv_from_ring_buffer(uint16_t *buf, uint16_t len)
{
	static uint16_t offset = 0;
	uint16_t len_tp = 0;

	//LOGT("recv to ring buffer");
	if (len > uart_recv_total_len) {
		return -1;
	} else {
		uart_recv_total_len -= len;
		if ((offset + len) > (sizeof(uart_ring_buffer))/sizeof(uint16_t)) {
			len_tp = (sizeof(uart_ring_buffer)/sizeof(uint16_t)) - offset;
			memcpy(buf, uart_ring_buffer + offset, len_tp * sizeof(uint16_t));
			offset = 0;
			memcpy(buf + len_tp, uart_ring_buffer + offset, (len - len_tp) * sizeof(uint16_t));
			offset += (len - len_tp);
		} else {
			memcpy(buf, uart_ring_buffer + offset, len * sizeof(uint16_t));
			offset += len;
		}
		LOGT("offset:%d, size:%d\n",offset, sizeof(uint16_t));
	}
	return 0;
}
