#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../isp.h"
#include "../../include/device/isp_dev.h"
#include "../../include/device/video.h"
#include "../log_handle.h"
#include "../thread_pool.h"

#include "capture_image.h"
#include "raw_flow_opt.h"


#define CAPTURE_CHANNEL_MAX     HW_VIDEO_DEVICE_NUM
#define CAPTURE_RAW_FLOW_QUEUE_SIZE   201


struct hw_isp_media_dev *g_media_dev = NULL;
int                      g_sensor_set_flag;  // bit0-CAPTURE_CHANNEL_MAX-1 set flag vich 0~CAPTURE_CHANNEL_MAX-1

typedef struct _frame_buffer_info_s
{
	unsigned int	u32PoolId;
	unsigned int	u32Width;
	unsigned int	u32Height;
	unsigned int	u32PixelFormat;
	unsigned int	u32field;

	unsigned int	u32PhyAddr[3];
	void			*pVirAddr[3];
	unsigned int	u32Stride[3];

	struct timeval stTimeStamp;
} frame_buf_info;

typedef struct _vi_priv_attr_s
{
	int vich;
	int timeout;
	struct video_fmt vi_fmt;
	frame_buf_info vi_frame_info;
} vi_priv_attr;

typedef enum _cap_flag_e {
	CAP_STATUS_ON            = 0x0001,  // 0:cap video off, 1:cap video on
	CAP_THREAD_RUNNING       = 0x0002,  // 0:thread stopped, 1:thread running
	CAP_THREAD_STOP          = 0x0004,  // 0: not stop thread, 1:to stop thread
	CAP_RAW_FLOW_RUNNING     = 0x0008,  // 0: raw flow stopped, 1:raw flow running
	CAP_RAW_FLOW_START       = 0x0010,  // 0: stop raw flow, 1:to start raw flow
} cap_flag;

typedef struct _capture_params_s {
	vi_priv_attr             priv_cap;
	unsigned int             status;
	int                      frame_count;
	pthread_mutex_t          locker;
} capture_params;

typedef enum _cap_init_status_e {
	CAPTURE_INIT_NOT         = 0,
	CAPTURE_INIT_YES         = 1,
} cap_init_status;

cap_init_status              g_cap_init_status = CAPTURE_INIT_NOT;
capture_params               g_cap_handle[CAPTURE_CHANNEL_MAX];
pthread_mutex_t              g_cap_locker;

static void reset_cap_params(capture_params *cap_pa)
{
	if (cap_pa) {
		memset(&cap_pa->priv_cap, 0, sizeof(vi_priv_attr));
		cap_pa->status = 0;
		cap_pa->frame_count = 0;
	}
}

#define VALID_VIDEO_SEL(id)    \
	((id) >= 0 && (id) < CAPTURE_CHANNEL_MAX)

static int init_video(struct hw_isp_media_dev *media_dev, int vich)
{
	return isp_video_open(media_dev, vich);
}

static int exit_video(struct hw_isp_media_dev *media_dev, int vich)
{
	isp_video_close(media_dev, vich);
	return 0;
}

static int set_video_fmt(struct hw_isp_media_dev *media_dev, int vich, struct video_fmt *vi_fmt)
{
	struct isp_video_device *video = NULL;

	if (!VALID_VIDEO_SEL(vich) || NULL == media_dev->video_dev[vich]) {
		LOG("%s: invalid vin ch(%d)\n", __FUNCTION__, vich);
		return -1;
	} else {
		video = media_dev->video_dev[vich];
	}

	return video_set_fmt(video, vi_fmt);
}

static int get_video_fmt(struct hw_isp_media_dev *media_dev, int vich, struct video_fmt *vi_fmt)
{
	struct isp_video_device *video = NULL;

	if (!VALID_VIDEO_SEL(vich) || NULL == media_dev->video_dev[vich]) {
		LOG("%s: invalid vin ch(%d)\n", __FUNCTION__, vich);
		return -1;
	} else {
		video = media_dev->video_dev[vich];
	}
	
	memset(vi_fmt, 0, sizeof(struct video_fmt));
	video_get_fmt(video, vi_fmt);

	return 0;
}

static int enable_video(struct hw_isp_media_dev *media_dev, int vich)
{
	struct isp_video_device *video = NULL;
	struct buffers_pool *pool = NULL;
	struct video_fmt vfmt;
	int i;

	if (!VALID_VIDEO_SEL(vich) || NULL == media_dev->video_dev[vich]) {
		LOG("%s: invalid vin ch(%d)\n", __FUNCTION__, vich);
		return -1;
	} else {
		video = media_dev->video_dev[vich];
	}
	
	pool = buffers_pool_new(video);
	if (!pool) {
		return -1;
	}

	if (video_req_buffers(video, pool) < 0) {
		return -1;
	}

	memset(&vfmt, 0, sizeof(vfmt));
	video_get_fmt(video, &vfmt);
	for (i = 0; i < vfmt.nbufs; i++) {
		video_queue_buffer(video, i);
	}

	return video_stream_on(video);
}

