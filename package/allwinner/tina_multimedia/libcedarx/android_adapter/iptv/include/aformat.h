/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : aformat.h
 * Description : aformat
 * History :
 *
 */

#ifndef AFORMAT_H
#define AFORMAT_H

typedef enum {
    AFORMAT_UNKNOWN = -1,
    AFORMAT_MPEG   = 0,
    AFORMAT_PCM_S16LE = 1,
    AFORMAT_AAC   = 2,
    AFORMAT_AC3   = 3,
    AFORMAT_ALAW = 4,
    AFORMAT_MULAW = 5,
    AFORMAT_DTS = 6,
    AFORMAT_PCM_S16BE = 7,
    AFORMAT_FLAC = 8,
    AFORMAT_COOK = 9,
    AFORMAT_PCM_U8 = 10,
    AFORMAT_ADPCM = 11,
    AFORMAT_AMR  = 12,
    AFORMAT_RAAC  = 13,
    AFORMAT_WMA  = 14,
    AFORMAT_WMAPRO   = 15,
    AFORMAT_PCM_BLURAY  = 16,
    AFORMAT_ALAC  = 17,
    AFORMAT_VORBIS    = 18,
    AFORMAT_AAC_LATM   = 19,
    AFORMAT_APE   = 20,
    AFORMAT_UNSUPPORT ,
    AFORMAT_MAX

} aformat_t;

#define AUDIO_EXTRA_DATA_SIZE   (4096)
#define IS_AFMT_VALID(afmt)    ((afmt > AFORMAT_UNKNOWN) && (afmt < AFORMAT_MAX))

#define IS_AUIDO_NEED_EXT_INFO(afmt) ((afmt == AFORMAT_ADPCM) \
                                 ||(afmt == AFORMAT_WMA) \
                                 ||(afmt == AFORMAT_WMAPRO) \
                                 ||(afmt == AFORMAT_PCM_S16BE) \
                                 ||(afmt == AFORMAT_PCM_S16LE) \
                                 ||(afmt == AFORMAT_PCM_U8) \
                                 ||(afmt == AFORMAT_PCM_BLURAY) \
                                 ||(afmt == AFORMAT_AMR)\
                                 ||(afmt == AFORMAT_ALAC)\
                                 ||(afmt == AFORMAT_AC3) \
                                 ||(afmt == AFORMAT_APE) \
                                 ||(afmt == AFORMAT_FLAC) )

#define IS_AUDIO_NOT_SUPPORT_EXCEED_2CH(afmt) ((afmt == AFORMAT_RAAC) \
                                        ||(afmt == AFORMAT_COOK) \
                                        ||(afmt == AFORMAT_FLAC))

#define IS_AUIDO_NEED_PREFEED_HEADER(afmt) ((afmt == AFORMAT_VORBIS) )

#endif /* AFORMAT_H */
