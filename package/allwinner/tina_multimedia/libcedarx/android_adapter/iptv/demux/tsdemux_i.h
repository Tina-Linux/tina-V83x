/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : tsdemux_i.h
 * Description : tsdemux_i
 * History :
 *
 */

#ifndef DTV_TSDEMUX_I_H
#define DTV_TSDEMUX_I_H

#ifdef __cplusplus
extern "C" {
#endif

#define AV_RB16(p) ((*(p)) << 8 | (*((p) + 1)))
#define AV_RB32(p)  ((*(p))<<24|(*((p)+1))<<16|(*((p)+2))<< 8 |(*((p) + 3)))

#define NB_PID_MAX              8192
#define MAX_SECTION_SIZE        4096
#define PES_START_SIZE          9
#define MAX_PES_HEADER_SIZE     (9 + 255)

//* define callback function to push data
typedef int (*push_data_cb)(unsigned char* data, int len, int new_frm, void* parm);

typedef enum MPEGTSSTATE
{
    MPEGTS_HEADER = 0,
    MPEGTS_PESHEADER_FILL,
    MPEGTS_PAYLOAD,
    MPEGTS_SKIP
} mpegts_state_t;

typedef struct FILTER
{
    unsigned char*    cur_ptr; //* current pointer
    unsigned int      free_size; //* free size of buffer for pushing data
    unsigned int      valid_size; //* written size
    unsigned int      ctrl_bits; //* control bits

    int               pid;
    int               is_first;

    md_data_info_t    data_info;
    md_buf_t          md_buf;
    pdemux_callback_t requestbufcb;
    pdemux_callback_t updatedatacb;
    mpegts_state_t    state;

    //* variables below are used for getting PES format
    int64_t           pts;          //* presentation tim stamp
    int64_t           dts;
    int64_t           pre_pts;      //* previous pts
    int64_t           pts_to_send;  //* PTS to send

    push_data_cb      push_es_data; //* push PES data callback function

    unsigned int      data_index; //index to indicate location of handled PES header
    unsigned int      total_size;
    unsigned int      payload_size;
    unsigned int      pes_header_size;
    unsigned char     header[MAX_PES_HEADER_SIZE]; //save header data

    demux_codec_type  codec_type;

    void*             cookie;
} filter_t;


typedef struct MPEGTSCONTEXT
{
    pthread_mutex_t mutex;
    filter_t*       pids[NB_PID_MAX];
    int    rawPacketSize;
} mpegts_context_t;

#ifdef __cplusplus
}
#endif

#endif
