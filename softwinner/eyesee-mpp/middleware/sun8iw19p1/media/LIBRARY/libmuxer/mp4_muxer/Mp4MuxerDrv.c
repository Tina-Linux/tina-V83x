// #include <CDX_LogNDebug.h>
#define LOG_TAG "Mp4MuxerDrv.c"
#include <utils/plat_log.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "Mp4Muxer.h"
//#include <recorde_writer.h>
#include <FsWriter.h>
#include <vencoder.h>
//#include <type_camera.h>
#include <aencoder.h>


unsigned int gps_pack_method = GPS_PACK_IN_MDAT;


int Mp4MuxerWriteVos(void *handle, unsigned char *vosData, unsigned int vosLen, unsigned int idx)
{
	AVFormatContext *Mp4MuxerCtx = (AVFormatContext *)handle;
	MOVContext *mov = Mp4MuxerCtx->priv_data;
	MOVTrack *trk = &mov->tracks[idx];
	if(vosLen)
	{
    	trk->vosData = (char *)malloc(vosLen);
    	trk->vosLen  = vosLen;
        memcpy(trk->vosData,vosData,vosLen);
	}
    else
    {
        trk->vosData = NULL;
        trk->vosLen  = 0;
    }

	return 0;
}

int Mp4MuxerWriteHeader(void *handle)
{
	AVFormatContext *Mp4MuxerCtx = (AVFormatContext *)handle;
    //MOVContext *mov = Mp4MuxerCtx->priv_data;
    char *pCache = NULL;
    unsigned int nCacheSize = 0;
    if(Mp4MuxerCtx->mpFsWriter)
    {
        aloge("fatal error! why mov->mpFsWriter[%p]!=NULL", Mp4MuxerCtx->mpFsWriter);
        return -1;
    }
    if(Mp4MuxerCtx->pb_cache)
    {
        FSWRITEMODE mode = Mp4MuxerCtx->mFsWriteMode;
        if(FSWRITEMODE_CACHETHREAD == mode)
        {
    		if (Mp4MuxerCtx->mCacheMemInfo.mCacheSize > 0 && Mp4MuxerCtx->mCacheMemInfo.mpCache != NULL)
    		{
    			mode = FSWRITEMODE_CACHETHREAD;
                pCache = Mp4MuxerCtx->mCacheMemInfo.mpCache;
                nCacheSize = Mp4MuxerCtx->mCacheMemInfo.mCacheSize;
    		}
    		else
    		{
                aloge("fatal error! not set cacheMemory but set mode FSWRITEMODE_CACHETHREAD! use FSWRITEMODE_DIRECT.");
                mode = FSWRITEMODE_DIRECT;
    		}
        }
        else if(FSWRITEMODE_SIMPLECACHE == mode)
        {
            pCache = NULL;
            nCacheSize = Mp4MuxerCtx->mFsSimpleCacheSize;
        }
        Mp4MuxerCtx->mpFsWriter = createFsWriter(mode, Mp4MuxerCtx->pb_cache, pCache, nCacheSize, Mp4MuxerCtx->streams[0]->codec.codec_id);
        if(NULL == Mp4MuxerCtx->mpFsWriter)
        {
            aloge("fatal error! create FsWriter() fail!");
            return -1;
        }
    }
	return mov_write_header(Mp4MuxerCtx);
}

int Mp4MuxerWriteTrailer(void *handle)
{
	AVFormatContext *Mp4MuxerCtx = (AVFormatContext *)handle;
	return mov_write_trailer(Mp4MuxerCtx);
}

int Mp4MuxerWritePacket(void *handle, void *pkt)
{
	AVFormatContext *Mp4MuxerCtx = (AVFormatContext *)handle;
	return mov_write_packet(Mp4MuxerCtx, (AVPacket *)pkt);
}

