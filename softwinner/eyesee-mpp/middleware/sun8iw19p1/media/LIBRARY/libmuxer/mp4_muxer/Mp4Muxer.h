#ifndef		__MP4MUXER_H
#define		__MP4MUXER_H

//#include "cedarv_osal_linux.h"
//#include <recorde_writer.h>
#include <FsWriter.h>
#include <encoder_type.h>
#include "mp4_mux_lib.h"

#define eLIBs_printf			alogv
#define __inf					alogv
#define __wrn					alogw

//#define __SIM_TEST

#define MAX_STREAMS_IN_FILE 3

#define MKTAG(a,b,c,d) (a | (b << 8) | (c << 16) | (d << 24))

//#define ByteIOContext FILE
#define url_ftell ftell
#define url_fseek fseek

#define offset_t int64_t
//#define MAX_STREAMS 2
#define AV_ROUND_UP 3

//#define av_free free
//#define av_malloc malloc
//#define av_freep free

// star add 
#define esFSYS_remove	remove
#define eLIBs_sprintf	sprintf
#define MOV_BUF_NAME_LEN	(128)

#ifdef __SIM_TEST
#define MOV_CACHE_TINY_PAGE_SIZE (128)
#else
#define MOV_CACHE_TINY_PAGE_SIZE (1024*2)//set it to 2K !!attention it must set to below MOV_RESERVED_CACHE_ENTRIES
#endif

#define MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE (MOV_CACHE_TINY_PAGE_SIZE*4)
#define globalTimescale 1000

#define MODE_MP4  0x01

// #define UINT32_MAX 0xffffffff 
// #define INT32_MAX 2147483647

//typedef struct AVRational{
//    int num; ///< numerator
//    int den; ///< denominator
//} AVRational;

//typedef struct AVCodecContext {
//
//    int bit_rate;
//	int width;
//	int height;
//	enum CodecType codec_type; /* see CODEC_TYPE_xxx */
//	unsigned int codec_tag;
//    unsigned int codec_id;
//
//	int channels;
//	int frame_size;   //一个audio帧的解码之后的sample的数量
//	int frame_rate;
//
//	int bits_per_sample;
//	int sample_rate;
//
//	AVRational time_base;
//}AVCodecContext;

typedef struct MOVIentry {
    unsigned int        size;
    unsigned int        pos;
    int		 samplesInChunk;
    unsigned int        key_frame;
    int		 entries;
    //int64_t      dts;
} MOVIentry;

typedef struct MOVContext MOVContext;
typedef struct MOVIndex {
    int         mode;
    int         entry;
    int        timescale;
	int        sampleDuration;
    int        time;
    int64_t      trackDuration;
    int        sampleCount;
    int        sampleSize;
    int         hasKeyframes;
    int         hasBframes;
    int         language;
    int         trackID;
	int		  stream_type;
    MOVContext    *mov;//used for reverse access
    int         tag; ///< stsd fourcc, e,g, 'sowt', 'mp4a'
    AVCodecContext *enc;

    int         vosLen;
    char         *vosData;
    MOVIentry    *cluster;
    int         audio_vbr;

    unsigned int   stsz_size;
    unsigned int   stco_size;
    unsigned int   stsc_size;
    unsigned int   stts_size;
    unsigned int   stss_size;

	unsigned int   stsz_tiny_pages;
    unsigned int   stco_tiny_pages;
    unsigned int   stsc_tiny_pages;
    unsigned int   stts_tiny_pages;

    unsigned int   keyFrame_num;

    unsigned int   stsc_value;//usual equal framerate/2
    //unsigned int   sample_cnt;//used for loop counter sample 
} MOVTrack;

typedef enum CHUNKID
{
	STCO_ID = 0,//don't change all
	STSZ_ID = 1,
	STSC_ID = 2,
	STTS_ID = 3
}CHUNKID;

#ifdef __SIM_TEST

