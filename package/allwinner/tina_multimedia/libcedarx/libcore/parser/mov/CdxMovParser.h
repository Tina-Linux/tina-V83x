/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxMovParser.h
 * Description :
 * History :
 *
 */

#ifndef CDX__mov_parser_H
#define CDX__mov_parser_H

#include <stdint.h>
#include <stdio.h>
#include <CdxTypes.h>
#include <CdxParser.h>
#include <CdxStream.h>
#include <CdxMemory.h>
#include <CdxAtomic.h>

#include "CdxVirCache.h"
#include "CdxMovList.h"

#include "Id3Base.h"

#define CDX_U8 cdx_uint8
#define CDX_S8 cdx_int8
#define CDX_U16 cdx_uint16
#define CDX_S16 cdx_int16
#define CDX_U32 cdx_uint32
#define CDX_S32 cdx_int32
#define CDX_U64 cdx_uint64
#define CDX_S64 cdx_int64

#define SAVE_VIDEO 0

#ifndef SEEK_SET        /* pre-ANSI systems may not define this; */
#define SEEK_SET  0        /* if not, assume 0 is correct */
#endif

#ifndef SEEK_CUR        /* pre-ANSI systems may not define this; */
#define SEEK_CUR  1        /* if not, assume 0 is correct */
#endif

#ifndef SEEK_END        /* pre-ANSI systems may not define this; */
#define SEEK_END  2        /* if not, assume 0 is correct */
#endif

#define MAX_CHUNK_BUF_SIZE      (1024 * 256)    //256k
#define AUDIO_INFO_SIZE         (1024*8)        //8k byte
#define SUBTITLE_INFO_SIZE      (1024*8)        //8k byte
#define MAX_LANG_CHAR_SIZE      (32)

//   'tfhd'  Box need
#define CDX_MOV_TFHD_BASE_DATA_OFFSET       0x01
#define CDX_MOV_TFHD_STSD_ID                0x02
#define CDX_MOV_TFHD_DEFAULT_DURATION       0x08
#define CDX_MOV_TFHD_DEFAULT_SIZE           0x10
#define CDX_MOV_TFHD_DEFAULT_FLAGS          0x20
#define CDX_MOV_TFHD_DURATION_IS_EMPTY      0x010000

// 'trun'
#define CDX_MOV_TRUN_DATA_OFFSET            0x01
#define CDX_MOV_TRUN_FIRST_SAMPLE_FLAGS     0x04
#define CDX_MOV_TRUN_SAMPLE_DURATION       0x100
#define CDX_MOV_TRUN_SAMPLE_SIZE           0x200
#define CDX_MOV_TRUN_SAMPLE_FLAGS          0x400
#define CDX_MOV_TRUN_SAMPLE_CTS            0x800

#define CDX_MOV_FRAG_SAMPLE_FLAG_DEGRADATION_PRIORITY_MASK 0x0000ffff
#define CDX_MOV_FRAG_SAMPLE_FLAG_IS_NON_SYNC               0x00010000
#define CDX_MOV_FRAG_SAMPLE_FLAG_PADDING_MASK              0x000e0000
#define CDX_MOV_FRAG_SAMPLE_FLAG_REDUNDANCY_MASK           0x00300000
#define CDX_MOV_FRAG_SAMPLE_FLAG_DEPENDED_MASK             0x00c00000
#define CDX_MOV_FRAG_SAMPLE_FLAG_DEPENDS_MASK              0x03000000

#define CDX_MOV_FRAG_SAMPLE_FLAG_DEPENDS_NO                0x02000000
#define CDX_MOV_FRAG_SAMPLE_FLAG_DEPENDS_YES               0x01000000

struct CdxMovParser
{
    CdxParserT     parserinfo;
    CdxStreamT *stream;
    CdxPacketT  packet;                  // for prefetch in prefetched status
    int         exitFlag;
    int            mErrno;
    int         mStatus;
    cdx_atomic_t ref;
    int          bDashSegment;          // 1: DASH have serveral segments mp4 file ( HDWorld)
    int          bSmsSegment;           // smooth streaming.