int Mp4MuxerIoctrl(void *handle, unsigned int uCmd, unsigned int uParam, void *pParam2)
{
	AVFormatContext *Mp4MuxerCtx = (AVFormatContext *)handle;
	MOVContext *mov = Mp4MuxerCtx->priv_data;
	_media_file_inf_t *pMediaInf = 0;
	
	switch (uCmd)
	{
//	case SET_VIDEO_CODEC_ID:  /* gushiming compressed source */
//		mov->video_codec_id = (int)uParam;
//		break;
//
//	case SET_AUDIO_CODEC_ID:  /* gushiming compressed source */
//		mov->audio_codec_id = (int)uParam;
//		break;

	case SETAVPARA:	
		pMediaInf = (_media_file_inf_t *)pParam2;
		
		if (NULL == pMediaInf)
		{
			__inf("error in param\n");
			return -1;
		}
		
		//set video parameters
		Mp4MuxerCtx->streams[0]->codec.height = pMediaInf->nHeight;
		Mp4MuxerCtx->streams[0]->codec.width  = pMediaInf->nWidth;
		Mp4MuxerCtx->streams[0]->codec.frame_rate = pMediaInf->uVideoFrmRate;
		mov->create_time = pMediaInf->create_time;

        switch(pMediaInf->mVideoEncodeType)
        {
            case VENC_CODEC_JPEG:
                Mp4MuxerCtx->streams[0]->codec.codec_id = CODEC_ID_MJPEG;
                break;
            case VENC_CODEC_H264:
                Mp4MuxerCtx->streams[0]->codec.codec_id = CODEC_ID_H264;
                break;
            case VENC_CODEC_H265:
                Mp4MuxerCtx->streams[0]->codec.codec_id = CODEC_ID_H265;
                break;
            default:
                aloge("fatal error! unknown video encode type[0x%x]", pMediaInf->mVideoEncodeType);
                Mp4MuxerCtx->streams[0]->codec.codec_id = CODEC_ID_H264;
                break;
        }

		Mp4MuxerCtx->streams[0]->codec.rotate_degree = pMediaInf->rotate_degree; //set rotate degree
		Mp4MuxerCtx->nb_streams++;
		mov->keyframe_interval = pMediaInf->maxKeyInterval;//29;

		//set audio parameters
		Mp4MuxerCtx->streams[1]->codec.channels = pMediaInf->channels;
		Mp4MuxerCtx->streams[1]->codec.bits_per_sample = pMediaInf->bits_per_sample;
		Mp4MuxerCtx->streams[1]->codec.frame_size  = pMediaInf->frame_size;
		Mp4MuxerCtx->streams[1]->codec.sample_rate = pMediaInf->sample_rate;
		mov->tracks[1].timescale = Mp4MuxerCtx->streams[1]->codec.sample_rate;
		mov->tracks[1].sampleDuration = 1;

        switch(pMediaInf->audio_encode_type)
        {
            case AUDIO_ENCODER_PCM_TYPE:
                Mp4MuxerCtx->streams[1]->codec.codec_id = CODEC_ID_PCM;
                break;
            case AUDIO_ENCODER_AAC_TYPE:
                Mp4MuxerCtx->streams[1]->codec.codec_id = CODEC_ID_AAC;
                break;
            default:
                aloge("fatal error! unknown audio encode type[0x%x]", pMediaInf->audio_encode_type);
                Mp4MuxerCtx->streams[1]->codec.codec_id = CODEC_ID_AAC;
                break;
        }

		Mp4MuxerCtx->nb_streams++;


		// location
		mov->geo_available = pMediaInf->geo_available;
		mov->latitudex10000 = pMediaInf->latitudex10000;
		mov->longitudex10000 = pMediaInf->longitudex10000;

        if(mov->geo_available && gps_pack_method==GPS_PACK_IN_MDAT)  // to prepare buffer needed by mp4
        { 
            if(mov->gps_entry_buff)
            {
                alogw("Be careful! free gps_entry_buff[%p] first.", mov->gps_entry_buff);
                free(mov->gps_entry_buff);
                mov->gps_entry_buff = NULL;
            }
            mov->gps_entry_buff = (GPS_ENTRY *)malloc(sizeof(GPS_ENTRY)*MOV_GPS_MAX_ENTRY_NUM);
            //alogd("sizeof(GPS_ENTRY)=%d, total=%d", sizeof(GPS_ENTRY), sizeof(GPS_ENTRY)*MOV_GPS_MAX_ENTRY_NUM);
            if(NULL == mov->gps_entry_buff)
            {
                mov->geo_available = 0;     // no reource to store gps info
            }
        }
        if(mov->geo_available && gps_pack_method==GPS_PACK_IN_TRACK)
        {
            //set text parameters
            Mp4MuxerCtx->streams[2]->codec.codec_id = CODEC_ID_GGAD; // pMediaInf->text_encode_type;
            mov->tracks[2].timescale = 1000;
            Mp4MuxerCtx->nb_streams++;
        }
               
		/*switch(Mp4MuxerCtx->streams[0]->codec.frame_rate)
		{
		case 25000:
			mov->tracks[0].timescale = 600;
			mov->tracks[0].sampleDuration = 24;
            mov->tracks[0].stsc_value = 0; //12
			break;
		case 30000:
			mov->tracks[0].timescale = 600;
			mov->tracks[0].sampleDuration = 20;
            mov->tracks[0].stsc_value = 0;//15
			break;
		case 24000:
			mov->tracks[0].timescale = 600;
			mov->tracks[0].sampleDuration = 25;
            mov->tracks[0].stsc_value = 0;
			break;
		default:
			mov->tracks[0].timescale = Mp4MuxerCtx->streams[0]->codec.frame_rate;
			mov->tracks[0].sampleDuration = 100;
			break;
		}*/
		mov->tracks[0].timescale = 1000;
		break;	
	case SETTOTALTIME: //ms
		mov->tracks[0].trackDuration = (unsigned int)(((__u64)uParam * mov->tracks[0].timescale) / 1000);
		mov->tracks[1].trackDuration = (unsigned int)(((__u64)uParam * mov->tracks[1].timescale) / 1000);
		mov->tracks[2].trackDuration = (unsigned int)(((__u64)uParam * mov->tracks[2].timescale) / 1000);
		break;
    case SETFALLOCATELEN:
        Mp4MuxerCtx->mFallocateLen = uParam;
        break;
	case SETCACHEFD:
    {
        //s->pb = (FILE *)uParam;
        CedarXDataSourceDesc datasourceDesc;
        memset(&datasourceDesc, 0, sizeof(CedarXDataSourceDesc));
        datasourceDesc.source_url = (char*)pParam2;
        datasourceDesc.source_type = CEDARX_SOURCE_FILEPATH;
        datasourceDesc.stream_type = CEDARX_STREAM_LOCALFILE;
        Mp4MuxerCtx->pb_cache = create_outstream_handle(&datasourceDesc);
        if(NULL == Mp4MuxerCtx->pb_cache)
        {
            aloge("fatal error! create mp4 outstream fail.");
            return -1;
        }
		break;
    }
    case SETCACHEFD2:
    {
        CedarXDataSourceDesc datasourceDesc;
        memset(&datasourceDesc, 0, sizeof(CedarXDataSourceDesc));
        datasourceDesc.ext_fd_desc.fd = (int)uParam;
        datasourceDesc.source_type = CEDARX_SOURCE_FD;
        datasourceDesc.stream_type = CEDARX_STREAM_LOCALFILE;
        Mp4MuxerCtx->pb_cache = create_outstream_handle(&datasourceDesc);
        if(NULL == Mp4MuxerCtx->pb_cache)
        {
            aloge("fatal error! create aac outstream fail.");
            return -1;
        }
        if(Mp4MuxerCtx->mFallocateLen > 0)
        {
            if(Mp4MuxerCtx->pb_cache->fallocate(Mp4MuxerCtx->pb_cache, 0x01, 0, Mp4MuxerCtx->mFallocateLen) < 0) 
            {
                aloge("fatal error! Failed to fallocate size %d, fd[%d](%s)", Mp4MuxerCtx->mFallocateLen, Mp4MuxerCtx->pb_cache->fd_desc.fd, strerror(errno));
            }
        }
		break;
    }
	case  REGISTER_WRITE_CALLBACK:
		Mp4MuxerCtx->datasource_desc.source_url = (char*)pParam2;
		Mp4MuxerCtx->datasource_desc.source_type = CEDARX_SOURCE_WRITER_CALLBACK;
		if(Mp4MuxerCtx->OutStreamHandle == NULL) {
			Mp4MuxerCtx->OutStreamHandle = create_outstream_handle(&Mp4MuxerCtx->datasource_desc);
            if (NULL == Mp4MuxerCtx->OutStreamHandle)
            {
                aloge("fatal error! create callback outstream fail.");
                return -1;
            }
		}
		else {
			aloge("RawMuxerCtx->OutStreamHandle not NULL");
		}
		break;

    case SETSDCARDSTATE:
        Mp4MuxerCtx->mbSdcardDisappear = !uParam;
        alogd("SETSDCARDSTATE, Mp4MuxerCtx->mbSdcardDisappear[%d]", Mp4MuxerCtx->mbSdcardDisappear);
        break;
    case SETCACHEMEM:
        Mp4MuxerCtx->mCacheMemInfo = *(FsCacheMemInfo*)pParam2;
        break;
    case SET_FS_WRITE_MODE:
        Mp4MuxerCtx->mFsWriteMode = uParam;
        break;
    case SET_FS_SIMPLE_CACHE_SIZE:
        Mp4MuxerCtx->mFsSimpleCacheSize = (int)uParam;
        break;
    case SET_STREAM_CALLBACK:
    {
        cdx_write_callback_t *callback = (cdx_write_callback_t*)pParam2;
        if (Mp4MuxerCtx->pb_cache != NULL) {
            Mp4MuxerCtx->pb_cache->callback.hComp = callback->hComp;
            Mp4MuxerCtx->pb_cache->callback.cb = callback->cb;
        } else {
            alogw("Mp4MuxerCtx->pb_cache not initialize!!");
        }
        break;
    }
	default:
		break;
	}

	return 0;
}

