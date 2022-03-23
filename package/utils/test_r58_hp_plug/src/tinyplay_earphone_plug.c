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

#define AIF1IN0R_MUX					"AIF1IN0R Mux"
#define AIF1IN0L_MUX					"AIF1IN0L Mux"
#define DACR_MIXER_AIF1DA0R_SWITCH		"DACR Mixer AIF1DA0R Switch"
#define DACL_MIXER_AIF1DA0L_SWITCH		"DACL Mixer AIF1DA0L Switch"

/*headphone out*/
#define HEADPHONE_VOLUME				"headphone volume"
#define HP_R_MUX						"HP_R Mux"
#define HP_L_MUX						"HP_L Mux"
#define HEADPHONE_SWITCH				"Headphone Switch"
/*spk out*/
#define SPEAKER_VOLUME					"speaker volume"
#define RIGHT_OUTPUT_MIXER_DACR_SWITCH	"Right Output Mixer DACR Switch"
#define LEFT_OUTPUT_MIXER_DACL_SWITCH	"Left Output Mixer DACL Switch"
#define SPK_L_MUX						"SPK_L Mux"
#define	SPK_R_MUX						"SPK_R Mux"
#define EXTERNAL_SPEAKER_SWITCH			"External Speaker Switch"

#define PRESS_DATA_NAME    "headset"
#define PRESS_SYSFS_PATH   "/sys/class/input"

int fd;
pthread_t ntid;
int hp_plug_judge = 1;

struct input_event buff;

void play_sample(FILE *file, unsigned int device, unsigned int channels,
                 unsigned int rate, unsigned int bits);

void sigint_handler(int sig)
{
    hp_plug_judge = 0;
}

int spk_enable(int enable)
{
	struct mixer *mixer;
	/*SPK OUT*/
	struct mixer_ctl *speaker_volume;
	struct mixer_ctl *aif1in0r_mux;
	struct mixer_ctl *aif1in0l_mux;
	struct mixer_ctl *dacr_mixer_aif1da0r_switch;
	struct mixer_ctl *dacl_mixer_aif1da0l_switch;
	struct mixer_ctl *right_output_mixer_dacr_switch;
	struct mixer_ctl *left_output_mixer_dacl_switch;
	struct mixer_ctl *spk_l_mux;
	struct mixer_ctl *spk_r_mux;
	struct mixer_ctl *external_speaker_switch;

	mixer = mixer_open(0);
    if (!mixer) {
        fprintf(stderr, "Failed to open mixer\n");
        return EXIT_FAILURE;
    }

    speaker_volume = mixer_get_ctl_by_name(mixer, SPEAKER_VOLUME);
    if (!speaker_volume) {
		printf("Unable to get speaker_volume, aborting.");
	    return -1;
    }
    aif1in0r_mux = mixer_get_ctl_by_name(mixer, AIF1IN0R_MUX);
    if (!aif1in0r_mux) {
		printf("Unable to get aif1in0r_mux, aborting.");
	    return -1;
    }
    aif1in0l_mux = mixer_get_ctl_by_name(mixer, AIF1IN0L_MUX);
    if (!aif1in0l_mux) {
		printf("Unable to get aif1in0l_mux, aborting.");
	    return -1;
    }
	dacr_mixer_aif1da0r_switch = mixer_get_ctl_by_name(mixer, DACR_MIXER_AIF1DA0R_SWITCH);
    if (!dacr_mixer_aif1da0r_switch) {
		printf("Unable to get dacr_mixer_aif1da0r_switch, aborting.");
	    return -1;
    }
    dacl_mixer_aif1da0l_switch = mixer_get_ctl_by_name(mixer, DACL_MIXER_AIF1DA0L_SWITCH);
    if (!dacl_mixer_aif1da0l_switch) {
		printf("Unable to get dacl_mixer_aif1da0l_switch, aborting.");
	    return -1;
    }
    right_output_mixer_dacr_switch = mixer_get_ctl_by_name(mixer, RIGHT_OUTPUT_MIXER_DACR_SWITCH);
    if (!right_output_mixer_dacr_switch) {
		printf("Unable to get right_output_mixer_dacr_switch, aborting.");
	    return -1;
    }
    left_output_mixer_dacl_switch = mixer_get_ctl_by_name(mixer, LEFT_OUTPUT_MIXER_DACL_SWITCH);
    if (!left_output_mixer_dacl_switch) {
		printf("Unable to get left_output_mixer_dacl_switch, aborting.");
	    return -1;
    }
    spk_l_mux = mixer_get_ctl_by_name(mixer, SPK_L_MUX);
    if (!spk_l_mux) {
		printf("Unable to get spk_l_mux, aborting.");
	    return -1;
    }
    spk_r_mux = mixer_get_ctl_by_name(mixer, SPK_R_MUX);
    if (!spk_r_mux) {
		printf("Unable to get spk_r_mux, aborting.");
	    return -1;
    }
    external_speaker_switch = mixer_get_ctl_by_name(mixer, EXTERNAL_SPEAKER_SWITCH);
    if (!external_speaker_switch) {
		printf("Unable to get external_speaker_switch, aborting.");
	    return -1;
    }
	if (enable) {
		mixer_ctl_set_value(speaker_volume, 0, 30);
		mixer_ctl_set_enum_by_string(aif1in0r_mux, "AIF1_DA0R");
		mixer_ctl_set_enum_by_string(aif1in0l_mux, "AIF1_DA0L");
		mixer_ctl_set_value(dacr_mixer_aif1da0r_switch, 0, 1);
		mixer_ctl_set_value(dacl_mixer_aif1da0l_switch, 0, 1);
		mixer_ctl_set_value(right_output_mixer_dacr_switch, 0, 1);
		mixer_ctl_set_value(left_output_mixer_dacl_switch, 0, 1);
		mixer_ctl_set_enum_by_string(spk_l_mux, "MIXEL Switch");
		mixer_ctl_set_enum_by_string(spk_r_mux, "MIXER Switch");
		mixer_ctl_set_value(external_speaker_switch, 0, 1);
	} else {
		mixer_ctl_set_value(dacr_mixer_aif1da0r_switch, 0, 0);
		mixer_ctl_set_value(dacl_mixer_aif1da0l_switch, 0, 0);
		mixer_ctl_set_value(right_output_mixer_dacr_switch, 0, 0);
		mixer_ctl_set_value(left_output_mixer_dacl_switch, 0, 0);
		mixer_ctl_set_value(external_speaker_switch, 0, 0);
	}
	mixer_close(mixer);
	return 0;
}

