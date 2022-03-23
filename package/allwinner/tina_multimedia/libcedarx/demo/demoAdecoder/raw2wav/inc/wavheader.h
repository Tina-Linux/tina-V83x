/*
 * Copyright (c) 2008-2017 chengkan@allwinnertech.com
 * All rights reserved.
 *
 * File : wavheader.h
 * Description : patch wav header
 * History :
 *
 */

#ifndef WAVHEADER_H
#define WAVHEADER_H

#define WAVE_FORMAT_PCM      0x0001
#define WAVE_FORMAT_ALAW     0x0006 /* ALAW */
#define WAVE_FORMAT_MULAW    0x0007 /* MULAW */

typedef struct __WAVE_HEADER
{
    unsigned int    uRiffFcc;       // four character code, "RIFF"
    unsigned int    uFileLen;       // file total length, don't care it

    unsigned int    uWaveFcc;       // four character code, "WAVE"

    unsigned int    uFmtFcc;        // four character code, "fmt "
    unsigned int    uFmtDataLen;    // Length of the fmt data (=16)
    unsigned short  uWavEncodeTag;  // WAVE File Encoding Tag
    unsigned short  uChannels;      // Channels: 1 = mono, 2 = stereo
    unsigned int    uSampleRate;    // Samples per second: e.g., 44100
    unsigned int    uBytesPerSec;   // sample rate * block align
    unsigned short  uBlockAlign;    // channels * bits/sample / 8
    unsigned short  uBitsPerSample; // 8 or 16

    unsigned int    uDataFcc;       // four character code "data"
    unsigned int    uSampDataSize;  // Sample data size(n)

} wave_header_t;

void Setaudiobs_pcm(char *wavheader,int nChannelNum,int nSampleRate,int nBitsPerSample,
                int ecodemode,int lenadsf);


#endif
