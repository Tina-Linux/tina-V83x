//#define LOG_NDEBUG 0
#define LOG_TAG "tsc_dev"
#include <CDX_Debug.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>


#include <pthread.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/prctl.h>

#include <base/dtv_base.h>
#include <demux/tsc.h>
#include "dvb_drv_sun5i.h"
#include "tsc_hal.h"
#include "tsc_dev.h"

extern int MemAdapterOpen(void);
extern void MemAdapterClose(void);
extern void* MemAdapterPalloc(int nSize);
extern void  MemAdapterPfree(void* pMem);
extern void  MemAdapterFlushCache(void* pMem, int nSize);
extern void* MemAdapterGetPhysicAddress(void* pVirtualAddress);
extern void* MemAdapterGetVirtualAddress(void* pPhysicAddress);

static void tsc_dev_dump_register(void *handle, int chan);
static void tsc_dev_dump(void *handle, int chan);

#define DEVICE_NAME         ("/dev/ts0")

//Buffer size is asigned to tsf 54 regitster with 21 bits,
#define MAX_CHANNEL_BUF_SIZE (0x1FF000)
#define NOTIFY_PACKET_NUM    /*(1024)*/(512)
#define RECORDER_BUF_SIZE    (1024 * 188 * 10)

#define ENABLE_INTERRUPT (0)

#define FIRST_LIVE_CHANNEL (0)
#define LAST_LIVE_CHANNEL (TSC_CHAN_NUM-RECORD_DES_CHANNEL_NUM-1-1)
#define FIRST_RECORD_DES_CHANNEL (TSC_CHAN_NUM-RECORD_DES_CHANNEL_NUM-1)
#define LAST_RECORD_DES_CHANNEL (TSC_CHAN_NUM-2)
#define FIRST_RECORD_CHANNEL (TSC_CHAN_NUM-1)
#define LAST_RECORD_CHANNEL (TSC_CHAN_NUM-1)

static void* dvb_rx_maintask(void * arg);

static int32_t tsc_dev_open_channel(void* handle, chan_register_t* chan_register) {

	if(!handle || !chan_register) {
		ALOGE("invalid parameter");
		return -1;
	}

	ALOGV("tsc_dev_open_channel, pid %d", chan_register->pid);
	dvb_rx_t *dvb_rx = (dvb_rx_t*) handle;
	//assert pid
	if (chan_register->pid > PID_ALL || chan_register->pid < 0) {
		ALOGW("invalid pid, open filter fail");
		return -1;
	}

	// 0~26:live, 27~30:record des, 31:record
	uint32_t chan_num_first = FIRST_LIVE_CHANNEL;
	uint32_t chan_num_last = LAST_LIVE_CHANNEL;
	if(chan_register->chan_type == CHAN_TYPE_RECORDE) {
		//only one chan for recoder.
		chan_num_first = FIRST_RECORD_CHANNEL;
		chan_num_last = LAST_RECORD_CHANNEL;
	} else if (chan_register->chan_type == CHAN_TYPE_RECORD_DESCRAMBLE) {
		chan_num_first = FIRST_RECORD_DES_CHANNEL + chan_register->desc_id;
		chan_num_last = FIRST_RECORD_DES_CHANNEL + chan_register->desc_id;
	}

	pthread_mutex_lock(&dvb_rx->tsf.mutex);

	uint32_t chan;
	pid_chan_t *channels = dvb_rx->tsf.channels;
	if (chan_register->chan_type != CHAN_TYPE_RECORD_DESCRAMBLE) {
		// one pid can not be opened in two different channel, here i check.
		for (chan = chan_num_first; chan <= chan_num_last; chan++) {
			if (dvb_rx->tsf.channels[chan].pid == chan_register->pid) {
				if (dvb_rx->tsf.channels[chan].is_opened == 1) {
					ALOGW("filter %d has been opened", chan_register->pid);
					pthread_mutex_unlock(&dvb_rx->tsf.mutex);
					return -1;
				}
			}
		}
	}

	if (dvb_rx->tsf.pcr_channel.is_reserved_chan_opened == 1) {
		if (dvb_rx->tsf.pcr_channel.pid == chan_register->pid) {
			ALOGW("filter %d has been opened", chan_register->pid);
			pthread_mutex_unlock(&dvb_rx->tsf.mutex);
			return -1;
		}
	}

	for (chan = chan_num_first; chan <= chan_num_last; chan++) {
		if (channels[chan].is_opened == 0)
			break;
	}

	if (chan == chan_num_last+1) {
		ALOGW("no channel to open.");
		pthread_mutex_unlock(&dvb_rx->tsf.mutex);
		return -1;
	}

	channels[chan].pid = chan_register->pid;
	channels[chan].callback = chan_register->callback;
	channels[chan].callbackparam = chan_register->callbackparam;

	//open channel
	uint8_t *phy_addr;
	if (chan_register->chan_type == CHAN_TYPE_RECORDE) {
		phy_addr = (uint8_t *)(channels[chan].buf - dvb_rx->tsf.record_buf + dvb_rx->tsf.record_phys_addr);
	} else {
		phy_addr = (uint8_t *)(channels[chan].buf - dvb_rx->tsf.buf + dvb_rx->tsf.phys_addr);
	}
	int32_t ret = tsf_open_chan(dvb_rx->tsc_ctx, channels[chan].pid, // pid.
			channels[chan].buf,
			phy_addr, // buffer for channel.
			channels[chan].buf_size, // the buffer size in bytes.
			TSF_TP_CHAN, // TS packet mode.
			channels[chan].chan, // channel id.
			ENABLE_INTERRUPT);
	if (ret < 0) {
		ALOGW("open chan err");
		pthread_mutex_unlock(&dvb_rx->tsf.mutex);
		return -1;
	}

	channels[chan].is_opened = 1;

	pthread_mutex_unlock(&dvb_rx->tsf.mutex);
	ALOGV("open channel success, pid %d, chan %d", channels[chan].pid, channels[chan].chan);
	return channels[chan].chan;
}

