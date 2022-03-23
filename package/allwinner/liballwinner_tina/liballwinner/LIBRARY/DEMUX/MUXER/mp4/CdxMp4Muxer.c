/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxMp4Muxer.c
 * Description : Allwinner MP4 Muxer Definition
 * History :
 *   Author  : Q Wang <eric_wang@allwinnertech.com>
 *   Date    : 2014/12/17
 *   Comment : 创建初始版本，用于V3 CedarX_1.0
 *
 *   Author  : GJ Wu <wuguanjian@allwinnertech.com>
 *   Date    : 2016/04/18
 *   Comment_: 修改移植用于CedarX_2.0
 */

#include "CdxMp4Muxer.h"

int initMov(Mp4MuxContext *impl)
{
    MuxMOVContext *mov = NULL;

    if ((mov = (MuxMOVContext*)malloc(sizeof(MuxMOVContext))) == NULL)
    {
        loge("Mp4MuxContext->mux_mov malloc failed\n");
        return -1;
    }
    memset(mov, 0, sizeof(MuxMOVContext));

    if ((mov->cache_keyframe_ptr = (cdx_uint32*)malloc(KEYFRAME_CACHE_SIZE)) == NULL)
    {
        loge("mov->cache_keyframe_ptr malloc failed\n");
        free(mov);
        return -2;
    }

    if ((impl->mov_inf_cache = (cdx_int8*)malloc(TOTAL_CACHE_SIZE * 4)) == NULL)
    {
        loge("Mp4MuxContext->mov_inf_cache malloc failed\n");
        free(mov->cache_keyframe_ptr);
        free(mov);
        return -3;
    }

    mov->cache_start_ptr[STCO_ID][CODEC_TYPE_VIDEO] =
        mov->cache_read_ptr[STCO_ID][CODEC_TYPE_VIDEO] =
        mov->cache_write_ptr[STCO_ID][CODEC_TYPE_VIDEO] =
        (cdx_uint32*)impl->mov_inf_cache;
    mov->cache_start_ptr[STCO_ID][CODEC_TYPE_AUDIO] =
        mov->cache_read_ptr[STCO_ID][CODEC_TYPE_AUDIO] =
        mov->cache_write_ptr[STCO_ID][CODEC_TYPE_AUDIO] =
        mov->cache_start_ptr[STCO_ID][CODEC_TYPE_VIDEO] + STCO_CACHE_SIZE;
    mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_VIDEO] =
        mov->cache_read_ptr[STSZ_ID][CODEC_TYPE_VIDEO] =
        mov->cache_write_ptr[STSZ_ID][CODEC_TYPE_VIDEO] =
        mov->cache_start_ptr[STCO_ID][CODEC_TYPE_AUDIO] + STCO_CACHE_SIZE;
    mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_AUDIO] =
        mov->cache_read_ptr[STSZ_ID][CODEC_TYPE_AUDIO] =
        mov->cache_write_ptr[STSZ_ID][CODEC_TYPE_AUDIO] =
        mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_VIDEO] + STSZ_CACHE_SIZE;
    mov->cache_start_ptr[STSC_ID][CODEC_TYPE_VIDEO] =
        mov->cache_read_ptr[STSC_ID][CODEC_TYPE_VIDEO] =
        mov->cache_write_ptr[STSC_ID][CODEC_TYPE_VIDEO] =
        mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_AUDIO] + STSZ_CACHE_SIZE;
    mov->cache_start_ptr[STSC_ID][CODEC_TYPE_AUDIO] =
        mov->cache_read_ptr[STSC_ID][CODEC_TYPE_AUDIO] =
        mov->cache_write_ptr[STSC_ID][CODEC_TYPE_AUDIO] =
        mov->cache_start_ptr[STSC_ID][CODEC_TYPE_VIDEO] + STSC_CACHE_SIZE;
    mov->cache_start_ptr[STTS_ID][CODEC_TYPE_VIDEO] =
        mov->cache_read_ptr[STTS_ID][CODEC_TYPE_VIDEO] =
        mov->cache_write_ptr[STTS_ID][CODEC_TYPE_VIDEO] =
        mov->cache_start_ptr[STSC_ID][CODEC_TYPE_AUDIO] + STSC_CACHE_SIZE;
    mov->cache_tiny_page_ptr = mov->cache_start_ptr[STTS_ID][CODEC_TYPE_VIDEO] + STTS_CACHE_SIZE;

    mov->cache_end_ptr[STCO_ID][CODEC_TYPE_VIDEO] =
        mov->cache_start_ptr[STCO_ID][CODEC_TYPE_VIDEO] + (STCO_CACHE_SIZE - 1);
    mov->cache_end_ptr[STSZ_ID][CODEC_TYPE_VIDEO] =
        mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_VIDEO] + (STSZ_CACHE_SIZE - 1);
    mov->cache_end_ptr[STSC_ID][CODEC_TYPE_VIDEO] =
        mov->cache_start_ptr[STSC_ID][CODEC_TYPE_VIDEO] + (STSC_CACHE_SIZE - 1);
    mov->cache_end_ptr[STTS_ID][CODEC_TYPE_VIDEO] =
        mov->cache_start_ptr[STTS_ID][CODEC_TYPE_VIDEO] + (STTS_CACHE_SIZE - 1);

    mov->cache_end_ptr[STCO_ID][CODEC_TYPE_AUDIO] =
        mov->cache_start_ptr[STCO_ID][CODEC_TYPE_AUDIO] + (STCO_CACHE_SIZE - 1);
    mov->cache_end_ptr[STSZ_ID][CODEC_TYPE_AUDIO] =
        mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_AUDIO] + (STSZ_CACHE_SIZE - 1);
    mov->cache_end_ptr[STSC_ID][CODEC_TYPE_AUDIO] =
        mov->cache_start_ptr[STSC_ID][CODEC_TYPE_AUDIO] + (STSC_CACHE_SIZE - 1);

    mov->last_stream_index = -1;

    impl->priv_data = mov;

    return 0;
}

