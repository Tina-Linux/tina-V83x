/*
 * audiocodec test
 * test_record_play.c
 * (C) Copyright 2010-2016
 * allwinnertech Technology Co., Ltd. <www.allwinnertech.com>
 * huangxin <huangxin@allwinnertech.com>
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
#include <sys/time.h>
#include<unistd.h>
#include <pthread.h>
#include <signal.h>
#include <math.h>
#include "include/tinyalsa/asoundlib.h"
#include "sine_detector.h"

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

#define FORMAT_PCM 1
#define MIXER_AUDIO_NORMAL_SPEAKER_HEADSET	"Speaker Function"

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

int capturing = 1;
static int capture_rate = 0;
static char capture_wav_file[64] = "record.wav";
pthread_t ntid;

void play_sample(FILE *file, unsigned int device, unsigned int channels,
					unsigned int rate, unsigned int bits);

unsigned int capture_sample(FILE *file, unsigned int device, unsigned int channels,
					unsigned int rate, unsigned int bits);

void sine_detector(FILE *file_in, int sample_freq, int freq, float threshold);

void sigint_handler(int sig)
{
    capturing = 0;
}

void sine_detector(FILE *file_in, int sample_freq, int freq, float threshold)
{
	short			BUFFER[NN];
	short			NewSp[NN];
	nf_ref_t		nf_reference;

	int	f_len = 0;
	int	total_frame;
	int	frame;
	int	DataNum = 0;
	int ret;
	int	  sin_flag;

/****************************************************************/
	ret = NF_init(&nf_reference, sample_freq, freq, threshold);
	if (!ret) {
		printf(" Calloc buffer error !");
	}

	fseek(file_in, 0, SEEK_END);
	f_len		= ftell(file_in);
	total_frame = f_len/BYTE/NN - 10;
	fseek(file_in, 0, SEEK_SET);
	memset(BUFFER,0,sizeof(BUFFER));
	memset(NewSp,0,sizeof(NewSp));

	for (frame = 4; frame < total_frame + 1; frame++) {
		DataNum = (int)fread(BUFFER, sizeof(short), NN, file_in);
		sin_flag = notch_filter(&nf_reference,BUFFER,NewSp,DataNum);
	}
	sin_flag = NF_end(&nf_reference);
	printf(" frame number:  end\n");

	if (sin_flag) {
		printf("pass:SINE signal\n");
		remove(capture_wav_file);
	} else {
		printf("fail:Not A SINE signal\n");
	}
}

unsigned int capture_sample(FILE *capture_file, unsigned int device,
                            unsigned int channels, unsigned int rate,
                            unsigned int bits)
{
    struct pcm_config config;
    struct pcm *pcm;
    char *buffer;
    unsigned int size;
    unsigned int bytes_read = 0;
	int dtime = 30;
	int loop_time = 0;
	int i = 0;

    config.channels = channels;
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

    pcm = pcm_open(0, device, PCM_IN, &config);
    if (!pcm || !pcm_is_ready(pcm)) {
        fprintf(stderr, "Unable to open PCM device (%s)\n",
                pcm_get_error(pcm));
        return 0;
    }

    size = pcm_get_buffer_size(pcm);
    buffer = malloc(size);
    if (!buffer) {
        free(buffer);
        pcm_close(pcm);
        return 0;
    }
    loop_time = dtime*(config.rate)/1024;
    printf("capture loop_time:%d\n", loop_time);
    printf("Capturing sample: %u ch, %u hz, %u bit\n", channels, rate, bits);

    while (capturing && !pcm_read(pcm, buffer, size)) {
        if (fwrite(buffer, 1, size, capture_file) != size) {
            fprintf(stderr,"Error capturing sample\n");
            break;
        }
        bytes_read += size;
                i++;
        if(i > loop_time)
		break;
    }

    free(buffer);
    pcm_close(pcm);
    return bytes_read / ((bits / 8) * channels);
}