static int32_t tsc_dev_close_channel(void* handle, uint32_t chan) {
	ALOGV("tsc_dev_close_channel, chan %d", chan);

	if(chan >= TSC_CHAN_NUM)
		return -1;

	dvb_rx_t *dvb_rx = (dvb_rx_t*) handle;
#if 0
	tsf_port_e tsf_port = TSF_PORT_0;
	if(chan >= TSC_CHAN_NUM) {
		 tsf_port = TSF_PORT_1;
		 chan -= TSC_CHAN_NUM;
	}
#endif
	pid_chan_t *channels = dvb_rx->tsf.channels;

	pthread_mutex_lock(&dvb_rx->tsf.mutex);

	if (channels[chan].is_opened == 1) {
		channels[chan].is_opened = 0;

		tsf_close_chan(dvb_rx->tsc_ctx, chan);

		channels[chan].pid = 0;
		channels[chan].callback = NULL;
		channels[chan].callbackparam = NULL;
	}

	pthread_mutex_unlock(&dvb_rx->tsf.mutex);

	return 0;
}

static int32_t tsc_dev_open_pcr_detect(void* handle, chan_register_t* chan_register) {
	ALOGV("tsc_dev_open_pcr_detect");
	dvb_rx_t *dvb_rx = (dvb_rx_t*) handle;

	pid_chan_t *channels = dvb_rx->tsf.channels;
	pcr_chan_t *pcr_channel = &dvb_rx->tsf.pcr_channel;

	pthread_mutex_lock(&dvb_rx->tsf.mutex);
	uint32_t chan;
	for (chan = 0; chan < PCR_CHANNEL_INDEX; chan++) {
		if ((channels[chan].pid == chan_register->pid) && (channels[chan].is_opened == 1))
			break;
	}

	if (pcr_channel->is_detect_opened == 1) {
		tsf_close_pcr_detect(dvb_rx->tsc_ctx);
		pcr_channel->is_detect_opened = 0;
	}

	if (pcr_channel->is_reserved_chan_opened == 1) {
		tsf_close_chan(dvb_rx->tsc_ctx, pcr_channel->chan);
		pcr_channel->is_reserved_chan_opened = 0;
	}

	pcr_channel->pid = chan_register->pid;
	pcr_channel->callback = chan_register->callback;
	pcr_channel->callbackparam = chan_register->callbackparam;
	pcr_channel->attach_chan = chan;

	if (chan == PCR_CHANNEL_INDEX) {
		uint8_t *phy_addr = (uint8_t *)(pcr_channel->buf -
				dvb_rx->tsf.buf + dvb_rx->tsf.phys_addr);
		int32_t ret = tsf_open_chan(dvb_rx->tsc_ctx, pcr_channel->pid,
				pcr_channel->buf, phy_addr, pcr_channel->buf_size,
				TSF_TP_CHAN, pcr_channel->chan, ENABLE_INTERRUPT);
		if (ret < 0) {
			ALOGW("open chan fail");
			pthread_mutex_unlock(&dvb_rx->tsf.mutex);
			return -1;
		}
		pcr_channel->is_reserved_chan_opened = 1;
	}

	tsf_open_pcr_detect(dvb_rx->tsc_ctx, chan);
	pcr_channel->is_detect_opened = 1;

	pthread_mutex_unlock(&dvb_rx->tsf.mutex);
	return 0;
}

