/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxTsMuxer.h
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

#ifndef CDX_TS_MUXER_H
#define CDX_TS_MUXER_H

#include <cdx_log.h>
#include "CdxMuxer.h"
#include "CdxMuxerBaseDef.h"
#include "CdxFsWriter.h"

#define RSHIFT(a,b) ((a) > 0 ? ((a) + ((1<<(b))>>1))>>(b) : ((a) + ((1<<(b))>>1)-1)>>(b))
/* assume b>0 */
#define ROUNDED_DIV(a,b) (((a)>0 ? (a) + ((b)>>1) : (a) - ((b)>>1))/(b))
#define FFABS(a) ((a) >= 0 ? (a) : (-(a)))
#define FFSIGN(a) ((a) > 0 ? 1 : -1)
#define FFMAX(a,b) ((a) > (b) ? (a) : (b))
#define FFMAX3(a,b,c) (FFMAX(FFMAX(a,b),c))
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#define FFMIN3(a,b,c) FFMIN(FFMIN(a,b),c)


#define MAX_STREAMS_IN_TS_FILE 2
#define MAX_SERVERVICES_IN_FILE 1

#define ADTS_HEADER_SIZE 7

#define TS_FEC_PACKET_SIZE 204
#define TS_DVHS_PACKET_SIZE 192
#define TS_PACKET_SIZE 188
#define TS_MAX_PACKET_SIZE 204

#define NB_PID_MAX 8192
#define MAX_SECTION_SIZE 4096

#define AV_TIME_BASE            1000000

#define MKTAG(a,b,c,d) (a | (b << 8) | (c << 16) | (d << 24))

#define MAX_PCE_SIZE  304
#define MAX_TS_STREAMS 10

// TsPid
#define PAT_PID                     0x0000
#define SDT_PID                     0x0011
#define DEFAULT_PMT_START_PID       0x1000
#define DEFAULT_START_PID           0x0100

// TsTid
#define PAT_TID 0x00
#define PMT_TID 0x02
#define SDT_TID 0x42

typedef enum {
    STREAM_TYPE_VIDEO_MPEG1         = 0x01,
    STREAM_TYPE_VIDEO_MPEG2         = 0x02,
    STREAM_TYPE_AUDIO_MPEG1         = 0x03,
    STREAM_TYPE_AUDIO_MPEG2         = 0x04,
    STREAM_TYPE_PRIVATE_SECTION     = 0x05,
    STREAM_TYPE_PRIVATE_DATA        = 0x06,
    STREAM_TYPE_AUDIO_AAC           = 0x0f,
    STREAM_TYPE_VIDEO_MPEG4         = 0x10,
    STREAM_TYPE_VIDEO_H264          = 0x1b,
    STREAM_TYPE_VIDEO_VC1           = 0xea,
    STREAM_TYPE_VIDEO_DIRAC         = 0xd1,

    STREAM_TYPE_AUDIO_AC3           = 0x81,
    STREAM_TYPE_AUDIO_DTS           = 0x8a,
    STREAM_TYPE_AUDIO_LPCM          = 0x83
}TsStreamType;

typedef enum {
    AAC_PROFILE_MP  = 0,
    AAC_PROFILE_LC  = 1,
    AAC_PROFILE_SSR = 2,
    AAC_PROFILE_
} AAC_PROFILE_TYPE;

typedef enum {
    OUTPUT_TS_FILE,
    OUTPUT_M3U8_FILE,
    OUTPUT_CALLBACK_BUFFER,
}OUTPUT_BUFFER_MODE;

#define PKT_FLAG_KEY 1

#define DEFAULT_ONID            0x0001
#define DEFAULT_TSID            0x0000
#define DEFAULT_SID             0x0001

/* we retransmit the SI info at this rate */
#define SDT_RETRANS_TIME 500
#define PAT_RETRANS_TIME 100
#define PCR_RETRANS_TIME 20

#define DEFAULT_PES_HEADER_FREQ 16
#define DEFAULT_PES_PAYLOAD_SIZE ((DEFAULT_PES_HEADER_FREQ - 1) * 184 + 170)

