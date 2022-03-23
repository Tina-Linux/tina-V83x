/*
 * Copyright (c) 2008-2017 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : tinyplaydemo.h
 * Description : demoAdecoder
 * History :
 *
 */

#ifndef TINYPLAYDEMO_H
#define TINYPLAYDEMO_H

#include <tinyalsa/asoundlib.h>
#include "wavheader.h"

#define MAX_AUDIO_DEVICES 16
//#define CAPTURE_DATA
#define H6_AUDIO_HUB

typedef enum e_AUDIO_DEVICE_MANAGEMENT
{
    AUDIO_IN        = 0x01,
    AUDIO_OUT       = 0x02,
}e_AUDIO_DEVICE_MANAGEMENT;

typedef enum _bool
{
    false = 0,
    true  = 1,
}bool;

typedef struct _dsp_device_manager {
    char        name[32];
    char        card_id[32];
    int         card;
    int         device;
    int         flag_in;            //
    int         flag_in_active;     // 0: not used, 1: used to caputre
    int         flag_out;
    int         flag_out_active;    // 0: not used, 1: used to playback
    int         flag_exist;         // for hot-plugging
}dsp_device_manager;

typedef struct AudioDsp
{
    int start;
    int spr;
    int ch;
    int bps;
    int cardCODEC;
    int cardHDMI;
    int cardSPDIF;
    int cardMIX;
    int cardIn;
    int cardBT;
    int device;
    int period_size;
    int period_count;
    int raw_flag;
    struct pcm_config config;
#ifdef H6_AUDIO_HUB
    struct pcm *pcmhub;
    struct pcm_config hubconfig;
#endif
    struct pcm *pcm;
    dsp_device_manager dev_manager[MAX_AUDIO_DEVICES];
}AudioDsp;

int DspStop(AudioDsp* dsp);

int DspWrite(AudioDsp* dsp, void *pPcmData, int nPcmDataLen);

void DspWaitForDevConsume(int waitms);

AudioDsp* CreateAudioDsp();

void DeleteAudioDsp(AudioDsp* dsp);

#endif
