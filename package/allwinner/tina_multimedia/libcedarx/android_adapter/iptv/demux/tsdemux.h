/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : tsdemux.h
 * Description : ts demux for CTC
 * History :
 *
 */

#ifndef DTV_TSDEMUX_H
#define DTV_TSDEMUX_H

#ifdef __cplusplus
extern "C" {
#endif

//define callback function
typedef int (*pdemux_callback_t)(void* param, void* cookie);

typedef enum {
    DMX_CODEC_UNKOWN = 0,
    DMX_CODEC_H264,
    DMX_CODEC_H265
}demux_codec_type;

typedef struct DEMUX_FILTER_PARAM
{
    demux_codec_type    codec_type;
    pdemux_callback_t   request_buffer_cb;
    pdemux_callback_t   update_data_cb;
    void*               cookie;
}demux_filter_param_t;

#define PTS_VALID_BIT           0x2
#define FIRST_PART_BIT          0x8
#define LAST_PART_BIT           0x10
#define RANDOM_ACCESS_FRAME_BIT    0x40

typedef struct MEDIA_BUFFER
{
    unsigned char*  buf;
    unsigned int    bufSize;
}md_buf_t;

typedef struct MEDIA_DATA_INFO
{
    int64_t         pts;
    unsigned int    dataLen;
    unsigned int    ctrlBits;
}md_data_info_t;

void* ts_demux_open(void);
int   ts_demux_close(void* handle);
int   ts_demux_open_filter(void* handle, int pid, demux_filter_param_t* param);
int   ts_demux_close_filter(void* handle, int pid);
int   ts_demux_handle_packets(void* handle, unsigned char* pktdata, int data_size);

#ifdef __cplusplus
}
#endif

#endif