static int disable_video(struct hw_isp_media_dev *media_dev, int vich)
{
	struct isp_video_device *video = NULL;

	if (!VALID_VIDEO_SEL(vich) || NULL == media_dev->video_dev[vich]) {
		LOG("%s: invalid vin ch(%d)\n", __FUNCTION__, vich);
		return -1;
	} else {
		video = media_dev->video_dev[vich];
	}

	if (video_stream_off(video) < 0) {
		return -1;
	}
	if (video_free_buffers(video) < 0) {
		return -1;
	}
	buffers_pool_delete(video);

	return 0;
}

static int get_video_frame(struct hw_isp_media_dev *media_dev, int vich, frame_buf_info *frame_info, int timeout)
{
	struct isp_video_device *video = NULL;
	struct video_buffer buffer;
	struct video_fmt vfmt;
	int i;

	if (!VALID_VIDEO_SEL(vich) || NULL == media_dev->video_dev[vich]) {
		LOG("%s: invalid vin ch(%d)\n", __FUNCTION__, vich);
		return -1;
	} else {
		video = media_dev->video_dev[vich];
	}

	if (video_wait_buffer(video, timeout) < 0) {
		return -1;
	}
	
	if (video_dequeue_buffer(video, &buffer) < 0) {
		return -1;
	}
	
	memset(&vfmt, 0, sizeof(vfmt));
	video_get_fmt(video, &vfmt);
	for (i = 0; i < vfmt.nplanes; i++) {
		frame_info->pVirAddr[i] = buffer.planes[i].mem;
		frame_info->u32Stride[i] = buffer.planes[i].size;
		frame_info->u32PhyAddr[i] = buffer.planes[i].mem_phy;
	}
	frame_info->u32Width = vfmt.format.width;
	frame_info->u32Height = vfmt.format.height;
	frame_info->u32field = vfmt.format.field;
	frame_info->u32PixelFormat = vfmt.format.pixelformat;
	frame_info->stTimeStamp = buffer.timestamp;
	frame_info->u32PoolId = buffer.index;

	return 0;
}

static int release_video_frame(struct hw_isp_media_dev *media_dev, int vich, frame_buf_info *frame_info)
{
	struct isp_video_device *video = NULL;

	if (!VALID_VIDEO_SEL(vich) || NULL == media_dev->video_dev[vich]) {
		LOG("%s: invalid vin ch(%d)\n", __FUNCTION__, vich);
		return -1;
	} else {
		video = media_dev->video_dev[vich];
	}

	if (video_queue_buffer(video, frame_info->u32PoolId) < 0) {
		return -1;
	}

	return 0;
}

/*
 * frame thread
 */
static void *frame_loop_thread(void *params)
{
	int ret = -1, failed_times = 0;
	capture_params *cap_pa = (capture_params *)params;
	capture_format cap_fmt;
	cap_fmt.buffer = (unsigned char *)malloc(1 << 24); // 16M
	unsigned char *buffer = NULL;

	if (cap_pa && g_media_dev) {
		LOG("%s: channel %d starts\n", __FUNCTION__, cap_pa->priv_cap.vich);
		pthread_mutex_lock(&cap_pa->locker);
		cap_pa->status |= CAP_THREAD_RUNNING;
		pthread_mutex_unlock(&cap_pa->locker);
		while (1) {
			msleep(1);
			ret = pthread_mutex_trylock(&cap_pa->locker);
			if (0 == ret) { // lock ok
				if (!(CAP_STATUS_ON & cap_pa->status)) {
					cap_pa->status &= ~CAP_THREAD_RUNNING;
					pthread_mutex_unlock(&cap_pa->locker);
					LOG("%s: channel %d is off\n", __FUNCTION__, cap_pa->priv_cap.vich);
					break;
				}
				if (CAP_THREAD_STOP & cap_pa->status) {
					disable_video(g_media_dev, cap_pa->priv_cap.vich);
					exit_video(g_media_dev, cap_pa->priv_cap.vich);
					cap_pa->status &= ~CAP_STATUS_ON;
					cap_pa->status &= ~CAP_THREAD_RUNNING;
					pthread_mutex_unlock(&cap_pa->locker);
					LOG("%s: recv stop flag(channel %d)\n", __FUNCTION__, cap_pa->priv_cap.vich);
					break;
				}
				ret = get_video_frame(g_media_dev, cap_pa->priv_cap.vich, &cap_pa->priv_cap.vi_frame_info, cap_pa->priv_cap.timeout);
				if (ret < 0) {
					LOG("%s: failed to get frame(channel %d, %d)\n", __FUNCTION__, cap_pa->priv_cap.vich, failed_times);
					failed_times++;
					if (failed_times >= 10) {
						disable_video(g_media_dev, cap_pa->priv_cap.vich);
						exit_video(g_media_dev, cap_pa->priv_cap.vich);
						cap_pa->status &= ~CAP_STATUS_ON;
						cap_pa->status &= ~CAP_THREAD_RUNNING;
						pthread_mutex_unlock(&cap_pa->locker);
						LOG("%s: failed too many times(channel %d)\n", __FUNCTION__, cap_pa->priv_cap.vich);
						break;
					}
				} else {
					failed_times = 0;
					if (CAP_RAW_FLOW_START & cap_pa->status) {
						cap_fmt.width = cap_pa->priv_cap.vi_fmt.format.width;
						cap_fmt.height = cap_pa->priv_cap.vi_fmt.format.height;
						cap_fmt.format = cap_pa->priv_cap.vi_fmt.format.pixelformat;
						cap_fmt.planes_count = cap_pa->priv_cap.vi_fmt.nplanes;
						cap_fmt.width_stride[0] = cap_pa->priv_cap.vi_fmt.format.plane_fmt[0].bytesperline;
						cap_fmt.width_stride[1] = cap_pa->priv_cap.vi_fmt.format.plane_fmt[1].bytesperline;
						cap_fmt.width_stride[2] = cap_pa->priv_cap.vi_fmt.format.plane_fmt[2].bytesperline;
						buffer = cap_fmt.buffer;
						cap_fmt.length = 0;
						for (ret = 0; ret < cap_pa->priv_cap.vi_fmt.nplanes; ret++) {
							memcpy(buffer, cap_pa->priv_cap.vi_frame_info.pVirAddr[ret],
								cap_pa->priv_cap.vi_frame_info.u32Stride[ret]);
							buffer += cap_pa->priv_cap.vi_frame_info.u32Stride[ret];
							cap_fmt.length += cap_pa->priv_cap.vi_frame_info.u32Stride[ret];
						}
						if (cap_fmt.length < cap_fmt.width * cap_fmt.height) {
							LOG("%s: raw flow - %d < %dx%d, not matched\n", __FUNCTION__, cap_fmt.length, cap_fmt.width, cap_fmt.height);
						} else {
							queue_raw_flow(&cap_fmt);
							//msleep(8);
						}
						cap_pa->status |= CAP_RAW_FLOW_RUNNING;
					} else {
						cap_pa->status &= ~CAP_RAW_FLOW_RUNNING;
					}
					release_video_frame(g_media_dev, cap_pa->priv_cap.vich, &cap_pa->priv_cap.vi_frame_info);
				}
				pthread_mutex_unlock(&cap_pa->locker);
			}
		}

		LOG("%s: channel %d quits\n", __FUNCTION__, cap_pa->priv_cap.vich);
	}
	free(cap_fmt.buffer);
	cap_fmt.buffer = NULL;

	return 0;
}

