//#define LOG_NDEBUG 0
#define LOG_TAG "tsdemux"
#include <CDX_Debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


#include <base/dtv_base.h>
#include <base/dtv_semaphore.h>
#include <demux/tsc.h>
#include <demux/tsdemux.h>

#include "tsdemux_i.h"

typedef struct filter_info {
	uint8_t *buffer;
	int32_t buf_size;
	int32_t fd;
	int32_t pid;
} filter_info;



int32_t open_file(int32_t pid)
{
	char path[32];
	ALOGD("open file");

	sprintf(&path[0], "/data/media/data-%04d.ts", pid);
	int fd = open(&path[0], O_CREAT | O_RDWR | O_TRUNC, 0644);
	if(fd < 0) {
		ALOGE("open file %s failed. err:%s", path, strerror(errno));
	}
	return fd;
}

static inline int64_t get_pts(const uint8_t* p) {
	int64_t pts = (int64_t)((p[0] >> 1) & 0x07) << 30;
	pts |= (AV_RB16(p + 1) >> 1) << 15;
	pts |= AV_RB16(p + 3) >> 1;
	return pts;
}

static int32_t data_arrive_callback(void* param, void *cookie) {

	filter_t *filter = (filter_t *)cookie;
	if(filter) {
		dtv_sem_up(&filter->wait_data_sem);
	}
	return 0;
}

static int32_t pcr_notify(void* param, void *cookie) {
	mpegts_context_t  *context = (mpegts_context_t *)cookie;
	if(context) {
		context->pcrcallback(param, context->pcr_cookie);
	}
	return 0;
}

static int32_t find_start_code_pos(uint8_t *buf, int32_t len) {
	int32_t pos = -1;
	int32_t i;
	for(i = 2; i < len; i ++) {
		if(buf[i] == 1) {
			if(!buf[i -1] && !buf[i - 2]) {
				pos = i - 2;
			}
			break;
		}
	}
	return pos;
}

static int32_t push_es_data(uint8_t *data, uint32_t len, uint32_t new_frm,
		void *param) {
	uint8_t *dataptr;
	int32_t ret;

	dataptr = data;
	es_filter_t *es = (es_filter_t*) param;
	if (new_frm) {
		if (es->valid_size > 0) {
			es->ctrl_bits |= LAST_PART_BIT;

			//send data
			es->data_info.ctrl_bits = es->ctrl_bits;
			es->data_info.data_len = es->valid_size;
			es->updatedatacb(&es->data_info, es->cookie);

			//clear status
			es->free_size = 0;
			es->valid_size = 0;
			es->ctrl_bits = 0;
			es->payload_size = 0;
		}
	}

	while (len > 0) {
		if (es->free_size == 0) {
			//request buffer
			ret = es->requestbufcb(&es->md_buf, es->cookie);
			//ALOGV("request buf ret %d", ret);

			if (ret != 0 || es->md_buf.buf == NULL) {
				if (es->md_buf.buf_size == 1)
					return 1;
				else
					return -1;
			}

			es->cur_ptr = es->md_buf.buf;
			es->free_size = es->md_buf.buf_size;
		}

		if (new_frm) {
			new_frm = 0;
			es->ctrl_bits |= FIRST_PART_BIT;
			if (es->pts != -1) {
				es->ctrl_bits |= PTS_VALID_BIT;
				es->data_info.pts = es->pts*1000 / 90;
			}

			if (es->rap_flag) {
				es->ctrl_bits |= RANDOM_ACCESS_FRAME_BIT;
				es->rap_flag = 0;
			}
		}

		if (es->free_size > len) {
			memcpy(es->cur_ptr, dataptr, len);
			es->free_size -= len;
			es->cur_ptr += len;
			es->valid_size += len;

			if (es->payload_size > 0 && es->valid_size >= es->payload_size) {
				es->ctrl_bits |= LAST_PART_BIT;

				//send data
				es->data_info.ctrl_bits = es->ctrl_bits;
				es->data_info.data_len = es->valid_size;
				es->updatedatacb(&es->data_info, es->cookie);

				es->free_size = 0;
				es->valid_size = 0;
				es->payload_size = 0;
				es->ctrl_bits = FIRST_PART_BIT;
			}
			break;
		} else {
			memcpy(es->cur_ptr, dataptr, es->free_size);
			len -= es->free_size;
			dataptr += es->free_size;

			es->valid_size += es->free_size;

			if (es->payload_size > 0 && es->valid_size >= es->payload_size) {
				es->ctrl_bits |= LAST_PART_BIT;
				es->payload_size = 0;
			} else {
				if (es->payload_size > 0) {
					if (es->valid_size < es->payload_size)
						es->payload_size -= es->valid_size;
					else
						es->payload_size = 0;
				}
			}

			//send data
			es->data_info.ctrl_bits = es->ctrl_bits;
			es->data_info.data_len = es->valid_size;
			es->updatedatacb(&es->data_info, es->cookie);

			es->free_size = 0;
			es->valid_size = 0;
			es->ctrl_bits = 0;
		}
	}

	return 0;
}