void *Mp4MuxerOpen(int *ret)
{
	AVFormatContext *Mp4MuxerCtx;
	MOVContext *mov;
	AVStream *st;
	int	i;

	*ret = 0;
		
	Mp4MuxerCtx = (AVFormatContext *)malloc(sizeof(AVFormatContext));
	if(!Mp4MuxerCtx)
	{	
		*ret = -1;
		return NULL;
	}
	
	memset(Mp4MuxerCtx,0,sizeof(AVFormatContext));
	
	mov = (MOVContext *)malloc(sizeof(MOVContext));
	if(!mov) {
		*ret = -1;
        free(Mp4MuxerCtx);
		return NULL;
	}
	memset(mov,0,sizeof(MOVContext));

	Mp4MuxerCtx->priv_data = (void *)mov;
	
	for(i=0;i<MAX_STREAMS_IN_FILE;i++)
	{
		//MOVTrack *trk = &mov->tracks[i];

		st = (AVStream *)malloc(sizeof(AVStream));
		if(!st) 
        {
            aloge("fatal error! malloc fail!");
			*ret = -1;
            return (void*)Mp4MuxerCtx;
		}
		memset(st,0,sizeof(AVStream));
		
		Mp4MuxerCtx->streams[i] = st;

        st->codec.codec_type = (i==0) ? CODEC_TYPE_VIDEO : ((i==1) ? CODEC_TYPE_AUDIO : CODEC_TYPE_TEXT);
	}
    Mp4MuxerCtx->mov_inf_cache  = (unsigned char*)malloc(TOTAL_CACHE_SIZE*4);//(unsigned char*)PHYMALLOC(MAX_MOV_INFO_CACHE_SIZE, 1024); 

	//printf("Mp4MuxerCtx->mov_inf_cache: %p\n",Mp4MuxerCtx->mov_inf_cache);

	if(!Mp4MuxerCtx->mov_inf_cache)
    {
        __wrn("can't malloc mov info cache buffer!\n");
        *ret = -1;
    }

    mov->cache_keyframe_ptr = (unsigned int*)malloc(KEYFRAME_CACHE_SIZE);
    
	mov->cache_start_ptr[STCO_ID][CODEC_TYPE_VIDEO] = (unsigned int*)Mp4MuxerCtx->mov_inf_cache;
    mov->cache_read_ptr[STCO_ID][CODEC_TYPE_VIDEO] = (unsigned int*)Mp4MuxerCtx->mov_inf_cache; 
	mov->cache_write_ptr[STCO_ID][CODEC_TYPE_VIDEO] = (unsigned int*)Mp4MuxerCtx->mov_inf_cache;
    
	mov->cache_start_ptr[STCO_ID][CODEC_TYPE_AUDIO] = mov->cache_start_ptr[STCO_ID][CODEC_TYPE_VIDEO] + STCO_CACHE_SIZE;
    mov->cache_read_ptr[STCO_ID][CODEC_TYPE_AUDIO] = mov->cache_start_ptr[STCO_ID][CODEC_TYPE_VIDEO] + STCO_CACHE_SIZE; 
    mov->cache_write_ptr[STCO_ID][CODEC_TYPE_AUDIO] = mov->cache_start_ptr[STCO_ID][CODEC_TYPE_VIDEO] + STCO_CACHE_SIZE;

    mov->cache_start_ptr[STCO_ID][CODEC_TYPE_TEXT] = mov->cache_start_ptr[STCO_ID][CODEC_TYPE_AUDIO] + STCO_CACHE_SIZE;
    mov->cache_read_ptr[STCO_ID][CODEC_TYPE_TEXT] = mov->cache_start_ptr[STCO_ID][CODEC_TYPE_AUDIO] + STCO_CACHE_SIZE;
    mov->cache_write_ptr[STCO_ID][CODEC_TYPE_TEXT] = mov->cache_start_ptr[STCO_ID][CODEC_TYPE_AUDIO] + STCO_CACHE_SIZE;
    
	
	mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_VIDEO] = mov->cache_start_ptr[STCO_ID][CODEC_TYPE_AUDIO] + 2*STCO_CACHE_SIZE;
    mov->cache_read_ptr[STSZ_ID][CODEC_TYPE_VIDEO] = mov->cache_start_ptr[STCO_ID][CODEC_TYPE_AUDIO] + 2*STCO_CACHE_SIZE;
	mov->cache_write_ptr[STSZ_ID][CODEC_TYPE_VIDEO] = mov->cache_start_ptr[STCO_ID][CODEC_TYPE_AUDIO] + 2*STCO_CACHE_SIZE;
    
	mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_AUDIO] = mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_VIDEO] + STSZ_CACHE_SIZE;
    mov->cache_read_ptr[STSZ_ID][CODEC_TYPE_AUDIO] = mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_VIDEO] + STSZ_CACHE_SIZE;
    mov->cache_write_ptr[STSZ_ID][CODEC_TYPE_AUDIO] = mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_VIDEO] + STSZ_CACHE_SIZE;
    
	mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_TEXT] = mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_AUDIO] + STSZ_CACHE_SIZE;
    mov->cache_read_ptr[STSZ_ID][CODEC_TYPE_TEXT] = mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_AUDIO] + STSZ_CACHE_SIZE;
    mov->cache_write_ptr[STSZ_ID][CODEC_TYPE_TEXT] = mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_AUDIO] + STSZ_CACHE_SIZE;
	
	mov->cache_start_ptr[STSC_ID][CODEC_TYPE_VIDEO] = mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_AUDIO] + 2*STSZ_CACHE_SIZE;
    mov->cache_read_ptr[STSC_ID][CODEC_TYPE_VIDEO] = mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_AUDIO] + 2*STSZ_CACHE_SIZE;
    mov->cache_write_ptr[STSC_ID][CODEC_TYPE_VIDEO] = mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_AUDIO] + 2*STSZ_CACHE_SIZE;

    mov->cache_start_ptr[STSC_ID][CODEC_TYPE_AUDIO] = mov->cache_start_ptr[STSC_ID][CODEC_TYPE_VIDEO] + STSC_CACHE_SIZE;
    mov->cache_read_ptr[STSC_ID][CODEC_TYPE_AUDIO] = mov->cache_start_ptr[STSC_ID][CODEC_TYPE_VIDEO] + STSC_CACHE_SIZE;
    mov->cache_write_ptr[STSC_ID][CODEC_TYPE_AUDIO] = mov->cache_start_ptr[STSC_ID][CODEC_TYPE_VIDEO] + STSC_CACHE_SIZE;
    
	mov->cache_start_ptr[STSC_ID][CODEC_TYPE_TEXT] = mov->cache_start_ptr[STSC_ID][CODEC_TYPE_AUDIO] + STSC_CACHE_SIZE;
    mov->cache_read_ptr[STSC_ID][CODEC_TYPE_TEXT] = mov->cache_start_ptr[STSC_ID][CODEC_TYPE_AUDIO] + STSC_CACHE_SIZE;
	mov->cache_write_ptr[STSC_ID][CODEC_TYPE_TEXT] = mov->cache_start_ptr[STSC_ID][CODEC_TYPE_AUDIO] + STSC_CACHE_SIZE;

	mov->cache_start_ptr[STTS_ID][CODEC_TYPE_VIDEO] = mov->cache_start_ptr[STSC_ID][CODEC_TYPE_AUDIO] + 2*STSC_CACHE_SIZE;
    mov->cache_read_ptr[STTS_ID][CODEC_TYPE_VIDEO] = mov->cache_start_ptr[STSC_ID][CODEC_TYPE_AUDIO] + 2*STSC_CACHE_SIZE;
	mov->cache_write_ptr[STTS_ID][CODEC_TYPE_VIDEO] = mov->cache_start_ptr[STSC_ID][CODEC_TYPE_AUDIO] + 2*STSC_CACHE_SIZE;
    
	mov->cache_tiny_page_ptr = mov->cache_start_ptr[STTS_ID][CODEC_TYPE_VIDEO] + STTS_CACHE_SIZE;
	
	mov->cache_end_ptr[STCO_ID][CODEC_TYPE_VIDEO] = mov->cache_start_ptr[STCO_ID][CODEC_TYPE_VIDEO] + (STCO_CACHE_SIZE - 1);
	mov->cache_end_ptr[STSZ_ID][CODEC_TYPE_VIDEO] = mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_VIDEO] + (STSZ_CACHE_SIZE - 1);
	mov->cache_end_ptr[STSC_ID][CODEC_TYPE_VIDEO] = mov->cache_start_ptr[STSC_ID][CODEC_TYPE_VIDEO] + (STSC_CACHE_SIZE - 1);
	mov->cache_end_ptr[STTS_ID][CODEC_TYPE_VIDEO] = mov->cache_start_ptr[STTS_ID][CODEC_TYPE_VIDEO] + (STTS_CACHE_SIZE - 1);
	
	mov->cache_end_ptr[STCO_ID][CODEC_TYPE_AUDIO] = mov->cache_start_ptr[STCO_ID][CODEC_TYPE_AUDIO] + (STCO_CACHE_SIZE - 1);
	mov->cache_end_ptr[STSZ_ID][CODEC_TYPE_AUDIO] = mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_AUDIO] + (STSZ_CACHE_SIZE - 1);
	mov->cache_end_ptr[STSC_ID][CODEC_TYPE_AUDIO] = mov->cache_start_ptr[STSC_ID][CODEC_TYPE_AUDIO] + (STSC_CACHE_SIZE - 1);

	mov->cache_end_ptr[STCO_ID][CODEC_TYPE_TEXT] = mov->cache_start_ptr[STCO_ID][CODEC_TYPE_TEXT] + (STCO_CACHE_SIZE - 1);
	mov->cache_end_ptr[STSZ_ID][CODEC_TYPE_TEXT] = mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_TEXT] + (STSZ_CACHE_SIZE - 1);
	mov->cache_end_ptr[STSC_ID][CODEC_TYPE_TEXT] = mov->cache_start_ptr[STSC_ID][CODEC_TYPE_TEXT] + (STSC_CACHE_SIZE - 1);

