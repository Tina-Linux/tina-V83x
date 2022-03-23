/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : tsdemux.c
 * Description : ts demux for CTC
 * History :
 *
 */

//#define CONFIG_LOG_LEVEL OPTION_LOG_LEVEL_DETAIL
#define LOG_TAG "tsdemux"
#include <cutils/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "tsdemux.h"
#include "tsdemux_i.h"
#include <cdx_log.h>
#include <CdxTypes.h>

//* packet size of different format
#define TS_FEC_PACKET_SIZE          204
#define TS_DVHS_PACKET_SIZE         192
#define TS_PACKET_SIZE              188

#define TS_RB16(p) ((*(p)) << 8 | (*((p) + 1)))
#define TS_RB32(p)  ((*(p))<<24|(*((p)+1))<<16|(*((p)+2))<< 8 |(*((p) + 3)))

static inline int64_t get_pts(const unsigned char* pt)
{
    int64_t timeStamp = (int64_t)((pt[0] >> 1) & 0x07) << 30;
    timeStamp |= (TS_RB16(pt + 1) >> 1) << 15;
    timeStamp |= TS_RB16(pt + 3) >> 1;
    return timeStamp;
}


static int push_es_data(unsigned char* data, int len, int new_frm, void* param)
{
    unsigned char* dataptr;
    int            ret;

    //logd("push_es_data   len: %d, new_frm: %d", len, new_frm);
    dataptr = data;
    filter_t* es = (filter_t*) param;
    if (new_frm)
    {
        if (es->valid_size > 0)
        {
            logv("es->valid_size(%d)", es->valid_size);
            es->ctrl_bits |= LAST_PART_BIT;

            //* send data
            es->data_info.ctrlBits = es->ctrl_bits;
            es->data_info.dataLen  = es->valid_size;
            es->updatedatacb(&es->data_info, es->cookie);

            //* clear status
            es->free_size = 0;
            es->valid_size = 0;
            es->ctrl_bits = 0;
            es->payload_size = 0;
        }
    }

    while (len > 0)
    {
        if (es->free_size == 0)
        {
            //* request buffer
            ret = es->requestbufcb(&es->md_buf, es->cookie);
            if (ret != 0 || es->md_buf.buf == NULL)
            {
                if (es->md_buf.bufSize == 1)    //* switching audio.
                    return 1;
                else
                    return -1;
            }

            es->cur_ptr   = es->md_buf.buf;
            es->free_size = es->md_buf.bufSize;
        }

        if (new_frm)
        {
            new_frm = 0;
            es->ctrl_bits |= FIRST_PART_BIT;
            if (es->pts != -1)
            {
                es->ctrl_bits |= PTS_VALID_BIT;
                es->data_info.pts = es->pts*1000 / 90;

                if(es->codec_type == DMX_CODEC_H265)
                    es->data_info.pts = es->dts*1000 / 90; // only for h265
            }
        }

        if ((int)es->free_size > len)
        {
            //logd("11111 es->cur_ptr(%p), dataptr(%p), len(%d)", es->cur_ptr, dataptr, len);
            memcpy(es->cur_ptr, dataptr, len);
            es->free_size  -= len;
            es->cur_ptr    += len;
            es->valid_size += len;

            if (es->payload_size > 0 && es->valid_size >= es->payload_size)
            {
                logv("es->updatedatacb");
                es->ctrl_bits |= LAST_PART_BIT;

                //* send data
                es->data_info.ctrlBits = es->ctrl_bits;
                es->data_info.dataLen  = es->valid_size;
                es->updatedatacb(&es->data_info, es->cookie);

                es->free_size    = 0;
                es->valid_size   = 0;
                es->payload_size = 0;
                es->ctrl_bits = FIRST_PART_BIT;
            }
            break;
        }
        else
        {
            //logd("22222222 es->cur_ptr(%p), dataptr(%p), es->free_size(%d)",
            //es->cur_ptr, dataptr, es->free_size);
            memcpy(es->cur_ptr, dataptr, es->free_size);
            len            -= es->free_size;
            dataptr        += es->free_size;
            es->valid_size += es->free_size;

            if (es->payload_size > 0 && es->valid_size >= es->payload_size)
            {
                es->ctrl_bits |= LAST_PART_BIT;
                es->payload_size = 0;
            }
            else
            {
                if (es->payload_size > 0)
                {
                    if (es->valid_size < es->payload_size)
                        es->payload_size -= es->valid_size;
                    else
                        es->payload_size = 0;
                }
            }

            //* send data
            es->data_info.ctrlBits = es->ctrl_bits;
            es->data_info.dataLen  = es->valid_size;
            es->updatedatacb(&es->data_info, es->cookie);

            es->free_size = 0;
            es->valid_size = 0;
            es->ctrl_bits = 0;
        }
    }

    return 0;
}