static void tsc_dev_close_pcr_detect(void* handle) {
	ALOGV("tsc_dev_close_pcr_detect");

	dvb_rx_t *dvb_rx = (dvb_rx_t*) handle;
	pcr_chan_t *pcr_channel = &dvb_rx->tsf.pcr_channel;

	pthread_mutex_lock(&dvb_rx->tsf.mutex);

	if (pcr_channel->is_detect_opened == 1) {
		tsf_close_pcr_detect(dvb_rx->tsc_ctx);
		pcr_channel->is_detect_opened = 0;
	}

	if (pcr_channel->is_reserved_chan_opened == 1) {
		tsf_close_chan(dvb_rx->tsc_ctx, pcr_channel->chan);
		pcr_channel->is_reserved_chan_opened = 0;
	}

	pcr_channel->pid = 0;
	pcr_channel->callback = NULL;
	pcr_channel->callbackparam = NULL;
	pcr_channel->attach_chan = 0;

	pthread_mutex_unlock(&dvb_rx->tsf.mutex);
}

static int32_t open_chan_31(void* handle) {
	int32_t ret;
	dvb_rx_t *dvb_rx = (dvb_rx_t*) handle;

	pthread_mutex_lock(&dvb_rx->tsf.mutex);
	uint8_t *phy_addr = (uint8_t *)(dvb_rx->tsf.channels[31].buf -
			dvb_rx->tsf.buf + dvb_rx->tsf.phys_addr);
	ret = tsf_open_chan(dvb_rx->tsc_ctx, PID_ALL, dvb_rx->tsf.channels[31].buf,
			phy_addr, dvb_rx->tsf.channels[31].buf_size,
			TSF_TP_CHAN, 31, ENABLE_INTERRUPT);

	if (ret < 0) {
		ALOGW("open chan failed");
		pthread_mutex_unlock(&dvb_rx->tsf.mutex);
		return -1;
	}

	pthread_mutex_unlock(&dvb_rx->tsf.mutex);

	return 0;
}

static void close_chan_31(void* handle) {
	dvb_rx_t *dvb_rx = (dvb_rx_t*) handle;
	tsf_close_chan(dvb_rx->tsc_ctx, 31);
}

static int32_t tsc_dev_read_channel_data(void* pdata, uint32_t pkt_num_require, uint32_t chan,
		void* handle) {
	uint8_t* data0;
	uint8_t* data1;
	uint32_t pkt_num0 = 0;
	uint32_t pkt_num1 = 0;
	uint8_t* buf;
	ALOGV("tsc_dev_read_channel_data");
	dvb_rx_t *dvb_rx = (dvb_rx_t*) handle;

	if (chan >=  TSC_CHAN_NUM)
		return 0;

	pthread_mutex_lock(&dvb_rx->tsf.mutex);
	tsf_request_data(dvb_rx->tsc_ctx, chan, &data0, &pkt_num0, &data1, &pkt_num1);
	pthread_mutex_unlock(&dvb_rx->tsf.mutex);

	if(pkt_num0) {
		MemAdapterFlushCache(data0, pkt_num0);
	}
	if(pkt_num1) {
		MemAdapterFlushCache(data1, pkt_num1);
	}
	pkt_num0 /= 188;
	pkt_num1 /= 188;

	if (pkt_num0 + pkt_num1 == 0)
		return 0;

	if (pkt_num_require == 0)
		return 0;

	buf = (uint8_t*) pdata;
	int32_t packet_num = 0;

	pthread_mutex_lock(&dvb_rx->tsf.mutex);
	if (pkt_num0 >= pkt_num_require) {
		memcpy(buf, data0, pkt_num_require * 188);
		tsf_flush_chan(dvb_rx->tsc_ctx, chan, pkt_num_require * 188);
		packet_num = pkt_num_require;
	} else if (pkt_num1 == 0) {
		memcpy(buf, data0, pkt_num0 * 188);
		tsf_flush_chan(dvb_rx->tsc_ctx, chan, pkt_num0 * 188);
		packet_num = pkt_num0;
	} else if (pkt_num0 + pkt_num1 >= pkt_num_require) {
		memcpy(buf, data0, pkt_num0 * 188);
		memcpy(buf + pkt_num0 * 188, data1, (pkt_num_require - pkt_num0) * 188);
		tsf_flush_chan(dvb_rx->tsc_ctx, chan, pkt_num_require * 188);
		packet_num = pkt_num_require;
	} else {
		memcpy(buf, data0, pkt_num0 * 188);
		memcpy(buf + pkt_num0 * 188, data1, pkt_num1 * 188);
		tsf_flush_chan(dvb_rx->tsc_ctx, chan, (pkt_num0 + pkt_num1) * 188);
		packet_num = (pkt_num0 + pkt_num1);
	}
	pthread_mutex_unlock(&dvb_rx->tsf.mutex);
	return packet_num;
}

