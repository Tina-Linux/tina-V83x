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

/*play*/
#define AIF1IN0R_MUX					"AIF1IN0R Mux"
#define AIF1IN0L_MUX					"AIF1IN0L Mux"
#define DACR_MIXER_AIF1DA0R_SWITCH		"DACR Mixer AIF1DA0R Switch"
#define DACL_MIXER_AIF1DA0L_SWITCH		"DACL Mixer AIF1DA0L Switch"

/*spk out*/
#define SPEAKER_VOLUME							"speaker volume"
#define RIGHT_OUTPUT_MIXER_DACR_SWITCH			"Right Output Mixer DACR Switch"
#define LEFT_OUTPUT_MIXER_DACL_SWITCH			"Left Output Mixer DACL Switch"
#define SPK_L_MUX								"SPK_L Mux"
#define	SPK_R_MUX								"SPK_R Mux"
#define EXTERNAL_SPEAKER_SWITCH					"External Speaker Switch"

/*record*/
#define AIF1OUT0L_MUX							"AIF1OUT0L Mux"
#define AIF1OUT0R_MUX							"AIF1OUT0R Mux"
#define AIF1_AD0L_MIXER_ADCL_SWITCH				"AIF1 AD0L Mixer ADCL Switch"
#define AIF1_AD0R_MIXER_ADCR_SWITCH				"AIF1 AD0R Mixer ADCR Switch"
#define ADCR_MUX								"ADCR Mux"
#define ADCL_MUX								"ADCL Mux"

/*mic1*/
#define LEFT_ADC_INPUT_MIXER_MIC1_BOOST_SWITCH	"LEFT ADC input Mixer MIC1 boost Switch"
#define RIGHT_ADC_INPUT_MIXER_MIC1_BOOST_SWITCH	"RIGHT ADC input Mixer MIC1 boost Switch"
#define MIC1_BOOST_AMPLIFIER_GAIN				"MIC1 boost amplifier gain"

/*mic2*/
#define LEFT_ADC_INPUT_MIXER_MIC2_BOOST_SWITCH	"LEFT ADC input Mixer MIC2 boost Switch"
#define RIGHT_ADC_INPUT_MIXER_MIC2_BOOST_SWITCH	"RIGHT ADC input Mixer MIC2 boost Switch"
#define MIC2_SRC								"MIC2 SRC"
#define MIC2_BOOST_AMPLIFIER_GAIN				"MIC2 boost amplifier gain"

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
	total_frame = f_len/BYTE/NN - 8;
	fseek(file_in, 0, SEEK_SET);
	memset(BUFFER,0,sizeof(BUFFER));
	memset(NewSp,0,sizeof(NewSp));
	printf("%s,l:%d,total_frame:%d\n", __func__, __LINE__, total_frame);
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

