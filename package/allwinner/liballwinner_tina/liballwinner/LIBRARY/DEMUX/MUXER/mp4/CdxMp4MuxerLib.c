/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxMp4MuxerLib.c
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
#include <errno.h>
#include <sys/stat.h>

#define WRITE_FREE_TAG (0)

/***************************** Write Data to Stream File *****************************/
static cdx_int32 writeBufferStream(ByteIOContext *s, cdx_int8 *buf, cdx_int32 size)
{
#if FS_WRITER
    return s->fsWrite(s, buf, size);
#else
    return s->cdxWrite(s, (cdx_uint8*)buf, size);
#endif
}

static cdx_int32 writeByteStream(ByteIOContext *s, cdx_int32 b)
{
    return writeBufferStream(s, (cdx_int8*)(&b), 1);
}

static cdx_int32 writeTagStream(ByteIOContext *s, const char *tag)
{
    cdx_int32 ret;
    while (*tag)
    {
        ret = writeByteStream(s, *tag++);
        if (ret < 0)
        {
            break;
        }
    }
    return ret;
}

static cdx_int32 writeBe16Stream(ByteIOContext *s, cdx_uint32 val)
{
    cdx_int32 ret;
    if ((ret = writeByteStream(s, val >> 8)) < 0)
    {
        return ret;
    }
    ret = writeByteStream(s, val);
    return ret;
}

static cdx_int32 writeBe24Stream(ByteIOContext *s, cdx_uint32 val)
{
    cdx_int32 ret;
    if ((ret = writeBe16Stream(s, val >> 8)) < 0)
    {
        return ret;
    }
    ret = writeByteStream(s, val);
    return ret;
}

static cdx_int32 writeBe32Stream(ByteIOContext *s, cdx_uint32 val)
{
    val= ((val<<8)&0xFF00FF00) | ((val>>8)&0x00FF00FF);
    val= (val>>16) | (val<<16);
    return writeBufferStream(s, (cdx_int8*)&val, 4);
}

static cdx_int32 writeLe32Stream(ByteIOContext *s, cdx_uint32 val)
{
    return writeBufferStream(s, (cdx_int8*)&val, 4);
}

/***************************** Write Data to Stream File *****************************/

/********************************** Write Video Tag **********************************/
#define MOV_AV_RB32(x)  ((((const cdx_uint8*)(x))[0] << 24) | \
                     (((const cdx_uint8*)(x))[1] << 16) | \
                     (((const cdx_uint8*)(x))[2] <<  8) | \
                      ((const cdx_uint8*)(x))[3])

static int movBswap32(unsigned int val)
{
    val= ((val<<8)&0xFF00FF00) | ((val>>8)&0x00FF00FF);
    val= (val>>16) | (val<<16);
    return val;
}

cdx_uint8 *findAvcStartcode(cdx_uint8 *p, cdx_uint8 *end)
{
    cdx_uint8 *a = p + 4 - ((long)p & 3);

    for (end -= 3; p < a && p < end; p++)
    {
        if (p[0] == 0 && p[1] == 0 && p[2] == 1)
        {
            return p;
        }
    }

    for ( end -= 3; p < end; p += 4 )
    {
        cdx_uint32 x = *(const cdx_uint32*)p;
        if ((x - 0x01010101) & (~x) & 0x80808080)
        { // generic
            if (p[1] == 0)
            {
                if (p[0] == 0 && p[2] == 1)
                {
                    return (p - 1);
                 }
                if (p[2] == 0 && p[3] == 1)
                {
                    return p;
                }
            }
            if ( p[3] == 0 )
            {
                if ( p[2] == 0 && p[4] == 1 )
                {
                    return (p + 1);
                }
                if ( p[4] == 0 && p[5] == 1 )
                {
                    return (p + 2);
                }
            }
        }
    }

    for (end += 3; p < end; p++)
    {
        if (p[0] == 0 && p[1] == 0 && p[2] == 1)
        {
            return p;
        }
    }

    return (end + 3);
}

int parseAvcNalus(cdx_uint8 *buf_in, cdx_uint8 **buf, int *size)
{
    cdx_uint8 *p = buf_in,*ptr_t;
    cdx_uint8 *end = p + *size;
    cdx_uint8 *nal_start, *nal_end;
    unsigned int nal_size_b;

    ptr_t = *buf = malloc(*size + 256);
    nal_start = findAvcStartcode(p, end);
    while (nal_start < end)
    {
        unsigned int nal_size;
        while (!*(nal_start++));
        nal_end = findAvcStartcode(nal_start, end);
        nal_size = nal_end - nal_start;
        nal_size_b = movBswap32(nal_size);
        memcpy(ptr_t, &nal_size_b, 4);
        ptr_t += 4;
        memcpy(ptr_t, nal_start, nal_size);
        ptr_t += nal_size;
        nal_start = nal_end;
    }

    *size = ptr_t - *buf;
    return 0;
}

static cdx_uint32 avccGetSpsPpsSize(cdx_uint8 *data, cdx_int32 len)
{
    cdx_uint32 avcc_size = 0;
    if (len > 6)
    {
        /* check for h264 start code */
        if (MOV_AV_RB32(data) == 0x00000001)
        {
            cdx_uint8 *buf=NULL, *end, *start;
            cdx_uint32 sps_size=0, pps_size=0;

            int ret = parseAvcNalus(data, &buf, &len);
            if (ret < 0)
            {
                logw("(f:%s, l:%d) fatal error! ret[%d] of parseAvcNalus() < 0",
                    __FUNCTION__, __LINE__, ret);
                buf = NULL;
                return 0;
            }
            start = buf;
            end = buf + len;

            /* look for sps and pps */
            while (buf < end)
            {
                unsigned int size;
                cdx_uint8 nal_type;
                size = MOV_AV_RB32(buf);
                nal_type = buf[4] & 0x1f;
                if (nal_type == 7)
                { /* SPS */
                    sps_size = size;
                }
                else if (nal_type == 8)
                { /* PPS */
                    pps_size = size;
                }
                buf += size + 4;
            }
            avcc_size = (6 + 2 + sps_size + 1 + 2 + pps_size);
            free(start);
        }
        else
        {
            avcc_size = len;
        }
    }
    return avcc_size;
}

int avccWriteSpsPps(ByteIOContext *pb, cdx_uint8 *data, int len, MOVTrack *track)
{
    if (len > 6)
    {
        /* check for h264 start code */
        if (MOV_AV_RB32(data) == 0x00000001)
        {
            cdx_uint8 *buf=NULL, *end, *start;
            cdx_uint32 sps_size=0, pps_size=0;
            cdx_uint8 *sps=0, *pps=0;

            int ret = parseAvcNalus(data, &buf, &len);
            if (ret < 0)
            {
                buf = NULL;
                return ret;
            }
            start = buf;
            end = buf + len;

            /* look for sps and pps */
            while (buf < end)
            {
                unsigned int size;
                cdx_uint8 nal_type;
                size = MOV_AV_RB32(buf);
                nal_type = buf[4] & 0x1f;
                if (nal_type == 7)
                { /* SPS */
                    sps = buf + 4;
                    sps_size = size;
                } else if (nal_type == 8)
                { /* PPS */
                    pps = buf + 4;
                    pps_size = size;
                }
                buf += size + 4;
            }

            writeByteStream(pb, 1); /* version */
            writeByteStream(pb, sps[1]); /* profile */
            writeByteStream(pb, sps[2]); /* profile compat */
            writeByteStream(pb, sps[3]); /* level */
            /* 6 bits reserved (111111) + 2 bits nal size length - 1 (11) */
            writeByteStream(pb, 0xff);
            /* 3 bits reserved (111) + 5 bits number of sps (00001) */
            writeByteStream(pb, 0xe1);

            writeBe16Stream(pb, sps_size);
            writeBufferStream(pb, (cdx_int8*)sps, sps_size);
            writeByteStream(pb, 1); /* number of pps */
            writeBe16Stream(pb, pps_size);
            writeBufferStream(pb, (cdx_int8*)pps, pps_size);
            free(start);
        }
        else
        {
            writeBufferStream(pb, (cdx_int8*)data, len);
        }
    }
    return 0;
}

static cdx_uint32 videoGetAvccTagSize(MOVTrack *track)
{
    cdx_uint32 avcc_tag_size = 8;
    avcc_tag_size += avccGetSpsPpsSize((cdx_uint8*)track->vos_data, track->vos_len);
    return avcc_tag_size;
}
static int videoWriteAvccTag(ByteIOContext *pb, MOVTrack *track)
{
    cdx_uint32 avcc_tag_size = videoGetAvccTagSize(track);
    writeBe32Stream(pb, avcc_tag_size);   /* size */
    writeTagStream(pb, "avcC");
    avccWriteSpsPps(pb, (cdx_uint8*)track->vos_data, track->vos_len, track);
    return avcc_tag_size;
}

static cdx_uint32 calDescrLength(cdx_uint32 len)
{
    cdx_int32 i;
    for (i=1; len >> (7 * i); i++);
    return (len + 1 + i);
}

static void writeDescrStream(ByteIOContext *pb, cdx_int32 tag, cdx_uint32 size)
{
    cdx_int32 i= calDescrLength(size) - size - 2;
    writeByteStream(pb, tag);
    for (; i > 0; i--)
    {
        writeByteStream(pb, (size >> (7*i)) | 0x80);
     }
    writeByteStream(pb, size & 0x7F);
}