static int32_t tsc_dev_request_channel_data(void* handle, int32_t chan,
		tsf_data_t* tsf_data) {
//	ALOGV("tsc_dev_request_channel_data, chan %d", chan);
//	tsc_dev_dump(handle, chan);

	if (chan >= TSC_CHAN_NUM) {
		tsf_data->data = NULL;
		tsf_data->pkt_num = 0;
		tsf_data->ring_data = NULL;
		tsf_data->ring_pkt_num = 0;
		return -1;
	}

	dvb_rx_t *dvb_rx = (dvb_rx_t*) handle;
	int32_t err = 0;
	pthread_mutex_lock(&dvb_rx->tsf.mutex);
	if (dvb_rx->tsf.channels[chan].is_opened == 1) {
		tsf_request_data(dvb_rx->tsc_ctx, chan, &tsf_data->data,
				&tsf_data->pkt_num,	&tsf_data->ring_data, &tsf_data->ring_pkt_num);
		if(tsf_data->pkt_num) {
			MemAdapterFlushCache(tsf_data->data, tsf_data->pkt_num);
		}
		if(tsf_data->ring_pkt_num) {
			MemAdapterFlushCache(tsf_data->ring_data, tsf_data->ring_pkt_num);
		}
		tsf_data->pkt_num /= 188; //return as packet size
		tsf_data->ring_pkt_num /= 188;
	} else {
		tsf_data->data = NULL;
		tsf_data->pkt_num = 0;
		tsf_data->ring_data = NULL;
		tsf_data->ring_pkt_num = 0;
		err = -1;
	}
	pthread_mutex_unlock(&dvb_rx->tsf.mutex);
	return err;
}

static int32_t tsc_dev_flush_channel_data(void* handle, int32_t chan, int32_t pkt_num) {
	dvb_rx_t *dvb_rx = (dvb_rx_t*) handle;
	if (chan >=  TSC_CHAN_NUM)
		return -1;
#if 0
	tsf_port_e tsf_port = TSF_PORT_0;
	if(chan >= TSC_CHAN_NUM) {
		 tsf_port = TSF_PORT_1;
		 chan -= TSC_CHAN_NUM;
	}
#endif
	pthread_mutex_lock(&dvb_rx->tsf.mutex);

	if (dvb_rx->tsf.channels[chan].is_opened == 1)
		tsf_flush_chan(dvb_rx->tsc_ctx, chan, pkt_num * 188);

	pthread_mutex_unlock(&dvb_rx->tsf.mutex);

	return 0;
}

static int32_t tsc_dev_get_channel_packet_num(void* handle, int32_t chan) {
	int32_t result;
	dvb_rx_t *dvb_rx = (dvb_rx_t*) handle;
	ALOGV("tsc_dev_get_channel_packet_num");
	if(!dvb_rx)
		return -1;

	if (chan >= TSC_CHAN_NUM)
		return -1;
#if 0
	tsf_port_e tsf_port = TSF_PORT_0;
	if(chan >= TSC_CHAN_NUM) {
		 tsf_port = TSF_PORT_1;
		 chan -= TSC_CHAN_NUM;
	}
#endif
	pthread_mutex_lock(&dvb_rx->tsf.mutex);

	if (dvb_rx->tsf.channels[chan].is_opened == 1) {
		result = tsf_check_data_size(dvb_rx->tsc_ctx, chan);
		result /= 188;
	} else
		result = 0;

	pthread_mutex_unlock(&dvb_rx->tsf.mutex);

	return result;
}

static int32_t tsc_dev_get_free_channel_num(void *handle) {
	ALOGV("tsc_dev_get_free_channel_num");
	dvb_rx_t *dvb_rx = (dvb_rx_t*) handle;
	if(!dvb_rx)
		return TSC_CHAN_NUM;

	int32_t i;
	int32_t free_channel_num = TSC_CHAN_NUM;

	pid_chan_t* channels = &dvb_rx->tsf.channels[0];
	for(i = 0; i < TSC_CHAN_NUM; i ++) {
		if(channels[i].is_opened) {
			free_channel_num --;
		}
	}
	return free_channel_num;
}

static void tsc_dev_reset_channel(void *handle, int chan)
{
	dvb_rx_t *dvb_rx = (dvb_rx_t*) handle;
	if(!dvb_rx)
		return ;

	if (chan >=  TSC_CHAN_NUM)
		return;
#if 0
	tsf_port_e tsf_port = TSF_PORT_0;
	if(chan >= TSC_CHAN_NUM) {
		 tsf_port = TSF_PORT_1;
		 chan -= TSC_CHAN_NUM;
	}
#endif
	pthread_mutex_lock(&dvb_rx->tsf.mutex);
	tsf_reset_chan(dvb_rx->tsc_ctx, chan);
	pthread_mutex_unlock(&dvb_rx->tsf.mutex);
}

