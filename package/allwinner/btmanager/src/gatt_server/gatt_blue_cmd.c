/*
 *  Copyright (C) 2019 Realtek Semiconductor Corporation.
 *
 *  Author: Alex Lu <alex_lu@realsil.com.cn>
 */

#include <poll.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <gio/gio.h>
#include <glib.h>

#define DEBUG
#include "log.h"
#include "gatt_blue_cmd.h"

static GList *gatt_bta_cmdq = NULL;
static pthread_mutex_t gatt_cmd_mutex = PTHREAD_MUTEX_INITIALIZER;
static int gatt_commfds[2];
static pthread_t gatt_bta_mainloop_tid;
static bcmd_handler_t gatt_bta_cmd_handler;

void gatt_set_bta_mainloop_id(pthread_t tid)
{
	pr_info("tid: %lu", (unsigned long)tid);
	if (tid != 0)
		gatt_bta_mainloop_tid = tid;
	else
		pr_error("Invalid thread id 0");
}

void gatt_set_bta_cmd_fd(int fd[2])
{
	gatt_commfds[0] = fd[0];	/* 0: bluetooth read/write */
	gatt_commfds[1] = fd[1];	/* 1: caller read/write */

	pr_info("gatt_commfds %d %d", fd[0], fd[1]);
}

void gatt_bta_register_handler(bcmd_handler_t handler)
{
	pr_info("++");
	gatt_bta_cmd_handler = handler;
}

void gatt_bta_clear_commands(void)
{
	pthread_mutex_lock(&gatt_cmd_mutex);
	g_list_free_full(gatt_bta_cmdq, free);
	pthread_mutex_unlock(&gatt_cmd_mutex);
}

int gatt_bta_submit_command_wait(uint16_t opcode, int argc, void **argv)
{
	struct bta_command *cmd;
	static uint32_t cmd_id = 1;
	uint8_t c = BTA_WAKE_CODE;
	struct pollfd p[1];
	ssize_t ret;
	int result = -1;

	pr_debug("++");
	if (gatt_bta_mainloop_tid == 0) {
		pr_error("Please set bluetooth thread id");
		return -1;
	}

	/* Running in the bta mainloop */
	if (gatt_bta_mainloop_tid == pthread_self()) {
		/* call lib api directly
		 * result = xxxx(...)
		 * */
		pr_info("Calling api directly");
		if (gatt_bta_cmd_handler)
			result = gatt_bta_cmd_handler(opcode, argc, argv);
		else
			result = -1;
		return result;
	}

	cmd = malloc(sizeof(*cmd));
	if (!cmd)
		return -1;
	cmd->opcode = opcode;
	cmd->argc = argc;
	cmd->argv = argv;
	cmd->req_status = REQ_PENDING;
	cmd->req_result = 0;
	memset(cmd->rparams, 0, sizeof(cmd->rparams));
	if (cmd_id < 1)
		cmd_id = 1;
	cmd->id = cmd_id++;
	pthread_mutex_lock(&gatt_cmd_mutex);
	gatt_bta_cmdq = g_list_append(gatt_bta_cmdq, cmd);
	/* Wake up the bluetooth thread */
	ret = write(gatt_commfds[1], &c, 1);
	if (ret != 1) {
		pr_error("Couldn't wake up bluetooth thread");
		gatt_bta_cmdq = g_list_remove(gatt_bta_cmdq, cmd);
		pthread_mutex_unlock(&gatt_cmd_mutex);
		goto free;
	}
	pthread_mutex_unlock(&gatt_cmd_mutex);

	p[0].fd = gatt_commfds[1];
	p[0].events = POLLIN | POLLERR | POLLHUP;

	while (1) {
		p[0].revents = 0;
		result = poll(p, 1, 2000);	/* 2000 msec */
		if (result <= 0 || p[0].revents & (POLLERR | POLLHUP)) {
			if (result == 0)
				pr_error("Poll timeout, %04x", cmd->opcode);
			else if (result < 0)
				pr_error("Poll error, %s, opcode %04x",
					 strerror(errno), opcode);
			else
				pr_error("Poll err or hup %d", p[0].revents);
			/* NOTE if the cmd is running too long, the mutex lock
			 * will stick here. That should never happen */
			pthread_mutex_lock(&gatt_cmd_mutex);
			gatt_bta_cmdq = g_list_remove(gatt_bta_cmdq, cmd);
			result = -1;
			pthread_mutex_unlock(&gatt_cmd_mutex);
			break;
		}

		/* Sometime the byte is read by other thread, but that doesn't
		 * matter */
		ret = read(gatt_commfds[1], &c, 1);

		pthread_mutex_lock(&gatt_cmd_mutex);
		if (cmd->req_status == REQ_DONE) {
			result = cmd->req_result;
			pthread_mutex_unlock(&gatt_cmd_mutex);
			break;
		}
		pthread_mutex_unlock(&gatt_cmd_mutex);
	}

free:
	free(cmd);

	return result;
}

/* This function runs in bluetooth thread context */
void btgatt_run_command(void)
{
	GList *l = NULL;
	struct bta_command *cmd = NULL;
	ssize_t ret;
	uint8_t c = BTA_WAKE_CODE;
	int result;

	pr_debug();

	if (gatt_bta_mainloop_tid != pthread_self()) {
		pr_error("Run in the wrong context, mainloop tid:%lu, self:%lu",
			 (unsigned long)gatt_bta_mainloop_tid, (unsigned long)pthread_self());
		return;
	}

	pthread_mutex_lock(&gatt_cmd_mutex);
	l = g_list_first(gatt_bta_cmdq);
	if (l) {
		cmd = l->data;
		/* Remove the link and free the node but not the data */
		gatt_bta_cmdq = g_list_delete_link(gatt_bta_cmdq, l);
		/* Should never block long time here */
		if (gatt_bta_cmd_handler)
			result = gatt_bta_cmd_handler(cmd->opcode, cmd->argc,
						      cmd->argv);
		else
			result = -1;
		cmd->req_status = REQ_DONE;
		cmd->req_result = result;

		pr_debug("Wake up thread waiting result");
		/* Wake up the thread that issues command */
		ret = write(gatt_commfds[0], &c, 1);
		if (ret < 0) {
			if (errno == EINTR || errno == EAGAIN)
				ret = write(gatt_commfds[0], &c, 1);
			if (ret < 0) {
				pr_error("Execute %04x failed", cmd->opcode);
				cmd->req_result = -1;
			}
		} else if (ret == 0)
			pr_error("Can't write signal to sk fd");
	}
	pthread_mutex_unlock(&gatt_cmd_mutex);

	/* No longer access cmd */
}