static int32_t push_pes_data(uint8_t *data, uint32_t len, uint32_t is_start,
		void *param) {

	pes_filter_t *pes = (pes_filter_t*) param;
	if(!pes->started && !is_start) {
		ALOGD("wating first complete PES packet");
		return 0;
	}

	if (is_start) {
		//ALOGV("new pes, valid size %d", pes->valid_size);
		if (pes->valid_size > 0) {
			pes->ctrl_bits |= LAST_PART_BIT;

			//send data
			pes->data_info.ctrl_bits = pes->ctrl_bits;
			pes->data_info.data_len = pes->valid_size;
			pes->updatedatacb(&pes->data_info, pes->cookie);

			//clear status
			pes->free_size = 0;
			pes->valid_size = 0;
			pes->ctrl_bits = 0;
		}
		if(!pes->started) {
			pes->started = 1;
		}
	}
	int32_t err;
	uint8_t *buf = data;
	while (len > 0) {
		if (pes->free_size == 0) {
			//request buffer
			err = pes->requestbufcb(&pes->md_buf, pes->cookie);

			if (err != 0 || pes->md_buf.buf == NULL) {
				if (pes->md_buf.buf_size == 1)
					return 1;
				else
					return -1;
			}

			pes->cur_ptr = pes->md_buf.buf;
			pes->free_size = pes->md_buf.buf_size;
		}

		if (is_start) {
			is_start = 0;
			pes->ctrl_bits |= FIRST_PART_BIT;
		}

		int32_t copy_size = pes->free_size > len ? len : pes->free_size;
		memcpy(pes->cur_ptr, buf, copy_size);
		pes->free_size -= copy_size;
		pes->cur_ptr += copy_size;
		pes->valid_size += copy_size;
		buf += copy_size;
		len -= copy_size;

		if(pes->free_size <= 0 || pes->valid_size >= pes->total_size) {
			//buffer is not enough, or we get a complete PES packet,
			//or PES packet length is unkown.
			pes->ctrl_bits |= LAST_PART_BIT;
			//send data
			pes->data_info.ctrl_bits = pes->ctrl_bits;
			pes->data_info.data_len = pes->valid_size;
			pes->updatedatacb(&pes->data_info, pes->cookie);

			pes->free_size = 0;
			pes->valid_size = 0;
			if(pes->free_size == 0) {
				//we send a complete PES packet.
				pes->ctrl_bits = FIRST_PART_BIT;
			} else {
				pes->ctrl_bits = 0;
			}
		}

	}
	return 0;
}

static int32_t push_ts_data(uint8_t *data, uint32_t len, uint32_t new_frm,
		void *param) {
	uint8_t *dataptr;
	uint32_t tmp;
	tmp = new_frm;
	dataptr = data;
	ts_filter_t *ts = (ts_filter_t*) param;

	while (len > 0) {
		//request buffer
		ts->requestbufcb(&ts->md_buf, ts->cookie);
		if (ts->md_buf.buf == NULL) {
			return -1;
		}

		ts->cur_ptr = ts->md_buf.buf;
		ts->free_size = ts->md_buf.buf_size;

		if (ts->free_size < len) {
			memcpy(ts->cur_ptr, dataptr, ts->free_size);
			ts->updatedatacb(&ts->free_size, ts->cookie);
			len -= ts->free_size;
			ts->free_size = 0;
			ts->cur_ptr = NULL;
		} else {
			memcpy(ts->cur_ptr, dataptr, len);
			ts->updatedatacb(&len, ts->cookie);
			len = 0;
			ts->free_size = 0;
			ts->cur_ptr = NULL;
		}
	}

	return 0;
}

static int32_t push_section_data(uint8_t *data, uint32_t len,
		uint32_t new_section, void *param) {
	uint8_t *dataptr;

	dataptr = data;

	section_filter_t *tss = (section_filter_t*) param;
	if (new_section) {
		tss->free_size = 0;
		tss->cur_ptr = NULL;
		if (tss->section_h_size > 0) {
			if (tss->end_of_section_reached != 0) {
				//find nes section,
				//but the last section has not closed, discard the parsed datas
				tss->updatedatacb(&tss->section_h_size, tss->cookie);
				tss->free_size = 0;
			} else {
				tss->cur_ptr = tss->md_buf.buf;
				tss->free_size = tss->md_buf.buf_size;
			}
		}

		tss->end_of_section_reached = 0;
		tss->section_index = 0;
		tss->section_h_size = (AV_RB16(dataptr + 1) & 0xfff) + 3;
		if (tss->section_h_size > MAX_SECTION_SIZE) {
			tss->section_h_size = MAX_SECTION_SIZE;
		}
	}

	while (len > 0) {
		if (tss->free_size == 0) {
			//request buffer
			tss->requestbufcb(&tss->md_buf, tss->cookie);
			if (tss->md_buf.buf == NULL) {
				return -1;
			}

			tss->cur_ptr = tss->md_buf.buf;
			tss->free_size = tss->md_buf.buf_size;
		} //if tss->free_size

		if (tss->free_size < len) {
			memcpy(tss->cur_ptr, dataptr, tss->free_size);
			tss->cur_ptr += tss->free_size;
			tss->section_index += tss->free_size;
			len = 0;

			tss->free_size = 0;
			tss->end_of_section_reached = 1;
			tss->updatedatacb(&tss->section_h_size, tss->cookie);
			tss->section_h_size = 0;
		} else {
			memcpy(tss->cur_ptr, dataptr, len);
			tss->free_size -= len;
			tss->cur_ptr += len;
			tss->section_index += len;
			len = 0;
		} //end if else

		if ((tss->section_h_size != 0)
				&& (tss->section_index >= tss->section_h_size)) {
			tss->end_of_section_reached = 1;
			tss->updatedatacb(&tss->section_h_size, tss->cookie);
			tss->free_size = 0;
			tss->section_h_size = 0;
		}
	}

	return 0;
}

static int32_t push_recoder_data(uint8_t *data, uint32_t len, uint32_t new_frm,
		void *param) {

	uint8_t *dataptr = data;
	uint32_t tmp;

	tmp = new_frm;
	recorder_filter_t *recorder = (recorder_filter_t*) param;
	int32_t err = 0;
	int32_t offset = 0;
	while(len > 0) {
		md_buf_t md_buf;
		memset(&md_buf, 0, sizeof(md_buf_t));
		err = recorder->requestbufcb(&md_buf, recorder->cookie);
		if(err || md_buf.buf == NULL) {
			err = -1;
			break;
		}
		if(md_buf.buf_size > len) {
			md_buf.buf_size = len;
		}
		else
		{
			ALOGW("packets len is too long, len %d!", len);
		}

		memcpy(md_buf.buf, dataptr + offset, md_buf.buf_size);

		recorder->updatedatacb(&md_buf.buf_size, recorder->cookie);
		len -= md_buf.buf_size;
		offset += md_buf.buf_size;
		if(len > 0) {
			usleep(10*1000);
		}
	}

	return err;
}

