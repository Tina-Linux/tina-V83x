/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : mediaInfo.h
 * Description : mediaInfo
 * History :
 *
 */


#ifndef MEDIA_INFO_H
#define MEDIA_INFO_H

#include "player.h"             //* player library in "LIBRARY/PLAYER/"
#include "CdxParser.h"          //* parser library in "LIBRARY/DEMUX/PARSER/include/"

namespace android {

enum ECONTAINER
{
    CONTAINER_TYPE_MOV      = CDX_PARSER_MOV,
    CONTAINER_TYPE_MKV      = CDX_PARSER_MKV,
    CONTAINER_TYPE_ASF      = CDX_PARSER_ASF,
    CONTAINER_TYPE_MPG      = CDX_PARSER_MPG,
    CONTAINER_TYPE_TS       = CDX_PARSER_TS,
    CONTAINER_TYPE_AVI      = CDX_PARSER_AVI,
    CONTAINER_TYPE_FLV      = CDX_PARSER_FLV,
    CONTAINER_TYPE_PMP      = CDX_PARSER_PMP,

    CONTAINER_TYPE_HLS      = CDX_PARSER_HLS,
    CONTAINER_TYPE_DASH     = CDX_PARSER_DASH,
    CONTAINER_TYPE_MMS      = CDX_PARSER_MMS,
    CONTAINER_TYPE_BD       = CDX_PARSER_BD,
    CONTAINER_TYPE_OGG      = CDX_PARSER_OGG,
    CONTAINER_TYPE_M3U9     = CDX_PARSER_M3U9,
    CONTAINER_TYPE_RMVB     = CDX_PARSER_RMVB,
    CONTAINER_TYPE_PLAYLIST = CDX_PARSER_PLAYLIST,
    CONTAINER_TYPE_WAV      = CDX_PARSER_WAV,
    CONTAINER_TYPE_APE      = CDX_PARSER_APE,
    CONTAINER_TYPE_AMR      = CDX_PARSER_AMR,
    CONTAINER_TYPE_ATRAC    = CDX_PARSER_ATRAC,
    CONTAINER_TYPE_MP3      = CDX_PARSER_MP3,
    CONTAINER_TYPE_DTS      = CDX_PARSER_DTS,
    CONTAINER_TYPE_AC3      = CDX_PARSER_AC3,
    CONTAINER_TYPE_AAC      = CDX_PARSER_AAC,
    CONTAINER_TYPE_WVM      = CDX_PARSER_WVM,
    CONTAINER_TYPE_RTSP     = CDX_PARSER_REMUX,
    CONTAINER_TYPE_MMSHTTP  = CDX_PARSER_MMSHTTP,

    CONTAINER_TYPE_UNKNOWN = 0x100,
    CONTAINER_TYPE_RAW     = 0x101,
};

typedef struct MEDIAINFO
{
    int64_t             nFileSize;
    int64_t             nDurationMs;
    int                 nBitrate;
    unsigned char       cRotation[4];
    int64_t                nFirstPts;

    enum ECONTAINER     eContainerType;
    int                 bSeekable;
    int                 nVideoStreamNum;
    int                 nAudioStreamNum;
    int                 nSubtitleStreamNum;

    VideoStreamInfo*    pVideoStreamInfo;
    AudioStreamInfo*    pAudioStreamInfo;
    SubtitleStreamInfo* pSubtitleStreamInfo;

}MediaInfo;

}   //* namespace android.

#endif
