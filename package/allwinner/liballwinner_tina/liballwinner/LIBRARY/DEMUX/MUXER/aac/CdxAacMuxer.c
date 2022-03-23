/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxAacMuxer.c
 * Description : Allwinner AAC Muxer Definition
 * History :
 *
 */

#include "CdxAacMuxer.h"

#define ADTS_HEADER_SIZE 7

typedef struct AacMuxContext
{
    CdxMuxerT   muxInfo;
    CdxWriterT  *stream_writer;

    CdxMuxerMediaInfoT mediaInfo;

    char        adts[ADTS_HEADER_SIZE];
    void        *priv_data;

    FSWRITEMODE mFsWriteMode;
} AacMuxContext;


typedef struct adts_header
{
    uint16_t syncword;
    uint8_t id;
    uint8_t layer;
    uint8_t protection_absent;
    uint8_t profile;
    uint8_t sf_index;
    uint8_t private_bit;
    uint8_t channel_configuration;
    uint8_t original;
    uint8_t home;
    uint8_t emphasis;
    uint8_t copyright_identification_bit;
    uint8_t copyright_identification_start;
    uint16_t aac_frame_length;
    uint16_t adts_buffer_fullness;
    uint8_t no_raw_data_blocks_in_frame;
    uint16_t crc_check;

    /* control param */
    uint8_t old_format;
} adts_header;

static int generateAdtsHeader(AacMuxContext *s, unsigned int length)
{
    int frame_len = length + ADTS_HEADER_SIZE;

    s->adts[3] &= 0xFC;
    s->adts[3] |= (char)(frame_len >> 11)& 0x03;
    s->adts[4]  = (char)(frame_len >> 3) & 0xFF;
    s->adts[5] &= 0x1F;
    s->adts[5] |= (char)(frame_len & 0x0007) << 5;

    return 0;
}

static unsigned const sampleRateTable[16] =
{
  96000, 88200, 64000, 48000,
  44100, 32000, 24000, 22050,
  16000, 12000, 11025, 8000,
  7350, 0, 0, 0
};

static int getSampleRateIndex(unsigned int sampleRate)
{
    int i;
    for (i = 0; i < 16; i++)
    {
        if (sampleRate == sampleRateTable[i])
        {
            return i;
        }
    }
    return -1;
}


static int initADTSHeader(AacMuxContext *s, unsigned int sampleRate, unsigned int channels)
{
    int sf_index;
    adts_header adts;
    //unsigned int frame_len;

    sf_index = getSampleRateIndex(sampleRate);
    if (sf_index < 0)
    {
        return -1;
    }

    adts.syncword = 0xfff;
    adts.id = 0;
    adts.layer = 0;
    adts.protection_absent = 1;
    adts.profile = 1;
    adts.sf_index = sf_index;
    adts.protection_absent = 1;
    adts.private_bit = 0;
    adts.channel_configuration = channels;
    adts.original = 0;
    adts.home = 0;
    adts.copyright_identification_bit = 0;
    adts.copyright_identification_start = 0;
    adts.adts_buffer_fullness = 0x7ff;
    adts.no_raw_data_blocks_in_frame = 0;

    memset(s->adts, 0, ADTS_HEADER_SIZE);

    s->adts[0]  = 0xff;

    s->adts[1]  = 0xf0;                                   //syncword
    s->adts[1] |= (adts.layer & 0x03) << 1;               //layer
    s->adts[1] |= (adts.protection_absent & 0x01);        //protection_absent

    s->adts[2]  = (adts.profile & 0x03) << 6;             //profile
    s->adts[2] |= (adts.sf_index & 0x0F) << 2;
    s->adts[2] |= (adts.private_bit & 0x01) << 1;
    s->adts[2] |= (adts.channel_configuration & 0x04) >> 2;

    s->adts[3]  = (adts.channel_configuration & 0x03) << 6;
    s->adts[3] |= (adts.original & 0x01) << 5;
    s->adts[3] |= (adts.home & 0x01) << 4;
    s->adts[3] |= (adts.copyright_identification_bit & 0x01) << 3;
    s->adts[3] |= (adts.copyright_identification_start & 0x01) << 2;
    //s->adts[3] |= (char)(frame_len >> 11) & 0x03;

    //s->adts[4]  = (char)(frame_len >> 3)& 0xFF;

    //s->adts[5] |= ((char)frame_len & 0x07) << 5;
    s->adts[5]  = (char)(adts.adts_buffer_fullness >> 6) & 0x1F;

    s->adts[6]  = (char)(adts.adts_buffer_fullness & 0x003F) << 2;
    s->adts[6] |= adts.no_raw_data_blocks_in_frame & 0x03;

    return 0;
}