static cdx_uint32 videoGetEsdsTagSize(MOVTrack *track)
{
    cdx_uint32 esds_tag_size = 12;
    cdx_uint32 dec_spec_info_size = track->vos_len ? calDescrLength(track->vos_len):0;
    cdx_uint32 dec_config_descri_size = calDescrLength(13 + dec_spec_info_size);
    cdx_uint32 sl_config_descri_size = calDescrLength(1);
    cdx_uint32 es_decri_size = calDescrLength(3 + dec_config_descri_size + sl_config_descri_size);
    esds_tag_size += es_decri_size;
    return esds_tag_size;
}

static cdx_int32 videoWriteEsdsTag(ByteIOContext *pb, MOVTrack *track) // Basic
{
    cdx_uint32 esds_tag_size = videoGetEsdsTagSize(track);
    cdx_int32 dec_spec_info_len = track->vos_len ? calDescrLength(track->vos_len) : 0;

    writeBe32Stream(pb, esds_tag_size); /* size */
    writeTagStream(pb, "esds");
    writeBe32Stream(pb, 0); // Version

    // ES descriptor
    writeDescrStream(pb, 0x03, 3 + calDescrLength(13 + dec_spec_info_len) +
        calDescrLength(1));
    writeBe16Stream(pb, track->track_id);
    writeByteStream(pb, 0x00); // flags (= no flags)

    // DecoderConfig descriptor
    writeDescrStream(pb, 0x04, 13 + dec_spec_info_len);

    // Object type indication
    writeByteStream(pb, track->enc.codec_id);

    // the following fields is made of 6 bits to identify the streamtype (4 for video, 5 for audio)
    // plus 1 bit to indicate upstream and 1 bit set to 1 (reserved)
    if (track->enc.codec_type == CODEC_TYPE_AUDIO)
    {
        writeByteStream(pb, 0x15); // flags (= Audiostream)
    }
    else
    {
        writeByteStream(pb, 0x11); // flags (= Visualstream)
    }

    writeByteStream(pb,  0);    // Buffersize DB (24 bits)
    writeBe16Stream(pb,  0); // Buffersize DB

    writeBe32Stream(pb, 0); // maxbitrate (FIXME should be max rate in any 1 sec window)
    writeBe32Stream(pb, 0); // vbr

    if (track->vos_len)
    {
        // DecoderSpecific info descriptor
        writeDescrStream(pb, 0x05, track->vos_len);
        writeBufferStream(pb, track->vos_data, track->vos_len);
    }

    // SL descriptor
    writeDescrStream(pb, 0x06, 1);
    writeByteStream(pb, 0x02);
    //return updateSize(pb, pos);
    return esds_tag_size;
}

static cdx_uint32 stsdGetVideoTagSize(MOVTrack *track)
{
    cdx_uint32 video_tag_size = 16 + 4 + 12 + 18 + 32 + 4;
    if (track->tag == MOV_MKTAG('a','v','c','1'))
    {
        video_tag_size += videoGetAvccTagSize(track);
    }
    else if (track->tag == MOV_MKTAG('m','p','4','v'))
    {
        video_tag_size += videoGetEsdsTagSize(track);
    }
    return video_tag_size;
}

static cdx_int32 stsdWriteVideoTag(ByteIOContext *pb, MOVTrack *track)
{
    cdx_uint32 video_tag_size = stsdGetVideoTagSize(track);
    cdx_int8 compressor_name[32];

    writeBe32Stream(pb, video_tag_size); /* size */
    writeLe32Stream(pb, track->tag); // store it byteswapped
    writeBe32Stream(pb, 0); /* Reserved */
    writeBe16Stream(pb, 0); /* Reserved */
    writeBe16Stream(pb, 1); /* Data-reference index */

    writeBe16Stream(pb, 0); /* Codec stream_writer version */
    writeBe16Stream(pb, 0); /* Codec stream_writer revision (=0) */
    {
        writeBe32Stream(pb, 0); /* Reserved */
        writeBe32Stream(pb, 0); /* Reserved */
        writeBe32Stream(pb, 0); /* Reserved */
    }
    writeBe16Stream(pb, track->enc.width); /* Video width */
    writeBe16Stream(pb, track->enc.height); /* Video height */
    writeBe32Stream(pb, 0x00480000); /* Horizontal resolution 72dpi */
    writeBe32Stream(pb, 0x00480000); /* Vertical resolution 72dpi */
    writeBe32Stream(pb, 0); /* Data size (= 0) */
    writeBe16Stream(pb, 1); /* Frame count (= 1) */

    memset(compressor_name,0,32);
    writeByteStream(pb, strlen((const char *)compressor_name));
    writeBufferStream(pb, compressor_name, 31);

    writeBe16Stream(pb, 0x18); /* Reserved */
    writeBe16Stream(pb, 0xffff); /* Reserved */
    if (track->tag == MOV_MKTAG('a','v','c','1'))
    {
        videoWriteAvccTag(pb, track);
    }
    else if (track->tag == MOV_MKTAG('m','p','4','v'))
    {
        videoWriteEsdsTag(pb, track);
    }

    return video_tag_size;
}
/********************************** Write Video Tag **********************************/

/*********************************** Write Audio Tag *********************************/
static cdx_uint32 stsdGetAudioTagSize(MOVTrack *track)
{
    cdx_int32 version = 0;
    cdx_uint32 audio_tag_size = 16 + 8 + 12;
    if (track->enc.codec_id == MUX_CODEC_ID_ADPCM)
    {
        version = 1;
    }
    if (version == 1)
    { /* SoundDescription V1 extended info */
        audio_tag_size += 16;
    }
    if (track->tag == MOV_MKTAG('m','p','4','a'))
    {
        audio_tag_size += videoGetEsdsTagSize(track);
    }

    return audio_tag_size;
}

static cdx_int32 stsdWriteAudioTag(ByteIOContext *pb, MOVTrack *track)
{
    cdx_uint32 audio_tag_size = stsdGetAudioTagSize(track);
    //offset_t pos = url_ftell(pb);
    cdx_int32 version = 0;

    if (track->enc.codec_id == MUX_CODEC_ID_ADPCM)
    {
        version = 1;
    }

    writeBe32Stream(pb, audio_tag_size); /* size */
    writeLe32Stream(pb, track->tag); // store it byteswapped
    writeBe32Stream(pb, 0); /* Reserved */
    writeBe16Stream(pb, 0); /* Reserved */
    writeBe16Stream(pb, 1); /* Data-reference index, XXX  == 1 */

    /* SoundDescription */
    writeBe16Stream(pb, version); /* Version */
    writeBe16Stream(pb, 0); /* Revision level */
    writeBe32Stream(pb, 0); /* vendor */

    { /* reserved for mp4/3gp */
        writeBe16Stream(pb, track->enc.channels); //channel
        writeBe16Stream(pb, track->enc.bits_per_sample);//bits per sample
        writeBe16Stream(pb, 0); /* compression id = 0*/
    }

    writeBe16Stream(pb, 0); /* packet size (= 0) */
    writeBe16Stream(pb, track->enc.sample_rate); /* Time scale !!!??? */
    writeBe16Stream(pb, 0); /* Reserved */

    if (version == 1)
    { /* SoundDescription V1 extended info */
        if (track->enc.codec_id == MUX_CODEC_ID_ADPCM)
        {
            writeBe32Stream(pb, track->enc.frame_size); /* Samples per packet */
            /* Bytes per packet */
            writeBe32Stream(pb, track->enc.frame_size * (track->enc.bits_per_sample >> 3));
            writeBe32Stream(pb, track->enc.frame_size*(track->enc.bits_per_sample >> 3) *
                                    track->enc.channels); /* Bytes per frame */
            writeBe32Stream(pb, 2); /* Bytes per sample */
        }
        else
        {
            writeBe32Stream(pb, track->enc.frame_size); /* Samples per packet */
            writeBe32Stream(pb, track->sample_size / track->enc.channels); /* Bytes per packet */
            writeBe32Stream(pb, track->sample_size); /* Bytes per frame */
            writeBe32Stream(pb, 2); /* Bytes per sample */
        }
    }

    if (track->tag == MOV_MKTAG('m','p','4','a'))
    {
        videoWriteEsdsTag(pb, track);
    }

    //return updateSize(pb, pos);
    return audio_tag_size;
}
/*********************************** Write Audio Tag *********************************/

/****************************** Write Stbl Level Info *********************************/
static cdx_uint32 stblGetStsdTagSize(MOVTrack *track)
{
    cdx_uint32 stsd_tag_size = 16;
    if (track->enc.codec_type == CODEC_TYPE_VIDEO)
    {
        stsd_tag_size += stsdGetVideoTagSize(track);
    }
    else if (track->enc.codec_type == CODEC_TYPE_AUDIO)
    {
        stsd_tag_size += stsdGetAudioTagSize(track);
    }
    return stsd_tag_size;
}

static cdx_int32 stblWriteStsdTag(ByteIOContext *pb, MOVTrack *track)
{
    cdx_uint32 stsd_tag_size = stblGetStsdTagSize(track);
    writeBe32Stream(pb, stsd_tag_size); /* size */
    writeTagStream(pb, "stsd");
    writeBe32Stream(pb, 0); /* version & flags */
    writeBe32Stream(pb, 1); /* entry count */
    if (track->enc.codec_type == CODEC_TYPE_VIDEO)
    {
        stsdWriteVideoTag(pb, track);
    }
    else if (track->enc.codec_type == CODEC_TYPE_AUDIO)
    {
        stsdWriteAudioTag(pb, track);
    }

    return stsd_tag_size;
}

static cdx_uint32 movGetSttsTagSize(MOVTrack *track)
{
    cdx_uint32 entries = 0;
    cdx_uint32 atom_size;
    if (track->enc.codec_type == CODEC_TYPE_AUDIO)
    {
        entries = 1;
        atom_size = 16 + (entries * 8);
    }
    else
    {
        entries = track->stts_total_num;
        atom_size = 16 + (entries * 8);
    }
    return atom_size;
}