int mic1_enable(int enable)
{
	struct mixer *mixer;
	struct mixer_ctl *aif1out0l_mux;
	struct mixer_ctl *aif1out0r_mux;
	struct mixer_ctl *aif1_ad0l_mixer_adcl_switch;
	struct mixer_ctl *aif1_ad0r_mixer_adcr_switch;
	struct mixer_ctl *adcr_mux;
	struct mixer_ctl *adcl_mux;
	struct mixer_ctl *left_adc_input_mixer_mic1_boost_switch;
	struct mixer_ctl *right_adc_input_mixer_mic1_boost_switch;
	struct mixer_ctl *mic1_boost_amplifier_gain;

	mixer = mixer_open(0);
    if (!mixer) {
        fprintf(stderr, "Failed to open mixer\n");
        return EXIT_FAILURE;
    }

    aif1out0l_mux = mixer_get_ctl_by_name(mixer, AIF1OUT0L_MUX);
    if (!aif1out0l_mux) {
		printf("Unable to get aif1out0l_mux, aborting.");
	    return -1;
    }
    aif1out0r_mux = mixer_get_ctl_by_name(mixer, AIF1OUT0R_MUX);
    if (!aif1out0r_mux) {
		printf("Unable to get aif1out0r_mux, aborting.");
	    return -1;
    }
    aif1_ad0l_mixer_adcl_switch = mixer_get_ctl_by_name(mixer, AIF1_AD0L_MIXER_ADCL_SWITCH);
    if (!aif1_ad0l_mixer_adcl_switch) {
		printf("Unable to get aif1_ad0l_mixer_adcl_switch, aborting.");
	    return -1;
    }
    aif1_ad0r_mixer_adcr_switch = mixer_get_ctl_by_name(mixer, AIF1_AD0R_MIXER_ADCR_SWITCH);
    if (!aif1_ad0r_mixer_adcr_switch) {
		printf("Unable to get aif1_ad0r_mixer_adcr_switch, aborting.");
	    return -1;
    }
    adcr_mux = mixer_get_ctl_by_name(mixer, ADCR_MUX);
    if (!adcr_mux) {
		printf("Unable to get adcr_mux, aborting.");
	    return -1;
    }
    adcl_mux = mixer_get_ctl_by_name(mixer, ADCL_MUX);
    if (!adcl_mux) {
		printf("Unable to get adcl_mux, aborting.");
	    return -1;
    }
    left_adc_input_mixer_mic1_boost_switch = mixer_get_ctl_by_name(mixer, LEFT_ADC_INPUT_MIXER_MIC1_BOOST_SWITCH);
    if (!left_adc_input_mixer_mic1_boost_switch) {
		printf("Unable to get left_adc_input_mixer_mic1_boost_switch, aborting.");
	    return -1;
    }
    right_adc_input_mixer_mic1_boost_switch = mixer_get_ctl_by_name(mixer, RIGHT_ADC_INPUT_MIXER_MIC1_BOOST_SWITCH);
    if (!right_adc_input_mixer_mic1_boost_switch) {
		printf("Unable to get right_adc_input_mixer_mic1_boost_switch, aborting.");
	    return -1;
    }
    mic1_boost_amplifier_gain = mixer_get_ctl_by_name(mixer, MIC1_BOOST_AMPLIFIER_GAIN);
    if (!mic1_boost_amplifier_gain) {
		printf("Unable to get mic1_boost_amplifier_gain, aborting.");
	    return -1;
    }
	if (enable) {
		mixer_ctl_set_enum_by_string(aif1out0l_mux, "AIF1_AD0L");
		mixer_ctl_set_enum_by_string(aif1out0r_mux, "AIF1_AD0R");
		mixer_ctl_set_value(aif1_ad0l_mixer_adcl_switch, 0, 1);
		mixer_ctl_set_value(aif1_ad0r_mixer_adcr_switch, 0, 1);
		mixer_ctl_set_enum_by_string(adcr_mux, "ADC");
		mixer_ctl_set_enum_by_string(adcl_mux, "ADC");
		mixer_ctl_set_value(left_adc_input_mixer_mic1_boost_switch, 0, 1);
		mixer_ctl_set_value(right_adc_input_mixer_mic1_boost_switch, 0, 1);
		mixer_ctl_set_value(mic1_boost_amplifier_gain, 0, 7);
	} else {
		mixer_ctl_set_value(aif1_ad0l_mixer_adcl_switch, 0, 0);
		mixer_ctl_set_value(aif1_ad0r_mixer_adcr_switch, 0, 0);
		mixer_ctl_set_value(left_adc_input_mixer_mic1_boost_switch, 0, 0);
		mixer_ctl_set_value(right_adc_input_mixer_mic1_boost_switch, 0, 0);
	}
	mixer_close(mixer);
	return 0;
}

