/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxTsMuxer.c
 * Description : Allwinner TS Muxer Definition
 * History :
 *   Author  : Q Wang <eric_wang@allwinnertech.com>
 *   Date    : 2014/12/17
 *   Comment : 创建初始版本，用于V3 CedarX_1.0
 *
 *   Author  : GJ Wu <wuguanjian@allwinnertech.com>
 *   Date    : 2016/04/18
 *   Comment_: 修改移植用于CedarX_2.0
 */

#include "CdxTsMuxer.h"

int setTsVideoInfo(TsMuxContext *impl, MuxerVideoStreamInfoT *p_v_info)
{
    impl->streams[impl->nb_streams]->codec.codec_type = CODEC_TYPE_VIDEO;
    printf("in setTsVideoInfo(), nb_streams == %d\n", impl->nb_streams);
    if (p_v_info->eCodeType == VENC_CODEC_H264)
    {
        impl->streams[impl->nb_streams]->codec.codec_id = MUX_CODEC_ID_H264;
    }
    else
    {
        loge("unlown codectype(%d)\n", impl->streams[impl->nb_streams]->codec.codec_id );
        return -1;
    }
    impl->streams[impl->nb_streams]->codec.height = p_v_info->nHeight;
    impl->streams[impl->nb_streams]->codec.width  = p_v_info->nWidth;
    impl->streams[impl->nb_streams]->codec.frame_rate = p_v_info->nFrameRate;

    impl->nb_streams++;
    return 0;
}

int setTsAudioInfo(TsMuxContext *impl, MuxerAudioStreamInfoT *p_a_info)
{
    impl->streams[impl->nb_streams]->codec.codec_type = CODEC_TYPE_AUDIO;
    printf("in setTsAudioInfo(), nb_streams == %d\n", impl->nb_streams);
    printf("in setTsAudioInfo(), eCodecFormat = %d AUDIO_ENCODER_PCM_TYPE %d\n", p_a_info->eCodecFormat, AUDIO_ENCODER_PCM_TYPE);
    if(p_a_info->eCodecFormat == AUDIO_ENCODER_PCM_TYPE)
    {
        impl->streams[impl->nb_streams]->codec.codec_id = MUX_CODEC_ID_PCM;
    }
    else if(p_a_info->eCodecFormat == AUDIO_ENCODER_AAC_TYPE) //for aac
    {
        impl->streams[impl->nb_streams]->codec.codec_id = MUX_CODEC_ID_AAC;
    }
    else if(p_a_info->eCodecFormat == AUDIO_ENCODER_MP3_TYPE)
    {
        impl->streams[impl->nb_streams]->codec.codec_id = MUX_CODEC_ID_MP3;
    }
    else
    {
        loge("unlown codectype(%d)\n", impl->streams[impl->nb_streams]->codec.codec_id );
        return -1;
    }
    impl->streams[impl->nb_streams]->codec.channels = p_a_info->nChannelNum;
    impl->streams[impl->nb_streams]->codec.bits_per_sample = p_a_info->nBitsPerSample;
    impl->streams[impl->nb_streams]->codec.frame_size  = p_a_info->nSampleCntPerFrame;
    impl->streams[impl->nb_streams]->codec.sample_rate = p_a_info->nSampleRate;
    impl->nb_streams++;
    logd("p_a_info->nSampleRate: %d\n", p_a_info->nSampleRate);
    return 0;
}