// Time-to-Sample Atom
static cdx_int32 stblWriteSttsTag(ByteIOContext *pb, MOVTrack *track)
{
    MuxMOVContext *mov = track->mov;
    SttsTable stts_entries[1];
    cdx_uint32 entries = 0;
    cdx_uint32 atom_size;
    cdx_uint32 i,j;
    cdx_uint32 pos;

    if (track->enc.codec_type == CODEC_TYPE_AUDIO)
    {
        stts_entries[0].count = track->stsz_total_num;
        // for uncompressed audio, one frame == one sample
        if (track->enc.codec_id == MUX_CODEC_ID_PCM)
        {
            stts_entries[0].duration = 1;
        }
        else
        {
            stts_entries[0].duration = track->enc.frame_size;
        }
        entries = 1;
        atom_size = 16 + (entries * 8);
        writeBe32Stream(pb, atom_size); /* size */
        writeTagStream(pb, "stts");
        writeBe32Stream(pb, 0); /* version & flags */
        writeBe32Stream(pb, entries); /* entry count */
        for (i = 0; i < entries; i++)
        {
            writeBe32Stream(pb, stts_entries[i].count);
            writeBe32Stream(pb, stts_entries[i].duration);
        }
    }
    else
    {
        cdx_int32 skip_first_frame = 1;

        entries = track->stts_total_num;
        atom_size = 16 + (entries * 8);
        writeBe32Stream(pb, atom_size); /* size */
        writeTagStream(pb, "stts");
        writeBe32Stream(pb, 0); /* version & flags */
        writeBe32Stream(pb, entries); /* entry count */

        if (mov->play_time_length)
        {
            cdx_uint32 remainder_len = track->track_duration % track->stts_total_num;
            for (i = 0; i < track->stts_total_num - 1; i++)
            {
                if (skip_first_frame)
                {
                    skip_first_frame = 0;
                    continue;
                }
                writeBe32Stream(pb, 1);//count
                writeBe32Stream(pb, track->average_duration);
            }
            writeBe32Stream(pb, 1);//count
            writeBe32Stream(pb, track->average_duration + remainder_len);
        }
        else if (track->stts_tiny_pages == 0)
        {
            for (i = 0; i < track->stts_total_num; i++)
            {
                if (skip_first_frame)
                {
                    skip_first_frame = 0;
                    mov->cache_read_ptr[STTS_ID][track->stream_type]++;
                    continue;
                }
                writeBe32Stream(pb, 1);//count
                writeBe32Stream(pb,*mov->cache_read_ptr[STTS_ID][track->stream_type]++);
            }
        }
        else
        {
            int stts_fd = mov->fd_stts[track->stream_type];
            pos = lseek(stts_fd, 0, SEEK_SET);
            logv("the position is :%d\n",pos);
            for (i = 0; i < track->stts_tiny_pages; i++)
            {
                read(stts_fd, mov->cache_tiny_page_ptr, MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE);
                for (j = 0;j < MOV_CACHE_TINY_PAGE_SIZE; j++)
                {
                    if (skip_first_frame)
                    {
                        skip_first_frame = 0;
                        continue;
                    }
                    writeBe32Stream(pb, 1);//count
                    writeBe32Stream(pb,mov->cache_tiny_page_ptr[j]);
                }
            }

            for (i = track->stts_tiny_pages * MOV_CACHE_TINY_PAGE_SIZE;
                 i < track->stts_total_num; i++)
            {
                writeBe32Stream(pb, 1);//count
                writeBe32Stream(pb,*mov->cache_read_ptr[STTS_ID][track->stream_type]++);
                if (mov->cache_read_ptr[STTS_ID][track->stream_type] >
                    mov->cache_end_ptr[STTS_ID][track->stream_type])
                {
                    mov->cache_read_ptr[STTS_ID][track->stream_type] =
                        mov->cache_start_ptr[STTS_ID][track->stream_type];
                }
            }
        }

        //write last packet duration, set it to 0??
        writeBe32Stream(pb, 1);//count
        writeBe32Stream(pb, 0);
    }

    return atom_size;
}

static cdx_uint32 stblGetStssTagSize(MOVTrack *track)
{
    cdx_int32 keyframes = track->key_frame_num;
    cdx_uint32 stss_tag_size = (keyframes + 4) * 4;
    return stss_tag_size;
}

/* Sync sample atom */
static cdx_int32 stblWriteStssTag(ByteIOContext *pb, MOVTrack *track)
{
    MuxMOVContext *mov = track->mov;
    cdx_int32 i, keyframes;
    cdx_uint32 stss_tag_size = stblGetStssTagSize(track);
    keyframes = track->key_frame_num;
    writeBe32Stream(pb, stss_tag_size); // size
    writeTagStream(pb, "stss");
    writeBe32Stream(pb, 0); // version & flags
    writeBe32Stream(pb, keyframes); // entry count
    for (i = 0; i < keyframes; i++)
    {
        writeBe32Stream(pb, mov->cache_keyframe_ptr[i]);
    }

    return stss_tag_size;//updateSize(pb, pos);
}

static cdx_uint32 stblGetStscTagSize(MOVTrack *track)
{
    cdx_uint32 stsc_tag_size = (track->stsc_total_num*3 + 4)*4;
    return stsc_tag_size;
}

/* Sample to chunk atom */
static cdx_int32 stblWriteStscTag(ByteIOContext *pb, MOVTrack *track)
{
    cdx_uint32 i;
    MuxMOVContext *mov = track->mov;
    cdx_uint32 stsc_tag_size = stblGetStscTagSize(track);
    //offset_t pos = url_ftell(pb);
    writeBe32Stream(pb, stsc_tag_size); /* size */
    writeTagStream(pb, "stsc");
    writeBe32Stream(pb, 0); // version & flags
    writeBe32Stream(pb, track->stsc_total_num); // entry count

    if (track->stsc_tiny_pages == 0)
    {
        for (i=0; i<track->stsc_total_num; i++)
        {
            writeBe32Stream(pb, i+1); // first chunk
            writeBe32Stream(pb,*mov->cache_read_ptr[STSC_ID][track->stream_type]++);
            writeBe32Stream(pb, 0x1); // sample description index
        }
    }
    else
    {
        int stsc_fd = mov->fd_stsc[track->stream_type];
        lseek(stsc_fd, 0, SEEK_SET);
        for (i = 0; i < track->stsc_tiny_pages * MOV_CACHE_TINY_PAGE_SIZE; i++)
        {
            writeBe32Stream(pb, i+1); // first chunk
            read(stsc_fd, mov->cache_tiny_page_ptr, 4);
            writeBe32Stream(pb,*mov->cache_tiny_page_ptr);
            writeBe32Stream(pb, 0x1); // sample description index
        }

        for (i=track->stsc_tiny_pages*MOV_CACHE_TINY_PAGE_SIZE; i<track->stsc_total_num; i++)
        {
            writeBe32Stream(pb, i+1); // first chunk
            writeBe32Stream(pb,*mov->cache_read_ptr[STSC_ID][track->stream_type]++);
            writeBe32Stream(pb, 0x1); // sample description index
            if (mov->cache_read_ptr[STSC_ID][track->stream_type] >
                mov->cache_end_ptr[STSC_ID][track->stream_type])
            {
                mov->cache_read_ptr[STSC_ID][track->stream_type] =
                    mov->cache_start_ptr[STSC_ID][track->stream_type];
            }
        }
    }

    return stsc_tag_size; //updateSize(pb, pos);
}

static cdx_uint32 stblGetStszTagSize(MOVTrack *track)
{
    cdx_uint32 stsz_tag_size;
    if (track->enc.codec_id == MUX_CODEC_ID_PCM)
    {
        stsz_tag_size = 5 * 4;
    }
    else
    {
        stsz_tag_size = (track->stsz_total_num + 5) * 4;
    }
    return stsz_tag_size;

}

/* Sample size atom */
static cdx_int32 stblWriteStszTag(ByteIOContext *pb, MOVTrack *track)
{
    MuxMOVContext *mov = track->mov;
    cdx_uint32 stsz_tag_size = stblGetStszTagSize(track);
    cdx_uint32 i,j;
    writeBe32Stream(pb, stsz_tag_size); /* size */
    writeTagStream(pb, "stsz");
    writeBe32Stream(pb, 0); /* version & flags */
    if (track->enc.codec_id == MUX_CODEC_ID_PCM)
    {
        writeBe32Stream(pb, track->enc.channels * (track->enc.bits_per_sample >> 3));// sample size
    }
    else
    {
        writeBe32Stream(pb, 0); // sample size
    }
    writeBe32Stream(pb, track->stsz_total_num); // sample count
    if (track->enc.codec_id == MUX_CODEC_ID_PCM)
    {
        return stsz_tag_size;
    }

    if (track->stsz_tiny_pages == 0)
    {
        for (i = 0; i < track->stsz_total_num; i++)
        {
            writeBe32Stream(pb, *mov->cache_read_ptr[STSZ_ID][track->stream_type]++);
        }
    }
    else
    {
        int stsz_fd = mov->fd_stsz[track->stream_type];
        lseek(stsz_fd, 0, SEEK_SET);
        for (i = 0; i < track->stsz_tiny_pages; i++)
        {
            read(stsz_fd, mov->cache_tiny_page_ptr, MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE);
            for (j = 0;j < MOV_CACHE_TINY_PAGE_SIZE; j++)
            {
                writeBe32Stream(pb,mov->cache_tiny_page_ptr[j]);
            }
        }

        for (i = track->stsz_tiny_pages * MOV_CACHE_TINY_PAGE_SIZE; i < track->stsz_total_num; i++)
        {
            writeBe32Stream(pb, *mov->cache_read_ptr[STSZ_ID][track->stream_type]++);
            if (mov->cache_read_ptr[STSZ_ID][track->stream_type] >
                mov->cache_end_ptr[STSZ_ID][track->stream_type])
            {
                mov->cache_read_ptr[STSZ_ID][track->stream_type] =
                    mov->cache_start_ptr[STSZ_ID][track->stream_type];
            }
        }
    }

    return stsz_tag_size; //updateSize(pb, pos);
}

