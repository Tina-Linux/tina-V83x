#ifndef DTV_TSC_HAL_H
#define DTV_TSC_HAL_H
#include "tsc_type.h"

#define TSC_CHAN_NUM        32      //* channel number for one port, there are one port in A64 tsc module.
#define PID_ALL             8192    //* there are 8192 pids at most
#define PID_PAT             0       //* pid for PAT table
#define PKT_LEN             188     //* packet length
#define NEW_SWITCH_METHOD   (0)
#define TSF_INTR_THRESHOLD  (1)//defines when to interrupt


typedef enum TSF_CHAN_TYPE_E {
	TSF_TP_CHAN = 0, TSF_PES_CHAN = 1
} tsf_chan_type_e;

//typedef enum TSF_PORT_E {
//	TSF_PORT_0 = 0
//} tsf_port_e;

typedef enum CA_TYPE_E {
	CA_TYPE_DVBCSA_1_1 = 0, CA_TYPE_UNKNOW
} ca_type_e;

typedef enum CW_TYPE_E {
	CW_ODD_LOW_32BITS = 0,
	CW_ODD_HIGH_32BITS = 1,
	CW_EVEN_LOW_32BITS = 2,
	CW_EVEN_HIGH_32_BITS = 3,
} cw_type_e;

typedef struct PORT_STATUS_T {
	uint32_t chan_open_flag;
	uint32_t chan_pes_flag;
	uint32_t chan_buf_size[TSC_CHAN_NUM];
	uint8_t* chan_buf[TSC_CHAN_NUM];
} port_status_t;

typedef struct TSC_CONTEXT_T {
	void* tsc_registers;
	port_status_t port;
} tsc_ctx_t;

//************************* TSC functions **************************//
void *tsc_init(void *regs);
void tsc_exit(void *handle);
int32_t tsc_open(void * handle);
void tsc_close(void *handle);

void tsf_get_status(void *handle, int32_t* port0_chan_intr_flags, int32_t* port0_pcr_intr_flag,
		int32_t* port1_chan_intr_flags, int32_t* port1_pcr_intr_flag);

//************************* TSF functions **************************//
int32_t tsf_open_chan(void* handle, uint32_t pid, uint8_t* virt_addr, uint8_t *phy_addr,
		uint32_t buf_size, tsf_chan_type_e chan_type, uint32_t chan_id, uint32_t interrupt_enable);
void tsf_close_chan(void *handle, uint32_t chan_id);
int32_t tsf_flush_chan(void *handle, uint32_t chan_id, uint32_t byte_count);
int32_t tsf_reset_chan(void *handle, uint32_t chan_id);
int32_t tsf_request_data(void *handle, uint32_t chan_id, uint8_t** data,
		uint32_t* size, uint8_t** ring_data, uint32_t* ring_size);
int32_t tsf_check_data_size(void *handle, uint32_t chan_id);
uint32_t tsf_get_pcr(void *handle);
void tsf_open_pcr_detect(void *handle, uint32_t chan_id);
void tsf_close_pcr_detect(void *handle );
int32_t tsf_set_ca_enable(void *handle, uint32_t chan_id, uint32_t en);
int32_t tsf_set_cw_index(void *handle, uint32_t chan_id, uint32_t cw_index);
void dump_register(void *handle, int chan);
//************************* TSD functions **************************//
int32_t tsd_set_ca_type(void *handle, ca_type_e ca_type);
int32_t tsd_set_cw(void *handle, uint32_t cw_index, cw_type_e cw_type, uint32_t cw_value);

//for debug
void tsf_dump_chan_register(void *handle, int chan);
void ts_dump_registers(void *handle);
#endif
