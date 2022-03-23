/*
 *  Copyright (c) 2019 Allwinner Technology. All Rights Reserved.
 *
 *  Author: laumy <liumingyuan@allwinnertech.com>
 *
 *  date: 2019-3-15
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <smg_event.h>
#include <smg_log.h>

struct sm_event events = {
	.fd={-1,-1},
	.enable=false,
};

int clear_sm_event()
{
	int ret = -1;
	char c;
	int flags;
	smg_printf(SMG_MSGDUMP,"enter -+- -+-\n");
	if(! events.enable){
		smg_printf(SMG_ERROR,"event socket is closed\n");
		return ret;
	}
	/* clear scan.sockets data before sending scan command*/
	if(flags = fcntl(events.fd[1],F_GETFL,0) < 0){
		smg_printf(SMG_ERROR,"fcntl getfl error\n");
		return -1;
	}

	flags |= O_NONBLOCK;

	if(fcntl(events.fd[1],F_SETFL,flags) < 0){
		smg_printf(SMG_ERROR,"fcntl setfl error\n");
		return -1;
	}

	while ((ret= TEMP_FAILURE_RETRY(read(events.fd[1],&c,1))) > 0){
		smg_printf(SMG_DEBUG,"clear data %d\n",c);
	}

	smg_printf(SMG_MSGDUMP,"quilt -+- -+-\n");
}

int sm_event_notice(int type)
{
	int ret = -1;
	char data;
	smg_printf(SMG_MSGDUMP,"enter -+- -+-\n");

	if(! events.enable){
		smg_printf(SMG_ERROR,"event socket is closed\n");
		return ret;
	}

	data = (char)type;
	ret = TEMP_FAILURE_RETRY(write(events.fd[0],&data, 1));

	smg_printf(SMG_MSGDUMP,"quilt -+- -+-\n");

	return ret;
}
int sm_event_wait(int timeout_ms)
{
	int ret = -1;
	struct pollfd rfds;
	char c;

	smg_printf(SMG_MSGDUMP,"enter -+- -+-\n");
	if(! events.enable){
		smg_printf(SMG_ERROR,"event socket is closed\n");
		return ret;
	}

	memset(&rfds,0,sizeof(struct pollfd));
	rfds.fd = events.fd[1];
	rfds.events |= POLLIN;

	/*wait  event.*/
	ret = TEMP_FAILURE_RETRY(poll(&rfds, 1, timeout_ms));
	if (ret < 0) {
		smg_printf(SMG_ERROR,"Error poll = %d\n", ret);
		return ret;
	}
	if(ret == 0) {
		smg_printf(SMG_ERROR,"poll time out!\n");
		return ret;
	}
	if (rfds.revents & POLLIN) {
		TEMP_FAILURE_RETRY(read(events.fd[1],&c,1));
		smg_printf(SMG_DEBUG,"read event %d\n",c);
		ret = (int)c;
	}
	smg_printf(SMG_MSGDUMP,"quilt -+- -+-\n");
	return ret;
}

void sm_event_deinit()
{
	if(events.enable){
		clear_sm_event();
		if(events.fd[0] >= 0) {
			close(events.fd[0]);
			events.fd[0] = -1;
		}
		if(events.fd[1] >= 0) {
			close(events.fd[1]);
			events.fd[1] = -1;
		}
	}
}

int sm_event_init()
{
	int ret = -1;
	if(events.fd[0] >=0 || events.fd[1] >= 0){
		sm_event_deinit();
	}
	events.fd[0] = events.fd[1] = -1;
	ret = socketpair(AF_UNIX,SOCK_STREAM,0,events.fd);
	if(ret == -1) {
		smg_printf(SMG_ERROR,"scan socketpair init error\n");
		return ret;
	}
	events.enable = true;
	return 0;
}


