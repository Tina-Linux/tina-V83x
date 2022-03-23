//#define LOG_NDEBUG 0
#define LOG_TAG "tsc_hal"
#include <CDX_Debug.h>

#include <string.h>
#include <stdlib.h>
#include <tsc.h>

#include <unistd.h>


//#include "dvb_drv.h"
#include "tsc_regs.h"
#include "tsc_config.h"
#include "tsc_hal.h"
static void print_regs(void *handle);
//************************* TSC functions **************************//
void *tsc_init(void *regs) {
	tsc_ctx_t *context = (tsc_ctx_t *)malloc(sizeof(tsc_ctx_t));
	if(context) {
		context->tsc_registers = regs;
	} else {
		ALOGE("init tsc failed");
	}
	return context;
}

void tsc_exit(void *handle) {
	tsc_ctx_t *context = (tsc_ctx_t *)handle;
	if(context) {
		free(context);
	}
}

int32_t tsc_open(void * handle) {
	tsc_ctx_t *context = (tsc_ctx_t *)handle;
	if(!context) {
		ALOGE("invalid tsc handle");
		return -1;
	}

	//* delay some time for clock stable.
	int32_t i;
	for (i = 0; i < 100; i++);

	//* get the register list of TS controller
	tsc_reg_list_t *register_handle = (tsc_reg_list_t *)context->tsc_registers;

#if (TS_INTERFACE_TYPE == 0)
	SetValue(register_handle->tsc._10_port_ctrl, 0x0);
	//* enable data receive, use spi port
#else
	SetValue(register_handle->tsc._10_port_ctrl, 0x1); //* enable data receive, use ssi port
#endif

	//* set tsc _14_port_param.
	uint32_t tmp = 0;
#if (TS_INTERFACE_PSYNC_SIGNAL_POLARITY == 1)  //* Data Valid signal is low level active.
	tmp |= 0x101;
#endif
#if (TS_INTERFACE_DATA_VALID_SIGNAL_POLARITY == 1)  //* Data Valid signal is low level active.
	tmp |= 0x202;
#endif
#if (TS_INTERFACE_ERROR_SIGNAL_POLARITY == 1)       //* Error signal is low level active.
	tmp |= 0x404;
#endif
#if (TS_INTERFACE_CLOCK_SIGNAL_POLARITY == 1)       //* Falling edge capture.
	tmp |= 0x808;
#endif
#if (TS_INTERFACE_SSI_DATA_ORDER == 1)              //* LSB first when using SSI mode.
	tmp |= 0x1010;
#endif

	SetValue(register_handle->tsc._14_port_param, tmp);

	//* set tsc _20_in_mux_ctrl.
	//* both port0 and port1 get input from input port 0, to support sigle tuner recording.
	SetValue(register_handle->tsc._20_in_mux_ctrl, TS_INTERFACE_TSF0_INPUT);
	//default TS_INTERFACE_TSF1_INPUT << 4

	//* enable tsf0.
	register_handle->tsf._00_ctrl.enable = 1;
	register_handle->tsf._00_ctrl.reset = 1;
	while (register_handle->tsf._00_ctrl.reset);

	//* set tsf _04_pkt_param
	tmp = 0;
#if (TS_INTERFACE_PACKET_SYNC_METHOD == 1)          //* by 0x47 sync byte.
	tmp |= (1 << 8);
#elif (TS_INTERFACE_PACKET_SYNC_METHOD == 2)        //* by both PSync signal and sync byte.
	tmp |= (2<<8);
#endif
	tmp |= 0x47 << 16; //* sync byte value.
	tmp |= 0x22 << 24; //* two packet to lock synchronization and unlock synchronization.
	SetValue(register_handle->tsf._04_pkt_param, tmp);

	//* set tsf _08_status to enable global DMA interrupt
	register_handle->tsf._08_status.dma_intr_en = 1; //* only enable dma interrupt, disable overlap interrupt.

	for (i = 0; i < TSC_CHAN_NUM; i++) {
		register_handle->tsf._3c_chan_idx.chan_idx = i;
		register_handle->tsf._58_chan_wt_pos.wt_pos = 0;
		register_handle->tsf._5c_chan_rd_pos.rd_pos = 0;
#if NEW_SWITCH_METHOD
		SetValue(register_handle->tsf._4c_chan_pid, (0<<16 | 0x1fff));
#endif
		register_handle->tsf._50_chan_buf_addr.addr = 0;
		register_handle->tsf._54_chan_buf_size.size = 0;
	}

#if NEW_SWITCH_METHOD
	SetValue(register_handle->tsf._30_chan_en, 0xffffffff);
#endif
	register_handle->tsf._00_ctrl.reset = 1;

	return 0;
}