//    mov->stsz_cache_offset_in_file[0] = STSZ_CACHE_OFFSET_INFILE_VIDEO;
//    mov->stsz_cache_offset_in_file[1] = STSZ_CACHE_OFFSET_INFILE_AUDIO;
//    mov->stco_cache_offset_in_file[0] = STCO_CACHE_OFFSET_INFILE_VIDEO;
//    mov->stco_cache_offset_in_file[1] = STCO_CACHE_OFFSET_INFILE_AUDIO;
//    mov->stsc_cache_offset_in_file[0] = STSC_CACHE_OFFSET_INFILE_VIDEO;
//    mov->stsc_cache_offset_in_file[1] = STSC_CACHE_OFFSET_INFILE_AUDIO;
//    mov->stts_cache_offset_in_file[0] = STTS_CACHE_OFFSET_INFILE_VIDEO;

    mov->last_stream_index = -1;
	Mp4MuxerCtx->firstframe[0] = 1;
	Mp4MuxerCtx->firstframe[1] = 1;
	mov->keyframe_interval = 1;

//	mov->payload_buffer_cache_start = (unsigned char *)malloc(PAYLOAD_CACHE_SIZE);
//
//	if(mov->payload_buffer_cache_start == NULL)
//	{	
//		aloge("malloc mov->payload_buffer_cache_start error");
//		*ret = -1;
//	}
	return (void*)Mp4MuxerCtx;
}