typedef struct MpegTSSection {
    int pid;
    int cc;
    int (*write_packet)(struct MpegTSSection *s, cdx_uint8 *packet);
    void *opaque;
}MpegTSSection;

typedef struct MpegTSService {
    MpegTSSection pmt; /* MPEG2 pmt table context */
    int sid;           /* service ID */
    char *name;
    char *provider_name;
    int pcr_pid;
    int pcr_packet_count;
    int pcr_packet_period;
} MpegTSService;

typedef struct MpegTSWriteStream
{
    MpegTSService *service;
    int pid; /* stream associated pid */
    int cc;
    int payload_index;
    int first_pts_check; ///< first pts check needed
    cdx_int64 payload_pts;
    cdx_int64 payload_dts;
    cdx_uint8 payload[DEFAULT_PES_PAYLOAD_SIZE];
} MpegTSWriteStream;

typedef struct TsWriter {
    MpegTSSection           pat; /* MPEG2 pat table */
    MpegTSSection           sdt; /* MPEG2 sdt table context */
    MpegTSService           **services;
    int                     sdt_packet_count;
    int                     sdt_packet_period;
    int                     pat_packet_count;
    int                     pat_packet_period;
    int                     nb_services;
    int                     onid;
    int                     tsid;
    cdx_uint64              cur_pcr;
    cdx_uint8               *ts_cache_start;
    cdx_uint8               *ts_cache_end;
    cdx_uint32              cache_size;
    cdx_uint64              cache_size_total;
    cdx_uint8               *ts_write_ptr;
    cdx_uint8               *ts_read_ptr;
    cdx_uint32              cache_page_num;
} TsWriter;

typedef struct AVCodecContext {

    cdx_int32 bit_rate;
    cdx_int32 width;
    cdx_int32 height;
    CodecType codec_type; /* see CODEC_TYPE_xxx */
    cdx_uint32 codec_tag;
    MuxCodecID codec_id;

    cdx_int32 channels;
    cdx_int32 frame_size;
    cdx_int32 frame_rate;

    cdx_int32 bits_per_sample;
    cdx_int32 sample_rate;

}AVCodecContext;

typedef struct AVStream {
    cdx_int32 index;    /**< stream index in AVFormatContext */
    cdx_int32 id;       /**< format specific stream id */
    AVCodecContext codec; /**< codec context */
    void *priv_data;

    /* internal data used in av_find_stream_info() */
    cdx_int64 first_dts;
    /** encoding: PTS generation when outputing stream */
    //struct AVFrac pts;


    //FIXME move stuff to a flags field?
    /** quality, as it has been removed from AVCodecContext and put in AVVideoFrame
     * MN: dunno if that is the right place for it */
    float quality;
    cdx_int64 start_time;
    cdx_int64 duration;

    cdx_int64 cur_dts;
    void *vos_data;
    cdx_uint32 vos_len;
    int   firstframeflag;
} AVStream;

typedef struct TsPacketHeader
{
    int  stream_type;
    int  size;
    long long pts;
}TsPacketHeader;

typedef struct TsMuxContext {
    CdxMuxerT               muxInfo;
    CdxWriterT              *stream_writer;
    void                    *priv_data;
    CdxFsWriterInfo         fs_writer_info;
    cdx_int32               nb_streams;
    AVStream                *streams[MAX_TS_STREAMS];

    cdx_uint32              max_delay;
    cdx_uint8               *cache_in_ts_stream;
    unsigned char           *pes_buffer;
    int                     pes_bufsize;
    long long               audio_frame_num;
    long long               pat_pmt_counter;
    int                     pat_pmt_flag;
    int                     output_buffer_mode;
    int                     first_video_pts;
    long long               base_video_pts;
    unsigned int            pcr_counter;
    cdx_uint8               is_sdcard_disappear;
} TsMuxContext;

extern int TsWritePacket(TsMuxContext *s, CdxMuxerPacketT *pkt);
extern int TsWriteTrailer(TsMuxContext *s);
extern int TsSectionWritePacket(MpegTSSection *s, cdx_uint8 *packet);
#endif /* CDX_TS_MUXER_H */