void tsc_close(void *handle) {
	tsc_ctx_t *context = (tsc_ctx_t *)handle;
	if(!context) {
		ALOGE("invalid tsc handle");
		return ;
	}
	tsc_reg_list_t *regs = (tsc_reg_list_t *)context->tsc_registers;
	if (regs != NULL) {
		//* disable tsf0 and tsf1.
		regs->tsf._00_ctrl.enable = 0;
		usleep(10000); //* delay some time for dma dram command finish.
	}

	return;
}

//************************* TSF functions **************************//
int32_t tsf_open_chan(void* handle, uint32_t pid, uint8_t* virt_addr, uint8_t *phy_addr,
		uint32_t buf_size, tsf_chan_type_e chan_type, uint32_t chan_id, uint32_t interrupt_enable) {

	tsc_ctx_t *context = (tsc_ctx_t *)handle;
	if(!context) {
		ALOGE("invalid tsc handle");
		return -1;
	}
	tsc_reg_list_t *register_handle = (tsc_reg_list_t *)context->tsc_registers;

	context->port.chan_open_flag |= (1 << chan_id);
	context->port.chan_pes_flag |= (((chan_type == TSF_PES_CHAN) ? 1 : 0)
			<< chan_id);
	context->port.chan_buf_size[chan_id] = buf_size;
	context->port.chan_buf[chan_id] = virt_addr;

	//* 1. set tsf _3c_chan_idx to choose a channel.
	register_handle->tsf._3c_chan_idx.chan_idx = chan_id;

#if !NEW_SWITCH_METHOD
	//* 2. set tsf _4c_chan_pid to set pid value.
	if(pid == PID_ALL)
		SetValue(register_handle->tsf._4c_chan_pid, 0);//* pass all pid
	else
		SetValue(register_handle->tsf._4c_chan_pid, (0x1fff<<16 | pid));
#endif

	//* 3. set tsf _50_chan_buf_addr to set buffer address.
	register_handle->tsf._50_chan_buf_addr.addr = (size_t)phy_addr;

	//* 4. set tsf _54_chan_buf_size to set buffer size and interrupt threshold.
	SetValue(register_handle->tsf._54_chan_buf_size,
			(TSF_INTR_THRESHOLD<<24 | (buf_size - 16)));


	//* 6. set tsf _10_dma_intr_en to enable channel dma interrupt.
	if (interrupt_enable)
		register_handle->tsf._10_dma_intr_en.en_ctrl_bits |= (1 << chan_id);
	else
		register_handle->tsf._10_dma_intr_en.en_ctrl_bits &= ~(1 << chan_id);

	//* 7. set tsf _34_chan_pes_en to enable or disable pes mode.
	if (chan_type == TSF_PES_CHAN)
		register_handle->tsf._34_chan_pes_en.pes_en_ctrl_bits |= (1 << chan_id);
	else
		register_handle->tsf._34_chan_pes_en.pes_en_ctrl_bits &=
				~(1 << chan_id);
	//* 8. set tsf _30_chan_en to enable the channel.
#if !NEW_SWITCH_METHOD
	register_handle->tsf._30_chan_en.filter_en_ctrl_bits |= (1<<chan_id);
#else
	if (pid == PID_ALL)
		SetValue(register_handle->tsf._4c_chan_pid, 0);
	//* pass all pid
	else
		SetValue(register_handle->tsf._4c_chan_pid, (0x1fff<<16 | pid));
#endif
	//* 5. set tsf _5c_chan_rd_pos.
	register_handle->tsf._5c_chan_rd_pos.rd_pos =
			register_handle->tsf._58_chan_wt_pos.wt_pos;
	//print_regs(context, 0);
	return 0;
}