#define STCO_CACHE_SIZE  (MOV_CACHE_TINY_PAGE_SIZE*2) 
#define STSZ_CACHE_SIZE  (MOV_CACHE_TINY_PAGE_SIZE*4) 
#define STSC_CACHE_SIZE  (STCO_CACHE_SIZE) 

#else

//1280*720. 30fps, 17.5 minute
#define STCO_CACHE_SIZE  (MOV_CACHE_TINY_PAGE_SIZE*4*2)   //(8*1024)          //about 1 hours !must times of tiny_page_size, unit:4byte.
#define STSZ_CACHE_SIZE  (MOV_CACHE_TINY_PAGE_SIZE*4*4) //(8*1024*4) //(8*1024*16)       //about 1 hours !must times of tiny_page_size
#define STTS_CACHE_SIZE  (STSZ_CACHE_SIZE)      //about 1 hours !must times of tiny_page_size
#define STSC_CACHE_SIZE  (STCO_CACHE_SIZE)      //about 1 hours !must times of tiny_page_size

#endif

#define STSZ_CACHE_OFFSET_INFILE_VIDEO  (0*1024*1024) 
#define STSZ_CACHE_OFFSET_INFILE_AUDIO  (2*1024*1024) 
#define STTS_CACHE_OFFSET_INFILE_VIDEO  (4*1024*1024)
#define STCO_CACHE_OFFSET_INFILE_VIDEO  (6*1024*1024) 
#define STCO_CACHE_OFFSET_INFILE_AUDIO  (6*1024*1024 + 512*1024) 
#define STSC_CACHE_OFFSET_INFILE_VIDEO  (7*1024*1024) 
#define STSC_CACHE_OFFSET_INFILE_AUDIO  (7*1024*1024 + 512*1024) 

#define TOTAL_CACHE_SIZE (STCO_CACHE_SIZE*3 + STSZ_CACHE_SIZE*3 + STSC_CACHE_SIZE*3 + STTS_CACHE_SIZE + MOV_CACHE_TINY_PAGE_SIZE) //4byte as one unit.
//#define PAYLOAD_CACHE_SIZE  (128*1024*4)

#define KEYFRAME_CACHE_SIZE (8*1024*16)

#define MOV_HEADER_RESERVE_SIZE     (1024*1024)

#define MOV_GPS_MAX_ENTRY_NUM (10*60*2)           // max 10minue *2/s

enum gps_pack_method_e
{
    GPS_PACK_IN_TRACK,
    GPS_PACK_IN_MDAT,    
};

typedef struct gps_entry_s
{
   unsigned int gps_data_pos;
   unsigned int gps_data_len_hold;
}GPS_ENTRY;