static void handle_payload_by_es(uint8_t *buf, int32_t buf_size, int32_t is_start,
		es_filter_t *es) {
	uint8_t *p;
	int32_t len;
	int32_t code;
	uint32_t first;
	int32_t j;

	first = 0;

	if (is_start) {
		es->state = MPEGTS_HEADER;
		es->data_index = 0;
		es->payload_size = 0;
	}

	p = buf;
	while (buf_size > 0) {
		switch (es->state) {
		case MPEGTS_HEADER:
			len = PES_START_SIZE - es->data_index;

			if (len > buf_size)
				len = buf_size;

			for (j = 0; j < len; j++) {
				es->header[es->data_index++] = *p++;
			}

			buf_size -= len;
			int32_t skip = 1;
			if (es->data_index == PES_START_SIZE) {
				/* we got all the PES or section header. We can now decide */
				if (es->header[0] == 0x00 && es->header[1] == 0x00
						&& es->header[2] == 0x01) {
					//it must be an mpeg2 PES stream
					code = es->header[3] | 0x100;
					if ((code >= 0x1c0 && code <= 0x1df)
							|| (code >= 0x1e0 && code <= 0x1ef)
							|| (code == 0x1bd) || (code == 0x1fd)) {

						es->state = MPEGTS_PESHEADER_FILL;

						//if pes->total_size = 0, it means the byte num followed is umlimited
						es->total_size = AV_RB16(es->header + 4);

						//NOTE: a zero total size means the PES size is unbounded
						if (es->total_size)
							es->total_size += 6;

						es->pes_header_size = es->header[8] + 9;

						if (es->total_size > 6) {
							es->payload_size = es->total_size - es->pes_header_size;
						}
						//because we have encountered stream with pes started not with an es start code,
						//we decide to check start code when the new_frm flag is set.
						//if it is not started with a start code, we clear the new_frm flag and treat the
						//data as part of the previous frame.
						//to prevend from sending data when valid_size is bigger than the payload_size,
						//we need to set the payload size to zero.
						if(es->codec_type == DMX_CODEC_AVC)
							es->payload_size = 0;
						skip = 0;
					}
				}

				if(skip) {
					//not a pes or unsuppored stream id
					es->state = MPEGTS_SKIP;
					continue;
				}
			}
			break;

			/**********************************************/
			/* PES packing parsing */
		case MPEGTS_PESHEADER_FILL:
			len = es->pes_header_size - es->data_index;
			if (len > buf_size)
				len = buf_size;

			for (j = 0; j < len; j++) {
				es->header[es->data_index++] = *p++;
			}
			buf_size -= len;
			if (es->data_index == es->pes_header_size) {
				const uint8_t* r;
				uint32_t flags;

				flags = es->header[7];
				r = es->header + 9;
				es->pts = (int64_t) - 1;

				if ((flags & 0xc0) == 0x80) { //PTS
					es->pts = get_pts(r);
					r += 5;
				} else if ((flags & 0xc0) == 0xc0) { //PTS and DTS
					es->pts = get_pts(r);
					r += 10;
				}
				/* we got the full header. We parse it and get the payload */
				es->state = MPEGTS_PAYLOAD;
				es->is_first = 1;
			}
			break;

		case MPEGTS_PAYLOAD:
			if (es->total_size) {
				len = es->total_size - es->data_index;
				if ((len > buf_size) || (len < 0)) {
					len = buf_size;
				}
			} else {
				len = buf_size;
			}

			if (len > 0) {
				first = es->is_first;
				if(first && es->codec_type == DMX_CODEC_AVC) {
					int pos = find_start_code_pos(p, len);
					if(pos > 0) {
						es->push_es_data(p, pos, 0, es);
						p += pos;
						len -= pos;
					} else if(pos < 0) {
						first = 0;
					} else {
						//pos is 0, this is what we wanted most.
					}
				}
				es->push_es_data(p, len, first, es);
				if(es->codec_type == DMX_CODEC_AVC) {
					if(first) {
						es->is_first = 0;
					}
				} else {
					es->is_first = 0;
				}
				if (es->pts != (int64_t) - 1)
					es->pre_pts = es->pts;

				es->pts = -1;
				return;
			} //if len
			buf_size = 0;
			break;

		case MPEGTS_SKIP:
			buf_size = 0;
			break;
		}
	}
}

static void handle_payload_by_pes(uint8_t* buf, int32_t buf_size,
		int32_t is_start, pes_filter_t* pes) {

	uint8_t *p = buf;

	if(is_start) {
		pes->total_size = AV_RB16(p + 4);
		if(pes->total_size) {
			pes->total_size += 6;
		} else {
			ALOGW("pes length is unkown.");
		}
	}
	pes->push_pes_data(p, buf_size, is_start, pes);
}

static void handle_es_packets(uint8_t* pktdata, uint32_t pkt_num, es_filter_t* es) {
	int32_t i;
	uint8_t* pkt;
	uint8_t* p;
	uint8_t* p_end;
	int32_t is_start;
	int32_t afc;

	pkt = pktdata - 188;
	for (i = pkt_num; i > 0; i--) {
		pkt += 188;

		is_start = pkt[1] & 0x40;

		/* skip adaptation field */
		afc = (pkt[3] >> 4) & 3;
		p = pkt + 4;
		if (afc == 0 || afc == 2) /* reserved value */
			continue;

		if (afc == 3) {
			/* skip adaptation field */
			if (is_start && (p[0] > 0)) {
				es->rap_flag = (p[1] & 0x40) >> 6; //get random_access_indicator
			}

			p += p[0] + 1;
		}

		/* if past the end of packet, ignore */
		p_end = pkt + 188;
		if (p >= p_end)
			continue;

		handle_payload_by_es(p, p_end - p, is_start, es);
	}
}

static void handle_pes_packets(uint8_t* pktdata, uint32_t pkt_num,
		pes_filter_t* pes) {
	int32_t i;
	uint8_t* pkt;
	uint8_t* p;
	uint8_t* p_end;
	int32_t is_start;
	int32_t afc;

	pkt = pktdata - 188;
	for (i = pkt_num; i > 0; i--) {
		pkt += 188;

		is_start = pkt[1] & 0x40;

		/* skip adaptation field */
		afc = (pkt[3] >> 4) & 3;
		p = pkt + 4;
		if (afc == 0 || afc == 2) /* reserved value */
			continue;

		if (afc == 3) {
			p += p[0] + 1;
		}

		/* if past the end of packet, ignore */
		p_end = pkt + 188;
		if (p >= p_end)
			continue;

		handle_payload_by_pes(p, p_end - p, is_start, pes);
	}
}