int Mp4MuxerClose(void *handle)
{
	AVFormatContext *Mp4MuxerCtx = (AVFormatContext *)handle;
	MOVContext *mov = Mp4MuxerCtx->priv_data;
    int i;
//	int	len;
//    char str_name_stsz[MOV_BUF_NAME_LEN] = "/mnt/extsd/mov_stsz_muxer";
//    char str_name_stts[MOV_BUF_NAME_LEN] = "/mnt/extsd/mov_stts_muxer";
//    char str_name_stco[MOV_BUF_NAME_LEN] = "/mnt/extsd/mov_stco_muxer";
//    char str_name_stsc[MOV_BUF_NAME_LEN] = "/mnt/extsd/mov_stsc_muxer";   
//    char str_extend[10] = ".buf";
//    char* str1,*str2,*str3,*str4;
//    len = strlen(str_name_stsz);
    if(Mp4MuxerCtx->mov_inf_cache)
    {
        free(Mp4MuxerCtx->mov_inf_cache);
        Mp4MuxerCtx->mov_inf_cache = 0;
    }

    for(i=0;i<MAX_STREAMS_IN_FILE;i++)
    {
        if(mov->fd_stts[i])
        {
            //esFSYS_fclose(mov->fd_stts[i]);
            destroy_outstream_handle((struct cdx_stream_info*)mov->fd_stts[i]);
            mov->fd_stts[i] = 0;
            if(0 == Mp4MuxerCtx->mbSdcardDisappear)
            {
                //esFSYS_remove(mov->FilePath_stts[i]);
                stream_remove_file(mov->FilePath_stts[i]);
                alogd("remove fd_stts[%d]name[%s]", i, mov->FilePath_stts[i]);
            }
        }

        if(mov->fd_stsz[i])
        {
            //esFSYS_fclose(mov->fd_stsz[i]);
            destroy_outstream_handle((struct cdx_stream_info*)mov->fd_stsz[i]);
            mov->fd_stsz[i] = 0;
            if(0 == Mp4MuxerCtx->mbSdcardDisappear)
            {
                //esFSYS_remove(mov->FilePath_stsz[i]);
                stream_remove_file(mov->FilePath_stsz[i]);
                alogd("remove fd_stsz[%d]name[%s]", i, mov->FilePath_stsz[i]);
            }
        }

        if(mov->fd_stco[i])
        {
            //esFSYS_fclose(mov->fd_stco[i]);
            destroy_outstream_handle((struct cdx_stream_info*)mov->fd_stco[i]);
            mov->fd_stco[i] = 0;
            if(0 == Mp4MuxerCtx->mbSdcardDisappear)
            {
                //esFSYS_remove(mov->FilePath_stco[i]);
                stream_remove_file(mov->FilePath_stco[i]);
                alogd("remove fd_stco[%d]name[%s]", i, mov->FilePath_stco[i]);
            }
        }

        if(mov->fd_stsc[i])
        {
            //esFSYS_fclose(mov->fd_stsc[i]);
            destroy_outstream_handle((struct cdx_stream_info*)mov->fd_stsc[i]);
            mov->fd_stsc[i] = 0;
            if(0 == Mp4MuxerCtx->mbSdcardDisappear)
            {
                //esFSYS_remove(mov->FilePath_stsc[i]);
                stream_remove_file(mov->FilePath_stsc[i]);
                alogd("remove fd_stsc[%d]name[%s]", i, mov->FilePath_stsc[i]);
            }
        }
    }
    
	for(i=0;i<MAX_STREAMS_IN_FILE;i++)
	{
        if(mov->tracks[i].vosData)
    	{
    		free(mov->tracks[i].vosData);
            mov->tracks[i].vosData = NULL;
    	}
		if(Mp4MuxerCtx->streams[i])
		{
			free(Mp4MuxerCtx->streams[i]);
            Mp4MuxerCtx->streams[i] = 0;
		}
	}

    if(mov->cache_keyframe_ptr)
    {
        free(mov->cache_keyframe_ptr);
        mov->cache_keyframe_ptr = NULL;
    }
    if(NULL != mov->gps_entry_buff)
    {
        free(mov->gps_entry_buff);
        mov->gps_entry_buff = NULL;
    }
//	if(mov->payload_buffer_cache_start)
//	{	
//		free(mov->payload_buffer_cache_start);
//		mov->payload_buffer_cache_start = NULL;
//	}
    if(Mp4MuxerCtx->mpFsWriter)
    {
        destroyFsWriter(Mp4MuxerCtx->mpFsWriter);
        Mp4MuxerCtx->mpFsWriter = NULL;
    }
    if(Mp4MuxerCtx->pb_cache)
    {
        destroy_outstream_handle(Mp4MuxerCtx->pb_cache);
        Mp4MuxerCtx->pb_cache = NULL;
    }
	if(Mp4MuxerCtx->priv_data)
	{
		free(Mp4MuxerCtx->priv_data);
		Mp4MuxerCtx->priv_data = NULL;
	}
	
	if(Mp4MuxerCtx)
	{
		free(Mp4MuxerCtx);
		Mp4MuxerCtx = NULL;
	}
	return 0;
}

CDX_RecordWriter record_writer_mp4 = {
	.info 				 = "recode write mp4"   ,
	.MuxerOpen           = Mp4MuxerOpen         ,
	.MuxerClose          = Mp4MuxerClose        ,
	.MuxerWriteExtraData = Mp4MuxerWriteVos     ,
	.MuxerWriteHeader    = Mp4MuxerWriteHeader  ,
	.MuxerWriteTrailer   = Mp4MuxerWriteTrailer ,
	.MuxerWritePacket    = Mp4MuxerWritePacket  ,
	.MuxerIoctrl         = Mp4MuxerIoctrl       ,
};