static int __TsMuxerSetMediaInfo(CdxMuxerT *mux, CdxMuxerMediaInfoT *p_media_info)
{
    TsMuxContext *impl = (TsMuxContext*)mux;
    TsWriter *ts = NULL;
    int i;

    if ((impl->cache_in_ts_stream = (cdx_uint8*)malloc(TS_PACKET_SIZE * 1024)) == NULL)
    {
        loge("impl->cache_in_ts_stream malloc failed\n");
        return -1;
    }

    if ((ts = (TsWriter*)malloc(sizeof(TsWriter))) == NULL)
    {
       loge("impl->priv_data malloc failed\n");
       return -1;
    }
    memset(ts, 0, sizeof(TsWriter));
    impl->priv_data = (void*)ts;

    ts->cur_pcr = 0;
    ts->nb_services = 1;
    ts->tsid = DEFAULT_TSID;
    ts->onid = DEFAULT_ONID;

    ts->sdt.pid = SDT_PID;
    ts->sdt.cc = 15; // Initialize at 15 so that it wraps and be equal to 0
    ts->sdt.write_packet = TsSectionWritePacket;
    ts->sdt.opaque = impl;

    ts->pat.pid = PAT_PID;
    ts->pat.cc = 15;
    ts->pat.write_packet = TsSectionWritePacket;
    ts->pat.opaque = impl;

    ts->ts_cache_start = impl->cache_in_ts_stream;
    ts->ts_cache_end = impl->cache_in_ts_stream + TS_PACKET_SIZE * 1024 - 1;

    ts->ts_write_ptr = ts->ts_read_ptr = ts->ts_cache_start;
    ts->cache_size = 0;
    ts->cache_page_num = 0;
    ts->cache_size_total = 0;

    ts->sdt_packet_period = 200;
    ts->pat_packet_period = 3;
    ts->pat_packet_count = ts->pat_packet_period;

    ts->services = NULL;
    if ((ts->services = (MpegTSService**)malloc(MAX_SERVERVICES_IN_FILE *
                                                sizeof(MpegTSService*))) == NULL)
    {
        loge("ts->services malloc failed\n");
        return -1;
    }

    if ((impl->pes_buffer = (cdx_uint8*)malloc(512 * 1024)) == NULL)
    {
        loge("impl->pes_buffer malloc failed\n");
        return -1;
    }

    impl->pes_bufsize = 512 * 1024;
    impl->max_delay = 1;
    impl->audio_frame_num = 0;
    impl->output_buffer_mode = OUTPUT_TS_FILE;
    impl->nb_streams = 0;
    impl->pat_pmt_counter = 0;
    impl->pat_pmt_flag = 1;
    impl->first_video_pts = 1;
    impl->base_video_pts = 0;
    impl->pcr_counter = 0;

    for(i = 0;i < MAX_SERVERVICES_IN_FILE; i++)
    {
        MpegTSService *service = NULL;
        if ((service = (MpegTSService*)malloc(sizeof(MpegTSService))) == NULL)
        {
            loge("ts->services[%d] malloc failed\n", i);
            return -1;
        }
        memset(service, 0, sizeof(MpegTSService));
        service->pmt.write_packet = TsSectionWritePacket;
        service->pmt.opaque = impl;
        service->pmt.cc = 15;
        service->pmt.pid = 0x0100;
        service->pcr_pid = 0x1000;
        service->sid = 0x00000001;

        ts->services[i] = service;
    }

    for(i=0;i<MAX_STREAMS_IN_TS_FILE;i++)
    {
        AVStream *st = NULL;
        MpegTSWriteStream *ts_st = NULL;
        if ((st = (AVStream *)malloc(sizeof(AVStream))) == NULL)
        {
            loge("impl->streams[%d] failed\n", i);
            return -1;
        }
        memset(st, 0, sizeof(AVStream));
        impl->streams[i] = st;
        impl->streams[i]->firstframeflag = 1;

        if ((ts_st = (MpegTSWriteStream*)malloc(sizeof(MpegTSWriteStream))) == NULL)
        {
            loge("st->priv_data malloc failed\n");
            return -1;
        }
        memset(ts_st, 0, sizeof(MpegTSWriteStream));
        st->priv_data = ts_st;
        ts_st->service = ts->services[0];
        ts_st->pid = DEFAULT_START_PID + i;

        logv("ts_st->pid: %x", ts_st->pid);
        ts_st->payload_pts = -1;
        ts_st->payload_dts = -1;
        ts_st->first_pts_check = 1;
        ts_st->cc = 15;
        st->codec.codec_type = (i==0) ? CODEC_TYPE_VIDEO : CODEC_TYPE_AUDIO;

        if( st->codec.codec_type == CODEC_TYPE_VIDEO)
        {
            ts_st->pid = 0x1011;
        }
        else
        {
            ts_st->pid = 0x1100;
        }
    }

    if (p_media_info->videoNum)
    {
        if (setTsVideoInfo(impl, &p_media_info->video))
        {
            return  -1;
        }
    }

    if (p_media_info->audioNum)
    {
        if (setTsAudioInfo(impl, &p_media_info->audio))
        {
            return -1;
        }
    }

    return 0;
}