    void        *privData;              //pointer to the MOVContext

    CDX_S8        hasVideo;         //video flag, =0:no video bitstream; >0:video bitstream count
    CDX_S8        hasAudio;         //audio flag, =0:no audio bitstream; >0:audio bitstream count
    CDX_S8        hasSubTitle;      //lyric flag, =0:no lyric bitstream; >0:lyric bitstream count

    CDX_U32       totalTime;              //total time (ms)
    CDX_S32       hasIdx;                 //if the media file has index to support ff/rr
    CDX_U32       nPreFRSpeed;            //previous ff/rr speed, for dynamic adjust
    CDX_U32       nFRSpeed;               //fast forward and fast backward speed, multiple of
                                          //normal speed
    CDX_U32       nFRPicShowTime;         //picture show time under fast forward and backward
    CDX_U32       nFRPicCnt;              //picture count under ff/rr, for check if need delay

    CDX_U32       nVidPtsOffset;          //video time offset
    CDX_U32       nAudPtsOffset;          //audio time offset
    CDX_U32       nSubPtsOffset;          //subtitle time offset

    CDX_S32     SubStreamSyncFlg;
    CDX_U8        *pktData;                  // subtitle extra_data

    CDX_U32        chkType;                   //subtitle chunk type, it is unused

    void        *pVideoExtraData;       //vop header pointer
    CDX_S32     nVideoExtraDataLen;      //vop header left size
    CDX_S32       keyItl;
    CDX_S32       bDiscardAud;
#ifndef ONLY_ENABLE_AUDIO
    VideoStreamInfo             vFormat;
    SubtitleStreamInfo          tFormat;    //subtitle format
#endif
    VirCacheContext                 *vc;

    #if SAVE_VIDEO
    FILE *fp;
    #endif
};

#define MKTAG(a,b,c,d) (a | (b << 8) | (c << 16) | (d << 24))
#define MKBETAG(a,b,c,d) (d | (c << 8) | (b << 16) | (a << 24))
#define MOV_MAX_STREAMS 10

enum CodecType
{
    CODEC_TYPE_UNKNOWN = -1,
    CODEC_TYPE_VIDEO,
    CODEC_TYPE_AUDIO,
    CODEC_TYPE_SUBTITLE,
    CODEC_TYPE_DATA
};

typedef enum MovTrackType
{
  MOV_TRACK_TYPE_NONE     = 0x0,
  MOV_TRACK_TYPE_VIDEO    = 0x1,
  MOV_TRACK_TYPE_AUDIO    = 0x2,
  MOV_TRACK_TYPE_COMPLEX  = 0x3,
  MOV_TRACK_TYPE_LOGO     = 0x10,
  MOV_TRACK_TYPE_SUBTITLE = 0x11,
  MOV_TRACK_TYPE_CONTROL  = 0x20,
} MovTrackType;

enum ReadRetCode
{
    READ_FREE = 0,
    READ_FINISH = 1
};

#define  MAXENTRY 720000
typedef CDX_U32 MOV_OFFSET;

typedef struct MOV_atom_t
{
    CDX_U32            type;
    CDX_U32            size;                /* total size (excluding the size and type fields) */
    CDX_U32            offset;
} MOV_atom_t;

typedef struct MOV_CHUNK
{
    CDX_U32            type;
    CDX_U32            length;
    CDX_U64            offset;
    CDX_S32            index;
    /* Add by khan , start
     * 2017-3-10
     * for some raw audio data, audio should be fed to decoder frame by frame, so we
     * should demux frame by frame, because these audio frame has no sync flag ( AAC-LC ADTS)
     * If one audio chunk contains such serials frames, try to get frames in a loop to the end.
     * Here is the controll varibles:
     *     no_completed_read  ----  whether we loop to the data end.
     *     cycle_num                ----  current loop counts.
     *     cycle_to                   ----  try loop counts.
     *     audio_segsz             ----  size of loop audio frame once.
     */
    CDX_S32            no_completed_read;
    CDX_S32            cycle_num;
    CDX_S32            cycle_to;
    CDX_S32            audio_segsz;
    // Add by khan , end
} MOV_CHUNK;