typedef struct MOVContext {
    int    mode;
    int64_t  create_time;
    int    nb_streams;
    offset_t mdat_pos;
	offset_t mdat_start_pos;//raw bitstream start pos
    int64_t mdat_size;
    int    timescale;
	int    geo_available;
	int    latitudex10000;
	int    longitudex10000;
	int	 rotate_degree;
	
    MOVTrack tracks[MAX_STREAMS];

    __hdle  fd_cache;
    __hdle  fd_stsz[MAX_STREAMS_IN_FILE];
    __hdle  fd_stco[MAX_STREAMS_IN_FILE];
    __hdle  fd_stsc[MAX_STREAMS_IN_FILE];
    __hdle  fd_stts[MAX_STREAMS_IN_FILE];

    unsigned int *cache_start_ptr[4][MAX_STREAMS_IN_FILE]; //[4]stco,stsz,stsc,stts [3]video audio text
    unsigned int *cache_end_ptr[4][MAX_STREAMS_IN_FILE];
    unsigned int *cache_write_ptr[4][MAX_STREAMS_IN_FILE];
    unsigned int *cache_read_ptr[4][MAX_STREAMS_IN_FILE];
	unsigned int *cache_tiny_page_ptr;
    unsigned int *cache_keyframe_ptr;

	offset_t stsz_cache_offset_in_file[MAX_STREAMS_IN_FILE];
	offset_t stco_cache_offset_in_file[MAX_STREAMS_IN_FILE];
	offset_t stsc_cache_offset_in_file[MAX_STREAMS_IN_FILE];
	offset_t stts_cache_offset_in_file[MAX_STREAMS_IN_FILE];

	int stsz_cache_size[MAX_STREAMS_IN_FILE];
	int stco_cache_size[MAX_STREAMS_IN_FILE];   //unit:4byte
	int stsc_cache_size[MAX_STREAMS_IN_FILE];
	int stts_cache_size[MAX_STREAMS_IN_FILE];

    int   stsc_cnt;//sample count in one chunk
    int   last_stream_index;
    int	keyframe_interval;
    int64_t   last_video_packet_pts;
	
	//CDX_U8 * payload_buffer_cache_start;
//	CDX_U8 * payload_buffer_cache_end;
	//int  payload_buffer_cache_offset;
	//int  payload_buffer_cache_size;

    //tmp file path
    char FilePath_stsz[MAX_STREAMS][MOV_BUF_NAME_LEN];
    char FilePath_stts[MAX_STREAMS][MOV_BUF_NAME_LEN];
    char FilePath_stco[MAX_STREAMS][MOV_BUF_NAME_LEN];
    char FilePath_stsc[MAX_STREAMS][MOV_BUF_NAME_LEN];

	/* gushiming compressed source */
	int keyframe_num;
	//int audio_codec_id;
	//int video_codec_id;

    offset_t free_pos;

    GPS_ENTRY *gps_entry_buff;
    int gps_entry_buff_rd;
    int gps_entry_buff_wt;
} MOVContext_t;

typedef struct {
    int count;
    int duration;
} MOV_stts_t;

//typedef struct AVStream {
//    int index;    /**< stream index in AVFormatContext */
//    int id;       /**< format specific stream id */
//    AVCodecContext codec; /**< codec context */
//    AVRational r_frame_rate;
//    void *priv_data;
//
//    /* internal data used in av_find_stream_info() */
//    int64_t first_dts;
//    /** encoding: PTS generation when outputing stream */
    //struct AVFrac pts;
//
//
//    AVRational time_base;
    //FIXME move stuff to a flags field?
//    /** quality, as it has been removed from AVCodecContext and put in AVVideoFrame
//     * MN: dunno if that is the right place for it */
//    float quality;
//    int64_t start_time;
//    int64_t duration;
//
//    int64_t cur_dts;
//
//} AVStream;

//typedef struct AVFormatContext {
//	AVRational time_base;
//    void *priv_data;
//    ByteIOContext *pb;
//    int nb_streams;
//    AVStream *streams[MAX_STREAMS];
//    char filename[1024]; /**< input or output filename */
//
//    int64_t data_offset; /** offset of the first packet */
//    int index_built;
//
//    unsigned int nb_programs;
//	int64_t timestamp;
//	char firstframe[2];
//    unsigned int total_video_frames;
//
//    CDX_U8 *mov_inf_cache;
//} AVFormatContext;

//typedef struct AVPacket {
//    int64_t pts;
//	int64_t dts;
//    char    *data0;
//    int   size0;
//   char    *data1;
//    int   size1;
//    int   stream_index;
//    int   flags;
//    int   duration;                         ///< presentation duration in time_base units (0 if not available)
//    int64_t pos;                            ///< byte position in stream, -1 if unknown
//} AVPacket;

#define PKT_FLAG_KEY 1

int mov_write_header(AVFormatContext *s);
int mov_write_packet(AVFormatContext *s, AVPacket *pkt);
int mov_write_trailer(AVFormatContext *s);
int movCreateTmpFile(MOVContext *mov);


#endif