void tsf_close_chan(void *handle, uint32_t chan_id) {
	tsc_ctx_t *context = (tsc_ctx_t *)handle;
	if(!context) {
		ALOGE("invalid tsc handle");
		return ;
	}
	tsc_reg_list_t *register_handle = (tsc_reg_list_t *)context->tsc_registers;
	register_handle->tsf._3c_chan_idx.chan_idx = chan_id;
	//* set tsf _10_dma_intr_en to disable dma interrupt of the channel.
	register_handle->tsf._10_dma_intr_en.en_ctrl_bits &= ~(1 << chan_id);

#if !NEW_SWITCH_METHOD
	//* set tsf _30_chan_en to disable the channel.
	register_handle->tsf._30_chan_en.filter_en_ctrl_bits &= ~(1<<chan_id);
//	register_handle->tsf.filter_en_ctrl_bits &= ~(1<<chan_id);
#else
	//SetValue(register_handle->tsf._4c_chan_pid, (0<<16 | 0x1fff));
#endif
	uint32_t value;
	GetValue(register_handle->tsf._4c_chan_pid, value);
	ALOGV("chan:%d, 4c:0x%08x", chan_id, value);

	context->port.chan_open_flag &= ~(1 << chan_id);
	context->port.chan_pes_flag &= ~(1 << chan_id);
	context->port.chan_buf_size[chan_id] = 0;
	context->port.chan_buf[chan_id] = 0;
	//* close desc
	register_handle->tsf._38_chan_descramble_en.descramble_en_ctrl_bits &= ~(1<<chan_id);
	return;
}

int32_t tsf_flush_chan(void *handle, uint32_t chan_id, uint32_t byte_count) {
	uint32_t read_pos;
	uint32_t write_pos;
	uint32_t valid_size;

	tsc_ctx_t *context = (tsc_ctx_t *)handle;
	if(!context) {
		ALOGE("invalid tsc handle");
		return -1;
	}
	tsc_reg_list_t *register_handle = (tsc_reg_list_t *)context->tsc_registers;

	//* selete a channel to operate.
	register_handle->tsf._3c_chan_idx.chan_idx = chan_id;

	//* get readPos and writePos.
	read_pos = register_handle->tsf._5c_chan_rd_pos.rd_pos;
	write_pos = register_handle->tsf._58_chan_wt_pos.wt_pos;

	//* calculate total bytes
	if (write_pos >= read_pos)
		valid_size = write_pos - read_pos;
	else
		valid_size = write_pos + context->port.chan_buf_size[chan_id]
				- read_pos;
	//ALOGD("chan %d, pos 0x%x vs 0x%x, size 0x%x vs 0x%x", chan_id, read_pos, write_pos, valid_size, byte_count);
	if (byte_count > valid_size) {
		ALOGW("chan %d, maybe over writing, pos 0x%08x vs 0x%08x, size 0x%x vs 0x%x",
				chan_id, read_pos, write_pos, valid_size, byte_count);
		byte_count = valid_size;
	}

	//* readPos += byte_count, if readPos > bufferSize, readPos -= bufferSize
	read_pos += byte_count;
	if (read_pos > context->port.chan_buf_size[chan_id])
		read_pos -= context->port.chan_buf_size[chan_id];
	//* set readPos to register
	register_handle->tsf._5c_chan_rd_pos.rd_pos = read_pos;

	return 0;
}

int32_t tsf_reset_chan(void *handle, uint32_t chan_id) {
	uint32_t read_pos;
	uint32_t write_pos;
	uint32_t valid_size;

	tsc_ctx_t *context = (tsc_ctx_t *)handle;
	if(!context) {
		ALOGE("invalid tsc handle");
		return -1;
	}
	tsc_reg_list_t *register_handle = (tsc_reg_list_t *)context->tsc_registers;
	//* selete a channel to operate.
	register_handle->tsf._3c_chan_idx.chan_idx = chan_id;
	int pos = register_handle->tsf._58_chan_wt_pos.wt_pos;
	register_handle->tsf._5c_chan_rd_pos.rd_pos = pos;
	return 0;
}