static void tsc_dev_dump_register(void *handle, int chan)
{
	dvb_rx_t *dvb_rx = (dvb_rx_t*) handle;
	if(!dvb_rx)
		return ;

	if (chan >= TSC_CHAN_NUM)
		return;
#if 0
	tsf_port_e tsf_port = TSF_PORT_0;
	if(chan >= TSC_CHAN_NUM) {
		 tsf_port = TSF_PORT_1;
		 chan -= TSC_CHAN_NUM;
	}
#endif
	pthread_mutex_lock(&dvb_rx->tsf.mutex);
	tsf_dump_chan_register(dvb_rx->tsc_ctx, chan);
	pthread_mutex_unlock(&dvb_rx->tsf.mutex);
}

static void tsc_dev_dump(void *handle, int chan)
{
	ALOGD("chan=%d", chan);
	dvb_rx_t *dvb_rx = (dvb_rx_t*) handle;
	if(!dvb_rx)
		return ;

	if (chan >= TSC_CHAN_NUM)
		return;
#if 0
	tsf_port_e tsf_port = TSF_PORT_0;
	if(chan >= TSC_CHAN_NUM) {
		 tsf_port = TSF_PORT_1;
		 chan -= TSC_CHAN_NUM;
	}
#endif
	pthread_mutex_lock(&dvb_rx->tsf.mutex);
	ts_dump_registers(dvb_rx->tsc_ctx);
	pthread_mutex_unlock(&dvb_rx->tsf.mutex);
}

static int32_t tsc_desc_set_pid(void *handle, int desc_id, int chan_id, int pid)
{
	dvb_rx_t *dvb_rx = (dvb_rx_t*) handle;
	if(!dvb_rx)
		return -1;

	tsf_set_cw_index(dvb_rx->tsc_ctx,  chan_id, desc_id);
	tsf_set_ca_enable(dvb_rx->tsc_ctx,  chan_id, 1);
	return 0;
}

static int32_t tsc_desc_set_key(void *handle, int desc_id, uint8_t *odd_key, uint8_t *even_key)
{
	dvb_rx_t *dvb_rx = (dvb_rx_t*) handle;
	if(!dvb_rx)
		return -1;

	if (odd_key) {
		tsd_set_cw(dvb_rx->tsc_ctx, desc_id, CW_ODD_LOW_32BITS, odd_key[0] | (odd_key[1]<<8) | (odd_key[2]<<16) | (odd_key[3]<<24));
		tsd_set_cw(dvb_rx->tsc_ctx, desc_id, CW_ODD_HIGH_32BITS, odd_key[4] | (odd_key[5]<<8) | (odd_key[6]<<16) | (odd_key[7]<<24));
	}
	if (even_key) {
		tsd_set_cw(dvb_rx->tsc_ctx, desc_id, CW_EVEN_LOW_32BITS, even_key[0] | (even_key[1]<<8) | (even_key[2]<<16) | (even_key[3]<<24));
		tsd_set_cw(dvb_rx->tsc_ctx, desc_id, CW_EVEN_HIGH_32_BITS, even_key[4] | (even_key[5]<<8) | (even_key[6]<<16) | (even_key[7]<<24));
	}

	return 0;
}

static void* dvb_rx_maintask(void * arg) {
	uint32_t chan;
	uint32_t pcr;
	int32_t err;
	uint32_t chan_status;
	dvb_rx_t* dvb_rx;
	prctl(PR_SET_NAME, (unsigned long)"dvb_rx");

	struct intrstatus interrupt_status;
	dvb_rx = (dvb_rx_t*) arg;

	while (0 == dvb_rx->thread_exit) {
//		ALOGV("waiting interrupt");
		err = ioctl(dvb_rx->fd, TSCDEV_WAIT_INT, (unsigned long)&interrupt_status);
//		print_regs(dvb_rx->tsc_ctx);

		dvb_rx->chan_status |= interrupt_status.port0chan;

		interrupt_status.port0pcr &= 0x00000004; //get PCR bit
		interrupt_status.port0pcr >>= 2;
		if (interrupt_status.port0pcr) {
			if (dvb_rx->tsf.pcr_channel.is_detect_opened == 1) {
				pcr = tsf_get_pcr(dvb_rx->tsc_ctx);
				if (dvb_rx->tsf.pcr_channel.callback)
					dvb_rx->tsf.pcr_channel.callback(&pcr, dvb_rx->tsf.pcr_channel.callbackparam);
			}
		}
		chan_status = dvb_rx->chan_status;
		dvb_rx->chan_status = 0;

        //chan_status &= 0x7fffffff;
		chan = 0;
		while (chan_status) {
			if (chan_status & 0x1) {
				if (dvb_rx->tsf.channels[chan].is_opened == 1) {
					if (dvb_rx->tsf.channels[chan].callback) {
						dvb_rx->tsf.channels[chan].callback(NULL, (void *) dvb_rx->tsf.channels[chan].callbackparam);
					}
				}
			}
			chan_status >>= 1;
			chan++;
		} //while chan_status
	} //while(1)

	pthread_exit(NULL);

	return (void *) 0;
}