static void handle_section_packets(uint8_t* pktdata, uint32_t pkt_num,
		section_filter_t* section) {
	uint32_t i;
	uint8_t* pkt;
	uint8_t* p;
	uint8_t* p_end;
	int32_t is_start;
	int32_t afc;
	int32_t cc;
	int32_t cc_ok;
	int32_t len;

	pkt = pktdata - 188;
	for (i = pkt_num; i > 0; i--) {
		pkt += 188;
		is_start = pkt[1] & 0x40;
		if (is_start == 0) {
			if (section->need_start)
				continue;
		}

		/* skip adaptation field */
		afc = (pkt[3] >> 4) & 3;
		p = pkt + 4;
		if (afc == 0 || afc == 2) /* reserved value */
			continue;

		if (afc == 3)
			p += p[0] + 1;

		/* if past the end of packet, ignore */
		p_end = pkt + 188;
		if (p >= p_end)
			continue;

		cc = pkt[3] & 0xf; //get continuity_counter field
		cc_ok = (section->last_cc < 0)
				|| (((section->last_cc + 1) & 0xf) == cc);
		section->last_cc = cc;

		if (is_start) {
			//pointer field present.
			len = *p++;
			if ((p + len) > p_end)
				return;

			if (len && cc_ok && !section->need_start) {
				section->push_section_data(p, len, 0, section);
			}

			p += len;
			if (p < p_end) {
				section->push_section_data(p, p_end - p, is_start, section);
			}
		} else if (cc_ok) {
			section->push_section_data(p, p_end - p, is_start, section);
		}

		if (is_start)
			section->need_start = 0;
	}

	return;
}

static void handle_recorder_packets(uint8_t* pktdata, uint32_t pkt_num,
		recorder_filter_t* recorder) {

	uint8_t *pkt = pktdata - 188;
	int32_t i;
	int32_t err = 0;
	if(recorder->count == 1 && recorder->pid[0] == 8192) {
		//push all ts packet.
		err = recorder->push_data(pkt, 188*pkt_num, 0, recorder);
	}
	else{
		for (i = pkt_num; i > 0; i--) {
			pkt += 188;
			if(err) {
				//ALOGE("error occurs while pushing recorder data.");
				break;
			}
			if(pkt[0] != 0x47) {
				ALOGE("packet not sync, sync bytes 0x%02x, pktnum %d, i %d", pkt[0], pkt_num, i);
				err = -1;
				continue;
			}

			int32_t j;
			int32_t pid = (pkt[1] << 8 | pkt[2]) & 0x1fff;
			for(j = 0; j < recorder->count; j ++) {
				if(recorder->pid[j] == pid) {
					//filter this pid.
					//ALOGD("pushing pid 0x%04x, count %d", pid, recorder->count);
					uint32_t len = 188;
					err = recorder->push_data(pkt, len, 0, recorder);
					break;
				}
			}
		}
	}
}

static void* es_main_task(void* arg) {
	int32_t result;
	tsf_data_t tsf_data;

	filter_t *filter = (filter_t *) arg;
	es_filter_t *es = &filter->u.es_filter;
	int32_t chan = filter->chan;
	ts_dev_t *tsc_handle = filter->tsc_handle;
	ALOGI("thread start, pid %d, stream type %d, codec type %d",
			es->pid, es->stream_type, es->codec_type);
	while (filter->status != DEMUX_STATUS_STOPPED) {
		//wait for data
		pthread_mutex_lock(&filter->mutex);
		if(filter->status == DEMUX_STATUS_STARTED) {
			//get data buffer pointer
			result = tsc_handle->request_channel_data(tsc_handle->cookie, chan, &tsf_data);
			if (result != 0) {
				pthread_mutex_unlock(&filter->mutex);
				break;
			}
			//ALOGD("packet num %d vs %d", tsf_data.pkt_num, tsf_data.ring_pkt_num);
			//handle packets

			if (tsf_data.pkt_num) {
				handle_es_packets(tsf_data.data, tsf_data.pkt_num, es);
			}

			if (tsf_data.ring_pkt_num) {
				handle_es_packets(tsf_data.ring_data, tsf_data.ring_pkt_num, es);
			}
			tsc_handle->flush_channel_data(tsc_handle->cookie, chan,
					tsf_data.pkt_num + tsf_data.ring_pkt_num);
		} else if(filter->status == DEMUX_STATUS_STOPPED) {
			//TODO:nothing to do
		} else {
			pthread_cond_wait(&filter->condition, &filter->mutex);
		}
		pthread_mutex_unlock(&filter->mutex);
		usleep(5 * 1000);
	}
	pthread_exit(NULL);
	return NULL;
}

static void* pes_main_task(void* arg) {

	int32_t result;
	tsf_data_t tsf_data;

	filter_t *filter = (filter_t *) arg;
	pes_filter_t *pes = &filter->u.pes_filter;
	int32_t chan = filter->chan;
	ts_dev_t *tsc_handle = filter->tsc_handle;

	while (filter->status != DEMUX_STATUS_STOPPED) {
		//get data buffer pointer
		result = tsc_handle->request_channel_data(tsc_handle->cookie, chan, &tsf_data);
		ALOGD("###result: %d", result);
		if (result != 0)
			break;
		//handle packets
		if (tsf_data.pkt_num) {
			handle_pes_packets(tsf_data.data, tsf_data.pkt_num, pes);
		}
		if (tsf_data.ring_pkt_num) {
			handle_pes_packets(tsf_data.ring_data, tsf_data.ring_pkt_num, pes);
		}

		tsc_handle->flush_channel_data(tsc_handle->cookie, chan,
				tsf_data.pkt_num + tsf_data.ring_pkt_num);
		usleep(10 * 1000);
	}
	pthread_exit(NULL);
	return NULL;
}