static cdx_uint32 stblGetStcoTagSize(MOVTrack *track)
{
    cdx_uint32 stco_tag_size = (track->stco_total_num + 4) * 4;
    return stco_tag_size;
}

/* Chunk offset atom */
static cdx_int32 stblWriteStcoTag(ByteIOContext *pb, MOVTrack *track)
{
    cdx_uint32 i,j;
    MuxMOVContext *mov = track->mov;
    cdx_uint32 stco_tag_size = stblGetStcoTagSize(track);
    writeBe32Stream(pb, stco_tag_size); /* size */
    writeTagStream(pb, "stco");
    writeBe32Stream(pb, 0); /* version & flags */
    writeBe32Stream(pb, track->stco_total_num); /* entry count */

    if (track->stco_tiny_pages == 0)
    {
        for (i=0; i<track->stco_total_num; i++)
        {
            writeBe32Stream(pb,*mov->cache_read_ptr[STCO_ID][track->stream_type]++);
        }
    }
    else
    {
        int stco_fd = mov->fd_stco[track->stream_type];
        lseek(stco_fd, 0, SEEK_SET);
        for (i = 0; i < track->stco_tiny_pages;i++)
        {
            read(stco_fd, mov->cache_tiny_page_ptr, MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE);
            for (j = 0;j < MOV_CACHE_TINY_PAGE_SIZE; j++)
            {
                writeBe32Stream(pb, mov->cache_tiny_page_ptr[j]);
            }
        }

        for (i = track->stco_tiny_pages * MOV_CACHE_TINY_PAGE_SIZE; i<track->stco_total_num; i++)
        {
            writeBe32Stream(pb, *mov->cache_read_ptr[STCO_ID][track->stream_type]++);
            if (mov->cache_read_ptr[STCO_ID][track->stream_type] >
                mov->cache_end_ptr[STCO_ID][track->stream_type])
            {
                mov->cache_read_ptr[STCO_ID][track->stream_type] =
                    mov->cache_start_ptr[STCO_ID][track->stream_type];
            }
        }
    }

    return stco_tag_size; //updateSize(pb, pos);
}
/****************************** Write Stbl Level Info *********************************/

/******************************* Write Minf Level Info ********************************/
static cdx_uint32 minfGetVmhdTagSize()
{
    return 0x14;
}

static cdx_int32 minfWriteVmhdTag(ByteIOContext *pb, MOVTrack *track)
{
    cdx_uint32 vmhd_tag_size = minfGetVmhdTagSize();
    writeBe32Stream(pb, vmhd_tag_size); /* size (always 0x14) */
    writeTagStream(pb, "vmhd");
    writeBe32Stream(pb, 0x01); /* version & flags */
    writeBe32Stream(pb, 0x0);
    writeBe32Stream(pb, 0x0);
    return vmhd_tag_size;
}

static cdx_uint32 minfGetSmhdTagSize()
{
    return 16;
}

static cdx_int32 minfWriteSmhdTag(ByteIOContext *pb, MOVTrack *track)
{
    cdx_uint32 smhd_tag_size = minfGetSmhdTagSize();
    writeBe32Stream(pb, smhd_tag_size); /* size */
    writeTagStream(pb, "smhd");
    writeBe32Stream(pb, 0); /* version & flags */
    writeBe16Stream(pb, 0); /* reserved (balance, normally = 0) */
    writeBe16Stream(pb, 0); /* reserved */
    return smhd_tag_size;
}

static cdx_uint32 dinfGetDrefTagSize()
{
    return 28;
}

static cdx_int32 dinfWriteDrefTag(ByteIOContext *pb, MOVTrack *track)
{
    cdx_uint32 dref_tag_size = dinfGetDrefTagSize();
    writeBe32Stream(pb, dref_tag_size); /* size */
    writeTagStream(pb, "dref");
    writeBe32Stream(pb, 0); /* version & flags */
    writeBe32Stream(pb, 1); /* entry count */

    writeBe32Stream(pb, 0xc); /* size */
    writeTagStream(pb, "url ");
    writeBe32Stream(pb, 1); /* version & flags */

    return dref_tag_size;
}
static cdx_uint32 minfGetDinfTagSize()
{
    cdx_uint32 dinf_tag_size = 8;
    dinf_tag_size += dinfGetDrefTagSize();
    return dinf_tag_size;
}

static cdx_int32 minfWriteDinfTag(ByteIOContext *pb, MOVTrack *track)
{
    cdx_uint32 dinf_tag_size = minfGetDinfTagSize();
    writeBe32Stream(pb, dinf_tag_size); /* size */
    writeTagStream(pb, "dinf");
    dinfWriteDrefTag(pb, track);
    return dinf_tag_size;
}

static cdx_uint32 minfGetStblTagSize(MOVTrack *track)
{
    cdx_uint32 stbl_tag_size = 8;
    stbl_tag_size += stblGetStsdTagSize(track);
    stbl_tag_size += movGetSttsTagSize(track);
    if (track->enc.codec_type == CODEC_TYPE_VIDEO)
    {
        stbl_tag_size += stblGetStssTagSize(track);
    }
    stbl_tag_size += stblGetStscTagSize(track);
    stbl_tag_size += stblGetStszTagSize(track);
    stbl_tag_size += stblGetStcoTagSize(track);
    return stbl_tag_size;
}

static cdx_int32 minfWriteStblTag(ByteIOContext *pb, MOVTrack *track)
{
    cdx_uint32 stbl_tag_size = minfGetStblTagSize(track);
    //offset_t pos = url_ftell(pb);
    writeBe32Stream(pb, stbl_tag_size); /* size */
    writeTagStream(pb, "stbl");
    stblWriteStsdTag(pb, track);
    stblWriteSttsTag(pb, track);
    if (track->enc.codec_type == CODEC_TYPE_VIDEO)
    {
        stblWriteStssTag(pb, track);
    }
    stblWriteStscTag(pb, track);
    stblWriteStszTag(pb, track);
    stblWriteStcoTag(pb, track);

    return stbl_tag_size;
}
/******************************* Write Minf Level Info ********************************/

/******************************** Write Mdia Level Info ********************************/
static cdx_uint32 mdiaGetMdhdTagSize()
{
    return 0x20;
}

static cdx_int32 mdiaWriteMdhdTag(ByteIOContext *pb, MOVTrack *track)
{
    cdx_uint32 mdhd_tag_size = mdiaGetMdhdTagSize();
    writeBe32Stream(pb, mdhd_tag_size); /* size */
    writeTagStream(pb, "mdhd");
    writeByteStream(pb, 0);
    writeBe24Stream(pb, 0); /* flags */

    writeBe32Stream(pb, track->time); /* creation time */
    writeBe32Stream(pb, track->time); /* modification time */
    writeBe32Stream(pb, track->track_timescale); /* time scale (sample rate for audio) */
    writeBe32Stream(pb, track->track_duration*track->track_timescale/GLOBAL_TIMESCALE); /* duration */
	//writeBe32Stream(pb, track->track_duration); /* duration */
    writeBe16Stream(pb, /*track->language*/0); /* language */
    writeBe16Stream(pb, 0); /* reserved (quality) */

    return mdhd_tag_size;
}

static cdx_uint32 mdiaGetHdlrTagSize(MOVTrack *track)
{
    cdx_uint32 hdlr_tag_size = 32 + 1;

    if (!track)
    { /* no media --> data handler */
        hdlr_tag_size += strlen("DataHandler");
    }
    else
    {
        if (track->enc.codec_type == CODEC_TYPE_VIDEO)
        {
            hdlr_tag_size += strlen("VideoHandler");
        }
        else
        {
            hdlr_tag_size += strlen("SoundHandler");
        }
    }
    return hdlr_tag_size;
}

static cdx_int32 mdiaWriteHdlrTag(ByteIOContext *pb, MOVTrack *track)
{
    char *descr, *hdlr, *hdlr_type;
    //offset_t pos = url_ftell(pb);

    if (!track)
    { /* no media --> data handler */
        hdlr = "dhlr";
        hdlr_type = "url ";
        descr = "DataHandler";
    }
    else
    {
        hdlr = "\0\0\0\0";
        if (track->enc.codec_type == CODEC_TYPE_VIDEO)
        {
            hdlr_type = "vide";
            descr = "VideoHandler";
        }
        else
        {
            hdlr_type = "soun";
            descr = "SoundHandler";
        }
    }
    cdx_uint32 hdlr_tag_size = mdiaGetHdlrTagSize(track);
    writeBe32Stream(pb, hdlr_tag_size); /* size */
    writeTagStream(pb, "hdlr");
    writeBe32Stream(pb, 0); /* Version & flags */
    writeBufferStream(pb, (cdx_int8*)hdlr, 4); /* handler */
    writeTagStream(pb, hdlr_type); /* handler type */
    writeBe32Stream(pb ,0); /* reserved */
    writeBe32Stream(pb ,0); /* reserved */
    writeBe32Stream(pb ,0); /* reserved */
    writeByteStream(pb, strlen((const char *)descr)); /* string counter */
    writeBufferStream(pb, (cdx_int8*)descr, strlen((const char *)descr)); /* handler description */

    return hdlr_tag_size;
}