void freeMov(Mp4MuxContext* impl)
{
    MuxMOVContext *mov = (MuxMOVContext*)impl->priv_data;
    int i;

    for(i = 0; i < 2; i++)
    {
        if(mov->fd_stts[i] > 0)
        {
            close(mov->fd_stts[i]);
            mov->fd_stts[i] = 0;
            if(0 == impl->is_sdcard_disappear)
            {
                remove(mov->file_path_stts[i]);
                logd("(f:%s, l:%d) remove fd_stts[%d]name[%s]",
                    __FUNCTION__, __LINE__, i, mov->file_path_stts[i]);
            }
        }
        if(mov->fd_stsz[i] > 0)
        {
            close(mov->fd_stsz[i]);
            mov->fd_stsz[i] = 0;
            if(0 == impl->is_sdcard_disappear)
            {
                remove(mov->file_path_stsz[i]);
                logd("(f:%s, l:%d) remove fd_stsz[%d]name[%s]",
                    __FUNCTION__, __LINE__, i, mov->file_path_stsz[i]);
            }
        }
        if(mov->fd_stco[i] > 0)
        {
            close(mov->fd_stco[i]);
            mov->fd_stco[i] = 0;
            if(0 == impl->is_sdcard_disappear)
            {
                remove(mov->file_path_stco[i]);
                logd("(f:%s, l:%d) remove fd_stco[%d]name[%s]",
                    __FUNCTION__, __LINE__, i, mov->file_path_stco[i]);
            }
        }
        if(mov->fd_stsc[i] > 0)
        {
            close(mov->fd_stsc[i]);
            mov->fd_stsc[i] = 0;
            if(0 == impl->is_sdcard_disappear)
            {
                remove(mov->file_path_stsc[i]);
                logd("(f:%s, l:%d) remove fd_stsc[%d]name[%s]",
                    __FUNCTION__, __LINE__, i, mov->file_path_stsc[i]);
            }
        }
    }

    for(i = 0; i < MAX_STREAMS_IN_MP4_FILE; i++)
    {
        if (mov->tracks[i])
        {
            mov->tracks[i]->mov = NULL;
            if (mov->tracks[i]->vos_data)
            {
                free(mov->tracks[i]->vos_data);
                mov->tracks[i]->vos_data = NULL;
            }
            free(mov->tracks[i]);
            mov->tracks[i] = NULL;
        }
    }

    if(mov->cache_keyframe_ptr)
    {
        free(mov->cache_keyframe_ptr);
        mov->cache_keyframe_ptr = NULL;
    }

    free(impl->priv_data);
}

int setVideoInfo(MuxMOVContext *mov, MuxerVideoStreamInfoT *p_v_info)
{
    MOVTrack *trk = NULL;
    MuxAVCodecContext *p_video = NULL;

    if ((trk = (MOVTrack*)malloc(sizeof(MOVTrack))) == NULL)
    {
        loge("tracks[CODEC_TYPE_VIDEO] malloc failed\n");
        return -1;
    }
    memset(trk, 0, sizeof(MOVTrack));
    trk->track_timescale = GLOBAL_TIMESCALE;
    trk->mov = mov;

    p_video = &trk->enc;
    p_video->codec_type = CODEC_TYPE_VIDEO;
    p_video->height = p_v_info->nHeight;
    p_video->width = p_v_info->nWidth;
    p_video->frame_rate = p_v_info->nFrameRate;
    p_video->rotate_degree = p_v_info->nRotateDegree;

    if (p_v_info->eCodeType == VENC_CODEC_H264)
    {
        p_video->codec_id = MUX_CODEC_ID_H264;
    }
    else if (p_v_info->eCodeType == VENC_CODEC_JPEG)
    {
        p_video->codec_id = MUX_CODEC_ID_MJPEG;
    }
    else
    {
        loge("unlown codectype(%d)\n", p_video->codec_id);
    }

    mov->tracks[CODEC_TYPE_VIDEO] = trk;
    mov->create_time = p_v_info->nCreatTime;

    return 0;
}

