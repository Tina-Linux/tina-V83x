#ifndef DTV_TSC_H
#define DTV_TSC_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_DES_CHANNEL_NUM (4) // 1 for video 3 for audio

typedef int (*ptsc_callback_t)(void *cookie, void *param);

enum {
	CHAN_TYPE_UNKOWN = 0,
	CHAN_TYPE_LIVE,
	CHAN_TYPE_RECORDE,
	CHAN_TYPE_INJECT,
	CHAN_TYPE_RECORD_DESCRAMBLE,
};

enum TS_STREAM_TYPE{
	//do not change the order or change the value.
	TS_STREAM_UNKOWN = 0,
	TS_STREAM_VIDEO  = 1,
	TS_STREAM_AUDIO  = 2,
	TS_STREAM_SUBTITLE = 3,
	TS_STREAM_SECTION  = 4,
};

typedef struct PID_CHANNEL_REGISTER
{
	int			stream_type;
	int			chan_type;
	int			desc_id;        //* for CHAN_TYPE_RECORD_DESCRAMBLE only
    int			pid;            //* one channel has one pid
    ptsc_callback_t callback;       //* when data is ready , tsc will call this function
    void*		callbackparam;  //* parameter for callback function
}chan_register_t;

typedef struct TSF_DATA
{
    unsigned char* data;        //* start address of ts packets
    unsigned int   pkt_num;       //* packet numbers
    unsigned char* ring_data;    //* ring part start address
    unsigned int   ring_pkt_num;   //* ring part packet numbers
}tsf_data_t;

typedef enum DES_CW_TYPE_E {
	DES_CW_ODD_LOW_32BITS = 0,
	DES_CW_ODD_HIGH_32BITS = 1,
	DES_CW_EVEN_LOW_32BITS = 2,
	DES_CW_EVEN_HIGH_32_BITS = 3,
}des_cw_type_e;

void* tsc_dev_open(void);
int tsc_dev_close(void* handle);

void* ts_parser_open(void);
int ts_parser_close(void* handle);

typedef struct TS_DEV_CONTEXT {
	void *cookie;//tsc or tsparser
	int (*open_channel)(void* handle, chan_register_t* chan_register);
	int (*close_channel)(void* handle, unsigned int chan);
	int (*get_channel_packet_num)(void* handle, int chan);
	int (*request_channel_data)(void* handle, int chan, tsf_data_t* tsf_data);
	int (*flush_channel_data)(void* handle, int chan, int pkt_num);
	int (*read_channel_data)(void* pdata, unsigned int pkt_num_require, unsigned int chan, void* handle);
	int (*open_pcr_detect)(void* handle, chan_register_t* chan_register);
	void (*close_pcr_detect)(void* handle);
	int (*get_free_channel_num)(void *handle);
	void (*reset_channel)(void *handle, int chan);
	void (*dump_register)(void *handle, int chan);
	void (*clear)(void *handle);
	//for injector.
	int (*set_buffer_size)(void *handle, int size);
	int (*write_data)(void *handle, void *buf, int size);

	//for descramble
	int (*open_descramble)(void* handle, int chan);
	int (*close_descramble)(void* handle, int index);
	int (*set_descramble_cw)(void* handle, int index, des_cw_type_e cw_type, uint32_t cw_value);
	int (*desc_set_pid)(void *handle, int desc_id, int chan_id, int pid);
	int (*desc_set_key)(void *handle, int desc_id, uint8_t *odd_key, uint8_t *even_key);

} ts_dev_t;

#ifdef __cplusplus
}
#endif

#endif