/*
 * start video node
 * returns CAP_ERR_NONE if OK, others if something went wrong
 */
int start_video(capture_params *cap_pa, capture_format *cap_fmt)
{
	int ret = CAP_ERR_NONE;
	int fmt_changed = 0;

	if (!cap_pa || !cap_fmt || !VALID_VIDEO_SEL(cap_fmt->channel)) {
		LOG("%s: invalid params\n", __FUNCTION__);
		return CAP_ERR_INVALID_PARAMS;
	}

	// check whether set sensor or not
 	if (!g_sensor_set_flag) {
		LOG("%s: not set sensor input yet(channel %d)\n", __FUNCTION__, cap_fmt->channel);
		return CAP_ERR_NOT_SET_INPUT;
	}

	pthread_mutex_lock(&cap_pa->locker);
	if (CAP_STATUS_ON & cap_pa->status) {
		if (cap_pa->priv_cap.vi_fmt.format.pixelformat != cap_fmt->format ||
			cap_pa->priv_cap.vi_fmt.format.width != cap_fmt->width ||
			cap_pa->priv_cap.vi_fmt.format.height != cap_fmt->height) {
			LOG("%s: vich%d format changes: fmt-%d, %dx%d -> fmt-%d, %dx%d\n", __FUNCTION__,
				cap_fmt->channel,
				cap_pa->priv_cap.vi_fmt.format.pixelformat,
				cap_pa->priv_cap.vi_fmt.format.width,
				cap_pa->priv_cap.vi_fmt.format.height,
				cap_fmt->format, cap_fmt->width, cap_fmt->height);

			disable_video(g_media_dev, cap_fmt->channel);
			exit_video(g_media_dev, cap_fmt->channel);
		} else {
			fmt_changed = 0;
			goto start_video_get_fmt;
		}
 	}
	
	fmt_changed = 1;
	cap_pa->status &= ~CAP_STATUS_ON;
	
	ret = init_video(g_media_dev, cap_fmt->channel);
	if (ret) {
		ret = CAP_ERR_CH_INIT;
		LOG("%s: failed to init channel %d\n", __FUNCTION__, cap_fmt->channel);
		goto start_video_end;
	}	
	
	cap_pa->priv_cap.vich = cap_fmt->channel;
	cap_pa->priv_cap.timeout = 2000;  // ms
	cap_pa->priv_cap.vi_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	cap_pa->priv_cap.vi_fmt.memtype = V4L2_MEMORY_MMAP;
	cap_pa->priv_cap.vi_fmt.format.pixelformat = cap_fmt->format;
	cap_pa->priv_cap.vi_fmt.format.field = V4L2_FIELD_NONE;
	cap_pa->priv_cap.vi_fmt.format.width = cap_fmt->width;
	cap_pa->priv_cap.vi_fmt.format.height = cap_fmt->height;
	cap_pa->priv_cap.vi_fmt.nbufs = 3;
	cap_pa->priv_cap.vi_fmt.nplanes = cap_fmt->planes_count;
	cap_pa->priv_cap.vi_fmt.capturemode = V4L2_MODE_VIDEO;
	cap_pa->priv_cap.vi_fmt.fps = cap_fmt->fps;
	cap_pa->priv_cap.vi_fmt.wdr_mode = cap_fmt->wdr;
	cap_pa->priv_cap.vi_fmt.use_current_win = 1;  //!!! not to change sensor input format
	cap_pa->priv_cap.vi_fmt.index = cap_fmt->index;

	ret = set_video_fmt(g_media_dev, cap_fmt->channel, &cap_pa->priv_cap.vi_fmt);
	if (ret) {
		exit_video(g_media_dev, cap_fmt->channel);
		ret = CAP_ERR_CH_SET_FMT;
		LOG("%s: failed to set format(channel %d)\n", __FUNCTION__, cap_fmt->channel);
		goto start_video_end;
	}

	ret = enable_video(g_media_dev, cap_fmt->channel);
	if (ret) {
		exit_video(g_media_dev, cap_fmt->channel);
		ret = CAP_ERR_CH_ENABLE;
		LOG("%s: failed to enble channel %d\n", __FUNCTION__, cap_fmt->channel);
		goto start_video_end;
	}

start_video_get_fmt:
	ret = get_video_fmt(g_media_dev, cap_fmt->channel, &cap_pa->priv_cap.vi_fmt);
	if (ret) {
		disable_video(g_media_dev, cap_fmt->channel);
		exit_video(g_media_dev, cap_fmt->channel);
		ret = CAP_ERR_CH_GET_FMT;
		LOG("%s: failed to get format(channel %d)\n", __FUNCTION__, cap_fmt->channel);
		goto start_video_end;
	}

	if (fmt_changed) {
		LOG("%s: vich%d format - fmt-%d, %dx%d@%d, wdr-%d, planes-%d[%d, %d, %d]\n", __FUNCTION__,
			cap_fmt->channel,
			cap_pa->priv_cap.vi_fmt.format.pixelformat,
			cap_pa->priv_cap.vi_fmt.format.width,
			cap_pa->priv_cap.vi_fmt.format.height,
			cap_pa->priv_cap.vi_fmt.fps,
			cap_pa->priv_cap.vi_fmt.wdr_mode,
			cap_pa->priv_cap.vi_fmt.nplanes,
			cap_pa->priv_cap.vi_fmt.format.plane_fmt[0].bytesperline,
			cap_pa->priv_cap.vi_fmt.format.plane_fmt[1].bytesperline,
			cap_pa->priv_cap.vi_fmt.format.plane_fmt[2].bytesperline);
	}
	
	cap_fmt->width = cap_pa->priv_cap.vi_fmt.format.width;
	cap_fmt->height = cap_pa->priv_cap.vi_fmt.format.height;
	cap_fmt->format = cap_pa->priv_cap.vi_fmt.format.pixelformat;
	cap_fmt->planes_count = cap_pa->priv_cap.vi_fmt.nplanes;
	cap_fmt->width_stride[0] = cap_pa->priv_cap.vi_fmt.format.plane_fmt[0].bytesperline;
	cap_fmt->width_stride[1] = cap_pa->priv_cap.vi_fmt.format.plane_fmt[1].bytesperline;
	cap_fmt->width_stride[2] = cap_pa->priv_cap.vi_fmt.format.plane_fmt[2].bytesperline;

	cap_pa->status |= CAP_STATUS_ON;
	if (!(CAP_THREAD_RUNNING & cap_pa->status)) {
		add_work(&frame_loop_thread, cap_pa);
	}
	ret = CAP_ERR_NONE;
	
start_video_end:
	pthread_mutex_unlock(&cap_pa->locker);
	if (CAP_ERR_NONE == ret) {
		do {
			msleep(1);
			pthread_mutex_lock(&cap_pa->locker);
			if (CAP_THREAD_RUNNING & cap_pa->status) {
				pthread_mutex_unlock(&cap_pa->locker);
				break;
			}
			pthread_mutex_unlock(&cap_pa->locker);
		} while (1);
	}
	return ret;
}