typedef struct AVCodecContext
{
    enum CodecType      codecType;            /* see CODEC_TYPE_xxx, video, audio, or subtitle */
    CDX_S32              max_bitrate;
    CDX_S32              avg_bitrate;
    CDX_S32              width, height;       // picture width and height
    CDX_S32              sampleRate;          // samples per second
    CDX_S32              channels;
    CDX_U32              codecTag;      // the tag of the codec in the file, is may be
                                        // unused( like: "mp4v", "xvid", ...)
    CDX_U32              bitsPerSample; // the depth of the pixel
    char *extradata;
    CDX_U32           extradataSize;
    //CDX_U32            frame_size;     // unused
} AVCodecContext;

typedef struct _MOVCtts MOVCtts;
struct _MOVCtts
{
    int count;
    int duration;
} ;

//segment index
typedef struct MOVSidx
{
    CDX_U64       current_dts;  // the start pts of this segment
    CDX_U64        offset;
    CDX_U64     duration;
    CDX_U32        size;
} MOVSidx;

typedef struct Mov_Index_Context_Extend
{
    CDX_U32            chunk_idx;
    CDX_U32            chunk_sample_idx;    //sample index in one chunk
    CDX_U32            sample_idx;            //stream acc sample index
    CDX_U32            chunk_sample_size;  //sample size in one chunk accumulation
    CDX_U32            stsc_idx;            //stsc offset
    CDX_U32            stsz_idx;
    CDX_U32            stco_idx;
    CDX_U32            stts_idx;
    CDX_S32            stss_idx;            //keyframe idx

    CDX_U32            stts_samples_acc;
    CDX_U32            stts_sample_duration;
    CDX_U64            stts_duration_acc;
    CDX_U64            current_dts;
    CDX_U64            current_duration;

    CDX_U32            stsc_first;
    CDX_U32            stsc_samples;
    CDX_U32            stsc_end;
} Mov_Index_Context_Extend;

/*******************************************************/
//*  all of the streams(audio, video, subtitle)
//*  are presented with this structure
/******************************************************/
typedef struct MOVStreamContext
{
    CDX_S32            ffindex;
    CDX_S32            next_chunk;

    CDX_U32            stsc_size;   // entries in 'stsc' atom
    CDX_U32            stsz_size;
    CDX_U32            stco_size;
    CDX_U32            stss_size;
    CDX_U32            real_stss_size;
    CDX_U32            stts_size;
    MOV_OFFSET        stsc_offset;
    MOV_OFFSET        stsz_offset;
    MOV_OFFSET        stco_offset;
    MOV_OFFSET        stss_offset;
    MOV_OFFSET        stts_offset;

    int             co64;         // the flag of co64, 0:stco; 1:co64
    CDX_S32            ctts_size;     // entries in 'ctts' atom, the atom is always not in the file
    MOVCtts*            ctts_data;
    MOV_OFFSET      ctts_offset;

    int             ctts_index;
    int             ctts_sample;

    // for example, the No.1 stream is video stream, No.2 and No.3 stream are audio streams
    // so the No.2 stream stsd_type is 2, and stream_index is 0
    // so the No.3 stream stsd_type is 2, and stream_index is 1
    int             stsd_type;  //0:unkown; 1:video; 2:audio; 3:subtitle
    int             stream_index;  // it is the index of all the video,audio,subtitle streams

    int             unsurpoort; // this type is not supported, for svq3

    //If all the samples are the same size, this field contains
    //that size value. If this field is set to 0, then the samples
    //have different sizes, and those sizes are
    //stored in the sample size table.
    CDX_S32            sample_size;        //stsz_sample_size

    CDX_S32         sample_duration;   //the first sample duration of stts atom,
                                       //for calculate framerate in video stream

    CDX_S32            time_scale;
    CDX_S32            time_rate;
    CDX_S32            current_sample;
    CDX_U32            bytes_per_frame;
    CDX_U32            samples_per_frame;
    CDX_S32            index;                /**< stream index in Mov_File_Context */
    CDX_S32            id;                    /**< format specific stream id */
    CDX_U8          language[MAX_LANG_CHAR_SIZE];
    AVCodecContext    codec;                /**< codec context */
    CDX_S32            width, height;      /**< pixel width and height, is not the picture
                                            width and height (maybe they are the same size)*/
    CDX_S64            duration;            // may be 32bit not enought
    CDX_S32         time_offset;        // time_offset
    CDX_S32            totaltime;
    CDX_S32         basetime;           //basetime
    CDX_S32            nb_frames;            //< number of frames in this stream if known or 0

    int64_t            track_end; // used for dts generation in fragmented movie files

    int             dts_shift;  //***< dts shift when ctts is negtive

    //**** for video, only have eCodecFormat
    //**** audio, eCodecFormat and SubFormat
    //**** subtitle,  eCodecFormat is the comingType, the eSubCodecFormat is the eDataEncodeType
    int             eCodecFormat;
    int             eSubCodecFormat;

    // the variables below used for read_sample
    Mov_Index_Context_Extend    mov_idx_curr;
    CDX_U16                        read_va_end;      // end flag of this stream
    CDX_S32                        audio_sample_size;
    CDX_S32                        chunk_sample_idx_assgin_by_seek;

    // for timedText subtitle, the flag of transport extradata when first prefetch
    int               SubStreamSyncFlg;
    cdx_uint8         rotate[4];    // it is  a string, we must initial it with '\0'

    unsigned int rap_group_count;
    unsigned int rap_seek_count;
    int          *rap_seek;
} MOVStreamContext;

