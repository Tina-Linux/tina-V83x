
#ifndef __VIDEO_PRIV_H_
#define __VIDEO_PRIV_H_

#include "../../include/V4l2Camera/sunxi_camera_v2.h"

struct isp_video_device {
	unsigned int id;
	unsigned int isp_id;
	struct media_entity *entity;

	enum v4l2_buf_type type;
	enum v4l2_memory memtype;

	struct v4l2_pix_format_mplane format;

	unsigned int nbufs;
	unsigned int nplanes;
	struct buffers_pool *pool;
	unsigned int fps;

	void *priv;
};

static inline void video_set_priv_data(struct isp_video_device *video, void *p)
{
	video->priv = p;
}

static inline void *video_get_priv_data(const struct isp_video_device *video)
{
	return video->priv;
}

#endif /* __CSI_PRIV_H_ */