void *capture_thread(void *arg)
{
	FILE *capture_file;
    struct wav_header capture_header;
    unsigned int device = 0;
    unsigned int channels = 1;
    unsigned int rate = capture_rate;
    unsigned int bits = 16;
    unsigned int frames;

    capture_file = fopen(capture_wav_file, "wb");
    if (!capture_file) {
        fprintf(stderr, "Unable to create file\n");
        return NULL;
    }

    capture_header.riff_id = ID_RIFF;
    capture_header.riff_sz = 0;
    capture_header.riff_fmt = ID_WAVE;
    capture_header.fmt_id = ID_FMT;
    capture_header.fmt_sz = 16;
    capture_header.audio_format = FORMAT_PCM;
    capture_header.num_channels = 1;
    capture_header.sample_rate = rate;
    capture_header.bits_per_sample = bits;
    capture_header.byte_rate = (capture_header.bits_per_sample / 8) * channels * rate;
    capture_header.block_align = channels * (capture_header.bits_per_sample / 8);
    capture_header.data_id = ID_DATA;
    /* leave enough room for header */
    fseek(capture_file, sizeof(struct wav_header), SEEK_SET);
	usleep(300000);
    frames = capture_sample(capture_file, device, capture_header.num_channels, capture_header.sample_rate, capture_header.bits_per_sample);

    /* write header now all information is known */
    capture_header.data_sz = frames * capture_header.block_align;
    fseek(capture_file, 0, SEEK_SET);
    fwrite(&capture_header, sizeof(struct wav_header), 1, capture_file);
    fclose(capture_file);

    return 0;
}

int main(int argc, char **argv)
{
    FILE *file;
    unsigned int device = 0;
	int err = 0;
    struct mixer *mixer;
	struct mixer_ctl *audio_spk_headset_switch;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s file.wav [-d device]\n", argv[0]);
        return 1;
    }
	capture_rate = atoi(argv[1]);
    file = fopen("/media/send.pcm", "rb");
    if (!file) {
        fprintf(stderr, "Unable to open file '%s'\n", argv[1]);
        return 1;
    }

    /* parse command line arguments */
    argv += 2;
    while (*argv) {
        if (strcmp(*argv, "-d") == 0) {
            argv++;
            device = atoi(*argv);
        }
        argv++;
    }

    mixer = mixer_open(0);
    if (!mixer) {
	    printf("Unable to open the mixer, aborting.");
	    fclose(file);
	    return -1;
    }

    audio_spk_headset_switch = mixer_get_ctl_by_name(mixer, MIXER_AUDIO_NORMAL_SPEAKER_HEADSET);
    if (!audio_spk_headset_switch) {
		printf("Unable to get audio_spk_headset_switch, aborting.");
		mixer_close(mixer);
		fclose(file);
	    return -1;
    }
	mixer_ctl_set_enum_by_string(audio_spk_headset_switch, "spk");
	mixer_close(mixer);

	err = pthread_create(&ntid, NULL, (void*)capture_thread, NULL);
    if (err != 0) {
		printf("create pthread error: %s, %d\n", __func__, __LINE__);
		exit(0);
    }

    /* install signal handler and begin capturing */
    play_sample(file, device, 1, capture_rate, 16);

    fclose(file);
    capturing = 0;

	usleep(1000000);
	printf("sine detector start\n");
	file = fopen(capture_wav_file, "rb");
	sine_detector(file, capture_rate, 1000, 10);
	fclose(file);
	printf("sine detector end\n");
    return 0;
}

void play_sample(FILE *file, unsigned int device, unsigned int channels,
                 unsigned int rate, unsigned int bits)
{
    struct pcm_config config;
    struct pcm *pcm0;
    char *buffer;
    int size;
    int num_read;

    config.channels = channels;
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
    if (!pcm0 || !pcm_is_ready(pcm0)) {
		fprintf(stderr, "Unable to open PCM device %u (%s)\n", device, pcm_get_error(pcm0));
		return;
	}

    size = pcm_get_buffer_size(pcm0);
    printf("size:%d\n",size);
    buffer = malloc(size);
    if (!buffer) {
        fprintf(stderr, "Unable to allocate %d bytes\n", size);
        free(buffer);
        pcm_close(pcm0);
        return;
    }
    printf("Playing sample: %u ch, %u hz, %u bit\n", channels, rate, bits);

    do {
        num_read = fread(buffer, 1, size, file);
        if (num_read > 0) {
            if (pcm_write(pcm0, buffer, num_read)) {
                fprintf(stderr, "Error playing sample\n");
                break;
            }
        }
    } while (num_read > 0);
	usleep(10000);
    free(buffer);
    pcm_close(pcm0);
}