typedef struct Mov_Index_Context
{
    CDX_U32            chunk_idx;
    CDX_U32            sample_idx;            //acc sample index
    CDX_U32            stsc_idx;
    CDX_U32            stsz_idx;
    CDX_U32            stco_idx;
    CDX_U32            stts_idx;
} Mov_Index_Context;

typedef struct MOVFragment
{
    unsigned int        track_id;
    uint64_t base_data_offset;  // the 'moof' box offset in the file, used for get the
                                // media data offset
    uint64_t moof_offset;       // the offset of 'moof' box in this segment
    unsigned int stsd_id;
    unsigned int duration;
    unsigned int size;
    unsigned int flags;
} MOVFragment;

typedef struct MOVTrackExt
{
    unsigned int track_id;
    unsigned int stsd_id;
    unsigned int duration;
    unsigned int size;
    unsigned int flags;
} MOVTrackExt;

typedef struct MOVSampleEnc
{
    //CDX_U8 iv[8];
    CDX_U64 iv;
    CDX_U32 region_count;
    CDX_U32 regions[32]; //16pairs max
} MOVSampleEnc;

typedef struct _Sample Sample;
struct _Sample
{
    CDX_U64 offset;
    CDX_U32 size;
    CDX_U32 duration;
    CDX_S32 keyframe; // 0: not keyframe; 1:keyframe
    CDX_S32 index;
};

typedef struct ExtendAndroidMdta
{
    unsigned char   android_mdta[128];
    int             index;
}ExtendAndroidMdta;