int setAudioInfo(MuxMOVContext *mov, MuxerAudioStreamInfoT *p_a_info)
{
    MOVTrack *trk = NULL;
    MuxAVCodecContext *p_audio = NULL;
    logd("go into setAudioInfo");
    if ((trk = (MOVTrack*)malloc(sizeof(MOVTrack))) == NULL)
    {
        loge("tracks[CODEC_TYPE_AUDIO] malloc failed\n");
        return -1;
    }
    memset(trk, 0, sizeof(MOVTrack));
    trk->mov = mov;

    p_audio = &trk->enc;
    p_audio->codec_type = CODEC_TYPE_AUDIO;
    p_audio->channels = p_a_info->nChannelNum;
    p_audio->bits_per_sample = p_a_info->nBitsPerSample;
    p_audio->frame_size = p_a_info->nSampleCntPerFrame;
    p_audio->sample_rate = p_a_info->nSampleRate;

    if(p_a_info->eCodecFormat == AUDIO_ENCODER_PCM_TYPE)
    {
        p_audio->codec_id = MUX_CODEC_ID_PCM;
    }
    else if(p_a_info->eCodecFormat == AUDIO_ENCODER_AAC_TYPE)
    {
        p_audio->codec_id = MUX_CODEC_ID_AAC;
    }
    else if(p_a_info->eCodecFormat == AUDIO_ENCODER_MP3_TYPE)
    {
        p_audio->codec_id = MUX_CODEC_ID_MP3;
    }
    else
    {
        loge("unlown codectype(%d)", p_audio->codec_id );
    }

    trk->track_timescale = p_audio->sample_rate;
    mov->tracks[CODEC_TYPE_AUDIO] = trk;

    return 0;
}

static int __Mp4MuxerSetMediaInfo(CdxMuxerT *mux, CdxMuxerMediaInfoT *p_media_info)
{
    Mp4MuxContext *impl = (Mp4MuxContext*)mux;
    MuxMOVContext *mov;

    if (p_media_info == NULL)
    {
        logv("p_media_info is NULL\n");
        return -1;
    }

    if (impl->priv_data == NULL)
    {
        if (initMov(impl) < 0)
        {
            logv("initMov(impl) failed\n");
            return -1;
        }
    }
    mov = (MuxMOVContext*)impl->priv_data;
    mov->mov_geo_available = p_media_info->geo_available;
    mov->mov_latitudex = p_media_info->latitudex;
    mov->mov_longitudex = p_media_info->longitudex;

    if (p_media_info->videoNum)
    {
        if (setVideoInfo(mov, &p_media_info->video))
        {
            return  -1;
        }
    }

    if (p_media_info->audioNum)
    {
        if (setAudioInfo(mov, &p_media_info->audio))
        {
            return -1;
        }
    }

    return 0;
}

static int __Mp4MuxerWriteHeader(CdxMuxerT *mux)
{
    Mp4MuxContext *impl = (Mp4MuxContext*)mux;
    MuxMOVContext *mov = impl->priv_data;
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
        cdx_int8 *p_cache = NULL;
        cdx_uint32 n_cache_size = 0;
        cdx_uint32 video_id = mov->tracks[0]->enc.codec_id;
        FSWRITEMODE mode = p_fs->m_fs_writer_mode;
        if (FSWRITEMODE_CACHETHREAD == mode)
        {
            logd("FSWRITEMODE_CACHETHREAD == mode\n");
            if (p_fs->m_cache_mem_info.m_cache_size > 0 && p_fs->m_cache_mem_info.mp_cache != NULL)
            {
                mode = FSWRITEMODE_CACHETHREAD;
                p_cache = p_fs->m_cache_mem_info.mp_cache;
                n_cache_size = p_fs->m_cache_mem_info.m_cache_size;
            }
            else
            {
                loge("(f:%s, l:%d) fatal error! not set cacheMemory but set \
                    mode FSWRITEMODE_CACHETHREAD! use FSWRITEMODE_DIRECT.", __FUNCTION__, __LINE__);
                mode = FSWRITEMODE_DIRECT;
            }
        }
        else if (FSWRITEMODE_SIMPLECACHE == mode)
        {
            logd("FSWRITEMODE_SIMPLECACHE == mode\n");
            p_cache = NULL;
            n_cache_size = p_fs->m_fs_simple_cache_size;
        }
        else if (FSWRITEMODE_DIRECT == mode)
        {
            logd("FSWRITEMODE_DIRECT == mode\n");
        }
        p_fs->mp_fs_writer = CreateFsWriter(mode, impl->stream_writer, p_cache,
                                            n_cache_size, video_id);
        if (p_fs->mp_fs_writer == NULL)
        {
            loge("(f:%s, l:%d) fatal error! create FsWriter() fail!", __FUNCTION__, __LINE__);
            return -1;
        }
    }