int init_capture_module()
{
	int i = 0;
	capture_params *cap_handle = NULL;

	if (CAPTURE_INIT_YES == g_cap_init_status) {
		exit_capture_module();
	}

	g_media_dev = isp_md_open("/dev/media0");
	if (!g_media_dev) {
		LOG("%s: failed to init media\n", __FUNCTION__);
		return CAP_ERR_MPI_INIT;
	}

	for (i = 0, cap_handle = g_cap_handle; i < CAPTURE_CHANNEL_MAX; i++, cap_handle++) {
		pthread_mutex_init(&cap_handle->locker, NULL);
		reset_cap_params(cap_handle);
	}
	
	pthread_mutex_init(&g_cap_locker, NULL);
	g_cap_init_status = CAPTURE_INIT_YES;

	LOG("%s: init done\n", __FUNCTION__);
	return CAP_ERR_NONE;
}

int exit_capture_module()
{
	int i = 0;
	capture_params *cap_handle = NULL;

	LOG("%s: ready to exit\n", __FUNCTION__);

	if (CAPTURE_INIT_YES == g_cap_init_status) {
		pthread_mutex_lock(&g_cap_locker);
		for (i = 0, cap_handle = g_cap_handle; i < CAPTURE_CHANNEL_MAX; i++, cap_handle++) {
			pthread_mutex_lock(&cap_handle->locker);
			cap_handle->status |= CAP_THREAD_STOP;  // set stop
			pthread_mutex_unlock(&cap_handle->locker);
		}
		msleep(32);

		for (i = 0, cap_handle = g_cap_handle; i < CAPTURE_CHANNEL_MAX; i++, cap_handle++) {
			do {
				msleep(32);
				pthread_mutex_lock(&cap_handle->locker);
				if (!(CAP_THREAD_RUNNING & cap_handle->status)) {
					pthread_mutex_unlock(&cap_handle->locker);
					break;
				}
				pthread_mutex_unlock(&cap_handle->locker);
			} while (1);

			pthread_mutex_lock(&cap_handle->locker);
			if (CAP_STATUS_ON & cap_handle->status) {
				disable_video(g_media_dev, cap_handle->priv_cap.vich);
				exit_video(g_media_dev, cap_handle->priv_cap.vich);
				cap_handle->status &= ~CAP_STATUS_ON;
			}
			pthread_mutex_unlock(&cap_handle->locker);
			pthread_mutex_destroy(&cap_handle->locker);
		}

		if (g_media_dev) {
			isp_md_close(g_media_dev);
			g_media_dev = NULL;
		}

		pthread_mutex_unlock(&g_cap_locker);
		pthread_mutex_destroy(&g_cap_locker);
		g_cap_init_status = CAPTURE_INIT_NOT;
	}

	LOG("%s: exits\n", __FUNCTION__);
	return CAP_ERR_NONE;
}