static void* ts_main_task(void *arg) {
	int32_t result;
	tsf_data_t tsf_data;

	filter_t *filter = (filter_t *) arg;
	ts_filter_t *ts = &filter->u.ts_filter;
	int32_t chan = filter->chan;
	ts_dev_t *tsc_handle = filter->tsc_handle;

	while (filter->status != DEMUX_STATUS_STOPPED) {
		//get data buffer pointer
		result = tsc_handle->request_channel_data(tsc_handle->cookie, chan, &tsf_data);
		if (result != 0)
			break;

		//handle packets
		if (tsf_data.pkt_num) {
			ts->push_ts_data(tsf_data.data, tsf_data.pkt_num * 188, 0,
					(void*) ts);
		}
		if (tsf_data.ring_pkt_num) {
			ts->push_ts_data(tsf_data.ring_data, tsf_data.ring_pkt_num * 188, 0,
					(void*) ts);
		}
		tsc_handle->flush_channel_data(tsc_handle->cookie, chan,
				tsf_data.pkt_num + tsf_data.ring_pkt_num);
		usleep(5 * 1000);
	}
	pthread_exit(NULL);
	return NULL;
}

static void* section_main_task(void* arg) {
	int32_t result;
	tsf_data_t tsf_data;

	filter_t *filter = (filter_t *) arg;
	section_filter_t *section = &filter->u.section_filter;
	int32_t chan = filter->chan;
	ts_dev_t *tsc_handle = filter->tsc_handle;

	section->need_start = 1;
	section->end_of_section_reached = 0;
	while (filter->status != DEMUX_STATUS_STOPPED) {
		//get data buffer pointer
		result = tsc_handle->request_channel_data(tsc_handle->cookie, chan, &tsf_data);
		if (result != 0)
			break;
		//handle packets
		if (tsf_data.pkt_num) {
			handle_section_packets(tsf_data.data, tsf_data.pkt_num, section);
		}
		if (tsf_data.ring_pkt_num) {
			handle_section_packets(tsf_data.ring_data, tsf_data.ring_pkt_num,
					section);

		}
		tsc_handle->flush_channel_data(tsc_handle->cookie, chan,
				tsf_data.pkt_num + tsf_data.ring_pkt_num);

		usleep(20*1000);
	}
	pthread_exit(NULL);
	return NULL;
}

static void* recorder_main_task(void* arg) {
	ALOGV("recorder_main_task");
	tsf_data_t tsf_data;
	recorder_filter_t *recorder = (recorder_filter_t *)arg;
	ts_dev_t *tsc_handle = recorder->tsc_handle;
	int32_t chan = recorder->chan;

	memset(&tsf_data, 0, sizeof(tsf_data_t));//add by xhw

	while (recorder->thread_exit == 0) {
		//get data buffer pointer
		 int32_t result = tsc_handle->request_channel_data(tsc_handle->cookie, chan, &tsf_data);
		if (result != 0)
			break;
		//handle packets
		if (tsf_data.pkt_num) {
			handle_recorder_packets(tsf_data.data, tsf_data.pkt_num, (void*) recorder);
		}
		if (tsf_data.ring_pkt_num) {
			handle_recorder_packets(tsf_data.ring_data, tsf_data.ring_pkt_num, (void*) recorder);
		}

		tsc_handle->flush_channel_data(tsc_handle->cookie, chan,
				tsf_data.pkt_num + tsf_data.ring_pkt_num);
		usleep(1 * 1000);
	}

	pthread_exit(NULL);
	return NULL;
}

static int32_t allocate_filter(int32_t pid, demux_filter_param_t* filter_param,
		mpegts_context_t* ctx) {
	//int32_t err;
	filter_t* filter;
	es_filter_t* es;
	pes_filter_t* pes;
	ts_filter_t* ts;
	section_filter_t* section;

	pthread_mutex_lock(&ctx->mutex);

	if (ctx->pids[pid]) {
		//pid aready exists
		ALOGE("pid %d already opend", pid);
		pthread_mutex_unlock(&ctx->mutex);
		return -1;
	}

	filter = (filter_t*) malloc(sizeof(filter_t));
	if (NULL == filter) {
		pthread_mutex_unlock(&ctx->mutex);
		return -1;
	}
	memset(filter, 0, sizeof(filter_t));

	dtv_sem_init(&filter->wait_data_sem, 0);
	//for TS and section, we start immediately.
	filter->status = DEMUX_STATUS_STARTED;
	if (filter_param->filter_type == DMX_FILTER_TYPE_ES) {
		es = &filter->u.es_filter;
		es->pid = pid;
		es->requestbufcb = filter_param->request_buffer_cb;
		es->updatedatacb = filter_param->update_data_cb;
		es->push_es_data = push_es_data;
		es->stream_type = filter_param->stream_type;
		es->codec_type = filter_param->codec_type;
		es->cookie = filter_param->cookie;
		filter->status = DEMUX_STATUS_IDLE;
	} else if (filter_param->filter_type == DMX_FILTER_TYPE_PES) {
		pes = &filter->u.pes_filter;
		pes->pid = pid;
		pes->requestbufcb = filter_param->request_buffer_cb;
		pes->updatedatacb = filter_param->update_data_cb;
		pes->push_pes_data = push_pes_data;
		pes->cookie = filter_param->cookie;
	} else if (filter_param->filter_type == DMX_FILTER_TYPE_TS) {
		ts = &filter->u.ts_filter;
		ts->pid = pid;
		ts->requestbufcb = filter_param->request_buffer_cb;
		ts->updatedatacb = filter_param->update_data_cb;
		ts->push_ts_data = (push_data_cb)push_ts_data;
		ts->cookie = filter_param->cookie;
	} else {
		section = &filter->u.section_filter;
		section->last_cc = -1;
		section->pid = pid;
		section->requestbufcb = filter_param->request_buffer_cb;
		section->updatedatacb = filter_param->update_data_cb;
		section->push_section_data = push_section_data;
		section->cookie = filter_param->cookie;
	}
	pthread_cond_init(&filter->condition, NULL);
	pthread_mutex_init(&filter->mutex, NULL);

	filter->pid = pid;
	filter->type = filter_param->filter_type;
	filter->tsc_handle = ctx->tsc_handle;

	ctx->pids[pid] = filter;
	pthread_mutex_unlock(&ctx->mutex);

	return 0;
}

static int32_t free_filter(int32_t pid, mpegts_context_t* ctx) {
	//int32_t err;
	filter_t* filter;

	pthread_mutex_lock(&ctx->mutex);
	filter = ctx->pids[pid];
	if (filter == NULL) {
		pthread_mutex_unlock(&ctx->mutex);
		return -1;
	}
	pthread_cond_destroy(&filter->condition);
	pthread_mutex_destroy(&filter->mutex);
	dtv_sem_deinit(&filter->wait_data_sem);
	free(filter);
	ctx->pids[pid] = NULL;

	pthread_mutex_unlock(&ctx->mutex);

	return 0;
}