int mic2_enable(int enable)
{
	struct mixer *mixer;
	struct mixer_ctl *aif1out0l_mux;
	struct mixer_ctl *aif1out0r_mux;
	struct mixer_ctl *aif1_ad0l_mixer_adcl_switch;
	struct mixer_ctl *aif1_ad0r_mixer_adcr_switch;
	struct mixer_ctl *adcr_mux;
	struct mixer_ctl *adcl_mux;
	struct mixer_ctl *left_adc_input_mixer_mic2_boost_switch;
	struct mixer_ctl *right_adc_input_mixer_mic2_boost_switch;
	struct mixer_ctl *mic2_src;
	struct mixer_ctl *mic2_boost_amplifier_gain;

	mixer = mixer_open(0);
    if (!mixer) {
        fprintf(stderr, "Failed to open mixer\n");
        return EXIT_FAILURE;
    }

    aif1out0l_mux = mixer_get_ctl_by_name(mixer, AIF1OUT0L_MUX);
    if (!aif1out0l_mux) {
		printf("Unable to get aif1out0l_mux, aborting.");
	    return -1;
    }
    aif1out0r_mux = mixer_get_ctl_by_name(mixer, AIF1OUT0R_MUX);
    if (!aif1out0r_mux) {
		printf("Unable to get aif1out0r_mux, aborting.");
	    return -1;
    }
    aif1_ad0l_mixer_adcl_switch = mixer_get_ctl_by_name(mixer, AIF1_AD0L_MIXER_ADCL_SWITCH);
    if (!aif1_ad0l_mixer_adcl_switch) {
		printf("Unable to get aif1_ad0l_mixer_adcl_switch, aborting.");
	    return -1;
    }
    aif1_ad0r_mixer_adcr_switch = mixer_get_ctl_by_name(mixer, AIF1_AD0R_MIXER_ADCR_SWITCH);
    if (!aif1_ad0r_mixer_adcr_switch) {
		printf("Unable to get aif1_ad0r_mixer_adcr_switch, aborting.");
	    return -1;
    }
    adcr_mux = mixer_get_ctl_by_name(mixer, ADCR_MUX);
    if (!adcr_mux) {
		printf("Unable to get adcr_mux, aborting.");
	    return -1;
    }
    adcl_mux = mixer_get_ctl_by_name(mixer, ADCL_MUX);
    if (!adcl_mux) {
		printf("Unable to get adcl_mux, aborting.");
	    return -1;
    }
    left_adc_input_mixer_mic2_boost_switch = mixer_get_ctl_by_name(mixer, LEFT_ADC_INPUT_MIXER_MIC2_BOOST_SWITCH);
    if (!left_adc_input_mixer_mic2_boost_switch) {
		printf("Unable to get left_adc_input_mixer_mic2_boost_switch, aborting.");
	    return -1;
    }
    right_adc_input_mixer_mic2_boost_switch = mixer_get_ctl_by_name(mixer, RIGHT_ADC_INPUT_MIXER_MIC2_BOOST_SWITCH);
    if (!right_adc_input_mixer_mic2_boost_switch) {
		printf("Unable to get right_adc_input_mixer_mic2_boost_switch, aborting.");
	    return -1;
    }
    mic2_src = mixer_get_ctl_by_name(mixer, MIC2_SRC);
    if (!mic2_src) {
		printf("Unable to get mic2_src, aborting.");
	    return -1;
    }
    mic2_boost_amplifier_gain = mixer_get_ctl_by_name(mixer, MIC2_BOOST_AMPLIFIER_GAIN);
    if (!mic2_boost_amplifier_gain) {
		printf("Unable to get mic2_boost_amplifier_gain, aborting.");
	    return -1;
    }
	if (enable) {
		mixer_ctl_set_enum_by_string(aif1out0l_mux, "AIF1_AD0L");
		mixer_ctl_set_enum_by_string(aif1out0r_mux, "AIF1_AD0R");
		mixer_ctl_set_value(aif1_ad0l_mixer_adcl_switch, 0, 1);
		mixer_ctl_set_value(aif1_ad0r_mixer_adcr_switch, 0, 1);
		mixer_ctl_set_enum_by_string(adcr_mux, "ADC");
		mixer_ctl_set_enum_by_string(adcl_mux, "ADC");
		mixer_ctl_set_value(left_adc_input_mixer_mic2_boost_switch, 0, 1);
		mixer_ctl_set_value(right_adc_input_mixer_mic2_boost_switch, 0, 1);
		mixer_ctl_set_enum_by_string(mic2_src, "MIC2");
		mixer_ctl_set_value(mic2_boost_amplifier_gain, 0, 7);
	} else {
		mixer_ctl_set_value(aif1_ad0l_mixer_adcl_switch, 0, 0);
		mixer_ctl_set_value(aif1_ad0r_mixer_adcr_switch, 0, 0);
		mixer_ctl_set_value(left_adc_input_mixer_mic2_boost_switch, 0, 0);
		mixer_ctl_set_value(right_adc_input_mixer_mic2_boost_switch, 0, 0);
	}
	mixer_close(mixer);
	return 0;
}