static	int tsc_dev_open_descramble(void* handle, int chan)
{
	int index;
	dvb_rx_t *dvb_rx = (dvb_rx_t*) handle;

	if(!handle || chan>=TSC_CHAN_NUM)
	{
		ALOGE("invalid parameter");
		return -1;
	}

	pthread_mutex_lock(&dvb_rx->tsf.mutex);
	if (dvb_rx->tsf.channels[chan].is_opened == 0)
	{
		ALOGW("filter %d has not opened", chan);
		pthread_mutex_unlock(&dvb_rx->tsf.mutex);
		return -1;
	}
	pthread_mutex_unlock(&dvb_rx->tsf.mutex);

	pthread_mutex_lock(&dvb_rx->tsd.mutex);
	for (index = 0; index < DSC_NUM; index++) {
		if (dvb_rx->tsd.dsc[index].is_opened==1 && dvb_rx->tsd.dsc[index].chan==chan)
		{
			ALOGW("filter %d has added descramble %d", chan,index);
			pthread_mutex_unlock(&dvb_rx->tsd.mutex);
			return -1;
		}
	}
	pthread_mutex_unlock(&dvb_rx->tsd.mutex);

	ALOGV("tsc_dev_open_channel, chan %d", chan);

	pthread_mutex_lock(&dvb_rx->tsd.mutex);
	for (index = 0; index < DSC_NUM; index++) {
		if (dvb_rx->tsd.dsc[index].is_opened == 0)
			break;
	}

	if(index > DSC_NUM)
	{
		ALOGW("no free descramble");
		pthread_mutex_unlock(&dvb_rx->tsd.mutex);
		return -1;
	}
	dvb_rx->tsd.dsc[index].is_opened = 1;
	dvb_rx->tsd.dsc[index].chan = chan;

	tsf_set_ca_enable(dvb_rx->tsc_ctx, chan, 1);
	tsf_set_cw_index(dvb_rx->tsc_ctx, chan, index);

	pthread_mutex_unlock(&dvb_rx->tsd.mutex);

	return index;
}

static	int tsc_dev_close_descramble(void* handle, int index)
{
	dvb_rx_t *dvb_rx = (dvb_rx_t*) handle;
	int chan;

	if(!handle || index>=DSC_NUM)
	{
		ALOGE("invalid parameter");
		return -1;
	}

	pthread_mutex_lock(&dvb_rx->tsd.mutex);
	if (dvb_rx->tsd.dsc[index].is_opened == 0)
	{
		ALOGW("descramble %d has not opened", index);
		pthread_mutex_unlock(&dvb_rx->tsd.mutex);
		return -1;
	}

	chan = dvb_rx->tsd.dsc[index].chan;
	if(chan >= TSC_CHAN_NUM) {
		ALOGE("chan num is too large");
		return -1;

	}

	tsf_set_ca_enable(dvb_rx->tsc_ctx, chan, 0);

	dvb_rx->tsd.dsc[index].is_opened = 0;

	pthread_mutex_unlock(&dvb_rx->tsd.mutex);

	return 0;
}

static	int tsc_dev_set_descramble_cw(void* handle, int index, des_cw_type_e cw_type, uint32_t cw_value)
{
	dvb_rx_t *dvb_rx = (dvb_rx_t*) handle;

	if(!handle || index>=DSC_NUM)
	{
		ALOGE("invalid parameter");
		return -1;
	}

	pthread_mutex_lock(&dvb_rx->tsd.mutex);
	if (dvb_rx->tsd.dsc[index].is_opened == 0)
	{
		ALOGW("descramble %d has not opened", index);
		pthread_mutex_unlock(&dvb_rx->tsd.mutex);
		return -1;
	}

	tsd_set_cw(dvb_rx->tsc_ctx, index, cw_type, cw_value);

	pthread_mutex_unlock(&dvb_rx->tsd.mutex);

	return 0;
}

static const uint32_t size_mul_array[4] = { 2, 4, 8, 16 };