#endif

    return Mp4WriteHeader(impl);
}

static int __Mp4MuxerWriteExtraData(CdxMuxerT *mux, unsigned char *vos_data, int vos_len, int idx)
{
    Mp4MuxContext *impl = (Mp4MuxContext*)mux;
    MuxMOVContext *mov = (MuxMOVContext*)impl->priv_data;
    MOVTrack *trk = mov->tracks[idx];
    if (vos_len)
    {
        trk->vos_data = (cdx_int8*)malloc(vos_len);
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

static int __Mp4MuxerWritePacket(CdxMuxerT *mux, CdxMuxerPacketT *packet)
{
    Mp4MuxContext *impl = (Mp4MuxContext*)mux;
    if (impl->is_sdcard_disappear)
    {
        loge("sdcard may be disappeared, can't write packet!");
        return -1;
    }
    return Mp4WritePacket(impl, packet);
}

static int __Mp4MuxerWriteTrailer(CdxMuxerT *mux)
{
    Mp4MuxContext *impl = (Mp4MuxContext*)mux;
    if (impl->is_sdcard_disappear)
    {
        loge("sdcard may be disappear, can't write trailer!");
        return -1;
    }
    return Mp4WriteTrailer(impl);
}

static int __Mp4MuxerControl(CdxMuxerT *mux, int u_cmd, void *p_param)
{
    Mp4MuxContext *impl = (Mp4MuxContext*)mux;
    MuxMOVContext *mov = (MuxMOVContext*)impl->priv_data;

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
        case SET_MP4_TMP_PATH:
        {
            strcpy(mov->mov_tmpfile_path, (char*)p_param);
            break;
        }
        case SET_PLAY_TIME_LENGTH:
        {
            mov->play_time_length = *((cdx_int64*)p_param);
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
        {
            break;
        }
    }
    return 0;
}

static int __Mp4MuxerClose(CdxMuxerT *mux)
{
    Mp4MuxContext *impl = (Mp4MuxContext*)mux;
    MuxMOVContext *mov = NULL;

    if(impl == NULL)
    {
        logv("Mp4MuxContext* is NULL\n");
        return -1;
    }
    mov = (MuxMOVContext*)impl->priv_data;

    if (impl->mov_inf_cache)
    {
        free(impl->mov_inf_cache);
        impl->mov_inf_cache = NULL;
    }

    if (impl->priv_data)
    {
       freeMov(impl);
       impl->priv_data = NULL;
    }

    if(impl->fs_writer_info.mp_fs_writer)
    {
        DestroyFsWriter(impl->fs_writer_info.mp_fs_writer);
        impl->fs_writer_info.mp_fs_writer = NULL;
    }

    if (impl->stream_writer)
    {
        impl->stream_writer = NULL;
    }

    if (impl)
    {
        free(impl);
        impl = NULL;
    }
    return 0;
}

static struct CdxMuxerOpsS mp4MuxerOps =
{
    .writeExtraData  = __Mp4MuxerWriteExtraData,
    .writeHeader     = __Mp4MuxerWriteHeader,
    .writePacket     = __Mp4MuxerWritePacket,
    .writeTrailer    = __Mp4MuxerWriteTrailer,
    .control         = __Mp4MuxerControl,
    .setMediaInfo    = __Mp4MuxerSetMediaInfo,
    .close           = __Mp4MuxerClose
};

CdxMuxerT* __CdxMp4MuxerOpen(CdxWriterT *stream_writer)
{
    Mp4MuxContext *mp4Mux;
    logd("__CdxMp4MuxerOpen");

    mp4Mux = (Mp4MuxContext*)malloc(sizeof(Mp4MuxContext));
    if(!mp4Mux)
    {
        return NULL;
    }
    memset(mp4Mux, 0x00, sizeof(Mp4MuxContext));

    mp4Mux->stream_writer = stream_writer;
    mp4Mux->muxInfo.ops = &mp4MuxerOps;

    return &mp4Mux->muxInfo;
}

CdxMuxerCreatorT mp4MuxerCtor =
{
    .create = __CdxMp4MuxerOpen
};