int get_vich_status()
{
	capture_params *cap_handle = NULL;
	int i = 0, ret = 0;
	for (i = 0, cap_handle = g_cap_handle; i < CAPTURE_CHANNEL_MAX; i++, cap_handle++) {
		if (CAP_STATUS_ON & cap_handle->status) {
			ret |= (1 << i);
		}
	}
	return ret;
}

int set_sensor_input(const sensor_input *sensor_in)
{
	capture_params *cap_handle = NULL;
	int ret = CAP_ERR_NONE;

	if (sensor_in && VALID_VIDEO_SEL(sensor_in->channel)) {
		// check whether same format or not
		//sensor_set_flag = g_sensor_set_flag & (1<<sensor_in->channel);
		//LOG("%s: channel %d, set flag %d\n", __FUNCTION__, sensor_in->channel, sensor_set_flag);
		//if (sensor_set_flag) {
		//	return CAP_ERR_NONE;
		//}
		
		pthread_mutex_lock(&g_cap_locker);
		cap_handle = g_cap_handle + sensor_in->channel;

		pthread_mutex_lock(&cap_handle->locker);
		if (CAP_STATUS_ON & cap_handle->status) {
			disable_video(g_media_dev, sensor_in->channel);
			exit_video(g_media_dev, sensor_in->channel);
		}
		ret = init_video(g_media_dev, sensor_in->channel);
		if (ret) {
			ret = CAP_ERR_CH_INIT;
			LOG("%s: failed to init channel %d\n", __FUNCTION__, sensor_in->channel);
			goto set_sensor_input_end;
		}
		cap_handle->status &= ~CAP_STATUS_ON;

		cap_handle->priv_cap.vich = sensor_in->channel;
		cap_handle->priv_cap.timeout = 2000;
		cap_handle->priv_cap.vi_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		cap_handle->priv_cap.vi_fmt.memtype = V4L2_MEMORY_MMAP;
		cap_handle->priv_cap.vi_fmt.format.pixelformat = V4L2_PIX_FMT_NV12M;
		cap_handle->priv_cap.vi_fmt.format.field = V4L2_FIELD_NONE;
		cap_handle->priv_cap.vi_fmt.format.width = sensor_in->width;
		cap_handle->priv_cap.vi_fmt.format.height = sensor_in->height;
		cap_handle->priv_cap.vi_fmt.nbufs = 3;
		cap_handle->priv_cap.vi_fmt.nplanes = 2;
		cap_handle->priv_cap.vi_fmt.capturemode = V4L2_MODE_VIDEO;
		cap_handle->priv_cap.vi_fmt.fps = sensor_in->fps;
		cap_handle->priv_cap.vi_fmt.wdr_mode = sensor_in->wdr;
		cap_handle->priv_cap.vi_fmt.use_current_win = 0;  //!!! try to change sensor input format
		cap_handle->priv_cap.vi_fmt.index = sensor_in->index;

		ret = set_video_fmt(g_media_dev, sensor_in->channel, &cap_handle->priv_cap.vi_fmt);
		if (ret) {
			exit_video(g_media_dev, sensor_in->channel);
			ret = CAP_ERR_CH_SET_FMT;
			LOG("%s: failed to set format(channel %d)\n", __FUNCTION__, sensor_in->channel);
			goto set_sensor_input_end;
		}
		
		ret = enable_video(g_media_dev, sensor_in->channel);
		if (ret) {
			exit_video(g_media_dev, sensor_in->channel);
			ret = CAP_ERR_CH_ENABLE;
			LOG("%s: failed to enable channel %d\n", __FUNCTION__, sensor_in->channel);
			goto set_sensor_input_end;
		}

		ret = get_video_fmt(g_media_dev, sensor_in->channel, &cap_handle->priv_cap.vi_fmt);
		if (ret) {
			disable_video(g_media_dev, sensor_in->channel);
			exit_video(g_media_dev, sensor_in->channel);
			ret = CAP_ERR_CH_GET_FMT;
			LOG("%s: failed to get format(channel %d)\n", __FUNCTION__, sensor_in->channel);
			goto set_sensor_input_end;
		}

		LOG("%s: vich%d format - fmt-%d, %dx%d@%d, wdr-%d, planes-%d[%d, %d, %d]\n", __FUNCTION__,
			sensor_in->channel,
			cap_handle->priv_cap.vi_fmt.format.pixelformat,
			cap_handle->priv_cap.vi_fmt.format.width,
			cap_handle->priv_cap.vi_fmt.format.height,
			cap_handle->priv_cap.vi_fmt.fps,
			cap_handle->priv_cap.vi_fmt.wdr_mode,
			cap_handle->priv_cap.vi_fmt.nplanes,
			cap_handle->priv_cap.vi_fmt.format.plane_fmt[0].bytesperline,
			cap_handle->priv_cap.vi_fmt.format.plane_fmt[1].bytesperline,
			cap_handle->priv_cap.vi_fmt.format.plane_fmt[2].bytesperline);

		g_sensor_set_flag |= (1<<sensor_in->channel);
		cap_handle->status |= CAP_STATUS_ON;
		if (!(CAP_THREAD_RUNNING & cap_handle->status)) {
			add_work(&frame_loop_thread, cap_handle);
		}
		ret = CAP_ERR_NONE;

set_sensor_input_end:
		pthread_mutex_unlock(&cap_handle->locker);
		if (CAP_ERR_NONE == ret) {
			do {
				msleep(1);
				pthread_mutex_lock(&cap_handle->locker);
				if (CAP_THREAD_RUNNING & cap_handle->status) {
					pthread_mutex_unlock(&cap_handle->locker);
					break;
				}
				pthread_mutex_unlock(&cap_handle->locker);
			} while (1);
		}
		pthread_mutex_unlock(&g_cap_locker);
		return ret;		
	}

	return -1;
}