static cdx_uint32 mdiaGetMinfTagSize(MOVTrack *track)
{
    cdx_uint32 minf_tag_size = 8;
    if (track->enc.codec_type == CODEC_TYPE_VIDEO)
    {
        minf_tag_size += minfGetVmhdTagSize();
    }
    else
    {
        minf_tag_size += minfGetSmhdTagSize();
    }

    minf_tag_size += minfGetDinfTagSize();
    minf_tag_size += minfGetStblTagSize(track);
    return minf_tag_size;
}

static cdx_int32 mdiaWriteMinfTag(ByteIOContext *pb, MOVTrack *track)
{
    cdx_uint32 minf_tag_size = mdiaGetMinfTagSize(track);
    writeBe32Stream(pb, minf_tag_size); /* size */
    writeTagStream(pb, "minf");
    if (track->enc.codec_type == CODEC_TYPE_VIDEO)
    {
        minfWriteVmhdTag(pb, track);
    }
    else
    {
        minfWriteSmhdTag(pb, track);
    }
    minfWriteDinfTag(pb, track);
    minfWriteStblTag(pb, track);

    return minf_tag_size;
}
/******************************** Write Mdia Level Info ********************************/

/********************************* Write Trak Level Info *******************************/
static cdx_int32 avRescaleRnd(cdx_int64 a, cdx_int64 b, cdx_int64 c)
{
    return (a * b + c - 1) / c;
}

static cdx_uint32 trakGetTkhdTagSize()
{
    return 0x5c;
}

static cdx_int32 trakWriteTkhdTag(ByteIOContext *pb, MOVTrack *track)
{
	cdx_int64 duration = avRescaleRnd(track->track_duration,
                                      track->track_timescale, track->track_timescale);
    //cdx_int64 duration = avRescaleRnd(track->track_duration,
    //                                  GLOBAL_TIMESCALE, track->track_timescale);
    cdx_int32 version = 0;
    cdx_uint32 tkhd_tag_size = trakGetTkhdTagSize();
    writeBe32Stream(pb, tkhd_tag_size); /* size */
    writeTagStream(pb, "tkhd");
    writeByteStream(pb, version);
    writeBe24Stream(pb, 0xf); /* flags (track enabled) */

    writeBe32Stream(pb, track->time); /* creation time */
    writeBe32Stream(pb, track->time); /* modification time */

    writeBe32Stream(pb, track->track_id); /* track-id */
    writeBe32Stream(pb, 0); /* reserved */
    writeBe32Stream(pb, duration);

    writeBe32Stream(pb, 0); /* reserved */
    writeBe32Stream(pb, 0); /* reserved */
    writeBe32Stream(pb, 0x0); /* reserved (Layer & Alternate group) */
    /* Volume, only for audio */
    if (track->enc.codec_type == CODEC_TYPE_AUDIO)
    {
        writeBe16Stream(pb, 0x0100);
    }
    else
    {
        writeBe16Stream(pb, 0);
    }
    writeBe16Stream(pb, 0); /* reserved */

    {
        int degrees = track->enc.rotate_degree;
        cdx_uint32 a = 0x00010000;
        cdx_uint32 b = 0;
        cdx_uint32 c = 0;
        cdx_uint32 d = 0x00010000;
        switch (degrees)
        {
            case 0:
                break;
            case 90:
                a = 0;
                b = 0x00010000;
                c = 0xFFFF0000;
                d = 0;
                break;
            case 180:
                a = 0xFFFF0000;
                d = 0xFFFF0000;
                break;
            case 270:
                a = 0;
                b = 0xFFFF0000;
                c = 0x00010000;
                d = 0;
                break;
            default:
                loge("Should never reach this unknown rotation");
                break;
        }

        writeBe32Stream(pb, a);           // a
        writeBe32Stream(pb, b);           // b
        writeBe32Stream(pb, 0);           // u
        writeBe32Stream(pb, c);           // c
        writeBe32Stream(pb, d);           // d
        writeBe32Stream(pb, 0);           // v
        writeBe32Stream(pb, 0);           // x
        writeBe32Stream(pb, 0);           // y
        writeBe32Stream(pb, 0x40000000);  // w
    }

    /* Track width and height, for visual only */
    if (track->enc.codec_type == CODEC_TYPE_VIDEO)
    {
        writeBe32Stream(pb, track->enc.width*0x10000);
        writeBe32Stream(pb, track->enc.height*0x10000);
    }
    else {
        writeBe32Stream(pb, 0);
        writeBe32Stream(pb, 0);
    }
    return tkhd_tag_size;
}

static cdx_uint32 trakGetMdiaTagSize(MOVTrack *track)
{
    cdx_uint32 mdia_tag_size = 8;
    mdia_tag_size += mdiaGetMdhdTagSize();
    mdia_tag_size += mdiaGetHdlrTagSize(track);
    mdia_tag_size += mdiaGetMinfTagSize(track);
    return mdia_tag_size;
}
static cdx_int32 trakWriteMdiaTag(ByteIOContext *pb, MOVTrack *track)
{
    cdx_uint32 mdia_tag_size = trakGetMdiaTagSize(track);

    writeBe32Stream(pb, mdia_tag_size); /* size */
    writeTagStream(pb, "mdia");
    mdiaWriteMdhdTag(pb, track);
    mdiaWriteHdlrTag(pb, track);
    mdiaWriteMinfTag(pb, track);

    return mdia_tag_size;
}
/********************************* Write Trak Level Info *******************************/

/********************************* Write Moov Level Info *******************************/
static void writeLatitude(ByteIOContext *pb, int degreex10000)
{
    int is_negative = (degreex10000 < 0);
    char sign = is_negative? '-': '+';

    // Handle the whole part
    char str[9];
    int wholePart = degreex10000 / 10000;
    if (wholePart == 0)
    {
        snprintf(str, 5, "%c%.2d.", sign, wholePart);
    }
    else
    {
        snprintf(str, 5, "%+.2d.", wholePart);
    }

    // Handle the fractional part
    int fractional_pat = degreex10000 - (wholePart * 10000);
    if (fractional_pat < 0)
    {
        fractional_pat = -fractional_pat;
    }
    snprintf(&str[4], 5, "%.4d", fractional_pat);

    // Do not write the null terminator
    writeBufferStream(pb, (cdx_int8 *)str, 8);
}

// Written in +/- DDD.DDDD format
static void writeLongitude(ByteIOContext *pb, int degreex10000)
{
    int is_negative = (degreex10000 < 0);
    char sign = is_negative? '-': '+';

    // Handle the whole part
    char str[10];
    int wholePart = degreex10000 / 10000;
    if (wholePart == 0)
    {
        snprintf(str, 6, "%c%.3d.", sign, wholePart);
    }
    else
    {
        snprintf(str, 6, "%+.3d.", wholePart);
    }

    // Handle the fractional part
    int fractional_pat = degreex10000 - (wholePart * 10000);
    if (fractional_pat < 0)
    {
        fractional_pat = -fractional_pat;
    }
    snprintf(&str[5], 5, "%.4d", fractional_pat);

    // Do not write the null terminator
    writeBufferStream(pb, (cdx_int8 *)str, 9);
}

static cdx_uint32 moovGetUdtaTagSize()
{
    cdx_uint32 udta_tag_size = 4+4+4+4  //(be32_cache, tag_cache, be32_cache, tag_cache)
                +4+8+9+1;      //(0x001215c7, latitude, longitude, 0x2F)
    return udta_tag_size;
}

static cdx_int32 moovWriteUdtaTag(ByteIOContext *pb, MuxMOVContext *mov)
{
    cdx_uint32 udta_tag_size = moovGetUdtaTagSize();

    writeBe32Stream(pb, udta_tag_size); /* size */

    writeTagStream(pb, "udta");

    writeBe32Stream(pb, udta_tag_size-8);
    writeTagStream(pb, "\xA9xyz");

    /*
     * For historical reasons, any user data start
     * with "\0xA9", must be followed by its assoicated
     * language code.
     * 0x0012: text string length
     * 0x15c7: lang (locale) code: en
     */
    writeBe32Stream(pb, 0x001215c7);

    writeLatitude(pb, mov->mov_latitudex);
    writeLongitude(pb, mov->mov_longitudex);
    writeByteStream(pb, 0x2F);

    return udta_tag_size;
}

static cdx_uint32 moovGetMvhdTagSize()
{
    return 108;
}

static cdx_int32 moovWriteMvhdTag(ByteIOContext *pb, MuxMOVContext *mov)
{
    cdx_int32 max_track_id = 1,i;

    for (i = 0; i < MAX_STREAMS_IN_MP4_FILE/*mov->nb_streams*/; i++)
    {
        if (mov->tracks[i])
        {
            if (max_track_id < mov->tracks[i]->track_id)
            {
                max_track_id = mov->tracks[i]->track_id;
            }
        }
    }
    cdx_uint32 mvhd_tag_size = moovGetMvhdTagSize();
    writeBe32Stream(pb, mvhd_tag_size); /* size */
    writeTagStream(pb, "mvhd");
    writeByteStream(pb, 0);
    writeBe24Stream(pb, 0); /* flags */

    writeBe32Stream(pb, mov->create_time); /* creation time */
    writeBe32Stream(pb, mov->create_time); /* modification time */
    writeBe32Stream(pb, mov->mov_timescale); /* timescale */
    writeBe32Stream(pb, mov->tracks[0]->track_duration); /* duration of longest track */

    writeBe32Stream(pb, 0x00010000); /* reserved (preferred rate) 1.0 = normal */
    writeBe16Stream(pb, 0x0100); /* reserved (preferred volume) 1.0 = normal */
    writeBe16Stream(pb, 0); /* reserved */
    writeBe32Stream(pb, 0); /* reserved */
    writeBe32Stream(pb, 0); /* reserved */

    /* Matrix structure */
    writeBe32Stream(pb, 0x00010000); /* reserved */
    writeBe32Stream(pb, 0x0); /* reserved */
    writeBe32Stream(pb, 0x0); /* reserved */
    writeBe32Stream(pb, 0x0); /* reserved */
    writeBe32Stream(pb, 0x00010000); /* reserved */
    writeBe32Stream(pb, 0x0); /* reserved */
    writeBe32Stream(pb, 0x0); /* reserved */
    writeBe32Stream(pb, 0x0); /* reserved */
    writeBe32Stream(pb, 0x40000000); /* reserved */

    writeBe32Stream(pb, 0); /* reserved (preview time) */
    writeBe32Stream(pb, 0); /* reserved (preview duration) */
    writeBe32Stream(pb, 0); /* reserved (poster time) */
    writeBe32Stream(pb, 0); /* reserved (selection time) */
    writeBe32Stream(pb, 0); /* reserved (selection duration) */
    writeBe32Stream(pb, 0); /* reserved (current time) */
    writeBe32Stream(pb, max_track_id + 1); /* Next track id */
    return mvhd_tag_size;
}

