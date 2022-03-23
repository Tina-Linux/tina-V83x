/*
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#include "bootanimation.h"

#include <libubox/uloop.h>

static struct uloop_process boot_play_proc;
static struct uloop_process music_play_proc;
#ifdef COMPLETE_TIMEOUT
static struct uloop_timeout exit_timer;
#define EXIT_TIMEOUT	(COMPLETE_TIMEOUT*1000)
#endif

#define MUSIC_PLAY		"/sbin/music-play"
#define MUSIC_PLAY_BOOT		"/usr/res/boot-play/boot.mp3"
#define MUSIC_PLAY_SHUTDOWN	"/usr/res/boot-play/poweroff.mp3"

static void
signal_shutdown(int signal, siginfo_t *siginfo, void *data)
{
	LOG("boot-play shutdown\n");
	fflush(stderr);
	sync();
	if (boot_play_proc.pid != 0)
		kill(boot_play_proc.pid, SIGKILL);
	if (music_play_proc.pid != 0)
		kill(music_play_proc.pid, SIGKILL);
	exit(0);
}

static struct sigaction sa_shutdown = {
	.sa_sigaction = signal_shutdown,
	.sa_flags = SA_SIGINFO
};

static void
music_play_cb(struct uloop_process *proc, int ret)
{
	proc->pid = 0;
	LOG("music-play complete\n");
	if (!boot_play_proc.pid)
		kill(getpid(), SIGTERM);
#ifdef COMPLETE_MUSIC
	kill(getpid(), SIGTERM);
#endif
}
static void
music_play(void)
{
	char *music_play[] = {MUSIC_PLAY, MUSIC_PLAY_BOOT, NULL};

	if (access(MUSIC_PLAY_BOOT, F_OK) != 0)
		return ;

	LOG("- music-play -\n");

	music_play_proc.cb = music_play_cb;
	music_play_proc.pid = fork();
	if (!music_play_proc.pid) {
		int fd;
		fd = open("/dev/null", O_RDWR);
		if (fd > -1) {
			dup2(fd, STDIN_FILENO);
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDERR_FILENO);
			if (fd > STDERR_FILENO)
				close(fd);
		}
		setpriority(PRIO_PROCESS, 0, BOOT_MUSIC_PRIORITY);
		execvp(music_play[0], music_play);
		ERROR("Failed to start boot-play\n");
		exit(-1);
	}
	if (music_play_proc.pid <= 0) {
		ERROR("Failed to start new music-play instance\n");
		return;
	}
	uloop_process_add(&music_play_proc);
}

static void
boot_play_cb(struct uloop_process *proc, int ret)
{
	proc->pid = 0;
	LOG("boot-play complete\n");
	if (!music_play_proc.pid)
		kill(getpid(), SIGTERM);
#ifdef COMPLETE_ANIMATION
	kill(getpid(), SIGTERM);
#endif
}
static void
boot_play(void)
{
	LOG("- boot-play -\n");

	boot_play_proc.cb = boot_play_cb;

	boot_play_proc.pid = fork();
	if (!boot_play_proc.pid) {
		bootanimation();
		LOG("bootanimation exit!\n");
		exit(0);
	}
	if (boot_play_proc.pid <= 0) {
		ERROR("Failed to start new boot-play instance\n");
		return;
	}
	uloop_process_add(&boot_play_proc);
}
#ifdef COMPLETE_TIMEOUT
static void
exit_timer_cb(struct uloop_timeout *t)
{
	LOG("- boot-play timeout -\n");
	kill(getpid(), SIGTERM);
}
#endif
int
main(int argc, char **argv)
{
	if (argc != 2)
		return -1;
	if (!strcmp("shutdown", argv[1])) {
		printf("boot-play shutdown!\n");
		char *music_play[] = {MUSIC_PLAY, MUSIC_PLAY_SHUTDOWN, NULL};

		setpriority(PRIO_PROCESS, 0, BOOT_MUSIC_PRIORITY);
		execvp(music_play[0], music_play);
		ERROR("Failed to start boot-play\n");
		exit(-1);
	} else if (!strcmp("boot", argv[1])) {
		ulog_open(ULOG_KMSG, LOG_DAEMON, "boot-play");

		sigaction(SIGTERM, &sa_shutdown, NULL);
		sigaction(SIGUSR1, &sa_shutdown, NULL);
		sigaction(SIGUSR2, &sa_shutdown, NULL);

		uloop_init();

		boot_play();
		music_play();
#ifdef COMPLETE_TIMEOUT
		exit_timer.cb = exit_timer_cb;
		uloop_timeout_set(&exit_timer, EXIT_TIMEOUT);
		uloop_timeout_add(&exit_timer);
#endif

		uloop_run();
		uloop_done();
	} else {
		fprintf(stderr, "[boot-play] unknow arg.\n");
	}

	return 0;
}