static int32_t find_start_code_pos(uint8_t *buf, int32_t len) {
    int32_t pos = -1;
    int32_t i;
    for(i = 2; i < len; i ++) {
        if(buf[i] == 1) {
            if(!buf[i -1] && !buf[i - 2]) {
                pos = i - 2;
            }
            break;
        }
    }
    return pos;
}
static int handle_packet_payload(unsigned char* buf, int buf_size, int is_start, filter_t* es)
{
    unsigned char* p;
    int            len;
    int            code;
    int            first;
    int            j;
    int            result;

    first = 0;

    if (is_start)
    {
        es->state = MPEGTS_HEADER;
        es->data_index   = 0;
        es->payload_size = 0;
    }

    p = buf;
    while (buf_size > 0)
    {
        switch (es->state)
        {
        case MPEGTS_HEADER:
            len = PES_START_SIZE - es->data_index;

            if (len > buf_size)
                len = buf_size;

            for (j = 0; j < len; j++)
            {
                es->header[es->data_index++] = *p++;
            }

            buf_size -= len;
            int skip = 1;
            if (es->data_index == PES_START_SIZE)
            {
                /* we got all the PES or section header. We can now decide */
                if (es->header[0] == 0x00 && es->header[1] == 0x00 && es->header[2] == 0x01)
                {
                    //* it must be an mpeg2 PES stream
                    code = es->header[3] | 0x100;
                    if ((code <= 0x1df && code >= 0x1c0 ) ||
                        (code <= 0x1ef && code >= 0x1e0 ) ||
                        (code == 0x1fd) || (code == 0x1bd))
                    {

                        es->state = MPEGTS_PESHEADER_FILL;

                        //* if pes->total_size = 0, it means the byte num followed is umlimited
                        es->total_size = AV_RB16(es->header + 4);

                        //* NOTE: a zero total size means the PES size is unbounded
                        if (es->total_size)
                            es->total_size += 6;

                        es->pes_header_size = es->header[8] + 9;

                        if (es->total_size > 6)
                            es->payload_size = es->total_size - es->pes_header_size;

                        if(es->codec_type == DMX_CODEC_H264 || es->codec_type == DMX_CODEC_H265)
                            es->payload_size = 0;
                        skip = 0;
                    }
                }

                if(skip)
                {
                    //not a pes or unsuppored stream id
                    es->state = MPEGTS_SKIP;
                    continue;
                }
            }
            break;

            /**********************************************/
            /* PES packing parsing */
        case MPEGTS_PESHEADER_FILL:
            len = es->pes_header_size - es->data_index;
            if (len > buf_size)
                len = buf_size;

            for (j = 0; j < len; j++)
            {
                es->header[es->data_index++] = *p++;
            }
            buf_size -= len;
            if (es->data_index == es->pes_header_size)
            {
                const unsigned char* r;
                unsigned int         flags;

                flags = es->header[7];
                r = es->header + 9;
                es->pts = (int64_t) - 1;

                if ((flags & 0xc0) == 0x80)
                { //PTS
                    es->pts = get_pts(r);
                    es->dts = es->pts;
                    r += 5;
                }
                else if ((flags & 0xc0) == 0xc0)
                { //PTS and DTS
                    es->pts = get_pts(r);
                    es->dts = get_pts(r+5);
                    r += 10;
                }
                /* we got the full header. We parse it and get the payload */
                es->state = MPEGTS_PAYLOAD;
                es->is_first = 1;
            }
            break;

        case MPEGTS_PAYLOAD:
            if (es->total_size)
            {
                len = es->total_size - es->data_index;
                if ((len > buf_size) || (len < 0))
                {
                    len = buf_size;
                }
            }
            else
            {
                len = buf_size;
            }

            if (len > 0)
            {
                first = es->is_first;
                if(first && (es->codec_type == DMX_CODEC_H264 || es->codec_type == DMX_CODEC_H265))
                {
                    int pos = find_start_code_pos(p, len);
                    if(pos > 0) {
                        es->push_es_data(p, pos, 0, es);
                        p += pos;
                        len -= pos;
                    } else if(pos < 0) {
                        first = 0;
                    } else {
                        //pos is 0, this is what we wanted most.
                    }
                }
                es->push_es_data(p, len, first, es);
                if(es->codec_type == DMX_CODEC_H264 || es->codec_type == DMX_CODEC_H265) {
                    if(first) {
                        es->is_first = 0;
                    }
                } else {
                    es->is_first = 0;
                }
                if (es->pts != (int64_t) - 1)
                    es->pre_pts = es->pts;

                es->pts = -1;
                return 0;
            } //if len
            buf_size = 0;
            break;

        case MPEGTS_SKIP:
            buf_size = 0;
            break;
        }
    }

    return 0;
}


