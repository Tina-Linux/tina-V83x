/*
 * audiocodec test
 * play_capture_normal_rate.c
 * (C) Copyright 2010-2016
 * reuuimllatech Technology Co., Ltd. <www.reuuimllatech.com>
 * huangxin <huangxin@reuuimllatech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <dirent.h>
#include <dlfcn.h>
#include <linux/input.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <limits.h>
#include "include/tinyalsa/asoundlib.h"




#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

#define FORMAT_PCM 1

struct wav_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t riff_fmt;
    uint32_t fmt_id;
    uint32_t fmt_sz;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint32_t data_id;
    uint32_t data_sz;
};

#define MIXER_AUDIO_NORMAL_SPEAKER_HEADSET			"Speaker Function"
#define PRESS_DATA_NAME    "headset"
#define PRESS_SYSFS_PATH   "/sys/class/input"

int fd;
pthread_t ntid;
int hp_plug_judge = 1;

struct mixer *mixer;
struct mixer_ctl *audio_spk_headset_switch;
struct input_event buff;

void play_sample(FILE *file, unsigned int device, unsigned int channels,
                 unsigned int rate, unsigned int bits);

void sigint_handler(int sig)
{
    hp_plug_judge = 0;
}

int hp_get_class_path(char *class_path)
{
	char dirname[] = PRESS_SYSFS_PATH;
	char buf[256];
	int res;
	DIR *dir;
	struct dirent *de;
	int fd = -1;
	int found = 0;

	dir = opendir(dirname);
	if (dir == NULL)
		return -1;

	while((de = readdir(dir))) {
		if (strncmp(de->d_name, "input", strlen("input")) != 0) {
		    continue;
		}

		sprintf(class_path, "%s/%s", dirname, de->d_name);
		snprintf(buf, sizeof(buf), "%s/name", class_path);

		fd = open(buf, O_RDONLY);
		if (fd < 0) {
		    continue;
		}
		if ((res = read(fd, buf, sizeof(buf))) < 0) {
		    close(fd);
		    continue;
		}
		buf[res - 1] = '\0';
		if (strcmp(buf, PRESS_DATA_NAME) == 0) {
		    found = 1;
		    close(fd);
		    break;
		}

		close(fd);
		fd = -1;
	}
	closedir(dir);
	if (found) {
		return 0;
	}else {
		*class_path = '\0';
		return -1;
	}
}

int test_hp_jack()
{
	int len = 0;
	char class_path[100];

	memset(class_path,0,sizeof(class_path));
	hp_get_class_path(class_path);
	len = strlen(class_path);
	printf("index: %c\n",class_path[len - 1]);

	sprintf(class_path, "/dev/input/event%c", class_path[len - 1]);
	printf("path: %s\n",class_path);

	fd = open(class_path, O_RDONLY); //may be the powerlinein is /dev/input/event1
	if (fd < 0) {
		perror("can not open device usblineinboard!");
		exit(1);
	}
	printf("--fd:%d--\n",fd);

	while(hp_plug_judge) {
		while(read(fd,&buff,sizeof(struct input_event))==0) {
			;
		}

		printf("%s,l:%d,d buff.code:%d, buff.value:%d\n", __FUNCTION__, __LINE__, buff.code, buff.value);
		if(buff.code == KEY_HP && buff.value == 1) {
			mixer_ctl_set_enum_by_string(audio_spk_headset_switch, "headset");
			printf("hp plug in\n");
		} else if (buff.code == KEY_HP && buff.value == 0) {
			mixer_ctl_set_enum_by_string(audio_spk_headset_switch, "spk");
			printf("hp plug out\n");
		}
	}
	close(fd);
	mixer_close(mixer);

	return 0;
}

void hp_jack_init()
{
    int fd;
    char jack_buf[16];
    int jack_state;
    char *path = "/sys/class/switch/h2w/state";
    int read_count;

    fd = open(path, O_RDONLY|O_NONBLOCK);
	if (fd < 0) {
		pthread_exit((void *)-1);
	}

    read_count = read(fd,jack_buf,sizeof(jack_buf));
	if (read_count < 0) {
		pthread_exit((void *)-1);
	}
	jack_state = atoi(jack_buf);
	if (jack_state > 0) {
		printf("headset: %s, %d\n", __func__, __LINE__);
		mixer_ctl_set_enum_by_string(audio_spk_headset_switch, "headset");
	} else {
		printf("spk:%s, %d\n", __func__, __LINE__);
		mixer_ctl_set_enum_by_string(audio_spk_headset_switch, "spk");
	}
	close(fd);
}

void* mixer_thread(void *arg)
{
	hp_jack_init();
	test_hp_jack();
	return NULL;
}

int main(int argc, char **argv)
{
	int err = 0;
	int ret = 0;
	FILE *file;
    unsigned int device = 0;
    unsigned int play_rate = 48000;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s file.wav [-d device] [-c channels] "
                "[-r rate] [-b bits]\n", argv[0]);
        return -1;
    }
	play_rate = atoi(argv[1]);
	mixer = mixer_open(0);
    if (!mixer) {
        fprintf(stderr, "Failed to open mixer\n");
        return EXIT_FAILURE;
    }

    audio_spk_headset_switch = mixer_get_ctl_by_name(mixer, MIXER_AUDIO_NORMAL_SPEAKER_HEADSET);
    if (!audio_spk_headset_switch) {
		printf("Unable to get audio_spk_headset_switch, aborting.");
	    return -1;
    }

	file = fopen("/media/48000-stere-10s.wav", "rb");

	err = pthread_create(&ntid, NULL, (void*)mixer_thread, NULL);
    if(err != 0){
	printf("create pthread error: %s, %d\n", __func__, __LINE__);
	exit(0);
    }
    /* install signal handler and begin hp_plug_judge */
    play_sample(file, device, 2, play_rate, 16);

    mixer_close(mixer);
	hp_plug_judge = 0;
	fclose(file);
    return ret;
}

