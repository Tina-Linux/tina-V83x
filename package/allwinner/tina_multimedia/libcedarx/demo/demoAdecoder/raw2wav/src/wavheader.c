/*
 * Copyright (c) 2008-2017 Chengkan@allwinnertech.com
 * All rights reserved.
 *
 * File : wavheader.c
 * Description : patch wav header unitest
 * History :
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "wavheader.h"

void Setaudiobs_pcm(char *wavheader,int nChannelNum,int nSampleRate,int nBitsPerSample,
                int ecodemode,int len)
{
    wave_header_t* tmpWavHdr = (wave_header_t*)wavheader;
    tmpWavHdr->uRiffFcc = ('R'<<0) | ('I'<<8) | ('F'<<16) | ('F'<<24);
    tmpWavHdr->uFileLen = len + 0x2c - 8;

    tmpWavHdr->uWaveFcc = ('W'<<0) | ('A'<<8) | ('V'<<16) | ('E'<<24);
    tmpWavHdr->uFmtFcc  = ('f'<<0) | ('m'<<8) | ('t'<<16) | (' '<<24);
    tmpWavHdr->uFmtDataLen = 16;
    tmpWavHdr->uWavEncodeTag = ecodemode & 0xffff;
    tmpWavHdr->uChannels = nChannelNum;
    tmpWavHdr->uSampleRate = nSampleRate;

    tmpWavHdr->uBlockAlign = nChannelNum * nBitsPerSample / 8;
    tmpWavHdr->uBytesPerSec = tmpWavHdr->uSampleRate * tmpWavHdr->uBlockAlign;
    tmpWavHdr->uBitsPerSample = nBitsPerSample;
    tmpWavHdr->uDataFcc = ('d'<<0) | ('a'<<8) | ('t'<<16) | ('a'<<24);
    tmpWavHdr->uSampDataSize = len;

}