static filter_t* open_filter(int32_t pid, demux_filter_param_t* filter_param,
		mpegts_context_t* ctx) {
	int32_t err;
	int32_t result;
	int32_t chan;
	filter_t* filter;
	chan_register_t chan_register;

	result = allocate_filter(pid, filter_param, ctx);
	if (result < 0) {
		ALOGE("allocate filter fail.");
		return NULL;
	}

	filter = ctx->pids[pid];

	chan_register.pid = filter->pid;
	chan_register.callback = (ptsc_callback_t)data_arrive_callback;
	chan_register.callbackparam = (void*)filter;
	chan_register.chan_type = CHAN_TYPE_LIVE;
	if(filter_param->stream_type == DMX_STREAM_VIDEO)
		chan_register.stream_type = TS_STREAM_VIDEO;
	else if(filter_param->stream_type == DMX_STREAM_AUDIO)
		chan_register.stream_type = TS_STREAM_AUDIO;
	else if(filter_param->stream_type == DMX_STREAM_SUBTITLE)
		chan_register.stream_type = TS_STREAM_SUBTITLE;
	else if(filter_param->stream_type == DMX_STREAM_SECTION)
		chan_register.stream_type = TS_STREAM_SECTION;
	else
		chan_register.stream_type = TS_STREAM_UNKOWN;

	chan = filter->tsc_handle->open_channel(filter->tsc_handle->cookie, &chan_register);
	if (chan < 0) {
		ALOGE("open tsc pid channel fail, pid = %d.", pid);
		return NULL;
	}

	filter->chan = chan;

	if (filter_param->filter_type == DMX_FILTER_TYPE_ES)
		err = pthread_create(&filter->thread_id, NULL, es_main_task,
				(void*) filter);
	else if (filter_param->filter_type == DMX_FILTER_TYPE_PES)
		err = pthread_create(&filter->thread_id, NULL, pes_main_task,
				(void*) filter);
	else if (filter_param->filter_type == DMX_FILTER_TYPE_TS)
		err = pthread_create(&filter->thread_id, NULL, ts_main_task,
				(void*) filter);
	else
		err = pthread_create(&filter->thread_id, NULL, section_main_task,
				(void*) filter);

	if (err != 0) {
		filter->status = DEMUX_STATUS_STOPPED;
		ALOGE("create thread fail.");
		filter->tsc_handle->close_channel(filter->tsc_handle->cookie, filter->chan);
		return NULL;
	}

	return filter;
}

static void close_filter(filter_t* filter, mpegts_context_t* ctx) {
	int32_t err;
	ALOGV("close filter");
	pthread_mutex_lock(&filter->mutex);
	filter->status = DEMUX_STATUS_STOPPED;
	pthread_cond_signal(&filter->condition);
	pthread_mutex_unlock(&filter->mutex);

	if (filter->thread_id > 0) {
		dtv_sem_up(&filter->wait_data_sem);
		err = pthread_join(filter->thread_id, NULL);
		ALOGV("chan %d, thread exit", filter->chan);
	}

	filter->tsc_handle->close_channel(filter->tsc_handle->cookie, filter->chan);
	free_filter(filter->pid, ctx);

	return;
}

void* ts_demux_open(int type) {
	//int32_t err;
	mpegts_context_t* mp;
	ALOGV("ts_demux_open");
	mp = (mpegts_context_t*) malloc(sizeof(mpegts_context_t));
	if (mp == NULL)
		return NULL;

	memset(mp, 0, sizeof(mpegts_context_t));

	pthread_mutex_init(&mp->mutex, NULL);
	if(type == DEMUX_TYPE_LIVE_AND_RECORDE) {
		mp->tsc_handle = tsc_dev_open();
	} else if(type == DEMUX_TYPE_INJECT){
		mp->tsc_handle = ts_parser_open();
	}
	if (mp->tsc_handle == NULL) {
		ALOGE("open tsc fail.");
		pthread_mutex_destroy(&mp->mutex);
		free(mp);
		return NULL;
	}
	mp->demux_type = type;
	return (void*) mp;
}

int32_t ts_demux_close(void* handle) {
	int32_t i;
	ALOGV("ts_demux_close");

	mpegts_context_t *mp = (mpegts_context_t*) handle;
	for (i = 0; i < NB_PID_MAX; i++) {
		if (mp->pids[i]) {
			close_filter(mp->pids[i], mp);
			mp->pids[i] = NULL;
		}
	}

	if (mp->detectpcr)
		ts_demux_close_pcr_filter(handle);

	if(mp->demux_type == DEMUX_TYPE_LIVE_AND_RECORDE) {
		tsc_dev_close(mp->tsc_handle);
	} else {
		ts_parser_close(mp->tsc_handle);
	}
	pthread_mutex_destroy(&mp->mutex);
	free(mp);

	return 0;
}

int32_t ts_demux_open_filter(void* handle, int32_t pid,
		demux_filter_param_t* filter_param) {
	mpegts_context_t* mp;
	ALOGV("open filter, pid %d", pid);
	mp = (mpegts_context_t*) handle;

	open_filter(pid, filter_param, mp);

	if (mp->pids[pid] != NULL)
		return 0;
	else
		return -1;
}

int32_t ts_demux_close_filter(void* handle, int32_t pid) {
	mpegts_context_t* mp;
	ALOGV("close filter, pid:%d", pid);
	mp = (mpegts_context_t*) handle;
	if (mp->pids[pid]) {
		close_filter(mp->pids[pid], mp);
		mp->pids[pid] = NULL;
	}

	return 0;
}