int32_t tsf_request_data(void *handle, uint32_t chan_id, uint8_t** data,
		uint32_t* size, uint8_t** ring_data, uint32_t* ring_size) {
	uint32_t read_pos;
	uint32_t write_pos;
	uint32_t valid_size;
	uint32_t chan_buf_size;
	uint8_t* chan_buf;

	tsc_ctx_t *context = (tsc_ctx_t *)handle;
	if(!context) {
		ALOGE("invalid tsc handle");
		return -1;
	}
	tsc_reg_list_t *register_handle = (tsc_reg_list_t *)context->tsc_registers;

	chan_buf_size = context->port.chan_buf_size[chan_id];
	chan_buf = context->port.chan_buf[chan_id];

	//* selete a channel to operate.
	register_handle->tsf._3c_chan_idx.chan_idx = chan_id;

	//* get readPos and writePos.
	read_pos = register_handle->tsf._5c_chan_rd_pos.rd_pos;
	write_pos = register_handle->tsf._58_chan_wt_pos.wt_pos;
	//ALOGV("chan %d, pos 0x%x vs 0x%x", chan_id, read_pos, write_pos);
	//* calculate total bytes
	if (write_pos >= read_pos)
		valid_size = write_pos - read_pos;
	else
		valid_size = write_pos + chan_buf_size - read_pos;

	if (valid_size > 0 && chan_buf != NULL) {
		if (valid_size + read_pos > chan_buf_size) {
			*data = chan_buf + read_pos;
			*size = chan_buf_size - read_pos;
			*ring_data = chan_buf;
			*ring_size = write_pos;
		} else {
			*data = chan_buf + read_pos;
			*size = valid_size;
			*ring_data = NULL;
			*ring_size = 0;
		}
	} else {
		*data = NULL;
		*size = 0;
		*ring_data = NULL;
		*ring_size = 0;
	}

	return 0;
}

int32_t tsf_check_data_size(void *handle, uint32_t chan_id) {
	uint32_t read_pos;
	uint32_t write_pos;
	uint32_t valid_size;
	uint32_t chan_buf_size;
	uint8_t* chan_buf;
	tsc_ctx_t *context = (tsc_ctx_t *)handle;
	if(!context) {
		ALOGE("invalid tsc handle");
		return -1;
	}
	tsc_reg_list_t *register_handle = (tsc_reg_list_t *)context->tsc_registers;

	chan_buf_size = context->port.chan_buf_size[chan_id];
	chan_buf = context->port.chan_buf[chan_id];
	register_handle = context->tsc_registers;

	if (chan_buf == NULL || chan_buf_size == 0)
		return 0;

	//* selete a channel to operate.
	register_handle->tsf._3c_chan_idx.chan_idx = chan_id;

	//* get readPos and writePos.
	read_pos = register_handle->tsf._5c_chan_rd_pos.rd_pos;
	write_pos = register_handle->tsf._58_chan_wt_pos.wt_pos;

	//* calculate total bytes
	if (write_pos >= read_pos)
		valid_size = write_pos - read_pos;
	else
		valid_size = write_pos + chan_buf_size - read_pos;

	return valid_size;
}

uint32_t tsf_get_pcr(void *handle) {
	uint32_t pcr_most_significant_32_bits;
	tsc_ctx_t *context = (tsc_ctx_t *)handle;
	if(!context) {
		ALOGE("invalid tsc handle");
		return 0;
	}
	tsc_reg_list_t *register_handle = (tsc_reg_list_t *)context->tsc_registers;
	pcr_most_significant_32_bits = register_handle->tsf._24_pcr_data.pcr_value;

	return pcr_most_significant_32_bits;
}

void tsf_open_pcr_detect(void *handle, uint32_t chan_id) {
	tsf20_pcr_ctrl_reg_t pcr_ctrl_reg;
	tsc_ctx_t *context = (tsc_ctx_t *)handle;
	if(!context) {
		ALOGE("invalid tsc handle");
		return ;
	}
	tsc_reg_list_t *register_handle = (tsc_reg_list_t *)context->tsc_registers;

	//* set pcr detect enable bit and the channel index to detect PCR.
	pcr_ctrl_reg = register_handle->tsf._20_pcr_ctrl;
	pcr_ctrl_reg.pcr_chan_idx = chan_id;
	pcr_ctrl_reg.pcr_detect_en = 1;
	register_handle->tsf._20_pcr_ctrl = pcr_ctrl_reg;

	//* enable pcr interrupt of this port.
	register_handle->tsf._08_status.pcr_intr_en = 1;

	return;
}