int get_capture_buffer(capture_format *cap_fmt)
{
	int ret = CAP_ERR_NONE;
	unsigned char *buffer = NULL;
	capture_params *cap_handle = NULL;
	unsigned char *uptr = NULL;
	unsigned int i;

	if (cap_fmt && VALID_VIDEO_SEL(cap_fmt->channel)) {
		pthread_mutex_lock(&g_cap_locker);
		cap_handle = g_cap_handle + cap_fmt->channel;
		//LOG("%s: %d\n", __FUNCTION__, __LINE__);
		ret = start_video(cap_handle, cap_fmt);
		//LOG("%s: %d\n", __FUNCTION__, __LINE__);
		if (CAP_ERR_NONE == ret) {
			// get frame
			//LOG("%s: ready to get frame(channel %d)\n", __FUNCTION__, cap_fmt->channel);
			//LOG("%s: %d\n", __FUNCTION__, __LINE__);
			pthread_mutex_lock(&cap_handle->locker);
			//LOG("%s: %d\n", __FUNCTION__, __LINE__);
			ret = get_video_frame(g_media_dev, cap_handle->priv_cap.vich, &cap_handle->priv_cap.vi_frame_info, cap_handle->priv_cap.timeout);
			//LOG("%s: get frame %d done(channel %d)\n", __FUNCTION__, frames_count, cap_fmt->channel);
			cap_handle->frame_count++;
			if (ret < 0) {
				ret = CAP_ERR_GET_FRAME;
				LOG("%s: failed to get frame %d(channel %d)\n", __FUNCTION__, cap_handle->frame_count, cap_fmt->channel);
			} else {
				if (cap_handle->frame_count % 50 == 0) {
					LOG("%s: get frame %d done(channel %d)\n", __FUNCTION__, cap_handle->frame_count, cap_fmt->channel);
				}
				buffer = cap_fmt->buffer;
				cap_fmt->length = 0;
				for (ret = 0; ret < cap_fmt->planes_count; ret++) {
					memcpy(buffer, cap_handle->priv_cap.vi_frame_info.pVirAddr[ret],
						cap_handle->priv_cap.vi_frame_info.u32Stride[ret]);
					buffer += cap_handle->priv_cap.vi_frame_info.u32Stride[ret];
					cap_fmt->length += cap_handle->priv_cap.vi_frame_info.u32Stride[ret];
				}
#if (ISP_VERSION == 521)
				if(cap_fmt->height % 16) { // height ALIGN
					uptr = cap_fmt->buffer + (cap_fmt->width * cap_fmt->height);
					for(i = 0; i < cap_fmt->width * cap_fmt->height / 2; i++) {
						*(uptr + i) = *(uptr + i + (cap_fmt->width * (16 - cap_fmt->height % 16)));
					}
				}
#endif
				if (cap_fmt->length < cap_fmt->width * cap_fmt->height) {
					LOG("%s: %d < %dx%d, not matched\n", __FUNCTION__, cap_fmt->length, cap_fmt->width, cap_fmt->height);
					ret = CAP_ERR_GET_FRAME;
				} else {
					ret = CAP_ERR_NONE;
				}
				release_video_frame(g_media_dev, cap_handle->priv_cap.vich, &cap_handle->priv_cap.vi_frame_info);
			}
			pthread_mutex_unlock(&cap_handle->locker);
		}
		pthread_mutex_unlock(&g_cap_locker);
	} else {
		ret = CAP_ERR_INVALID_PARAMS;
	}

	return ret;
}