int32_t ts_demux_open_pcr_filter(void* handle, int32_t pid,
		pdemux_callback_t callback, void* cookie) {
	mpegts_context_t* mp;

	mp = (mpegts_context_t*) handle;

	pthread_mutex_lock(&mp->mutex);
	if (mp->detectpcr)
		ts_demux_close_pcr_filter(handle);

	mp->detectpcr = 1;
	mp->pcrid = pid;
	mp->pcrcallback = callback;
	mp->pcr_cookie = cookie;

	chan_register_t chan_register;
	chan_register.pid = mp->pcrid;
	chan_register.callback = (ptsc_callback_t)pcr_notify;
	chan_register.callbackparam = mp;
	mp->tsc_handle->open_pcr_detect(mp->tsc_handle->cookie, &chan_register);
	pthread_mutex_unlock(&mp->mutex);

	return 0;
}

int32_t ts_demux_close_pcr_filter(void *handle) {
	mpegts_context_t *mp = (mpegts_context_t*) handle;
	pthread_mutex_lock(&mp->mutex);
	if (mp->detectpcr) {
		mp->tsc_handle->close_pcr_detect(mp->tsc_handle->cookie);
		mp->detectpcr = 0;
		mp->pcrid = 0;
		mp->pcrcallback = NULL;
		mp->pcr_cookie = NULL;
	}
	pthread_mutex_unlock(&mp->mutex);

	return 0;
}

int ts_demux_open_recorder(void *handle, demux_recoder_param_t *param)
{
	ALOGV("ts_demux_open_recoder");
	mpegts_context_t *mp = (mpegts_context_t*) handle;
	if(!mp || !param) {
		return -1;
	}
	if(param->count <= 0) {
		ALOGE("stream count is %d", param->count);
	}
	pthread_mutex_lock(&mp->mutex);
	if(mp->recorder) {
		ALOGE("recorder has started");

		pthread_mutex_unlock(&mp->mutex);
		return -1;
	}
	recorder_filter_t *recorder = (recorder_filter_t *)malloc(sizeof(recorder_filter_t));
	if(recorder == NULL) {
		ALOGE("malloc recorder filter failed");
		free(recorder);
		pthread_mutex_unlock(&mp->mutex);
		return -1;
	}
	recorder->tsc_handle = mp->tsc_handle;
	chan_register_t chan_register;
	chan_register.chan_type = CHAN_TYPE_RECORDE;
	chan_register.pid = 8192;
	chan_register.callback = (ptsc_callback_t)data_arrive_callback;
	chan_register.stream_type = TS_STREAM_UNKOWN;

	int32_t chan = mp->tsc_handle->open_channel(mp->tsc_handle->cookie, &chan_register);
	if (chan < 0) {
		ALOGE("open tsc pid channel for recorder fail");
		free(recorder);
		pthread_mutex_unlock(&mp->mutex);
		return -1;
	}
	recorder->chan = chan;

	recorder->cookie		= param->cookie;
	recorder->requestbufcb	= param->request_buffer_cb;
	recorder->updatedatacb	= param->update_data_cb;
	recorder->push_data		= (push_data_cb)push_recoder_data;
	recorder->count			= param->count;
	mp->recorder			= recorder;
	memcpy(&recorder->pid[0], param->pids, sizeof(int) * param->count);

	recorder->thread_exit = 0;
	int32_t err = pthread_create(&recorder->thread_id, NULL, recorder_main_task,
				(void*) recorder);

	if (err != 0) {
		recorder->thread_exit = 1;
		ALOGE("create thread fail.");
		mp->tsc_handle->close_channel(mp->tsc_handle->cookie, recorder->chan);

		free(recorder);
		mp->recorder = NULL;

		pthread_mutex_unlock(&mp->mutex);
		return -1;
	}
	ALOGI("recording pid num %d", recorder->count);

	pthread_mutex_unlock(&mp->mutex);
	return 0;
}

int ts_demux_close_recorder(void *handle)
{
	ALOGV("ts_demux_close_recoder");
	mpegts_context_t *mp = (mpegts_context_t*) handle;
	if(!mp) {
		return 0;
	}
	pthread_mutex_lock(&mp->mutex);
	recorder_filter_t *recorder = mp->recorder;
	if(recorder) {
		if (recorder->thread_id > 0 && recorder->thread_exit == 0) {
			recorder->thread_exit = 1;
			int32_t err = pthread_join(recorder->thread_id, NULL);
			ALOGV("chan %d, thread exit", recorder->chan);
		}

		mp->tsc_handle->close_channel(mp->tsc_handle->cookie, recorder->chan);

		free(recorder);
		mp->recorder = NULL;
	}
	pthread_mutex_unlock(&mp->mutex);
	return 0;
}

int ts_demux_clear(void *handle)
{
	ALOGV("ts_demux_clear");
	mpegts_context_t *mp = (mpegts_context_t*) handle;
	if(!mp) {
		return -1;
	}
	int32_t i;
	pthread_mutex_lock(&mp->mutex);
	if(mp->tsc_handle->clear) {
		mp->tsc_handle->clear(mp->tsc_handle->cookie);
	}
	for(i = 0; i < NB_PID_MAX; i ++) {
		filter_t *filter = mp->pids[i];
		if(filter && filter->type == DMX_FILTER_TYPE_ES) {
			pthread_mutex_lock(&filter->mutex);
			es_filter_t *es = &filter->u.es_filter;
			es->cur_ptr = 0;
			es->free_size = 0;
			es->valid_size = 0;
			es->ctrl_bits = 0;
			es->rap_flag = 0;
			es->state = MPEGTS_HEADER;

			es->is_first = 0;
			es->pre_pts = -1;
			es->pts = -1;
			es->data_index = 0;
			es->total_size = 0;
			es->payload_size = 0;
			es->pes_header_size = 0;
			pthread_mutex_unlock(&filter->mutex);
		}
	}
	pthread_mutex_unlock(&mp->mutex);
	return 0;
}

int ts_demux_start(void *handle)
{
	ALOGV("ts_demux_start");
	mpegts_context_t *mp = (mpegts_context_t*) handle;
	if(!mp) {
		return -1;
	}
	int32_t i;
	pthread_mutex_lock(&mp->mutex);
	for(i = 0; i < NB_PID_MAX; i ++) {
		filter_t *filter = mp->pids[i];
		if(filter && (filter->type == DMX_FILTER_TYPE_ES)) {
			pthread_mutex_lock(&filter->mutex);
			mp->tsc_handle->reset_channel(mp->tsc_handle->cookie, filter->chan);
			filter->status = DEMUX_STATUS_STARTED;
			pthread_cond_signal(&filter->condition);
			pthread_mutex_unlock(&filter->mutex);
		}
	}
	pthread_mutex_unlock(&mp->mutex);
	return 0;
}

