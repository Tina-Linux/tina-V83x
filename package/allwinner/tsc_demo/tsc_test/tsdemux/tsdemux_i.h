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

#define MAX_DESCRAMBLE		(8)

//* define callback function to push data
typedef int32_t (*push_data_cb)(uint8_t* data, uint32_t len, uint32_t new_frm,
		void * parm);

enum DEMUX_STATUS {
	DEMUX_STATUS_IDLE = 0,
	DEMUX_STATUS_STARTED,
	DEMUX_STATUS_STOPPED,
	DEMUX_STATUS_PAUSED,
};

typedef enum MPEGTSSTATE {
	MPEGTS_HEADER = 0,
	MPEGTS_PESHEADER_FILL,
	MPEGTS_PAYLOAD,
	MPEGTS_SKIP
} mpegts_state_t;

typedef struct ESFILTER {
	demux_stream_type stream_type;
	demux_codec_type codec_type;
	uint8_t *cur_ptr; //* current pointer
	uint32_t free_size; //* free size of buffer for pushing data
	uint32_t valid_size; //* written size
	uint32_t ctrl_bits; //* control bits
	uint32_t rap_flag; //* random access point

	int32_t pid;
	int32_t is_first;

	md_data_info_t data_info;
	md_buf_t md_buf;
	pdemux_callback_t requestbufcb;
	pdemux_callback_t updatedatacb;
	mpegts_state_t state;

	//* variables below are used for getting PES format
	int64_t pts; //* presentation tim stamp
	int64_t pre_pts; //* previous pts

	push_data_cb push_es_data; //* push PES data callback function

	uint32_t data_index; //index to indicate location of handled PES header
	uint32_t total_size;
	uint32_t payload_size;
	uint32_t pes_header_size;
	uint8_t header[MAX_PES_HEADER_SIZE]; //save header data

	void* cookie;
} es_filter_t;

typedef struct PESFILTER {
	uint8_t *cur_ptr; //* current pointer
	uint32_t free_size; //* free size of buffer for pushing data
	uint32_t valid_size; //* written size
	uint32_t ctrl_bits; //* control bits

	int32_t pid;
	md_data_info_t data_info;
	md_buf_t md_buf;
	pdemux_callback_t requestbufcb;
	pdemux_callback_t updatedatacb;
	push_data_cb push_pes_data; //* push PES data callback function
	uint32_t total_size;
	int32_t started;
	void* cookie;
} pes_filter_t;

typedef struct TSFILTER {
	uint8_t *cur_ptr; //* current pointer
	uint32_t free_size; //* free size of buffer for pushing data
	int32_t pid;

	md_buf_t md_buf;
	pdemux_callback_t requestbufcb;
	pdemux_callback_t updatedatacb;

	push_data_cb push_ts_data; //* push TS data callback function
	void *cookie;
} ts_filter_t;

typedef struct SECTIONFILTER {
	uint32_t need_start;
	int32_t section_index;
	int32_t section_h_size;

	int32_t check_crc;
	int32_t end_of_section_reached;
	int32_t last_cc; //continuity counter

	uint8_t *cur_ptr;
	uint32_t free_size;
	int32_t pid;

	md_buf_t md_buf;
	pdemux_callback_t requestbufcb; //request buffer callback function
	pdemux_callback_t updatedatacb; //update data callback function
	push_data_cb push_section_data; //push section data callback function
	void *cookie;
} section_filter_t;

//* define MPEG TS filter structure
typedef struct MPEGTSFILTER {
	int32_t pid;
	int32_t chan;
	demux_filter_type_t type;
	ts_dev_t *tsc_handle;
	dtv_sem_t wait_data_sem;
	pthread_t thread_id;
	pthread_mutex_t mutex;
	pthread_cond_t condition;
	int32_t status;
	union {
		es_filter_t es_filter;
		pes_filter_t pes_filter;
		ts_filter_t ts_filter;
		section_filter_t section_filter;
	} u;
} filter_t;

typedef struct RECODERFILTER {
	pthread_t thread_id;
	int32_t thread_exit;
	int32_t chan;
	int32_t opened;
	int32_t count;
	int32_t pid[32];
	void* tsc_handle;
	pdemux_callback_t requestbufcb;
	pdemux_callback_t updatedatacb;
	push_data_cb push_data;
	void *cookie;
}recorder_filter_t;

typedef struct DESCRAMBLE {
	int32_t is_open;
	int32_t chan;
	int32_t pid;
	int32_t des_indx;
}descramble_t;

typedef struct MPEGTSCONTEXT {
	int32_t demux_type;
	uint32_t pcrid;
	uint32_t detectpcr;
	pdemux_callback_t pcrcallback;
	void *pcr_cookie;

	pthread_mutex_t mutex;
	ts_dev_t *tsc_handle;
	filter_t *pids[NB_PID_MAX];

	recorder_filter_t *recorder;

	descramble_t descramble[MAX_DESCRAMBLE];
} mpegts_context_t;

#ifdef __cplusplus
}
#endif

#endif