void play_sample(FILE *file, unsigned int device, unsigned int channels,
                 unsigned int rate, unsigned int bits)
{
    struct pcm_config config;
    struct pcm *pcm0;
    char *buffer;
    int size;
    int num_read;
	int dtime = 12*60*60;
	int loop_time = 0;
	int i=0;

    config.channels = 2;
    config.rate = rate;
    config.period_size = 1024;
    config.period_count = 4;
    if (bits == 32)
        config.format = PCM_FORMAT_S32_LE;
    else if (bits == 16)
        config.format = PCM_FORMAT_S16_LE;
    config.start_threshold = 0;
    config.stop_threshold = 0;
    config.silence_threshold = 0;

	/*0 is audiocodec, 1 is hdmiaudio, 2 is spdif*/
    pcm0 = pcm_open(0, device, PCM_OUT, &config);
    printf("1111\n");
    if (!pcm0 || !pcm_is_ready(pcm0)) {
		fprintf(stderr, "Unable to open PCM device %u (%s)\n", device, pcm_get_error(pcm0));
		return;
	}

    size = pcm_get_buffer_size(pcm0);

    buffer = malloc(size);
    if (!buffer) {
        fprintf(stderr, "Unable to allocate %d bytes\n", size);
        free(buffer);
        pcm_close(pcm0);
        return;
    }


    loop_time = dtime*rate/1024;

    do {
        num_read = fread(buffer, 1, size, file);
        if (num_read > 0) {
            if (pcm_write(pcm0, buffer, num_read)) {
                fprintf(stderr, "Error playing sample\n");
		      break;
            }
		if (feof(file)) {
				fseek(file, 0L, SEEK_SET);
			}
        }
        i++;
        if(i > loop_time)
		break;
    } while ((num_read > 0));

    free(buffer);
    pcm_close(pcm0);
}