static cdx_uint32 moovGetTrakTagSize(MOVTrack *track)
{
    cdx_uint32 trak_tag_size = 8;
    trak_tag_size += trakGetTkhdTagSize();
    trak_tag_size += trakGetMdiaTagSize(track);
    return trak_tag_size;
}

static cdx_int32 moovWriteTrakTag(ByteIOContext *pb, MOVTrack *track)
{
    cdx_uint32 trak_tag_size = moovGetTrakTagSize(track);

    writeBe32Stream(pb, trak_tag_size); /* size */
    writeTagStream(pb, "trak");
    trakWriteTkhdTag(pb, track);

    trakWriteMdiaTag(pb, track);

    return trak_tag_size;
}
/********************************* Write Moov Level Info *******************************/

/********************************* Write File Level Info *******************************/
static cdx_int32 fileWriteFtypTag(ByteIOContext *pb, Mp4MuxContext *s)
{
    cdx_int32 minor = 0x200;
    cdx_int32 ret;

    writeBe32Stream(pb, 28); /* size */
    writeTagStream(pb, "ftyp");
    writeTagStream(pb, "isom");
    writeBe32Stream(pb, minor);

    writeTagStream(pb, "isom");
    writeTagStream(pb, "iso2");

    ret = writeTagStream(pb, "mp41");

    return ret;//updateSize(pb, pos);
}

static cdx_int32 fileWriteFreeTag(ByteIOContext *pb, cdx_int32 size)
{
    writeBe32Stream(pb, size);
    writeTagStream(pb, "free");
    return size;
}

static cdx_int32 fileWriteMdatTag(ByteIOContext *pb, MuxMOVContext *mov)
{
#if WRITE_FREE_TAG
    mov->mdat_pos = MOV_HEADER_RESERVE_SIZE; //url_ftell(pb);
#else
    mov->mdat_pos = 28;
#endif
    mov->mdat_start_pos = mov->mdat_pos + 8;

#if FS_WRITER
    pb->fsSeek(pb, mov->mdat_pos, SEEK_SET);
#else
    pb->cdxSeek(pb, mov->mdat_pos, SEEK_SET);
#endif

    writeBe32Stream(pb, 0); /* size placeholder */
    writeTagStream(pb, "mdat");
    return 0;
}

static cdx_uint32 fileGetMoovTagSize(MuxMOVContext *mov)
{
    cdx_int32 i;
    cdx_uint32 size = 0;
    size += 8;  //size, "moov" tag,
    if (mov->mov_geo_available)
    {
        size += moovGetUdtaTagSize();
    }
    size += moovGetMvhdTagSize();
    for (i = 0; i < MAX_STREAMS_IN_MP4_FILE/*mov->nb_streams*/; i++)
    {
        if (mov->tracks[i])
        {
            if (mov->tracks[i]->stsz_total_num> 0)
            {
                size += moovGetTrakTagSize(mov->tracks[i]);
            }
        }
    }
    return size;
}

static cdx_int32 fileWriteMoovTag(ByteIOContext *pb, MuxMOVContext *mov)
{
    cdx_int32 i;
    cdx_int32 moov_size;

    moov_size = fileGetMoovTagSize(mov);
#if WRITE_FREE_TAG
    if (moov_size + 28 + 8 <= MOV_HEADER_RESERVE_SIZE)
    {
#if FS_WRITER
        pb->fsSeek(pb, mov->free_pos, SEEK_SET);
#else
        CdxStreamSeek(pb, mov->free_pos, SEEK_SET);
#endif
    }
#endif
    writeBe32Stream(pb, moov_size); /* size placeholder*/
    writeTagStream(pb, "moov");
    mov->mov_timescale = GLOBAL_TIMESCALE;

    if (mov->mov_geo_available)
    {
        moovWriteUdtaTag(pb, mov);
    }

    for (i = 0; i < MAX_STREAMS_IN_MP4_FILE/*mov->nb_streams*/; i++)
    {
        if (mov->tracks[i])
        {
             mov->tracks[i]->time = mov->create_time;
             mov->tracks[i]->track_id = i+1;
             mov->tracks[i]->mov = mov;
             mov->tracks[i]->stream_type = i;
        }
    }

    moovWriteMvhdTag(pb, mov);
    for (i = 0; i < MAX_STREAMS_IN_MP4_FILE/*mov->nb_streams*/; i++)
    {
        if (mov->tracks[i])
        {
            if (mov->tracks[i]->stsz_total_num> 0)
            {
                 moovWriteTrakTag(pb, mov->tracks[i]);
            }
        }
    }
#if WRITE_FREE_TAG
    if (moov_size + 28 + 8 <= MOV_HEADER_RESERVE_SIZE)
    {
        fileWriteFreeTag(pb, MOV_HEADER_RESERVE_SIZE - moov_size - 28);
    }
#endif
    return moov_size;
}
/********************************* Write File Level Info *******************************/

/********************************** File Operation *************************************/
static cdx_int32 findCodecTag(MOVTrack *track)
{
    cdx_int32 tag = track->enc.codec_tag;

    switch(track->enc.codec_id)
    {
        case MUX_CODEC_ID_H264:
        {
            tag = MOV_MKTAG('a','v','c','1');
            break;
        }
        case MUX_CODEC_ID_MPEG4:
        {
            tag = MOV_MKTAG('m','p','4','v');
            break;
        }
        case MUX_CODEC_ID_AAC:
        {
            tag = MOV_MKTAG('m','p','4','a');
            break;
        }
        case MUX_CODEC_ID_PCM:
        {
            tag = MOV_MKTAG('s','o','w','t');
            break;
        }
        case MUX_CODEC_ID_ADPCM:
        {
            tag = MOV_MKTAG('m','s',0x00,0x11);
            break;
        }
        case MUX_CODEC_ID_MJPEG:  /* gushiming compressed source */
        {
            tag = MOV_MKTAG('m','j','p','a');  /* Motion-JPEG (format A) */
            break;
        }
        default:
        {
            break;
        }
    }

    return tag;
}

static cdx_int32 setPlayTimeLength(MuxMOVContext* mov)
{
    int i;
    for (i = 0; i < MAX_STREAMS_IN_MP4_FILE; i++)
    {
        MOVTrack *trk = mov->tracks[i];
        if (trk == NULL)
        {
            continue;
        }
        trk->track_duration = mov->play_time_length;
        trk->average_duration = trk->track_duration / trk->stts_total_num;
    }
}

// Until muxer working, "ftyp" and "free" are write to muxed stream file.
cdx_int32 Mp4WriteHeader(Mp4MuxContext* s)
{
#if FS_WRITER
    ByteIOContext *pb = s->fs_writer_info.mp_fs_writer;
#else
    ByteIOContext *pb = s->stream_writer;
#endif

    MuxMOVContext *mov = (MuxMOVContext*)s->priv_data;
    cdx_int32 i, ret;

    ret = fileWriteFtypTag(pb,s);
    if (ret < 0)
    {
        s->is_sdcard_disappear = 1;
        loge("sdcard may be disappeared!");
        return -1;
    }


    for (i = 0; i < MAX_STREAMS_IN_MP4_FILE; i++)
    {
        if (mov->tracks[i])
        {
             MOVTrack *track= mov->tracks[i];
             track->tag = findCodecTag(track);
        }
    }

#if WRITE_FREE_TAG
    fileWriteFreeTag(pb, MOV_HEADER_RESERVE_SIZE - 28);
#endif

    fileWriteMdatTag(pb, mov);

    mov->free_pos = 28;

    return 0;
}