static int allocate_filter(int pid, demux_filter_param_t* filter_param, mpegts_context_t* ctx)
{
    int       err;
    filter_t* filter;

    pthread_mutex_lock(&ctx->mutex);

    if (ctx->pids[pid])
    {
        //pid aready exists
        pthread_mutex_unlock(&ctx->mutex);
        return -1;
    }

    filter = (filter_t*) malloc(sizeof(filter_t));
    if (NULL == filter)
    {
        pthread_mutex_unlock(&ctx->mutex);
        return -1;
    }
    memset(filter, 0, sizeof(filter_t));

    filter->pid = pid;
    filter->requestbufcb = filter_param->request_buffer_cb;
    filter->updatedatacb = filter_param->update_data_cb;
    filter->push_es_data = push_es_data;
    filter->codec_type     = filter_param->codec_type;
    filter->cookie = filter_param->cookie;

    ctx->pids[pid] = filter;
    pthread_mutex_unlock(&ctx->mutex);

    return 0;
}


static int free_filter(int pid, mpegts_context_t* ctx)
{
    int       err;
    filter_t* filter;

    pthread_mutex_lock(&ctx->mutex);
    filter = ctx->pids[pid];
    if (filter == NULL)
    {
        pthread_mutex_unlock(&ctx->mutex);
        return -1;
    }

    free(filter);
    ctx->pids[pid] = NULL;

    pthread_mutex_unlock(&ctx->mutex);

    return 0;
}


void* ts_demux_open(void)
{
    int               err;
    mpegts_context_t* mp;

    mp = (mpegts_context_t*) malloc(sizeof(mpegts_context_t));
    if (mp == NULL)
        return NULL;
    memset(mp, 0, sizeof(mpegts_context_t));

    pthread_mutex_init(&mp->mutex, NULL);
    return (void*) mp;
}


int ts_demux_close(void* handle)
{
    int               i;
    mpegts_context_t* mp;

    mp = (mpegts_context_t*) handle;

    for (i = 0; i < NB_PID_MAX; i++)
    {
        if (mp->pids[i])
            free_filter(i, mp);
    }

    pthread_mutex_destroy(&mp->mutex);
    free(mp);

    return 0;
}


int ts_demux_open_filter(void* handle, int pid, demux_filter_param_t* filter_param)
{
    mpegts_context_t* mp;

    mp = (mpegts_context_t*) handle;

    allocate_filter(pid, filter_param, mp);

    if (mp->pids[pid] != NULL)
        return 0;
    else
        return -1;
}