void* tsc_dev_open(void) {
	int32_t err;
	uint32_t chan;
	uint32_t channel_buffer_size;
	uint32_t total_buffer_size;
	uint32_t size_multiply;
	dvb_rx_t* dvb_rx;
	ALOGV("tsc_dev_open");

	ts_dev_t *ts_dev = (ts_dev_t *)malloc(sizeof(ts_dev_t));
	if(ts_dev == NULL) {
		ALOGE("alloc ts device failed");
		return NULL;
	}
	// create module handle
	dvb_rx = (dvb_rx_t *) malloc(sizeof(dvb_rx_t));
	if (dvb_rx == NULL) {
		ALOGW("malloc failed.");
		free(ts_dev);
		return NULL;
	}
	memset(dvb_rx, 0, sizeof(dvb_rx_t));

	//open TSC driver
	dvb_rx->fd = open(DEVICE_NAME, O_RDWR, 0);
	if (dvb_rx->fd < 0) {
		ALOGW("open device error, %s", strerror(errno));
		free(dvb_rx);
		free(ts_dev);
		return NULL;
	}

	pthread_mutex_init(&dvb_rx->tsf.mutex, NULL);
	pthread_mutex_init(&dvb_rx->tsd.mutex, NULL);

	// malloc memory for all pid filter.
	size_multiply = size_mul_array[TSF_INTR_THRESHOLD];
	//buffer size is aligned by 1024 bytes
	channel_buffer_size = (NOTIFY_PACKET_NUM * 188 * size_multiply + 0x3ff) & ~0x3ff;
	DTV_ASSERT(channel_buffer_size <= MAX_CHANNEL_BUF_SIZE);

	total_buffer_size = channel_buffer_size * TSC_CHAN_NUM;
	total_buffer_size = (total_buffer_size + 0xfff) & ~0xfff; // PAGE_SIZE aligned
	MemAdapterOpen();
    printf("total_buffer_size = %d bytes\n",total_buffer_size);
	dvb_rx->tsf.buf = MemAdapterPalloc(total_buffer_size);
	if(dvb_rx->tsf.buf == NULL) {
		ALOGE("malloc buffer for tsc failed");
		goto _err_open;
	}
	memset(dvb_rx->tsf.buf, 0, total_buffer_size);
	//MemAdapterFlushCache(dvb_rx->tsf[0].buf,total_buffer_size);
	// get memory physics addresss
	dvb_rx->tsf.phys_addr = (size_t)MemAdapterGetPhysicAddress(dvb_rx->tsf.buf);

	dvb_rx->tsf.total_buf_size = total_buffer_size;

	ALOGV("buf info, virt %p, phys 0x%x, size 0x%x", dvb_rx->tsf.buf, dvb_rx->tsf.phys_addr,
			dvb_rx->tsf.total_buf_size);

	uint8_t *buf = dvb_rx->tsf.buf;
	for (chan = 0; chan < TSC_CHAN_NUM; chan++) {
		dvb_rx->tsf.channels[chan].chan = chan;
		dvb_rx->tsf.channels[chan].buf = buf;
		dvb_rx->tsf.channels[chan].buf_size = NOTIFY_PACKET_NUM * size_multiply
				* 188;
		buf += channel_buffer_size;
	}

	dvb_rx->tsf.pcr_channel.chan = PCR_CHANNEL_INDEX;
	dvb_rx->tsf.pcr_channel.buf = dvb_rx->tsf.channels[PCR_CHANNEL_INDEX].buf;
	dvb_rx->tsf.pcr_channel.buf_size = dvb_rx->tsf.channels[PCR_CHANNEL_INDEX].buf_size;

	int32_t recorder_size = RECORDER_BUF_SIZE & ~0xfff;
    printf("recorder_size = %d bytes\n",recorder_size);
	dvb_rx->tsf.record_buf = MemAdapterPalloc(recorder_size);
	if(dvb_rx->tsf.record_buf == NULL) {
		ALOGE("malloc buffer for record failed");
		goto _err_open;
	}
	// get memory physics addresss
	dvb_rx->tsf.record_phys_addr = (size_t)MemAdapterGetPhysicAddress(dvb_rx->tsf.record_buf);
	dvb_rx->tsf.record_total_buf_size = recorder_size;

	ALOGV("buf info, virt %p, phys 0x%x, size 0x%x", dvb_rx->tsf.record_buf, dvb_rx->tsf.record_phys_addr,
			dvb_rx->tsf.record_total_buf_size);
	//for recoder, use channel 31.
	//dvb_rx->tsf.channels[31].chan = 0;
	dvb_rx->tsf.channels[31].buf = dvb_rx->tsf.record_buf;
	dvb_rx->tsf.channels[31].buf_size = recorder_size;
	dvb_rx->tsf.channels[31].is_opened = 0;

	// mmap registers
	dvb_rx->regs = mmap(NULL, REGS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
			dvb_rx->fd, 0);
	int ret;
	ALOGD("dvb_rx->regs: %p", dvb_rx->regs);

	if (dvb_rx->regs == MAP_FAILED) {
		ALOGE("mmap registers failed");
		goto _err_open;
	}
	dvb_rx->total_regs_size = REGS_SIZE;
	ret = ioctl(dvb_rx->fd, TSCDEV_DISABLE_INT, 0);

	if(ret < 0)
	{
		ALOGE("ioctl diable int failed!");
		//goto _err_open;
	}

#if ENABLE_INTERRUPT
	// crate the main task
	dvb_rx->thread_exit = 0;
	err = pthread_create(&dvb_rx->thread_id, NULL, dvb_rx_maintask,
			(void*) dvb_rx);
	if (0 != err) {
		dvb_rx->thread_exit = 1;
		ALOGW("create maintask fail, tscopen fail.");
		goto _err_open;
	}
#endif
	dvb_rx->tsc_ctx = tsc_init(dvb_rx->regs);
	if (!dvb_rx->tsc_ctx) {
		ALOGE("init tsc failed");
		goto _err_open;
	}

	if (tsc_open((void*)dvb_rx->tsc_ctx) < 0) {
		ALOGE("open tsc failed");
		goto _err_open;
	}


	ts_dev->cookie = dvb_rx;
	ts_dev->open_channel = tsc_dev_open_channel;
	ts_dev->close_channel = tsc_dev_close_channel;
	ts_dev->get_channel_packet_num = tsc_dev_get_channel_packet_num;
	ts_dev->request_channel_data = tsc_dev_request_channel_data;
	ts_dev->flush_channel_data = tsc_dev_flush_channel_data;
	ts_dev->read_channel_data = tsc_dev_read_channel_data;
	ts_dev->open_pcr_detect = tsc_dev_open_pcr_detect;
	ts_dev->close_pcr_detect = tsc_dev_close_pcr_detect;
	ts_dev->get_free_channel_num = tsc_dev_get_free_channel_num;
	ts_dev->reset_channel = tsc_dev_reset_channel;
	ts_dev->dump_register = tsc_dev_dump_register;
	ts_dev->clear = NULL;
	ts_dev->set_buffer_size = NULL;
	ts_dev->write_data      = NULL;
	ts_dev->desc_set_pid    = tsc_desc_set_pid;
	ts_dev->desc_set_key    = tsc_desc_set_key;
	ts_dev->open_descramble = tsc_dev_open_descramble;
	ts_dev->close_descramble = tsc_dev_close_descramble;
	ts_dev->set_descramble_cw = tsc_dev_set_descramble_cw;


	return (void *) ts_dev;

_err_open:
	if (dvb_rx) {
		if (dvb_rx->tsc_ctx) {
			tsc_close(dvb_rx->tsc_ctx);
			tsc_exit(dvb_rx->tsc_ctx);
		}
		// release memory.
		if (dvb_rx->tsf.buf) {
			MemAdapterPfree(dvb_rx->tsf.buf);
		}
		if (dvb_rx->tsf.record_buf) {
			MemAdapterPfree(dvb_rx->tsf.record_buf);
		}
		MemAdapterClose();
		if (dvb_rx->regs) {
			munmap(dvb_rx->regs, dvb_rx->total_regs_size);
		}

		if (dvb_rx->fd) {
			close(dvb_rx->fd);
		}
		// release mutex
		pthread_mutex_destroy(&dvb_rx->tsf.mutex);

		free(dvb_rx);
		dvb_rx = NULL;
		if(ts_dev) {
			free(ts_dev);
		}
	}
	return NULL;
}