static int __TsMuxerWriteHeader(CdxMuxerT *mux)
{
    TsMuxContext *impl = (TsMuxContext*)mux;
    CdxFsWriterInfo *p_fs;

    if (impl->is_sdcard_disappear)
    {
        loge("sdcard may be disappeared, can't write header!");
        return -1;
    }

#if FS_WRITER
    p_fs = &impl->fs_writer_info;

    if (p_fs->mp_fs_writer)
    {
        loge("(f:%s, l:%d) fatal error! why mov->mp_fs_writer[%p]!=NULL",
             __FUNCTION__, __LINE__, p_fs->mp_fs_writer);
        return -1;
    }
    if (impl->stream_writer)
    {
        FSWRITEMODE mode = p_fs->m_fs_writer_mode;
        cdx_int8 *p_cache = NULL;
        cdx_uint32 cache_size = 0;
        cdx_uint32 video_id = impl->streams[0]->codec.codec_id;
        if (FSWRITEMODE_CACHETHREAD == mode)
        {
            logd("FSWRITEMODE_CACHETHREAD == mode\n");
            if (p_fs->m_cache_mem_info.m_cache_size > 0 && p_fs->m_cache_mem_info.mp_cache != NULL)
            {
                mode = FSWRITEMODE_CACHETHREAD;
                p_cache = (cdx_int8 *)p_fs->m_cache_mem_info.mp_cache;
                cache_size = p_fs->m_cache_mem_info.m_cache_size;
            }
            else
            {
                loge("(f:%s, l:%d) fatal error! not set cacheMemory but set \
                     mode FSWRITEMODE_CACHETHREAD! use FSWRITEMODE_DIRECT.",
                     __FUNCTION__, __LINE__);
                mode = FSWRITEMODE_DIRECT;
            }
        }
        else if (FSWRITEMODE_SIMPLECACHE == mode)
        {
            logd("FSWRITEMODE_SIMPLECACHE == mode\n");
            p_cache = NULL;
            cache_size = p_fs->m_fs_simple_cache_size;
        }
        else if (FSWRITEMODE_DIRECT == mode)
        {
            logd("FSWRITEMODE_DIRECT == mode\n");
        }
        p_fs->mp_fs_writer = CreateFsWriter(mode, impl->stream_writer,
                                            (cdx_uint8*)p_cache, cache_size, video_id);
        if (p_fs->mp_fs_writer == NULL)
        {
            loge("(f:%s, l:%d) fatal error! create FsWriter() fail!", __FUNCTION__, __LINE__);
            return -1;
        }
    }
#endif
    return 0;
}

static int __TsMuxerWriteExtraData(CdxMuxerT *mux, unsigned char *vos_data, int vos_len, int idx)
{
    TsMuxContext *impl = (TsMuxContext*)mux;
    AVStream *trk = impl->streams[idx];
    if (vos_len)
    {
        trk->vos_data = (cdx_uint8*)malloc(vos_len);
        trk->vos_len  = vos_len;
        memcpy(trk->vos_data, vos_data, vos_len);
    }
    else
    {
        trk->vos_data = NULL;
        trk->vos_len  = 0;
    }
    return 0;
}

static int __TsMuxerWritePacket(CdxMuxerT *mux, CdxMuxerPacketT *packet)
{
    TsMuxContext *impl = (TsMuxContext*)mux;
    if (impl->is_sdcard_disappear)
    {
        loge("sdcard may be disappeared, can't write packet!");
        return -1;
    }
    return TsWritePacket(impl, packet);
}