int hp_enable(int enable)
{
	struct mixer *mixer;
	/*HEADPHONE OUT*/
	struct mixer_ctl *headphone_volume;
	struct mixer_ctl *aif1in0r_mux;
	struct mixer_ctl *aif1in0l_mux;
	struct mixer_ctl *dacr_mixer_aif1da0r_switch;
	struct mixer_ctl *dacl_mixer_aif1da0l_switch;
	struct mixer_ctl *hp_r_mux;
	struct mixer_ctl *hp_l_mux;
	struct mixer_ctl *headphone_switch;

	mixer = mixer_open(0);
	if (!mixer) {
        fprintf(stderr, "Failed to open mixer\n");
        return EXIT_FAILURE;
    }
    headphone_volume = mixer_get_ctl_by_name(mixer, HEADPHONE_VOLUME);
    if (!headphone_volume) {
		printf("Unable to get headphone_volume, aborting.");
	    return -1;
    }
	aif1in0r_mux = mixer_get_ctl_by_name(mixer, AIF1IN0R_MUX);
    if (!aif1in0r_mux) {
		printf("Unable to get aif1in0r_mux, aborting.");
	    return -1;
    }
    aif1in0l_mux = mixer_get_ctl_by_name(mixer, AIF1IN0L_MUX);
    if (!aif1in0l_mux) {
		printf("Unable to get aif1in0l_mux, aborting.");
	    return -1;
    }
	dacr_mixer_aif1da0r_switch = mixer_get_ctl_by_name(mixer, DACR_MIXER_AIF1DA0R_SWITCH);
    if (!dacr_mixer_aif1da0r_switch) {
		printf("Unable to get dacr_mixer_aif1da0r_switch, aborting.");
	    return -1;
    }
    dacl_mixer_aif1da0l_switch = mixer_get_ctl_by_name(mixer, DACL_MIXER_AIF1DA0L_SWITCH);
    if (!dacl_mixer_aif1da0l_switch) {
		printf("Unable to get dacl_mixer_aif1da0l_switch, aborting.");
	    return -1;
    }
    hp_r_mux = mixer_get_ctl_by_name(mixer, HP_R_MUX);
    if (!hp_r_mux) {
		printf("Unable to get hp_r_mux, aborting.");
	    return -1;
    }
    hp_l_mux = mixer_get_ctl_by_name(mixer, HP_L_MUX);
    if (!hp_l_mux) {
		printf("Unable to get hp_l_mux, aborting.");
	    return -1;
    }
    headphone_switch = mixer_get_ctl_by_name(mixer, HEADPHONE_SWITCH);
    if (!headphone_switch) {
		printf("Unable to get headphone_switch, aborting.");
	    return -1;
    }
	if (enable) {
		mixer_ctl_set_value(headphone_volume, 0, 59);
		mixer_ctl_set_enum_by_string(aif1in0r_mux, "AIF1_DA0R");
		mixer_ctl_set_enum_by_string(aif1in0l_mux, "AIF1_DA0L");
		mixer_ctl_set_value(dacr_mixer_aif1da0r_switch, 0, 1);
		mixer_ctl_set_value(dacl_mixer_aif1da0l_switch, 0, 1);
		mixer_ctl_set_enum_by_string(hp_r_mux, "DACR HPR Switch");
		mixer_ctl_set_enum_by_string(hp_l_mux, "DACL HPL Switch");
		mixer_ctl_set_value(headphone_switch, 0, 1);
	} else {
		mixer_ctl_set_value(dacr_mixer_aif1da0r_switch, 0, 1);
		mixer_ctl_set_value(dacl_mixer_aif1da0l_switch, 0, 1);
		mixer_ctl_set_value(headphone_switch, 0, 0);
	}
	mixer_close(mixer);
	return 0;
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
			spk_enable(0);
			hp_enable(1);
			printf("hp plug in\n");
		} else if (buff.code == KEY_HP && buff.value == 0) {
			spk_enable(1);
			hp_enable(0);
			printf("hp plug out\n");
		}
	}
	close(fd);

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
		spk_enable(0);
		hp_enable(1);
	} else {
		printf("spk:%s, %d\n", __func__, __LINE__);
		hp_enable(0);
		spk_enable(1);
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
	file = fopen("/media/48000-stere-10s.wav", "rb");

	err = pthread_create(&ntid, NULL, (void*)mixer_thread, NULL);
    if (err != 0){
		printf("create pthread error: %s, %d\n", __func__, __LINE__);
		exit(0);
    }
    /* install signal handler and begin hp_plug_judge */
    play_sample(file, device, 2, play_rate, 16);

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