int32_t tsc_dev_close(void* handle) {
	ALOGV("tsc_dev_close");
	ts_dev_t *ts_dev = (ts_dev_t *)handle;
	if(ts_dev) {
		dvb_rx_t *dvb_rx = (dvb_rx_t*)ts_dev->cookie;
		if (dvb_rx) {
	#if ENABLE_INTERRUPT
			dvb_rx->thread_exit = 1;
			int32_t err = ioctl(dvb_rx->fd, TSCDEV_RELEASE_SEM, NULL);

			if(err < 0)
			{
				ALOGE("ioctl release sem failed!");
			}

			//kill main task thread
			if (dvb_rx->thread_id > 0)
				err = pthread_join(dvb_rx->thread_id, NULL);
	#endif

			if (dvb_rx->tsc_ctx) {
				tsc_close(dvb_rx->tsc_ctx);
				tsc_exit(dvb_rx->tsc_ctx);
			}

			if (dvb_rx->regs)
				munmap(dvb_rx->regs, dvb_rx->total_regs_size);

			// release memory.
			if (dvb_rx->tsf.buf) {
				MemAdapterPfree(dvb_rx->tsf.buf);
			}
			if(dvb_rx->tsf.record_buf) {
				MemAdapterPfree(dvb_rx->tsf.record_buf);
			}

			MemAdapterClose();

			if (dvb_rx->fd)
				close(dvb_rx->fd);

			pthread_mutex_destroy(&dvb_rx->tsf.mutex);
			free(dvb_rx);
			dvb_rx = NULL;
		}
		free(ts_dev);
	}

	return 0;
}