int mic1_l_mic2_r_enable(int enable)
{
	struct mixer *mixer;
	struct mixer_ctl *aif1out0l_mux;
	struct mixer_ctl *aif1out0r_mux;
	struct mixer_ctl *aif1_ad0l_mixer_adcl_switch;
	struct mixer_ctl *aif1_ad0r_mixer_adcr_switch;
	struct mixer_ctl *adcr_mux;
	struct mixer_ctl *adcl_mux;
	struct mixer_ctl *left_adc_input_mixer_mic1_boost_switch;
	struct mixer_ctl *right_adc_input_mixer_mic2_boost_switch;
	struct mixer_ctl *mic2_src;
	struct mixer_ctl *mic2_boost_amplifier_gain;
	struct mixer_ctl *mic1_boost_amplifier_gain;

	mixer = mixer_open(0);
    if (!mixer) {
        fprintf(stderr, "Failed to open mixer\n");
        return EXIT_FAILURE;
    }

    aif1out0l_mux = mixer_get_ctl_by_name(mixer, AIF1OUT0L_MUX);
    if (!aif1out0l_mux) {
		printf("Unable to get aif1out0l_mux, aborting.");
	    return -1;
    }
    aif1out0r_mux = mixer_get_ctl_by_name(mixer, AIF1OUT0R_MUX);
    if (!aif1out0r_mux) {
		printf("Unable to get aif1out0r_mux, aborting.");
	    return -1;
    }
    aif1_ad0l_mixer_adcl_switch = mixer_get_ctl_by_name(mixer, AIF1_AD0L_MIXER_ADCL_SWITCH);
    if (!aif1_ad0l_mixer_adcl_switch) {
		printf("Unable to get aif1_ad0l_mixer_adcl_switch, aborting.");
	    return -1;
    }
    aif1_ad0r_mixer_adcr_switch = mixer_get_ctl_by_name(mixer, AIF1_AD0R_MIXER_ADCR_SWITCH);
    if (!aif1_ad0r_mixer_adcr_switch) {
		printf("Unable to get aif1_ad0r_mixer_adcr_switch, aborting.");
	    return -1;
    }
    adcr_mux = mixer_get_ctl_by_name(mixer, ADCR_MUX);
    if (!adcr_mux) {
		printf("Unable to get adcr_mux, aborting.");
	    return -1;
    }
    adcl_mux = mixer_get_ctl_by_name(mixer, ADCL_MUX);
    if (!adcl_mux) {
		printf("Unable to get adcl_mux, aborting.");
	    return -1;
    }
    left_adc_input_mixer_mic1_boost_switch = mixer_get_ctl_by_name(mixer, LEFT_ADC_INPUT_MIXER_MIC1_BOOST_SWITCH);
    if (!left_adc_input_mixer_mic1_boost_switch) {
		printf("Unable to get left_adc_input_mixer_mic1_boost_switch, aborting.");
	    return -1;
    }
    right_adc_input_mixer_mic2_boost_switch = mixer_get_ctl_by_name(mixer, RIGHT_ADC_INPUT_MIXER_MIC2_BOOST_SWITCH);
    if (!right_adc_input_mixer_mic2_boost_switch) {
		printf("Unable to get right_adc_input_mixer_mic2_boost_switch, aborting.");
	    return -1;
    }
    mic2_src = mixer_get_ctl_by_name(mixer, MIC2_SRC);
    if (!mic2_src) {
		printf("Unable to get mic2_src, aborting.");
	    return -1;
    }
    mic2_boost_amplifier_gain = mixer_get_ctl_by_name(mixer, MIC2_BOOST_AMPLIFIER_GAIN);
    if (!mic2_boost_amplifier_gain) {
		printf("Unable to get mic2_boost_amplifier_gain, aborting.");
	    return -1;
    }
    mic1_boost_amplifier_gain = mixer_get_ctl_by_name(mixer, MIC1_BOOST_AMPLIFIER_GAIN);
    if (!mic1_boost_amplifier_gain) {
		printf("Unable to get mic1_boost_amplifier_gain, aborting.");
	    return -1;
    }
	if (enable) {
		mixer_ctl_set_enum_by_string(aif1out0l_mux, "AIF1_AD0L");
		mixer_ctl_set_enum_by_string(aif1out0r_mux, "AIF1_AD0R");
		mixer_ctl_set_value(aif1_ad0l_mixer_adcl_switch, 0, 1);
		mixer_ctl_set_value(aif1_ad0r_mixer_adcr_switch, 0, 1);
		mixer_ctl_set_enum_by_string(adcr_mux, "ADC");
		mixer_ctl_set_enum_by_string(adcl_mux, "ADC");
		mixer_ctl_set_value(left_adc_input_mixer_mic1_boost_switch, 0, 1);
		mixer_ctl_set_value(right_adc_input_mixer_mic2_boost_switch, 0, 1);
		mixer_ctl_set_enum_by_string(mic2_src, "MIC2");
		mixer_ctl_set_value(mic2_boost_amplifier_gain, 0, 7);
		mixer_ctl_set_value(mic1_boost_amplifier_gain, 0, 7);
	} else {
		mixer_ctl_set_value(aif1_ad0l_mixer_adcl_switch, 0, 0);
		mixer_ctl_set_value(aif1_ad0r_mixer_adcr_switch, 0, 0);
		mixer_ctl_set_value(left_adc_input_mixer_mic1_boost_switch, 0, 0);
		mixer_ctl_set_value(right_adc_input_mixer_mic2_boost_switch, 0, 0);
	}
	mixer_close(mixer);
	return 0;
}