int ts_demux_close_filter(void* handle, int pid)
{
    mpegts_context_t* mp;

    mp = (mpegts_context_t*) handle;
    if ((pid < (int)(sizeof(mp->pids) / sizeof(filter_t*))) && mp->pids[pid])
        free_filter(pid, mp);

    return 0;
}

static int tsAnalyze(const unsigned char* buf, int size, int packet_size, int* index)
{
    int stat[204];
    int i;
    int x=0;
    int best_score=0;

    memset(stat, 0, packet_size*sizeof(int));

    for(x=i=0; i<size; i++)
    {
        if(buf[i] == 0x47)
        {
            stat[x]++;
            if(stat[x] > best_score)
            {
                best_score= stat[x];
                if(index)
                    *index= x;
            }
        }

        x++;
        if(x == packet_size)
            x= 0;
    }

    return best_score;
}

/* auto detect FEC presence. Must have at least 1024 bytes  */
static cdx_int32 tsGetPacketSize(const cdx_uint8 *buf, cdx_uint32 size)
{
    int score, fec_score, dvhs_score;

    if (size < (TS_FEC_PACKET_SIZE * 5 + 1))
        return -1;

    score       = tsAnalyze(buf, size, TS_PACKET_SIZE, NULL);
    dvhs_score  = tsAnalyze(buf, size, TS_DVHS_PACKET_SIZE, NULL);
    fec_score   = tsAnalyze(buf, size, TS_FEC_PACKET_SIZE, NULL);

    if(score > fec_score && score > dvhs_score)
        return TS_PACKET_SIZE;
    else if(dvhs_score > score && dvhs_score > fec_score)
        return TS_DVHS_PACKET_SIZE;
    else if(fec_score > dvhs_score && fec_score > score)
        return TS_FEC_PACKET_SIZE;
    else
        return -1;
}


int ts_demux_handle_packets(void* handle, unsigned char* pktdata, int data_size)
{
    unsigned char*    pkt;
    unsigned char*    p;
    unsigned char*    p_end;
    unsigned char*    buf_end;
    int               is_start;
    int               afc;
    int               pid;
    int               pkt_num;
    int               pkt_size;
    int               left_bytes;
    int               result;
    filter_t*         es;
    mpegts_context_t* mp;

    mp = (mpegts_context_t*)handle;

    if(mp->rawPacketSize == 0)
    {
        if(data_size > 1024)
            mp->rawPacketSize = tsGetPacketSize(pktdata, data_size);
        else
            mp->rawPacketSize = 188;
        logd("[IPTV info] mp->rawPacketSize: %d", mp->rawPacketSize);
    }

    pkt_size = mp->rawPacketSize; //* 192 or 204.
    buf_end = pktdata + data_size;
    pkt     = pktdata;

    for(; pkt<=(buf_end - pkt_size); pkt += pkt_size)
    {
        if(*pkt != 0x47)
        {
            //* resync;
            logd("[IPTV warning] resync");
            for(; pkt <= (buf_end - pkt_size); pkt++)
            {
                if((*pkt == 0x47) && (*(pkt + pkt_size) == 0x47))
                    break;
            }

            if(pkt >= (buf_end - pkt_size))
                break;
        }

        pid = AV_RB16(pkt + 1) & 0x1fff;
        es  = mp->pids[pid];
        if(es == NULL)
            continue;

        is_start = pkt[1] & 0x40;

        /* skip adaptation field */
        afc = (pkt[3] >> 4) & 3;
        p = pkt + 4;
        if (afc == 0 || afc == 2) /* reserved value */
            continue;

        if (afc == 3)
        {
            /* skip adaptation field */
            p += p[0] + 1;
        }

        /* if past the end of packet, ignore */
        p_end = pkt + 188;
        if (p >= p_end)
            continue;

        result = handle_packet_payload(p, p_end - p, is_start, es);
        if(result < 0 && result != -2)
            return result;
        else if(result == -2)    //* the player is switching status,
                                 //* decoder can not give buffer at this time.
        {
            result = 0;
            break;
        }
    }

    left_bytes = buf_end - pkt;
    return left_bytes;
}