// When encoder output a frame of data, it will be write to muxed stream file as "mdat".
// Besides, moov info will be updataed to cache.
cdx_int32 Mp4WritePacket(Mp4MuxContext *s, CdxMuxerPacketT *pkt)
{
    MuxMOVContext *mov = (MuxMOVContext*)s->priv_data;
#if FS_WRITER
    ByteIOContext *pb = s->fs_writer_info.mp_fs_writer;
#else
    ByteIOContext *pb = s->stream_writer;
#endif
    MOVTrack *trk = NULL;
    cdx_int32 size = pkt->buflen;
    cdx_int8  *pkt_buf = (cdx_int8*)(pkt->buf);
    cdx_uint8 tmp_strm_byte;
    cdx_uint8 tmp_mjpeg_trailer[2];

    if (NULL == mov || NULL == s->stream_writer)
    {
        loge("in param is null\n");
        return -1;
    }

    if (pkt->streamIndex == -1)//last packet
    {
        if (mov->last_stream_index < 0)
        {
            logw("There is no packet before writing trailer!!!!");
            return 0;
        }
        *mov->cache_write_ptr[STSC_ID][mov->last_stream_index]++ = mov->stsc_cnt;
        mov->tracks[mov->last_stream_index]->stsc_total_num++;
        if (mov->tracks[CODEC_TYPE_VIDEO])
        {
            *mov->cache_write_ptr[STTS_ID][pkt->streamIndex]++ =
                pkt->pts - mov->tracks[CODEC_TYPE_VIDEO]->last_pts;
            mov->tracks[CODEC_TYPE_VIDEO]->track_duration +=
                pkt->pts - mov->tracks[CODEC_TYPE_VIDEO]->last_pts;
        }
        if (mov->tracks[CODEC_TYPE_AUDIO])
        {
            mov->tracks[CODEC_TYPE_AUDIO]->track_duration +=
                pkt->pts - mov->tracks[CODEC_TYPE_AUDIO]->last_pts;
        }
        //!!!! add protection
        return 0;
    }

    if (mov->tracks[pkt->streamIndex] == NULL)
    {
        loge("tracks[%d] is NULL\n", pkt->streamIndex);
        return -1;
    }
    trk = mov->tracks[pkt->streamIndex];

    if (trk->enc.codec_id == MUX_CODEC_ID_MJPEG)
    {  /* gushiming compressed source */
        size += 2;
    }

    //writeCallbackPacket(s, pkt);
    if (pkt->streamIndex != mov->last_stream_index)
    {
        *mov->cache_write_ptr[STCO_ID][pkt->streamIndex]++ = mov->mdat_size + mov->mdat_start_pos;
        trk->stco_total_num++;
        mov->stco_cache_size[pkt->streamIndex]++;

        if (mov->last_stream_index >= 0)
        {
            *mov->cache_write_ptr[STSC_ID][mov->last_stream_index]++ = mov->stsc_cnt;
            mov->tracks[mov->last_stream_index]->stsc_total_num++;
            mov->stsc_cache_size[mov->last_stream_index]++;

            if (mov->cache_write_ptr[STSC_ID][mov->last_stream_index] >
                mov->cache_end_ptr[STSC_ID][mov->last_stream_index])
            {
                mov->cache_write_ptr[STSC_ID][mov->last_stream_index] =
                    mov->cache_start_ptr[STSC_ID][mov->last_stream_index];
            }

            if (mov->stsc_cache_size[mov->last_stream_index] >= STSC_CACHE_SIZE)
            {
                cdx_int32 ret;
                int stsc_fd = 0;

                logv("stsc tinypage: %d\n", trk->stsc_tiny_pages);
                if (mov->fd_stsz[0] == 0)
                {
                    logd("(f:%s, l:%d) strm[%d] not create mp4 tmp file, create now!",
                         __FUNCTION__, __LINE__, mov->last_stream_index);
                    //not create mp4 tmp file, create now.
                    if (0!=Mp4CreateTmpFile(mov))
                    {
                        loge("(f:%s, l:%d) fatal error! Mp4CreateTmpFile() fail!",
                            __FUNCTION__, __LINE__);
                        return -1;
                    }
                }
                stsc_fd = mov->fd_stsc[mov->last_stream_index];
                lseek(stsc_fd, (trk->stsc_tiny_pages * MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE), SEEK_SET);
                ret = write(stsc_fd, mov->cache_read_ptr[STSC_ID][mov->last_stream_index],
                            MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE);
                if (ret < 0)
                {
                    return -1;
                }

                trk->stsc_tiny_pages++;
                mov->stsc_cache_size[mov->last_stream_index] -= MOV_CACHE_TINY_PAGE_SIZE;
                mov->cache_read_ptr[STSC_ID][mov->last_stream_index] += MOV_CACHE_TINY_PAGE_SIZE;
                if (mov->cache_read_ptr[STSC_ID][mov->last_stream_index] >
                    mov->cache_end_ptr[STSC_ID][mov->last_stream_index])
                {
                    mov->cache_read_ptr[STSC_ID][mov->last_stream_index] =
                        mov->cache_start_ptr[STSC_ID][mov->last_stream_index];
                }
            }
        }
        mov->stsc_cnt = 0;

        if (mov->cache_write_ptr[STCO_ID][pkt->streamIndex] >
            mov->cache_end_ptr[STCO_ID][pkt->streamIndex])
        {
            mov->cache_write_ptr[STCO_ID][pkt->streamIndex] =
                mov->cache_start_ptr[STCO_ID][pkt->streamIndex];
        }

        if (mov->stco_cache_size[pkt->streamIndex] >= STCO_CACHE_SIZE)
        {
            cdx_int32 ret;
            int stco_fd = 0;
            if (mov->fd_stsz[0] == 0)
            {
                logd("(f:%s, l:%d) strm[%d] not create mp4 tmp file, create now!",
                     __FUNCTION__, __LINE__, pkt->streamIndex);
                //not create mp4 tmp file, create now.
                if (0 != Mp4CreateTmpFile(mov))
                {
                    loge("(f:%s, l:%d) fatal error! Mp4CreateTmpFile() fail!",
                         __FUNCTION__, __LINE__);
                    return -1;
                }
            }
            stco_fd = mov->fd_stco[pkt->streamIndex];
            lseek(stco_fd, (trk->stco_tiny_pages * MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE), SEEK_SET);
            ret = write(stco_fd, mov->cache_read_ptr[STCO_ID][pkt->streamIndex],
                MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE);
            if (ret < 0)
            {
                return -1;
            }

            trk->stco_tiny_pages++;
            mov->stco_cache_size[pkt->streamIndex] -= MOV_CACHE_TINY_PAGE_SIZE;
            mov->cache_read_ptr[STCO_ID][pkt->streamIndex] += MOV_CACHE_TINY_PAGE_SIZE;
            if (mov->cache_read_ptr[STCO_ID][pkt->streamIndex] >
                mov->cache_end_ptr[STCO_ID][pkt->streamIndex])
            {
                mov->cache_read_ptr[STCO_ID][pkt->streamIndex] =
                    mov->cache_start_ptr[STCO_ID][pkt->streamIndex];
            }
        }
    }

    mov->last_stream_index = pkt->streamIndex;
    //uncompressed audio need not write stsz field, because sample size is fixed.
    if (trk->enc.codec_id == MUX_CODEC_ID_PCM)
    {
        trk->stsz_total_num+=size/(trk->enc.channels*(trk->enc.bits_per_sample >> 3)); // stsz size
        mov->stsc_cnt+=size/(trk->enc.channels*(trk->enc.bits_per_sample >> 3));
    }
    else
    {
        *mov->cache_write_ptr[STSZ_ID][pkt->streamIndex]++ = size;
        trk->stsz_total_num++; //av stsz size
        mov->stsz_cache_size[pkt->streamIndex]++;
        mov->stsc_cnt++;
    }

    if (mov->cache_write_ptr[STSZ_ID][pkt->streamIndex] >
        mov->cache_end_ptr[STSZ_ID][pkt->streamIndex])
    {
        mov->cache_write_ptr[STSZ_ID][pkt->streamIndex] =
            mov->cache_start_ptr[STSZ_ID][pkt->streamIndex];
    }

    if (mov->stsz_cache_size[pkt->streamIndex] >= STSZ_CACHE_SIZE)
    {
        cdx_int32 ret;
        int stsz_fd = 0;
        if (mov->fd_stsz[0] == 0)
        {
            logd("(f:%s, l:%d) strm[%d] not create mp4 tmp file, create now!",
                 __FUNCTION__, __LINE__, pkt->streamIndex);
            //not create mp4 tmp file, create now.
            if (0 != Mp4CreateTmpFile(mov))
            {
                loge("(f:%s, l:%d) fatal error! Mp4CreateTmpFile() fail!", __FUNCTION__, __LINE__);
                return -1;
            }
        }
        stsz_fd = mov->fd_stsz[pkt->streamIndex];
        lseek(stsz_fd, (trk->stsz_tiny_pages * MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE), SEEK_SET);
        ret = write(stsz_fd, mov->cache_read_ptr[STSZ_ID][pkt->streamIndex],
                    MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE);
        if (ret < 0)
        {
            return -1;
        }

        trk->stsz_tiny_pages++;
        mov->stsz_cache_size[pkt->streamIndex] -= MOV_CACHE_TINY_PAGE_SIZE;
        mov->cache_read_ptr[STSZ_ID][pkt->streamIndex] += MOV_CACHE_TINY_PAGE_SIZE;
        if (mov->cache_read_ptr[STSZ_ID][pkt->streamIndex] >
            mov->cache_end_ptr[STSZ_ID][pkt->streamIndex])
        {
            mov->cache_read_ptr[STSZ_ID][pkt->streamIndex] =
                mov->cache_start_ptr[STSZ_ID][pkt->streamIndex];
        }
    }

    if (pkt->streamIndex == 0)//video
    {
        //*mov->cache_write_ptr[STTS_ID][pkt->streamIndex]++ = pkt->duration;
        if (trk->last_pts > 0)
        {
            *mov->cache_write_ptr[STTS_ID][pkt->streamIndex]++ = pkt->pts - trk->last_pts;
        }

        trk->stts_total_num++;
        mov->stts_cache_size[pkt->streamIndex]++;

        if (mov->cache_write_ptr[STTS_ID][pkt->streamIndex] >
            mov->cache_end_ptr[STTS_ID][pkt->streamIndex])
        {
            mov->cache_write_ptr[STTS_ID][pkt->streamIndex] =
                mov->cache_start_ptr[STTS_ID][pkt->streamIndex];
        }

        if (mov->stts_cache_size[pkt->streamIndex] >= STTS_CACHE_SIZE)
        {
            cdx_int32 ret;
            int stts_fd = 0;
            if (mov->fd_stsz[0] == 0)
            {
                logd("(f:%s, l:%d) strm[%d] not create mp4 tmp file, create now!",
                     __FUNCTION__, __LINE__, pkt->streamIndex);

                //not create mp4 tmp file, create now.
                if (0 != Mp4CreateTmpFile(mov))
                {
                    loge("(f:%s, l:%d) fatal error! Mp4CreateTmpFile() fail!",
                         __FUNCTION__, __LINE__);
                    return -1;
                }
            }
            stts_fd = mov->fd_stts[pkt->streamIndex];
            lseek(stts_fd, (trk->stts_tiny_pages * MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE), SEEK_SET);
            ret = write(stts_fd, mov->cache_read_ptr[STTS_ID][pkt->streamIndex],
                        MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE);
            if (ret < 0)
            {
                return -1;
            }

            trk->stts_tiny_pages++;
            mov->stts_cache_size[pkt->streamIndex] -= MOV_CACHE_TINY_PAGE_SIZE;
            mov->cache_read_ptr[STTS_ID][pkt->streamIndex] += MOV_CACHE_TINY_PAGE_SIZE;
            if (mov->cache_read_ptr[STTS_ID][pkt->streamIndex] >
                mov->cache_end_ptr[STTS_ID][pkt->streamIndex])
            {
                mov->cache_read_ptr[STTS_ID][pkt->streamIndex] =
                    mov->cache_start_ptr[STTS_ID][pkt->streamIndex];
            }
        }
        tmp_strm_byte = pkt_buf[4];
        if (trk->enc.codec_id == MUX_CODEC_ID_H264)
        {
            if ((tmp_strm_byte&0x1f) == 5)
            {
                mov->cache_keyframe_ptr[trk->key_frame_num] = trk->stts_total_num;
                trk->key_frame_num++;
            }

        }
        else if (trk->enc.codec_id == MUX_CODEC_ID_MPEG4)
        {
            if ((tmp_strm_byte>>6) == 0)
            {
                mov->cache_keyframe_ptr[trk->key_frame_num] = trk->stts_total_num;
                trk->key_frame_num++;
            }
        }
        else if (trk->enc.codec_id == MUX_CODEC_ID_MJPEG)
        {  /* gushiming compressed source */
            if (mov->keyframe_num == 0)
            {
                mov->cache_keyframe_ptr[trk->key_frame_num] = trk->stts_total_num;
                trk->key_frame_num++;
            }
            mov->keyframe_num ++;
            if (mov->keyframe_num >= trk->enc.frame_rate)
            {
                mov->keyframe_num = 0;
            }
        }
        else
        {
            logv("err codec type!\n");
            return -1;
        }

        if (mov->tracks[CODEC_TYPE_VIDEO]->enc.codec_id == MUX_CODEC_ID_H264)
        {
            int pkt_size = movBswap32(size - 4);
            memcpy(pkt_buf, &pkt_size, 4);
        }
    }

    mov->mdat_size += size;

    if (pkt->buflen)
    {
        if (writeBufferStream(pb, pkt_buf, pkt->buflen) < 0)
        {
            loge("(f:%s, l:%d) !", __FUNCTION__, __LINE__);
            s->is_sdcard_disappear = 1;
            return -1;
        }
    }

    if (trk->enc.codec_id == MUX_CODEC_ID_MJPEG)
    {  /* gushiming compressed source */
        tmp_mjpeg_trailer[0] = 0xff;
        tmp_mjpeg_trailer[1] = 0xd9;
        if (writeBufferStream(pb, (cdx_int8*)tmp_mjpeg_trailer, 2) < 0)
        {
            loge("(f:%s, l:%d) sdcard may be disappear", __FUNCTION__, __LINE__);
            s->is_sdcard_disappear = 1;
            return -1;
        }
    }

    //trk->track_duration += 1.0 / trk->enc.frame_rate * trk->track_timescale;
    if (trk->last_pts > 0)
    {
        trk->track_duration += pkt->pts - trk->last_pts;
    }
    trk->last_pts = pkt->pts;

    return 0;
}