int get_capture_buffer_transfer(capture_format *cap_fmt)
{
	int ret = CAP_ERR_NONE;
	unsigned char *buffer = NULL;
	capture_params *cap_handle = NULL;
	int i = 0;
	unsigned char *uptr = NULL;

	if (cap_fmt && VALID_VIDEO_SEL(cap_fmt->channel)) {
		pthread_mutex_lock(&g_cap_locker);
		cap_handle = g_cap_handle + cap_fmt->channel;
		//LOG("%s: %d\n", __FUNCTION__, __LINE__);
		ret = start_video(cap_handle, cap_fmt);
		//LOG("%s: %d\n", __FUNCTION__, __LINE__);
		if (CAP_ERR_NONE == ret) {
			// get frame
			//LOG("%s: ready to get frame(channel %d)\n", __FUNCTION__, cap_fmt->channel);
			//LOG("%s: %d\n", __FUNCTION__, __LINE__);
			pthread_mutex_lock(&cap_handle->locker);
			//LOG("%s: %d\n", __FUNCTION__, __LINE__);
			cap_fmt->length = 0;
			buffer = cap_fmt->buffer;
			for(i=0; i<cap_fmt->framecount; i++){//save cap_fmt->framecount frames
				ret = get_video_frame(g_media_dev, cap_handle->priv_cap.vich, &cap_handle->priv_cap.vi_frame_info, cap_handle->priv_cap.timeout);
				//LOG("%s: get frame %d done(channel %d)\n", __FUNCTION__, cap_handle->frame_count, cap_fmt->channel);
				cap_handle->frame_count++;
				if (ret < 0) {
					ret = CAP_ERR_GET_FRAME;
					LOG("%s: failed to get frame %d(channel %d)\n", __FUNCTION__, cap_handle->frame_count, cap_fmt->channel);
				} else {
					if (cap_handle->frame_count % 50 == 0) {
						LOG("%s: get frame %d done(channel %d)\n", __FUNCTION__, cap_handle->frame_count, cap_fmt->channel);
					}
					if( (cap_fmt->length +  cap_fmt->width * cap_fmt->height*2) <(1 << 29)){
						for (ret = 0; ret < cap_fmt->planes_count; ret++) {
							memcpy(buffer, cap_handle->priv_cap.vi_frame_info.pVirAddr[ret],
								cap_handle->priv_cap.vi_frame_info.u32Stride[ret]);
							buffer += cap_handle->priv_cap.vi_frame_info.u32Stride[ret];
							cap_fmt->length += cap_handle->priv_cap.vi_frame_info.u32Stride[ret];
						}
#if (ISP_VERSION == 521)
						if(cap_fmt->height % 16) { // height ALIGN
							uptr = cap_fmt->buffer + (cap_fmt->width * cap_fmt->height);
							for(i = 0; i < cap_fmt->width * cap_fmt->height / 2; i++) {
								*(uptr + i) = *(uptr + i + (cap_fmt->width * (16 - cap_fmt->height % 16)));
							}
						}
#endif
						if (cap_fmt->length < cap_fmt->width * cap_fmt->height) {
							LOG("%s: %d < %dx%d, not matched\n", __FUNCTION__, cap_fmt->length, cap_fmt->width, cap_fmt->height);
							ret = CAP_ERR_GET_FRAME;
						} else {
							ret = CAP_ERR_NONE;
						}
					}
					release_video_frame(g_media_dev, cap_handle->priv_cap.vich, &cap_handle->priv_cap.vi_frame_info);
				}
			}
			pthread_mutex_unlock(&cap_handle->locker);
		}
		pthread_mutex_unlock(&g_cap_locker);
	} else {
		ret = CAP_ERR_INVALID_PARAMS;
	}

	return ret;
}