#define MAX_MDTA_NUMS 5
#define MOV_READ_SIZE   500*1024   // for seek
#define MAX_STREAM_NUM 5
typedef struct MOVContext
{
    CdxStreamT        *fp;
    CDX_U8          *moov_buffer;   // 'moov' atom (not contain 'moov' size and type)
    CDX_U32            moov_size;
    CDX_S32            file_size;
    MOV_CHUNK        chunk_info;
    int             prefetch_stream_index;   // va_sel in readSample

    int             bSeekAble;

    //the video,audio, subtitle stream to st index;
    int             v2st[MAX_STREAM_NUM];
    int             a2st[MAX_STREAM_NUM];
    int             s2st[MAX_STREAM_NUM];

    int                sidx_flag;
    MOVSidx*        sidx_buffer;
    CDX_S32            sidx_count;
    CDX_U64         sidx_time;
    CDX_U32            sidx_total_time;
    CDX_U64            first_moof_offset;  // used for getting the segment offset  in the file

    CDX_U32            mvhd_total_time;
    int                first_styp_offset;
    CDX_U64            moof_end_offset;  // offset of the moof  end
    CDX_U64            last_sample_pts;
    int                is_fragment;   // the mp4 file is fragment mp4
    MOVFragment        fragment;     //current fragment in moof atom
    MOVTrackExt     *trex_data;    // 'trex' atom, each trak has a trex
    int                trex_num;
    AW_List*            Vsamples;  //video sample
    AW_List*            Asamples;  // audio sample

    CDX_S32            time_scale;
    CDX_S32            duration;          /* duration of the longest track */
    CDX_S32            found_moov;        /* when both 'moov' and 'mdat' sections has been found */
    CDX_S32            found_mdat;        /* we suppose we have enough data to read the file  */
    //CDX_S32            mdat_offset;
    CDX_S32            total_streams;
    CDX_S32            nb_streams;        /*  stream number of the file  */

    CDX_U8            *pAvccHdrInfo;      /* avcc header information */
    CDX_S32           nAvccHdrInfoLen;    /* avcc header information length */

    MOVStreamContext    *streams[MOV_MAX_STREAMS];
    //Mov_Index_Context_Extend    mov_idx_curr[3];
    CDX_S32           basetime[3];        /*0: video 1: audio*/
    //const struct MOVParseTableEntry *parse_table;/*could be eventually used to change the table*/

    CDX_U16            video_stream_idx;
    CDX_U16            audio_stream_idx;        // the index of display audio stream
    CDX_U16            subtitle_stream_idx;

    CDX_U16            has_video;
    CDX_U16            has_audio;
    CDX_U16            has_subtitle;
    CDX_S32            audio_stream_num;
    CDX_S32            video_stream_num;
    CDX_S32            subtitle_stream_num;

    CDX_U16            stsc_one_entry;

    CDX_S32            mdat_count;
    CDX_S32            isom;                /* 1 if file is ISO Media (mp4/3gp) */
    CDX_S32            totaltime;

    CDX_S32            audio_sample_size;
    /* used for ff or rev sate save */
    CDX_U32            samples_per_chunk;
    struct CdxMovParser        *ptr_fpsr;
    CDX_U32          filesize;
    CDX_S32            bs_not_support;
    CDX_S32            chunk_sample_idx_assgin_by_seek;    // what is it mean, i donot know

    //metadata
    //**<  the geometry information for camera record
    cdx_uint8             location[32];
    cdx_uint8           title[32];
    cdx_uint8           album[32];
    cdx_uint8           artist[32];
    cdx_uint8           date[32];
    cdx_uint8           albumArtistic[32];
    cdx_uint8           genre[32];
    cdx_uint8           writer[32];

    cdx_int32           bDash;  // for get segment mp4,

    cdx_int32           bSmsSegment; //* for sms
    cdx_int64           nSmsMdatOffset;
    cdx_uint32          nDataOffsetDelta;

    cdx_int32           bPlayreadySegment; //* for playready sms
    MOVSampleEnc        *senc_data;
    ID3*                id3v2;
    ExtendAndroidMdta   extend_android_mdta[MAX_MDTA_NUMS];
    float               android_capture_fps;

    cdx_int32           isReadEnd;
} MOVContext;

#define INT_MAX      0x7fffffff
//#define UINT_MAX     0xffffffff

struct CdxMovParser* CdxMovInit(CDX_S32 *ret);
CDX_S16 CdxMovExit(struct CdxMovParser *p);
CDX_S32 CdxMovOpen(struct CdxMovParser *p, CdxStreamT *stream);
CDX_S16 CdxMovClose(struct CdxMovParser *p);
CDX_S16 CdxMovRead(struct CdxMovParser *p);
int CdxMovSeek(struct CdxMovParser *p, cdx_int64  timeUs, SeekModeType seekModeType);
CDX_S32 CdxMovSetStream(struct CdxMovParser *p);

#endif
