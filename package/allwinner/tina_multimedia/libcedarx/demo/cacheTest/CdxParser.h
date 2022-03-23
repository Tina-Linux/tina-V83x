/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxParser.h
 *
 * Description : fake parser header file
 * History :
 *   Author  : Zhao Zhili
 *   Date    : 2016/04/25
 *   Comment : first version
 *
 */

#ifndef CDXPARSER_H
#define CDXPARSER_H

#define CdxParserTypeT  int

#define MINOR_STREAM    0x0001 /*0 major stream, 1 minor stream*/
#define FIRST_PART      0x0002
#define LAST_PART       0x0004
#define KEY_FRAME       0x0008

enum CdxMediaTypeE
{
    CDX_MEDIA_UNKNOWN = -1,
    CDX_MEDIA_VIDEO = 0,
    CDX_MEDIA_AUDIO,
    CDX_MEDIA_SUBTITLE,
    CDX_MEDIA_DATA,
};

enum CdxParserTypeE
{
    CDX_PARSER_UNKNOW = -1,
    CDX_PARSER_MOV,
    CDX_PARSER_MKV,
    CDX_PARSER_ASF,
    CDX_PARSER_TS,
    CDX_PARSER_AVI,
    CDX_PARSER_FLV,
    CDX_PARSER_PMP,

    CDX_PARSER_HLS,
    CDX_PARSER_DASH,
    CDX_PARSER_MMS,
    CDX_PARSER_BD,
    CDX_PARSER_OGG,
    CDX_PARSER_M3U9,
    CDX_PARSER_RMVB,
    CDX_PARSER_PLAYLIST,
    CDX_PARSER_APE,
    CDX_PARSER_FLAC,
    CDX_PARSER_AMR,
    CDX_PARSER_ATRAC,
    CDX_PARSER_MP3,
    CDX_PARSER_DTS,
    CDX_PARSER_AC3,
    CDX_PARSER_AAC,
    CDX_PARSER_WAV,
    CDX_PARSER_REMUX, /* rtsp, etc... */
    CDX_PARSER_WVM,
    CDX_PARSER_MPG,
    CDX_PARSER_MMSHTTP,
    CDX_PARSER_AWTS,
    CDX_PARSER_SSTR,
    CDX_PARSER_CAF,
    CDX_PARSER_G729,
    CDX_PARSER_DSD,
    CDX_PARSER_ID3,
    CDX_PARSER_ENV,
    CDX_PARSER_SSTR_PLAYREADY,
    CDX_PARSER_AWRAWSTREAM,
    CDX_PARSER_AWSPECIALSTREAM,
};

#define cdx_atomic_t int

struct VideoInfo {
    cdx_atomic_t ref;
    int videoNum;
    struct {
        void *pCodecSpecificData;
    } video[0];
};

struct AudioInfo {
    cdx_atomic_t ref;
    int audioNum;
    struct {
        void *pCodecSpecificData;
    } audio[0];
};

struct SubtitleInfo {
    cdx_atomic_t ref;
};

inline int CdxAtomicDec(int *ref)
{
    (void)ref;
    return 0;
}

inline int CdxAtomicRead(int *ref)
{
    (void)ref;
    return 0;
}

#endif