void tsf_close_pcr_detect(void *handle) {
	tsf20_pcr_ctrl_reg_t pcr_ctrl_reg;
	tsc_ctx_t *context = (tsc_ctx_t *)handle;
	if(!context) {
		ALOGE("invalid tsc handle");
		return ;
	}
	tsc_reg_list_t *register_handle = (tsc_reg_list_t *)context->tsc_registers;

	//* set pcr detect enable bit to zero to disable pcr detect.
	pcr_ctrl_reg = register_handle->tsf._20_pcr_ctrl;
	pcr_ctrl_reg.pcr_detect_en = 0;
	register_handle->tsf._20_pcr_ctrl = pcr_ctrl_reg;

	//* disable pcr interrupt of this port.
	register_handle->tsf._08_status.pcr_intr_en = 0;

	return;
}

void tsf_dump_chan_register(void *handle, int chan)
{
	tsc_ctx_t *context = (tsc_ctx_t *)handle;
	if(!context) {
		ALOGE("invalid tsc handle");
		return ;
	}
	tsc_reg_list_t *register_handle = (tsc_reg_list_t *)context->tsc_registers;
	register_handle->tsf._3c_chan_idx.chan_idx = chan;
	print_regs(handle);
}

static void print_regs(void *handle)
{
	uint32_t value;
	tsc_ctx_t *context = (tsc_ctx_t *)handle;
	if(!context) {
		ALOGE("invalid tsc handle");
		return ;
	}
	tsc_reg_list_t *reg_list = (tsc_reg_list_t *)context->tsc_registers;

	ALOGW("regs %p", reg_list);
	if (reg_list == NULL) {
		ALOGW("get register address error");
		return;
	}
	GetValue(reg_list->tsf._00_ctrl, value);
	ALOGD("reg_list->tsf._00_ctrl	 0x%08x\n",  value);
	GetValue(reg_list->tsf._04_pkt_param, value);
	ALOGD("reg_list->tsf._04_pkt_param	 0x%08x\n", value);
	GetValue(reg_list->tsf._08_status, value);
	ALOGD("reg_list->tsf._08_status		 0x%08x\n", value);
	GetValue(reg_list->tsf._10_dma_intr_en, value);
	ALOGD("reg_list->tsf._10_dma_intr_en	 0x%08x\n",  value);
	GetValue(reg_list->tsf._14_overlap_intr_en, value);
	ALOGD("reg_list->tsf._14_overlap_intr_en	 0x%08x\n",  value);
	GetValue(reg_list->tsf._18_dma_status, value);
	ALOGD("reg_list->tsf._18_dma_status	 0x%08x\n",  value);
	GetValue(reg_list->tsf._1c_overlap_status, value);
	ALOGD("reg_list->tsf._1c_overlap_status		 0x%08x\n", value);
	GetValue(reg_list->tsf._20_pcr_ctrl, value);
	ALOGD("reg_list->tsf._20_pcr_ctrl	 0x%08x\n", value);
	GetValue(reg_list->tsf._24_pcr_data, value);
	ALOGD("reg_list->tsf._24_pcr_data	 0x%08x\n",  value);
	GetValue(reg_list->tsf._30_chan_en, value);
	ALOGD("reg_list->tsf._30_chan_en	 0x%08x\n",  value);
	GetValue(reg_list->tsf._34_chan_pes_en, value);
	ALOGD("reg_list->tsf._34_chan_pes_en	 0x%08x\n",  value);
	GetValue(reg_list->tsf._38_chan_descramble_en, value);
	ALOGD("reg_list->tsf._38_chan_descramble_en	 0x%08x\n",  value);
//	reg_list->tsf._3c_chan_idx.chan_idx = 0;
	GetValue(reg_list->tsf._3c_chan_idx, value);
	ALOGD("reg_list->tsf._3c_chan_idx	 0x%08x\n", value);
	GetValue(reg_list->tsf._40_chan_ctrl, value);
	ALOGD("reg_list->tsf._40_chan_ctrl	 0x%08x\n",  value);
	GetValue(reg_list->tsf._44_chan_status, value);
	ALOGD("reg_list->tsf._44_chan_status	 0x%08x\n",  value);
	GetValue(reg_list->tsf._48_chan_cw_idx, value);
	ALOGD("reg_list->tsf._48_chan_cw_idx	 0x%08x\n",  value);
	GetValue(reg_list->tsf._4c_chan_pid, value);
	ALOGD("reg_list->tsf._4c_chan_pid	 0x%08x\n",  value);
	GetValue(reg_list->tsf._50_chan_buf_addr, value);
	ALOGD("reg_list->tsf._50_chan_buf_addr	 0x%08x\n",  value);
	GetValue(reg_list->tsf._54_chan_buf_size, value);
	ALOGD("reg_list->tsf._54_chan_buf_size	 0x%08x\n",  value);
	GetValue(reg_list->tsf._58_chan_wt_pos, value);
	ALOGD("reg_list->tsf._58_chan_wt_pos 0x%08x\n",  value);
	GetValue(reg_list->tsf._5c_chan_rd_pos, value);
	ALOGD("reg_list->tsf._5c_chan_rd_pos 0x%08x\n",  value);
}