int start_raw_flow(capture_format *cap_fmt)
{
	int ret = CAP_ERR_NONE;
	capture_params *cap_handle = NULL;

	if (cap_fmt && VALID_VIDEO_SEL(cap_fmt->channel)) {	
		// check format
		if (!(cap_fmt->format == V4L2_PIX_FMT_SBGGR8 ||
			cap_fmt->format == V4L2_PIX_FMT_SGBRG8 ||
			cap_fmt->format == V4L2_PIX_FMT_SGRBG8 ||
			cap_fmt->format == V4L2_PIX_FMT_SRGGB8 ||
			cap_fmt->format == V4L2_PIX_FMT_SBGGR10 ||
			cap_fmt->format == V4L2_PIX_FMT_SGBRG10 ||
			cap_fmt->format == V4L2_PIX_FMT_SGRBG10 ||
			cap_fmt->format == V4L2_PIX_FMT_SRGGB10 ||
			cap_fmt->format == V4L2_PIX_FMT_SBGGR12 ||
			cap_fmt->format == V4L2_PIX_FMT_SGBRG12 ||
			cap_fmt->format == V4L2_PIX_FMT_SGRBG12 ||
			cap_fmt->format == V4L2_PIX_FMT_SRGGB12)) {
			LOG("%s: Not valid bayer format\n", __FUNCTION__);
			return CAP_ERR_INVALID_PARAMS;
		}

		// start channel
		pthread_mutex_lock(&g_cap_locker);
		cap_handle = g_cap_handle + cap_fmt->channel;
		ret = start_video(cap_handle, cap_fmt);
		if (CAP_ERR_NONE == ret) { // video on ok
			if (!(CAP_RAW_FLOW_RUNNING & cap_handle->status)) {
				// init raw flow
				ret = init_raw_flow(cap_fmt, CAPTURE_RAW_FLOW_QUEUE_SIZE);
				if (ret != ERR_RAW_FLOW_NONE) {
					ret = CAP_ERR_START_RAW_FLOW;
				} else {
					pthread_mutex_lock(&cap_handle->locker);
					cap_handle->status |= CAP_RAW_FLOW_START;
					pthread_mutex_unlock(&cap_handle->locker);
					do {
						msleep(1);
						pthread_mutex_lock(&cap_handle->locker);
						if (CAP_RAW_FLOW_RUNNING & cap_handle->status) {
							LOG("%s: vich%d done\n", __FUNCTION__, cap_handle->priv_cap.vich);
							pthread_mutex_unlock(&cap_handle->locker);
							break;
						}
						pthread_mutex_unlock(&cap_handle->locker);
					} while (1);
					ret = CAP_ERR_NONE;
				}
			} else {
				LOG("%s: raw flow is already running(vich%d)\n", __FUNCTION__, cap_handle->priv_cap.vich);
			}
		}
		pthread_mutex_unlock(&g_cap_locker);
	} else {
		ret = CAP_ERR_INVALID_PARAMS;
	}
	return ret;
}

int stop_raw_flow(int channel)
{
	capture_params *cap_handle = NULL;
	if (!VALID_VIDEO_SEL(channel)) {
		return CAP_ERR_INVALID_PARAMS;
	}
	
	pthread_mutex_lock(&g_cap_locker);
	cap_handle = g_cap_handle + channel;
	pthread_mutex_lock(&cap_handle->locker);
	cap_handle->status &= ~CAP_RAW_FLOW_START;
	pthread_mutex_unlock(&cap_handle->locker);
	do {
		msleep(1);
		pthread_mutex_lock(&cap_handle->locker);
		if (!(CAP_RAW_FLOW_RUNNING & cap_handle->status)) {
			LOG("%s: vich%d done\n", __FUNCTION__, cap_handle->priv_cap.vich);
			pthread_mutex_unlock(&cap_handle->locker);
			break;
		}
		pthread_mutex_unlock(&cap_handle->locker);
	} while (1);
	pthread_mutex_unlock(&g_cap_locker);
	
	if (exit_raw_flow() != ERR_RAW_FLOW_NONE) {
		return CAP_ERR_STOP_RAW_FLOW;
	} else {
		return CAP_ERR_NONE;
	}
}

int get_raw_flow_frame(capture_format *cap_fmt)
{
	capture_params *cap_handle = NULL;
	if (!cap_fmt || !VALID_VIDEO_SEL(cap_fmt->channel)) {
		return CAP_ERR_INVALID_PARAMS;
	}

	pthread_mutex_lock(&g_cap_locker);
	cap_handle = g_cap_handle + cap_fmt->channel;
	pthread_mutex_lock(&cap_handle->locker);
	if (!(CAP_RAW_FLOW_RUNNING & cap_handle->status)) {
		pthread_mutex_unlock(&cap_handle->locker);
		pthread_mutex_unlock(&g_cap_locker);
		return CAP_ERR_RAW_FLOW_NOT_RUN;
	}
	pthread_mutex_unlock(&cap_handle->locker);
	pthread_mutex_unlock(&g_cap_locker);

	if (dequeue_raw_flow(cap_fmt) != ERR_RAW_FLOW_NONE) {
		return CAP_ERR_GET_RAW_FLOW;
	} else {
		return CAP_ERR_NONE;
	}
}

void *do_save_raw_flow(void *params)
{
	FILE *fp = NULL;
	capture_format cap_fmt;
	int ret = 0, frames_count = 0;
	
	if (params) {
		fp = (FILE *)params;
		cap_fmt.buffer = (unsigned char *)malloc(1 << 24); // 16M
		do {
			ret = dequeue_raw_flow(&cap_fmt);
			if (ERR_RAW_FLOW_NONE == ret) {
				LOG("%s: %d\n", __FUNCTION__, __LINE__);
				fwrite(cap_fmt.buffer, cap_fmt.length, 1, fp);
				LOG("%s: %d\n", __FUNCTION__, __LINE__);
				//fflush(fp);
				frames_count++;
			} else {
				if (ERR_RAW_FLOW_QUEUE_EMPTY == ret) {
					continue;
				} else {
					break;
				}
			}
		} while (1);
		fclose(fp);
		fp = NULL;
		free(cap_fmt.buffer);
		cap_fmt.buffer = NULL;
		LOG("%s: save done(frames %d)\n", __FUNCTION__, frames_count);
	}
	return 0;
}

void save_raw_flow(const char *file_name)
{
	FILE *fp = NULL;

	if (file_name) {
		fp = fopen(file_name, "wb");
		if (fp) {
			add_work(&do_save_raw_flow, fp);
		} else {
			LOG("%s: failed to open %s\n", __FUNCTION__, file_name);
		}
	}
}