static int __TsMuxerWriteTrailer(CdxMuxerT *mux)
{
    TsMuxContext *impl = (TsMuxContext*)mux;
    if (impl->is_sdcard_disappear)
    {
        loge("sdcard may be disappeared, can't write trailer!");
        return -1;
    }
    return TsWriteTrailer(impl);
}

static int __TsMuxerControl(CdxMuxerT *mux, int u_cmd, void *p_param)
{
    TsMuxContext *impl = (TsMuxContext*)mux;

    switch (u_cmd)
    {
        case SET_FS_WRITE_MODE:
        {
            logd("SET_FS_WRITE_MODE\n");
            impl->fs_writer_info.m_fs_writer_mode = *((FSWRITEMODE*)p_param);
            break;
        }
        case SET_CACHE_MEM:
        {
            impl->fs_writer_info.m_cache_mem_info = *((CdxFsCacheMemInfo*)p_param);
            break;
        }
        case SET_FS_SIMPLE_CACHE_SIZE:
        {
            impl->fs_writer_info.m_fs_simple_cache_size = *((cdx_int32*)p_param);
            break;
        }
        case SET_IS_SDCARD_DISAPPEAR:
        {
            impl->is_sdcard_disappear = *((cdx_uint8*)p_param);
            break;
        }
        case GET_IS_SDCARD_DISAPPEAR:
        {
            *((cdx_uint8*)p_param) = impl->is_sdcard_disappear;
            break;
        }
        default:
            break;
    }
    return 0;
}

static int __TsMuxerClose(CdxMuxerT *mux)
{
    TsMuxContext *impl = (TsMuxContext*)mux;
    TsWriter *ts = (TsWriter*)impl->priv_data;
    cdx_int32 i;

    for(i=0;i<MAX_STREAMS_IN_TS_FILE;i++)
    {
        AVStream *st = impl->streams[i];
        if(st)
        {
            if(st->priv_data)
            {
                free(st->priv_data);
                st->priv_data = NULL;
            }
            free(st);
            st = NULL;
        }
    }

    if(*ts->services)
    {
        free(*ts->services);
        *ts->services = NULL;

        free(ts->services);
        ts->services = NULL;
    }

    if(impl->fs_writer_info.mp_fs_writer)
    {
        DestroyFsWriter(impl->fs_writer_info.mp_fs_writer);
        impl->fs_writer_info.mp_fs_writer = NULL;
    }

    if(impl->stream_writer)
    {
        impl->stream_writer = NULL;
    }

    if(impl->cache_in_ts_stream)
    {
        free(impl->cache_in_ts_stream);
        impl->cache_in_ts_stream = NULL;
    }
    if(impl->priv_data)
    {
        free(impl->priv_data);
        impl->priv_data = NULL;
    }

    if(impl->pes_buffer)
    {
        free(impl->pes_buffer);
        impl->pes_buffer = NULL;
    }

     free(impl);

    return 0;
}

static struct CdxMuxerOpsS tsMuxerOps =
{
    .writeExtraData  = __TsMuxerWriteExtraData,
    .writeHeader     = __TsMuxerWriteHeader,
    .writePacket     = __TsMuxerWritePacket,
    .writeTrailer    = __TsMuxerWriteTrailer,
    .control         = __TsMuxerControl,
    .setMediaInfo    = __TsMuxerSetMediaInfo,
    .close           = __TsMuxerClose
};

CdxMuxerT* __CdxTsMuxerOpen(CdxWriterT *stream_writer)
{
    TsMuxContext *mp4Mux;
    logd("__CdxTsMuxerOpen");

    mp4Mux = malloc(sizeof(TsMuxContext));
    if(!mp4Mux)
    {
        return NULL;
    }
    memset(mp4Mux, 0x00, sizeof(TsMuxContext));

    mp4Mux->stream_writer = stream_writer;
    mp4Mux->muxInfo.ops = &tsMuxerOps;

    return &mp4Mux->muxInfo;
}

CdxMuxerCreatorT tsMuxerCtor =
{
    .create = __CdxTsMuxerOpen
};