int dmic_enable(int enable)
{
	struct mixer *mixer;
	struct mixer_ctl *aif1out0l_mux;
	struct mixer_ctl *aif1out0r_mux;
	struct mixer_ctl *aif1_ad0l_mixer_adcl_switch;
	struct mixer_ctl *aif1_ad0r_mixer_adcr_switch;
	struct mixer_ctl *adcr_mux;
	struct mixer_ctl *adcl_mux;
	mixer = mixer_open(0);
    if (!mixer) {
        fprintf(stderr, "Failed to open mixer\n");
        return EXIT_FAILURE;
    }

    aif1out0l_mux = mixer_get_ctl_by_name(mixer, AIF1OUT0L_MUX);
    if (!aif1out0l_mux) {
		printf("Unable to get aif1out0l_mux, aborting.");
	    return -1;
    }
    aif1out0r_mux = mixer_get_ctl_by_name(mixer, AIF1OUT0R_MUX);
    if (!aif1out0r_mux) {
		printf("Unable to get aif1out0r_mux, aborting.");
	    return -1;
    }
    aif1_ad0l_mixer_adcl_switch = mixer_get_ctl_by_name(mixer, AIF1_AD0L_MIXER_ADCL_SWITCH);
    if (!aif1_ad0l_mixer_adcl_switch) {
		printf("Unable to get aif1_ad0l_mixer_adcl_switch, aborting.");
	    return -1;
    }
    aif1_ad0r_mixer_adcr_switch = mixer_get_ctl_by_name(mixer, AIF1_AD0R_MIXER_ADCR_SWITCH);
    if (!aif1_ad0r_mixer_adcr_switch) {
		printf("Unable to get aif1_ad0r_mixer_adcr_switch, aborting.");
	    return -1;
    }
    adcr_mux = mixer_get_ctl_by_name(mixer, ADCR_MUX);
    if (!adcr_mux) {
		printf("Unable to get adcr_mux, aborting.");
	    return -1;
    }
    adcl_mux = mixer_get_ctl_by_name(mixer, ADCL_MUX);
    if (!adcl_mux) {
		printf("Unable to get adcl_mux, aborting.");
	    return -1;
    }
	if (enable) {
		mixer_ctl_set_enum_by_string(aif1out0l_mux, "AIF1_AD0L");
		mixer_ctl_set_enum_by_string(aif1out0r_mux, "AIF1_AD0R");
		mixer_ctl_set_value(aif1_ad0l_mixer_adcl_switch, 0, 1);
		mixer_ctl_set_value(aif1_ad0r_mixer_adcr_switch, 0, 1);
		mixer_ctl_set_enum_by_string(adcr_mux, "DMIC");
		mixer_ctl_set_enum_by_string(adcl_mux, "DMIC");
	} else {
		mixer_ctl_set_value(aif1_ad0l_mixer_adcl_switch, 0, 0);
		mixer_ctl_set_value(aif1_ad0r_mixer_adcr_switch, 0, 0);
	}
	mixer_close(mixer);
	return 0;
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
		mixer_ctl_set_value(speaker_volume, 0, 29);
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

	mic1_l_mic2_r_enable(1);
	spk_enable(1);
	err = pthread_create(&ntid, NULL, (void*)capture_thread, NULL);
    if (err != 0) {
		printf("create pthread error: %s, %d\n", __func__, __LINE__);
		exit(0);
    }

    /* install signal handler and begin capturing */
    play_sample(file, device, 1, capture_rate, 16);

    fclose(file);
    capturing = 0;
	mic1_l_mic2_r_enable(0);
	spk_enable(0);

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