int ts_demux_pause(void *handle)
{
	ALOGV("ts_demux_pause");
	mpegts_context_t *mp = (mpegts_context_t*) handle;
	if(!mp) {
		return -1;
	}
	int32_t i;
	pthread_mutex_lock(&mp->mutex);
	for(i = 0; i < NB_PID_MAX; i ++) {
		filter_t *filter = mp->pids[i];
		if(filter && (filter->type == DMX_FILTER_TYPE_ES)) {
			ALOGI("pause filter %d", i);
			pthread_mutex_lock(&filter->mutex);
			filter->status = DEMUX_STATUS_PAUSED;
			pthread_mutex_unlock(&filter->mutex);
		}
	}
	pthread_mutex_unlock(&mp->mutex);
	return 0;
}

int32_t ts_demux_get_free_filter_num(void* handle) {
	mpegts_context_t* mp = (mpegts_context_t*) handle;
	if(mp) {
		mp->tsc_handle->get_free_channel_num(mp->tsc_handle->cookie);
	}
	return 32;
}

int32_t ts_demux_get_ts_packet_num(void* handle) {
	mpegts_context_t* mp = (mpegts_context_t*) handle;
	int32_t packet_num = 0;
	if(mp) {
		int32_t i;
		for (i = 0; i < NB_PID_MAX; i++) {
			if (mp->pids[i]) {
				packet_num += mp->tsc_handle->get_channel_packet_num(mp->tsc_handle->cookie, mp->pids[i]->chan);
			}
		}
	}

	return packet_num;
}

int ts_demux_set_buffer_size(void *handle, int size)
{
	ALOGV("ts_demux_set_buffer_size");
	mpegts_context_t* mp = (mpegts_context_t*) handle;
	if(mp == NULL) {
		ALOGE("invalid handle");
		return -1;
	}
	if(mp->demux_type == DEMUX_TYPE_INJECT) {
		mp->tsc_handle->set_buffer_size(mp->tsc_handle->cookie, size);
	}
	return 0;
}

int ts_demux_write_data(void *handle, void *buf, int size)
{
	mpegts_context_t* mp = (mpegts_context_t*) handle;
	if(mp == NULL) {
		ALOGE("invalid handle");
		return -1;
	}
	if(mp->demux_type == DEMUX_TYPE_INJECT) {
		return mp->tsc_handle->write_data(mp->tsc_handle->cookie,
				buf, size);
	}
	return 0;
}

int ts_demux_open_descramble(void* handle, int pid)
{
	int32_t i;
	int32_t ret = -1;
	int32_t chan = -1;
	int32_t index = -1;
	filter_t *filter = NULL;
	mpegts_context_t* mp = (mpegts_context_t*) handle;
	if(mp == NULL) {
		ALOGE("invalid handle");
		return -1;
	}

	pthread_mutex_lock(&mp->mutex);
	filter = mp->pids[pid];
	if(filter && (filter->pid == pid))
	{
		chan = filter->chan;
	}

	for(i=0; i<MAX_DESCRAMBLE; i++)
	{
		if(mp->descramble[i].is_open && mp->descramble[i].pid==pid)
		{
			ALOGE("pid %d descramble have alloc\n",pid);
			index = -1;
			break;
		}
		else if(!mp->descramble[i].is_open && index < 0)
		{
			index = i;
		}
	}
	pthread_mutex_unlock(&mp->mutex);

	if(chan < 0)
	{
		ALOGE("pid %d filter have not alloc\n",pid);
		return -1;
	}
	if(index < 0)
	{
		ALOGE("alloc descramble is failed\n");
		return -1;
	}

	ALOGV("ts_demux_open_descramble in, pid 0x%x\n",pid);

	pthread_mutex_lock(&mp->mutex);
	mp->descramble[index].chan = chan;
	mp->descramble[index].pid = pid;
	mp->descramble[index].des_indx = mp->tsc_handle->open_descramble(mp->tsc_handle->cookie, chan);
	if(mp->descramble[index].des_indx >= 0)
	{
		mp->descramble[index].is_open = 1;
		ret = index;
	}
	pthread_mutex_unlock(&mp->mutex);

	return ret;
}

int ts_demux_close_descramble(void* handle, int index)
{
	int ret = -1;
	mpegts_context_t* mp = (mpegts_context_t*) handle;
	if(mp == NULL) {
		ALOGE("invalid handle");
		return -1;
	}

	if(index >= MAX_DESCRAMBLE || index < 0)
	{
		ALOGE("invalid index %d\n",index);
		return -1;
	}

	pthread_mutex_lock(&mp->mutex);
	if(mp->descramble[index].is_open != 1)
	{
		pthread_mutex_unlock(&mp->mutex);
		ALOGE("invalid index %d\n",index);
		return -1;
	}
	ret = mp->tsc_handle->close_descramble(mp->tsc_handle->cookie, index);
	if(ret < 0)
	{
		ALOGE("close_descramble failed, ret %d\n",ret);
	}
	mp->descramble[index].is_open = 0;
	pthread_mutex_unlock(&mp->mutex);

	return ret;
}

int ts_demux_set_descramble_cw(void* handle, int index, ts_demux_cw_type_e cw_type, unsigned int cw_value)
{
	int ret = -1;
	mpegts_context_t* mp = (mpegts_context_t*) handle;
	if(mp == NULL) {
		ALOGE("invalid handle");
		return -1;
	}

	if(index >= MAX_DESCRAMBLE || index < 0)
	{
		ALOGE("invalid index %d\n",index);
		return -1;
	}

	pthread_mutex_lock(&mp->mutex);
	if(mp->descramble[index].is_open != 1)
	{
		pthread_mutex_unlock(&mp->mutex);
		ALOGE("invalid index %d\n",index);
		return -1;
	}

	ret = mp->tsc_handle->set_descramble_cw(mp->tsc_handle->cookie, index, cw_type, cw_value);

	if(ret < 0)
	{
		ALOGE("close_descramble failed, ret %d\n",ret);
	}

	pthread_mutex_unlock(&mp->mutex);

	return ret;
}
