#ifndef DTV_TSC_DEV_H
#define DTV_TSC_DEV_H
#include "tsc_type.h"

//reverse one channel.
#define RECODER_CHANNEL_NUM (1)
#define VALID_CHANNLE_NUM	(TSC_CHAN_NUM -1)
#define REVERSED_CHAN_INDEX (TSC_CHAN_NUM - 1)
#define PCR_CHANNEL_INDEX	(TSC_CHAN_NUM - 2)
#define TSF_NUM (1)
#define DSC_NUM (8)

typedef struct PID_CHANNEL {
	uint32_t is_opened;
	int32_t pid;
	ptsc_callback_t callback;
	void* callbackparam;

	uint32_t chan;
	uint8_t* buf;
	uint32_t buf_size;
} pid_chan_t;

typedef struct PCR_CHANNEL {
	uint32_t is_reserved_chan_opened;
	uint32_t is_detect_opened;
	uint32_t attach_chan;
	int32_t pid;
	ptsc_callback_t callback;
	void* callbackparam;
	uint32_t chan;
	uint8_t* buf;
	uint8_t* buf_end;
	uint32_t buf_size;
} pcr_chan_t;

typedef struct TSF_INFO {
	pthread_mutex_t mutex;
	pid_chan_t channels[TSC_CHAN_NUM];
	pcr_chan_t pcr_channel;
	uint8_t* buf;
	size_t phys_addr;

	uint32_t total_buf_size;
	uint8_t* record_buf;
	size_t record_phys_addr;
	uint32_t record_total_buf_size;
	uint32_t record_des_chan[4];
} tsf_info_t;

typedef struct DSC_IDX {
	uint32_t is_opened;
	int32_t chan;
} dsc_idx_t;

typedef struct TSD_INFO {
	pthread_mutex_t mutex;
	dsc_idx_t dsc[DSC_NUM];
} tsd_info_t;

typedef struct DVB_RX {
	int32_t fd; //* file descriptor for mmap memory
	pthread_t thread_id;
	int32_t thread_exit;
	uint32_t chan_status;

	//tsf 1 is used for recoding.
	//tsf 0 is used for receiving section data
	//and playback.
	tsf_info_t tsf;

	uint8_t* regs;
	uint32_t total_regs_size;
	void *tsc_ctx;

	tsd_info_t tsd;
} dvb_rx_t;

#endif