void ts_dump_registers(void *handle)
{
	uint32_t value;
	tsc_ctx_t *context = (tsc_ctx_t *)handle;
	if(!context) {
		ALOGE("invalid tsc handle");
		return ;
	}
	tsc_reg_list_t *reg_list = (tsc_reg_list_t *)context->tsc_registers;

	ALOGW("regs %p", reg_list);
	if (reg_list == NULL) {
		ALOGW("get register address error");
		return;
	}

	GetValue(reg_list->tsc._00_ctrl, value);
	ALOGD("reg_list->tsc._00_ctrl	 0x%08x\n", value);
	GetValue(reg_list->tsc._04_status, value);
	ALOGD("reg_list->tsc._04_status		 0x%08x\n", value);
	GetValue(reg_list->tsc._10_port_ctrl, value);
	ALOGD("reg_list->tsc._10_port_ctrl	 0x%08x\n", value);
	GetValue(reg_list->tsc._14_port_param, value);
	ALOGD("reg_list->tsc._14_port_param	 0x%08x\n", value);
	GetValue(reg_list->tsc._20_in_mux_ctrl, value);
	ALOGD("reg_list->tsc._20_in_mux_ctrl	 0x%08x\n", value);
	GetValue(reg_list->tsc._28_out_mux_ctrl, value);
	ALOGD("reg_list->tsc._28_out_mux_ctrl	 0x%08x\n", value);

	GetValue(reg_list->tsf._00_ctrl, value);
	ALOGD("reg_list->tsf._00_ctrl	 0x%08x\n",  value);
	GetValue(reg_list->tsf._04_pkt_param, value);
	ALOGD("reg_list->tsf._04_pkt_param	 0x%08x\n",  value);
	GetValue(reg_list->tsf._08_status, value);
	ALOGD("reg_list->tsf._08_status		 0x%08x\n",  value);
	GetValue(reg_list->tsf._10_dma_intr_en, value);
	ALOGD("reg_list->tsf._10_dma_intr_en	 0x%08x\n",  value);
	GetValue(reg_list->tsf._14_overlap_intr_en, value);
	ALOGD("reg_list->tsf._14_overlap_intr_en	 0x%08x\n",  value);
	GetValue(reg_list->tsf._18_dma_status, value);
	ALOGD("reg_list->tsf._18_dma_status	 0x%08x\n",  value);
	GetValue(reg_list->tsf._1c_overlap_status, value);
	ALOGD("reg_list->tsf._1c_overlap_status		 0x%08x\n",  value);
	GetValue(reg_list->tsf._20_pcr_ctrl, value);
	ALOGD("reg_list->tsf._20_pcr_ctrl	 0x%08x\n",  value);
	GetValue(reg_list->tsf._24_pcr_data, value);
	ALOGD("reg_list->tsf._24_pcr_data	 0x%08x\n",  value);
	GetValue(reg_list->tsf._30_chan_en, value);
	ALOGD("reg_list->tsf._30_chan_en	 0x%08x\n",  value);
	GetValue(reg_list->tsf._34_chan_pes_en, value);
	ALOGD("reg_list->tsf._34_chan_pes_en	 0x%08x\n",  value);
	GetValue(reg_list->tsf._38_chan_descramble_en, value);
	ALOGD("reg_list->tsf._38_chan_descramble_en	 0x%08x\n",  value);
	//	reg_list->tsf._3c_chan_idx.chan_idx = 0;
	GetValue(reg_list->tsf._3c_chan_idx, value);
	ALOGD("reg_list->tsf._3c_chan_idx	 0x%08x\n", value);
	GetValue(reg_list->tsf._40_chan_ctrl, value);
	ALOGD("reg_list->tsf._40_chan_ctrl	 0x%08x\n",  value);
	GetValue(reg_list->tsf._44_chan_status, value);
	ALOGD("reg_list->tsf._44_chan_status	 0x%08x\n",  value);
	GetValue(reg_list->tsf._48_chan_cw_idx, value);
	ALOGD("reg_list->tsf._48_chan_cw_idx	 0x%08x\n",  value);
	GetValue(reg_list->tsf._4c_chan_pid, value);
	ALOGD("reg_list->tsf._4c_chan_pid	 0x%08x\n",  value);
	GetValue(reg_list->tsf._50_chan_buf_addr, value);
	ALOGD("reg_list->tsf._50_chan_buf_addr	 0x%08x\n",  value);
	GetValue(reg_list->tsf._54_chan_buf_size, value);
	ALOGD("reg_list->tsf._54_chan_buf_size	 0x%08x\n",  value);
	GetValue(reg_list->tsf._58_chan_wt_pos, value);
	ALOGD("reg_list->tsf._58_chan_wt_pos 0x%08x\n",  value);
	GetValue(reg_list->tsf._5c_chan_rd_pos, value);
	ALOGD("reg_list->tsf._5c_chan_rd_pos 0x%08x\n",  value);

	GetValue(reg_list->tsd._00_ctrl, value);
	ALOGD("reg_list->tsd._00_ctrl 0x%08x\n",  value);
	GetValue(reg_list->tsd._04_status, value);
	ALOGD("reg_list->tsd._04_status 0x%08x\n",  value);
	GetValue(reg_list->tsd._1c_cw_idx, value);
	ALOGD("reg_list->tsd._1c_cw_idx 0x%08x\n",  value);
	GetValue(reg_list->tsd._20_cw, value);
	ALOGD("reg_list->tsd._20_cw 0x%08x\n",  value);
}