// When inputed stream is end, moov info saved in cache is writed to muxed stream file.
cdx_int32 Mp4WriteTrailer(Mp4MuxContext *s)
{
    MuxMOVContext *mov = (MuxMOVContext*)s->priv_data;

#if FS_WRITER
    ByteIOContext *pb = s->fs_writer_info.mp_fs_writer;
#else
    ByteIOContext *pb = s->stream_writer;
#endif

    offset_t filesize = 0;//moov_pos = url_ftell(pb);
    CdxMuxerPacketT pkt;

    pkt.buf = NULL;
    pkt.buflen = 0;
    pkt.streamIndex = -1;
    pkt.pts = mov->tracks[0]->last_pts + 33;

    Mp4WritePacket(s, &pkt);//update last packet

    if (mov->play_time_length)
    {
        setPlayTimeLength(mov);
    }

    fileWriteMoovTag(pb, mov);

#if FS_WRITER
    pb->fsSeek(pb, mov->mdat_pos, SEEK_SET);
    if (writeBe32Stream(pb, mov->mdat_size + 8) < 0)
    {
        loge("Mp4WriteTrailer failed\n");
        return -1;
    }
    pb->fsFlush(pb);
#else
    pb->cdxSeek(pb, mov->mdat_pos, SEEK_SET);
    if (writeBe32Stream(pb, mov->mdat_size + 8) < 0)
    {
        loge("Mp4WriteTrailer failed\n");
        return -1;
    }
#endif

    return filesize;
}

static void makeMovTmpFullFilePath(char *dir, char *p_path, char *filetype,
                                   cdx_int32 n_stream_index, void* n_filed_id)
{
    // /mnt/extsd/mov_stsz_muxer_0[0xaaaabbbb].buf
    if (dir[0] == 0)
    {
        strcpy(dir, MOV_TMPFILE_DIR);
    }
    strcpy(p_path, dir);
    if (!strcmp(filetype, "stsz"))
    {
        sprintf(p_path + strlen(p_path), "mov_stsz_muxer_%d[0x%p]"MOV_TMPFILE_EXTEND_NAME ,
                n_stream_index, n_filed_id);
    }
    else if (!strcmp(filetype, "stts"))
    {
        sprintf(p_path + strlen(p_path), "mov_stts_muxer_%d[0x%p]"MOV_TMPFILE_EXTEND_NAME ,
                n_stream_index, n_filed_id);
    }
    else if (!strcmp(filetype, "stco"))
    {
        sprintf(p_path + strlen(p_path), "mov_stco_muxer_%d[0x%p]"MOV_TMPFILE_EXTEND_NAME ,
                n_stream_index, n_filed_id);
    }
    else if (!strcmp(filetype, "stsc"))
    {
        sprintf(p_path + strlen(p_path), "mov_stsc_muxer_%d[0x%p]"MOV_TMPFILE_EXTEND_NAME ,
                n_stream_index, n_filed_id);
    }
    else
    {
        loge("(f:%s, l:%d) fatal error, mov tmp filetype[%s] is not support!",
             __FUNCTION__, __LINE__, filetype);
    }
    return;
}

static int openMovTmpFile(char *p_path)
{
    int fd_tmp = 0;
    if (p_path == NULL)
    {
        loge("in openMovTmpFile() p_path is NULL");
        return -1;
    }
    fd_tmp = open(p_path, O_RDWR | O_CREAT, 666);
    if (fd_tmp < 0)
    {
        loge("(f:%s, l:%d) fatal error! create mov tmp file fail.", __FUNCTION__, __LINE__);
        return -1;
    }
    return fd_tmp;
}

// If the inputed stream is too long, when cache where tag info saved is not enough,
// tmp file would be created to save tag info where in ahead of cache.
cdx_int32 Mp4CreateTmpFile(MuxMOVContext *mov)
{
    cdx_int32 i;

    mkdir(MOV_TMPFILE_DIR, 0777);
    for (i = 0; i < 2; i++)
    {
        if (mov->fd_stsz[i] == 0)
        {
            makeMovTmpFullFilePath(mov->mov_tmpfile_path, mov->file_path_stsz[i],
                                   "stsz", i, (void*)mov);
            mov->fd_stsz[i] = openMovTmpFile(mov->file_path_stsz[i]);
            if (mov->fd_stsz[i] < 0)
            {
                loge("error = %d\n",-errno);
                return -1;
            }
        }

        if (mov->fd_stts[i] == 0)
        {
            makeMovTmpFullFilePath(mov->mov_tmpfile_path, mov->file_path_stts[i],
                                   "stts", i, (void*)mov);
            mov->fd_stts[i] = openMovTmpFile(mov->file_path_stts[i]);
            if (mov->fd_stts[i] < 0)
            {
                logw("can not open %d mov_stts_muxer.buf\n",i);
                return -1;
            }
        }

        if (mov->fd_stco[i] == 0)
        {
            makeMovTmpFullFilePath(mov->mov_tmpfile_path, mov->file_path_stco[i],
                                   "stco", i, (void*)mov);
            mov->fd_stco[i] = openMovTmpFile(mov->file_path_stco[i]);
            if (mov->fd_stco[i] < 0)
            {
                logw("can not open %d mov_stco_muxer.buf\n",i);
                return -1;
            }
        }

        if (mov->fd_stsc[i] == 0)
        {
            makeMovTmpFullFilePath(mov->mov_tmpfile_path, mov->file_path_stsc[i],
                                   "stsc", i, (void*)mov);
            mov->fd_stsc[i] = openMovTmpFile(mov->file_path_stsc[i]);
            if (mov->fd_stsc[i] < 0)
            {
                logw("can not open %d mov_stsc_muxer.buf\n",i);
                return -1;
            }
        }
    }
    return 0;
}
/********************************** File Operation *************************************/