static int __AacMuxerWriteExtraData(CdxMuxerT* mux, unsigned char* data, int len, int idx)
{
    return 0;
}

static int __AacMuxerWriteHeader(CdxMuxerT* mux)
{
    AacMuxContext *impl = (AacMuxContext*)mux;

    return 0;
}

static int __AacMuxerWritePacket(CdxMuxerT *mux, CdxMuxerPacketT *packet)
{
    AacMuxContext *impl = (AacMuxContext*)mux;
    int ret;

    ret = impl->stream_writer->cdxWrite(impl->stream_writer, packet->buf, packet->buflen);
    if(ret < packet->buflen)
    {
        logd("=== ret: %d, packet->buflen: %d", ret, packet->buflen);
        return -1;
    }
    return 0;
}

static int __AacMuxerSetMediaInfo(CdxMuxerT *mux, CdxMuxerMediaInfoT *pMediaInfo)
{
    AacMuxContext *impl = (AacMuxContext*)mux;

    memcpy(&impl->mediaInfo.video, &pMediaInfo->video, sizeof(MuxerVideoStreamInfoT));
    memcpy(&impl->mediaInfo.audio, &pMediaInfo->audio, sizeof(MuxerAudioStreamInfoT));


    //initADTSHeader(impl, impl->mediaInfo.audio.nSampleRate, impl->mediaInfo.audio.nChannelNum);

    return 0;
}

static int __AacMuxerWriteTrailer(CdxMuxerT *mux)
{
    return 0;
}

static int __AacMuxerControl(CdxMuxerT *mux, int uCmd, void *pParam)
{
    AacMuxContext *impl = (AacMuxContext*)mux;
    return 0;
}

static int __AacMuxerClose(CdxMuxerT *mux)
{
    AacMuxContext *impl = (AacMuxContext*)mux;

    if(impl)
    {
        if(impl->stream_writer)
        {
            impl->stream_writer = NULL;
        }
        free(impl);
    }

    return 0;
}

static struct CdxMuxerOpsS aacMuxerOps =
{
    .writeExtraData =  __AacMuxerWriteExtraData,
    .writeHeader     = __AacMuxerWriteHeader,
    .writePacket     = __AacMuxerWritePacket,
    .writeTrailer    = __AacMuxerWriteTrailer,
    .control         = __AacMuxerControl,
    .setMediaInfo    = __AacMuxerSetMediaInfo,
    .close           = __AacMuxerClose
};


CdxMuxerT* __CdxAacMuxerOpen(CdxWriterT *stream)
{
    AacMuxContext *aacMux;
    logd("__CdxAacMuxerOpen");

    aacMux = malloc(sizeof(AacMuxContext));
    if(!aacMux)
    {
        return NULL;
    }
    memset(aacMux, 0x00, sizeof(AacMuxContext));

    aacMux->stream_writer = stream;
    aacMux->muxInfo.ops = &aacMuxerOps;

    return &aacMux->muxInfo;
}


CdxMuxerCreatorT aacMuxerCtor =
{
    .create = __CdxAacMuxerOpen
};