int32_t tsf_set_ca_enable(void *handle, uint32_t chan_id, uint32_t en) {
	tsc_ctx_t *context = (tsc_ctx_t *)handle;
	uint32_t enable;
	if (!context) {
		ALOGE("invalid tsc handle");
		return -1;
	}

	ALOGV("%s %d en:%d chan_id:%d\n", __FUNCTION__, __LINE__, en, chan_id);
	tsc_reg_list_t *register_handle = (tsc_reg_list_t *)context->tsc_registers;
	enable = register_handle->tsf._38_chan_descramble_en.descramble_en_ctrl_bits;
	if (en)
		enable |= (1 << chan_id);
	else
		enable &= ~(1 << chan_id);
	register_handle->tsf._38_chan_descramble_en.descramble_en_ctrl_bits = enable;
	return 0;
}

int32_t tsf_set_cw_index(void *handle,  uint32_t chan_id, uint32_t cw_index) {
	tsc_ctx_t *context = (tsc_ctx_t *)handle;
	if (!context) {
		ALOGE("invalid tsc handle");
		return -1;
	}
	tsc_reg_list_t *register_handle = (tsc_reg_list_t *)context->tsc_registers;
	register_handle->tsf._3c_chan_idx.chan_idx = chan_id;
	register_handle->tsf._48_chan_cw_idx.related_ctrl_word_idx = cw_index;
	return 0;
}

//************************* TSD functions **************************//
int32_t tsd_set_ca_type(void *handle, ca_type_e ca_type) {
	tsc_ctx_t *context = (tsc_ctx_t *)handle;
	if (!context) {
		ALOGE("invalid tsc handle");
		return -1;
	}
	tsc_reg_list_t *register_handle = (tsc_reg_list_t *)context->tsc_registers;
	register_handle->tsd._00_ctrl.descramble_method = ca_type;
	return 0;
}

int32_t tsd_set_cw(void *handle, uint32_t cw_index, cw_type_e cw_type, uint32_t cw_value) {
	tsc_ctx_t *context = (tsc_ctx_t *)handle;
	if (!context) {
		ALOGE("invalid tsc handle");
		return -1;
	}
	tsc_reg_list_t *register_handle = (tsc_reg_list_t *)context->tsc_registers;
	register_handle->tsd._1c_cw_idx.cw_idx = cw_index;
	register_handle->tsd._1c_cw_idx.cw_internal_idx = cw_type;
	register_handle->tsd._20_cw.cw_content = cw_value;
	return 0;
}
