#ifndef DTV_TSDEMUX_H
#define DTV_TSDEMUX_H

#ifdef __cplusplus
extern "C" {
#endif

//define callback function
typedef int (*pdemux_callback_t)(void* param, void* cookie);

#ifndef MAX_RECODER_NUM
#define MAX_RECORDER_NUM (32)
#endif

typedef enum DEMUX_TYPE {
	DEMUX_TYPE_LIVE_AND_RECORDE = 0,
	DEMUX_TYPE_INJECT = 1,
}demux_type_t;

typedef enum DEMUX_CODEC_TYPE {
	DMX_CODEC_UNKOWN = 0,
	DMX_CODEC_AVC,
	DMX_CODEC_AVS,
}demux_codec_type;

typedef enum DEMUX_FILTER_TYPE
{
    DMX_FILTER_TYPE_ES = 0,     //* get es stream from a filter.
    DMX_FILTER_TYPE_PES,        //* get pes packets from a filter.
    DMX_FILTER_TYPE_TS,         //* get ts packets from a filter.
    DMX_FILTER_TYPE_SECTION,    //* get si section from a filter.
    DMX_FILTER_TYPE_RECORD,     //* get record from a filter.
}demux_filter_type_t;

typedef enum DEMUX_STREAM_TYPE
{
	DMX_STREAM_UNKOWN = 0,
	DMX_STREAM_VIDEO,
	DMX_STREAM_AUDIO,
	DMX_STREAM_SUBTITLE,
	DMX_STREAM_SECTION,
}demux_stream_type;

typedef struct DEMUX_FILTER_PARAM
{
	int32_t				stream_type;
	int32_t				codec_type;
    demux_filter_type_t filter_type;
    pdemux_callback_t   request_buffer_cb;
    pdemux_callback_t   update_data_cb;
	void*               cookie;
}demux_filter_param_t;

typedef struct DEMUX_RECODER_PARAM
{
	int pids[MAX_RECORDER_NUM];
	int count;
    pdemux_callback_t   request_buffer_cb;
    pdemux_callback_t   update_data_cb;
	void*               cookie;
}demux_recoder_param_t;

#define PTS_VALID_BIT			0x2
#define FIRST_PART_BIT			0x8
#define LAST_PART_BIT			0x10
#define RANDOM_ACCESS_FRAME_BIT	0x40

typedef struct MEDIA_BUFFER
{
    unsigned char*  buf;
    unsigned int    buf_size;
}md_buf_t;

typedef struct MEDIA_DATA_INFO
{
    int64_t         pts;
    unsigned int    data_len;
    unsigned int    ctrl_bits;
}md_data_info_t;

typedef enum TS_DEMUX_CW_TYPE_E {
	TS_DEMUX_CW_ODD_LOW_32BITS = 0,
	TS_DEMUX_CW_ODD_HIGH_32BITS = 1,
	TS_DEMUX_CW_EVEN_LOW_32BITS = 2,
	TS_DEMUX_CW_EVEN_HIGH_32_BITS = 3,
}ts_demux_cw_type_e;

void* ts_demux_open(int type);
int   ts_demux_close(void* handle);
int   ts_demux_open_filter(void* handle, int pid,
		demux_filter_param_t* param);
int   ts_demux_close_filter(void* handle, int pid);
int   ts_demux_open_pcr_filter(void* handle, int pid,
		pdemux_callback_t callbak, void* cookie);
int   ts_demux_close_pcr_filter(void* handle);
int   ts_demux_clear(void *handle);
int   ts_demux_start(void *handle);
int   ts_demux_pause(void *handle);
//for recorder, when requesting buffer, must make sure buffer size is
//larger than a TS packet size.
int   ts_demux_open_recorder(void* handle, demux_recoder_param_t *param);
int   ts_demux_close_recorder(void* handle);

int   ts_demux_get_free_filter_num(void* handle);
int   ts_demux_get_ts_packet_num(void* handle);

//for injector.
int ts_demux_set_buffer_size(void *handle, int size);
int ts_demux_write_data(void *handle, void *buf, int size);

//for descramble
int ts_demux_open_descramble(void* handle, int pid);
int ts_demux_close_descramble(void* handle, int index);
int ts_demux_set_descramble_cw(void* handle, int index, ts_demux_cw_type_e cw_type, unsigned int cw_value);
int ts_desc_set_pid(void *handle, int desc_id, int pid);
int ts_desc_set_key(void *handle, int desc_id, uint8_t *odd_key, uint8_t *even_key);
#ifdef __cplusplus
}
#endif

#endif
