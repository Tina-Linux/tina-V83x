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

enum ECONTAINER
{
    CONTAINER_TYPE_MOV     = CDX_PARSER_MOV,
    CONTAINER_TYPE_MKV     = CDX_PARSER_MKV,
    CONTAINER_TYPE_ASF     = CDX_PARSER_ASF,
    CONTAINER_TYPE_MPG     = CDX_PARSER_MPG,
    CONTAINER_TYPE_TS      = CDX_PARSER_TS,
    CONTAINER_TYPE_AVI     = CDX_PARSER_AVI,
    CONTAINER_TYPE_FLV     = CDX_PARSER_FLV,
    CONTAINER_TYPE_PMP     = CDX_PARSER_PMP,
    CONTAINER_TYPE_BD      = CDX_PARSER_BD,
    CONTAINER_TYPE_HLS     = CDX_PARSER_HLS,
    CONTAINER_TYPE_DASH    = CDX_PARSER_DASH,
    CONTAINER_TYPE_MMS     = CDX_PARSER_MMS,

    //* multiple stream source of media es stream, such as rtsp.
    CONTAINER_TYPE_RTSP    = CDX_PARSER_REMUX,

    CONTAINER_TYPE_UNKNOWN = 0x100,
    CONTAINER_TYPE_RAW     = 0x101,
};


typedef struct MEDIAINFO
{
    int64_t             nFileSize;
    int                 nDurationMs;

    enum ECONTAINER     eContainerType;
    int                 bSeekable;
    int                 nVideoStreamNum;
    int                 nAudioStreamNum;
    int                 nSubtitleStreamNum;

    VideoStreamInfo*    pVideoStreamInfo;
    AudioStreamInfo*    pAudioStreamInfo;
    SubtitleStreamInfo* pSubtitleStreamInfo;

    cdx_uint8   album[64];
    cdx_int32   albumsz;
    cdx_int32   albumCharEncode;

    cdx_uint8   author[64];
    cdx_int32   authorsz;
    cdx_int32   authorCharEncode;

    cdx_uint8   genre[64];
    cdx_int32   genresz;
    cdx_int32   genreCharEncode;

    cdx_uint8   title[64];
    cdx_int32   titlesz;
    cdx_int32   titleCharEncode;

    cdx_uint8   year[64];
    cdx_int32   yearsz;
    cdx_int32   yearCharEncode;

}MediaInfo;

#endif
