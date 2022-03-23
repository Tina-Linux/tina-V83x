#include "CameraV4l2.h"

struct v4l2_capability cap;
struct CameraFrameInfo PlatformSupport[MAX_NUM];
struct CameraFrameInfo SensorSupport[MAX_NUM];

unsigned int format_index = 0;
unsigned int size_index = 0;
unsigned int framerate_index = 0;

CameraPixelFormat FormatDriverToUsr(unsigned int fourcc)
{
	switch(fourcc) {
		/* 4:2:0 */
		case V4L2_PIX_FMT_NV12:
			return PIXEL_YUV420SP;
		case V4L2_PIX_FMT_NV21:
			return PIXEL_YVU420SP;
		case V4L2_PIX_FMT_YUV420:
			return PIXEL_YUV420P;
		case V4L2_PIX_FMT_YVU420:
			return PIXEL_YVU420P;
		/* 4:2:2 */
		case V4L2_PIX_FMT_NV16:
			return PIXEL_YUV422SP;
		case V4L2_PIX_FMT_NV61:
			return PIXEL_YVU422SP;
		case V4L2_PIX_FMT_YUV422P:
			return PIXEL_YUV422P;
		/*videodev2.h not define
		case V4L2_PIX_FMT_YVU422P;
			return PIXEL_YVU422P;*/
		/* Y U Y V */
		case V4L2_PIX_FMT_YUYV:
			return PIXEL_YUYV;
		case V4L2_PIX_FMT_UYVY:
			return PIXEL_UYVY;
		case V4L2_PIX_FMT_YVYU:
			return PIXEL_YVYU;
		case V4L2_PIX_FMT_VYUY:
			return PIXEL_VYUY;
		/* codec */
		case V4L2_PIX_FMT_MJPEG:
			return PIXEL_MJPEG;
		case V4L2_PIX_FMT_H264:
			return PIXEL_H264;
		default:
			CamErr("[%s] fourcc %d is not supported\n", __func__, fourcc);
			return -1;
	}
}

unsigned int FormatUsrToDriver(CameraPixelFormat format)
{
	switch(format) {
		/* 4:2:0 */
		case PIXEL_YUV420SP:
			return V4L2_PIX_FMT_NV12;
		case PIXEL_YVU420SP:
			return V4L2_PIX_FMT_NV21;
		case PIXEL_YUV420P:
			return V4L2_PIX_FMT_YUV420;
		case PIXEL_YVU420P:
			return V4L2_PIX_FMT_YVU420;
		/* 4:2:2 */
		case PIXEL_YUV422SP:
			return V4L2_PIX_FMT_NV16;
		case PIXEL_YVU422SP:
			return V4L2_PIX_FMT_NV61;
		case PIXEL_YUV422P:
			return V4L2_PIX_FMT_YUV422P;
		/*videodev2.h not define
		case PIXEL_YVU422P:
			return "YVU422P";*/
		/* Y U Y V */
		case PIXEL_YUYV:
			return V4L2_PIX_FMT_YUYV;
		case PIXEL_UYVY:
			return V4L2_PIX_FMT_UYVY;
		case PIXEL_YVYU:
			return V4L2_PIX_FMT_YVYU;
		case PIXEL_VYUY:
			return V4L2_PIX_FMT_VYUY;
		/* codec */
		case PIXEL_MJPEG:
			return V4L2_PIX_FMT_MJPEG;
		case PIXEL_H264:
			return V4L2_PIX_FMT_H264;
		default:
			CamErr("[%s] format %d is not supported\n", __func__, format);
			return -1;
	}
}

unsigned char *FormatUsrToStr(CameraPixelFormat format)
{
	switch(format) {
		/* 4:2:0 */
		case PIXEL_YUV420SP:
			return "YUV420SP";
		case PIXEL_YVU420SP:
			return "YVU420SP";
		case PIXEL_YUV420P:
			return "YUV420P";
		case PIXEL_YVU420P:
			return "YVU420P";
		/* 4:2:2 */
		case PIXEL_YUV422SP:
			return "YUV422SP";
		case PIXEL_YVU422SP:
			return "YVU422SP";
		case PIXEL_YUV422P:
			return "YUV422P";
		/*videodev2.h not define
		case PIXEL_YVU422P:
			return "YVU422P";*/
		/* Y U Y V */
		case PIXEL_YUYV:
			return "YUYV";
		case PIXEL_UYVY:
			return "UYVY";
		case PIXEL_YVYU:
			return "YVYU";
		case PIXEL_VYUY:
			return "VYUY";
		/* codec */
		case PIXEL_MJPEG:
			return "MJPEG";
		case PIXEL_H264:
			return "H264";
		default:
			CamErr("[%s] format %d is not supported\n", __func__, format);
			return NULL;
	}
}

CameraPixelFormat FormatStrToUsr(unsigned char *name)
{
	if(!name) {
		CamErr("[%s] format name is NULL %s\n", __func__, name);
		return -1;
	}
	/* convert str to usr format */
	if (strcmp(name, "YVU420SP") == 0)
		return PIXEL_YVU420SP;
	else if(strcmp(name, "YUV420SP") == 0)
		return PIXEL_YUV420SP;
	else if(strcmp(name, "YUV420P") == 0)
		return PIXEL_YUV420P;
	else if(strcmp(name, "YVU420P") == 0)
		return PIXEL_YVU420P;
	else if(strcmp(name, "YUV422SP") == 0)
		return PIXEL_YUV422SP;
	else if(strcmp(name, "YVU422SP") == 0)
		return PIXEL_YVU422SP;
	else if(strcmp(name, "YUV422P") == 0)
		return PIXEL_YUV422P;
	/*videodev2.h not define
	else if(strcmp(name, "YVU422P") == 0)
		return PIXEL_YVU422P;*/
	else if(strcmp(name, "YUYV") == 0)
		return PIXEL_YUYV;
	else if(strcmp(name, "UYVY") == 0)
		return PIXEL_UYVY;
	else if(strcmp(name, "YVYU") == 0)
		return PIXEL_YVYU;
	else if(strcmp(name, "VYUY") == 0)
		return PIXEL_VYUY;
	else if(strcmp(name, "MJPEG") == 0)
		return PIXEL_MJPEG;
	else if(strcmp(name, "H264") == 0)
		return PIXEL_H264;
	else {
		CamErr("[%s] not support format name %s\n", __func__, name);
		return -1;
	}
}

unsigned int FormatStrToDriver(unsigned char *name)
{
	if(!name) {
		CamErr("[%s] format name is NULL %s\n", __func__, name);
		return -1;
	}
	/* convert str to driver fourcc */
	if (strcmp(name, "YVU420SP") == 0)
		return V4L2_PIX_FMT_NV12;
	else if(strcmp(name, "YUV420SP") == 0)
		return V4L2_PIX_FMT_NV21;
	else if(strcmp(name, "YUV420P") == 0)
		return V4L2_PIX_FMT_YUV420;
	else if(strcmp(name, "YVU420P") == 0)
		return V4L2_PIX_FMT_YVU420;
	else if(strcmp(name, "YUV422SP") == 0)
		return V4L2_PIX_FMT_NV16;
	else if(strcmp(name, "YVU422SP") == 0)
		return V4L2_PIX_FMT_NV61;
	else if(strcmp(name, "YUV422P") == 0)
		return V4L2_PIX_FMT_YUV422P;
	/*videodev2.h not define
	else if(strcmp(name, "YVU422P") == 0)
		return V4L2_PIX_FMT_YVU422P;*/
	else if(strcmp(name, "YUYV") == 0)
		return V4L2_PIX_FMT_YUYV;
	else if(strcmp(name, "UYVY") == 0)
		return V4L2_PIX_FMT_UYVY;
	else if(strcmp(name, "YVYU") == 0)
		return V4L2_PIX_FMT_YVYU;
	else if(strcmp(name, "VYUY") == 0)
		return V4L2_PIX_FMT_VYUY;
	else if(strcmp(name, "MJPEG") == 0)
		return V4L2_PIX_FMT_MJPEG;
	else if(strcmp(name, "H264") == 0)
		return V4L2_PIX_FMT_H264;
	else {
		CamErr("[%s] not support format name %s\n", __func__, name);
		return -1;
	}
}

unsigned char *FormatDriverToStr(unsigned int fourcc)
{
	switch(fourcc) {
		/* 4:2:0 */
		case V4L2_PIX_FMT_NV12:
			return "YUV420SP";
		case V4L2_PIX_FMT_NV21:
			return "YVU420SP";
		case V4L2_PIX_FMT_YUV420:
			return "YUV420P";
		case V4L2_PIX_FMT_YVU420:
			return "YVU420P";
		/* 4:2:2 */
		case V4L2_PIX_FMT_NV16:
			return "YUV422SP";
		case V4L2_PIX_FMT_NV61:
			return "YVU422SP";
		case V4L2_PIX_FMT_YUV422P:
			return "YUV422P";
		/*videodev2.h not define
		case V4L2_PIX_FMT_YVu422P;
			return "YVU422P";*/
		/* Y U Y V */
		case V4L2_PIX_FMT_YUYV:
			return "YUYV";
		case V4L2_PIX_FMT_UYVY:
			return "UYVY";
		case V4L2_PIX_FMT_YVYU:
			return "YVYU";
		case V4L2_PIX_FMT_VYUY:
			return "VYUY";
		/* codec */
		case V4L2_PIX_FMT_MJPEG:
			return "MJPEG";
		case V4L2_PIX_FMT_H264:
			return "H264";
		default:
			CamWarn("[%s] fourcc %d is not supported\n", __func__, fourcc);
			return NULL;
	}
}

int V4L2QueryCap(void *hdl)
{
	Camera *handle = (Camera *) hdl;

	/* ioctl vidioc_querycap */
	if (ioctl(handle->fd, VIDIOC_QUERYCAP, &cap) < 0) {
		CamErr("[%s] camera[%d] vidioc_querycap failed! errno(%d)\n",
				__func__, handle->index, errno);
		return -1;
	}
	/* check cap support capture */
	if ((cap.capabilities & (V4L2_CAP_VIDEO_CAPTURE |
		V4L2_CAP_VIDEO_CAPTURE_MPLANE)) <= 0) {
		CamErr("[%s] camera[%d] not supports the video capture interface!\n",
				__func__, handle->index);
		return -1;
	}

	return 0;
}

int V4L2SetInput(void *hdl)
{
	Camera *handle = (Camera *) hdl;
	struct v4l2_input inp;

	/* ioctl vidioc_s_input */
	inp.index = 0;
	inp.type = V4L2_INPUT_TYPE_CAMERA;
	if (ioctl(handle->fd, VIDIOC_S_INPUT, &inp) < 0) {
		CamErr("[%s] camera[%d] vidioc_s_input failed! errno(%d)\n",
				__func__, handle->index, errno);
		return -1;
	}

	return 0;
}

int V4L2EnumFmt(void *hdl)
{
	Camera *handle = (Camera *) hdl;
	struct v4l2_fmtdesc fmtdesc;
	int index =0;

	/* 4.Enumerate image formats */
	if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
		fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	else
		fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmtdesc.index = 0;


	memset(PlatformSupport, 0, sizeof(PlatformSupport));
	while (ioctl(handle->fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0)
	{
		/* save image formats */
		if (FormatDriverToStr(fmtdesc.pixelformat) != NULL) {
			memcpy(PlatformSupport[index].name, fmtdesc.description,
					sizeof(fmtdesc.description));
			PlatformSupport[index].fourcc = fmtdesc.pixelformat;
			PlatformSupport[index].format = FormatDriverToUsr(fmtdesc.pixelformat);
			index++;
		}
		fmtdesc.index++;
	}

	return 0;
}


int V4L2TryFmt(void *hdl)
{
	Camera *handle = (Camera *) hdl;
	struct v4l2_format fmt;
	int index = 0;
	int num = 0;

	/* try all format,here resolution is not important */
	memset(SensorSupport, 0, sizeof(SensorSupport));
	memset(&fmt, 0, sizeof(fmt));
	if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		fmt.fmt.pix_mp.width = handle->width;
		fmt.fmt.pix_mp.height = handle->height;
		fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
	} else {
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmt.fmt.pix.width = handle->width;
		fmt.fmt.pix.height = handle->height;
		fmt.fmt.pix.field = V4L2_FIELD_NONE;
	}
	/**/
	while(PlatformSupport[index].fourcc != 0) {
		if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
			fmt.fmt.pix_mp.pixelformat = PlatformSupport[index].fourcc;
		else
			fmt.fmt.pix.pixelformat = PlatformSupport[index].fourcc;
		/**/
		if (ioctl(handle->fd, VIDIOC_S_FMT, &fmt) == 0) {
			if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
				SensorSupport[num].fourcc = fmt.fmt.pix_mp.pixelformat;
			else
				SensorSupport[num].fourcc = fmt.fmt.pix.pixelformat;
			SensorSupport[num].format = \
								FormatDriverToUsr(SensorSupport[num].fourcc);
			num++;
		}
		index++;
	}

	return 0;
}

int V4L2EnumFrameSizes(void *hdl)
{
	Camera *handle = (Camera *) hdl;
	struct v4l2_frmsizeenum frmsize;
	int index = 0;

	while (SensorSupport[index].fourcc != 0) {
		memset(&frmsize,0,sizeof(frmsize));
		frmsize.index = 0;
		frmsize.pixel_format = SensorSupport[index].fourcc;
		/* vidioc_enum_framesize to fill siz*/
		while (ioctl(handle->fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {
			if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
				SensorSupport[index].size[frmsize.index].width = \
													frmsize.stepwise.max_width;
				SensorSupport[index].size[frmsize.index].height = \
													frmsize.stepwise.max_height;
			} else {
				SensorSupport[index].size[frmsize.index].width = \
														frmsize.discrete.width;
				SensorSupport[index].size[frmsize.index].height = \
														frmsize.discrete.height;
			}
			frmsize.index++;
			/* sometimes MAX_NUM is less */
			if (frmsize.index > MAX_NUM - 1) {
				CamWarn("Beyond the queryable range, please modify MAX_NUM\n");
				break;
			}
		}
		/* next frame */
		index++;
	}

	return 0;
}

int V4L2EnumFrameIntervals(void *hdl)
{
	Camera *handle = (Camera *) hdl;
	struct v4l2_frmivalenum frmival;
	int index = 0;
	int i;

	memset(&frmival,0, sizeof(frmival));
	while (SensorSupport[index].fourcc != 0) {
		i = 0;
		while (SensorSupport[index].size[i].width !=0 ) {
			i++;
			frmival.index = 0;
			frmival.width = SensorSupport[index].size[i].width;
			frmival.height = SensorSupport[index].size[i].height;
			frmival.pixel_format = SensorSupport[index].fourcc;
			frmival.type = V4L2_FRMIVAL_TYPE_DISCRETE;
			while (ioctl(handle->fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) == 0)
			{
				SensorSupport[index].size[i].framerate[frmival.index] = \
					frmival.discrete.denominator / frmival.discrete.numerator;
				frmival.index++;
			}
		}
		index++;
	}

	return 0;
}

int V4L2GetParm(void *hdl)
{
	Camera *handle = (Camera *) hdl;
	struct v4l2_streamparm parms;

	memset(&parms, 0, sizeof(struct v4l2_streamparm));
	if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
		parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	else
		parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(handle->fd, VIDIOC_G_PARM, &parms) < 0) {
		CamErr("[%s] camera[%d] vidioc_g_parm failed! errno(%d)\n",
				__func__, handle->index, errno);
		return -1;
	}

	handle->framerate = (unsigned int)
						(parms.parm.capture.timeperframe.denominator /
						parms.parm.capture.timeperframe.numerator);

	return 0;
}

int V4L2SetParm(void *hdl)
{
	Camera *handle = (Camera *) hdl;
	struct v4l2_streamparm parms;

	memset(&parms, 0, sizeof(struct v4l2_streamparm));
	parms.parm.capture.timeperframe.numerator = 1;
	if (handle->framerate)
		parms.parm.capture.timeperframe.denominator = handle->framerate;
	if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
		parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	else
		parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(handle->fd, VIDIOC_S_PARM, &parms) < 0) {
		CamErr("[%s] camera[%d] vidioc_s_parm failed! errno(%d)\n",
				__func__, handle->index, errno);
		return -1;
	}

	return 0;
}

int V4L2GetFormat(void *hdl)
{
	Camera *handle = (Camera *) hdl;
	struct v4l2_format fmt;

	memset(&fmt, 0, sizeof(struct v4l2_format));
	if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	else
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	/**/
	if (ioctl(handle->fd, VIDIOC_G_FMT, &fmt) < 0) {
		CamErr("[%s] camera[%d] vidioc_g_fmt failed! errno(%d)\n",
				__func__, handle->index, errno);
		return -1;
	}
	/**/
	if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
		handle->width = fmt.fmt.pix_mp.width ;
		handle->height = fmt.fmt.pix_mp.height;
		handle->format = FormatDriverToUsr(fmt.fmt.pix_mp.pixelformat);
	} else {
		handle->width = fmt.fmt.pix.width ;
		handle->height = fmt.fmt.pix.height;
		handle->format = FormatDriverToUsr(fmt.fmt.pix.pixelformat);
	}

	return 0;
}

int V4L2SetFormat(void *hdl)
{
	Camera *handle = (Camera *) hdl;
	struct v4l2_format fmt;

	memset(&fmt, 0, sizeof(struct v4l2_format));
	if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		fmt.fmt.pix_mp.width = handle->width;
		fmt.fmt.pix_mp.height = handle->height;
		fmt.fmt.pix_mp.pixelformat = FormatUsrToDriver(handle->format);
		fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
	} else {
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmt.fmt.pix.width = handle->width;
		fmt.fmt.pix.height = handle->height;
		fmt.fmt.pix.pixelformat = FormatUsrToDriver(handle->format);
		fmt.fmt.pix.field = V4L2_FIELD_NONE;
	}
	if (ioctl(handle->fd, VIDIOC_S_FMT, &fmt) < 0) {
		CamErr("[%s] camera[%d] vidioc_s_fmt failed! errno(%d)\n",
				__func__, handle->index, errno);
		return -1;
	}

	return 0;
}


int V4L2RequestQueue(void *hdl)
{
	Camera *handle = (Camera *) hdl;
	struct v4l2_requestbuffers req;
	struct v4l2_buffer buf;
	int index = 0;
	int i = 0;

	memset(&req, 0, sizeof(struct v4l2_requestbuffers));
	if (handle->reqbufcount > 3)
		req.count = handle->reqbufcount;
	else
		req.count = 3;
	if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
		req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	else
		req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	if (ioctl(handle->fd, VIDIOC_REQBUFS, &req) < 0) {
		CamErr("[%s] camera[%d] vidioc_reqbufs failed! errno(%d)\n",
				__func__, handle->index, errno);
		return -1;
	}
	/* calloc bufqueue to fill */
	handle->bufqueue = calloc(req.count, sizeof(CameraBuffer));

	for (index = 0; index < req.count; index++) {
		memset(&buf, 0, sizeof(struct v4l2_buffer));
		if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
			/* number of elements in the planes array for multi-plane buffers */
			buf.length = MAX_PLANES;
			/* calloc buf.m.planes when mplane */
			buf.m.planes =  (struct v4l2_plane *)calloc(buf.length,
							sizeof(struct v4l2_plane));
		} else
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = index;
		/* query and get phyaddr */
		if (ioctl(handle->fd, VIDIOC_QUERYBUF, &buf) < 0) {
			CamErr("[%s] camera[%d] vidioc_querybuf failed! errno(%d)\n",
					__func__, handle->index, errno);
			return -1;
		}
		/* mmap phyaddr to viraddr and fill in bufqueue */
		handle->bufqueue[index].index = index;
		if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
			for (i = 0; i < MAX_PLANES; i++) {
				handle->bufqueue[index].length[i] = buf.m.planes[i].length;
				handle->bufqueue[index].phyaddr[i] = (void *)\
												buf.m.planes[i].m.mem_offset;
				handle->bufqueue[index].viraddr[i] = (void *)\
											mmap(NULL, buf.m.planes[i].length, \
											PROT_READ | PROT_WRITE, \
											MAP_SHARED , handle->fd, \
											buf.m.planes[i].m.mem_offset);
				CamDebug("buf index: %d, viraddr: %p,len: %x, phyaddr: %x\n", \
						index, handle->bufqueue[index].viraddr[i], \
						buf.m.planes[i].length, buf.m.planes[i].m.mem_offset);
			}
		} else {
			handle->bufqueue[index].length[0] = buf.length;
			handle->bufqueue[index].phyaddr[0] = (void *)buf.m.offset;
			handle->bufqueue[index].viraddr[0] = (void *)
												mmap(NULL , buf.length, \
													PROT_READ | PROT_WRITE, \
													MAP_SHARED , handle->fd, \
													buf.m.offset);
			CamDebug("buf index: %d, viraddr: %p,len: %x, phyaddr: %x\n", \
						index, handle->bufqueue[index].viraddr[0], \
						buf.length, buf.m.offset);
		}
		/* qbuf to the driver */
		if (ioctl(handle->fd, VIDIOC_QBUF, &buf) < 0) {
			CamErr("[%s] camera[%d] vidioc_qbuf failed! errno(%d)\n",
					__func__, handle->index, errno);
			/* free buf.m.planes when mplane */
			if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
				free(buf.m.planes);

			return -1;
		}
		/* free buf.m.planes when mplane */
		if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
			free(buf.m.planes);
	}

	return 0;
}


int V4L2ReleaseQueue(void *hdl)
{
	Camera *handle = (Camera *) hdl;
	int index = 0;
	int i = 0;

	/* munmap all bufs in bufqueue */
	for (index = 0; index < handle->reqbufcount; index++) {
		if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
			for (i = 0; i < MAX_PLANES; i++)
				munmap(handle->bufqueue[index].viraddr[i], \
						handle->bufqueue[index].length[i]);
		} else {
	 		munmap(handle->bufqueue[index].viraddr[0], \
					handle->bufqueue[index].length[0]);
		}
	}
	/* free bufqueue */
	free(handle->bufqueue);

	return 0;
}

int V4L2StreamOn(void *hdl)
{
	Camera *handle = (Camera *) hdl;
	enum v4l2_buf_type type;

	if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	else
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (ioctl(handle->fd, VIDIOC_STREAMON, &type) < 0) {
		CamErr("[%s] camera[%d] vidioc_streamon failed! errno(%d)\n",
				__func__, handle->index, errno);
		return -1;
	}

	return 0;
}

int V4L2StreamOff(void *hdl)
{
	Camera *handle = (Camera *) hdl;
	enum v4l2_buf_type type;

	if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	else
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (ioctl(handle->fd, VIDIOC_STREAMOFF, &type) < 0) {
		CamErr("[%s] camera[%d] vidioc_streamoff failed! errno(%d)\n",
				__func__, handle->index, errno);
		return -1;
	}

	return 0;
}

int V4L2DQBUF(void *hdl, void *camerabuffer)
{
	Camera *handle = (Camera *) hdl;
	CameraBuffer *cambuf = (CameraBuffer *)camerabuffer;
	struct v4l2_buffer buf;

	memset(&buf, 0, sizeof(struct v4l2_buffer));
	if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		/* number of elements in the planes array for multi-plane buffers */
		buf.length = MAX_PLANES;
		/* calloc buf.m.planes when mplane */
		buf.m.planes =  (struct v4l2_plane *)calloc(buf.length,
						sizeof(struct v4l2_plane));
	} else
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (ioctl(handle->fd, VIDIOC_DQBUF, &buf) != 0) {
		CamErr("[%s] camera[%d] vidioc_dqbuf failed! errno(%d)\n",
				__func__, handle->index, errno);
		/* free buf.m.planes when mplane */
		if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
			free(buf.m.planes);

		return -1;
	} else {
		CamDebug("camera[%d] dq buf[%d]success \n", handle->index, buf.index);
	}

	*cambuf = handle->bufqueue[buf.index];
	CamDebug("cambuf index: %d, viraddr: %p,len: %x, phyaddr: %p\n", \
						cambuf->index, cambuf->viraddr[0], cambuf->length[0], \
						cambuf->phyaddr[0]);

	return 0;
}

int V4L2QBUF(void *hdl, void *camerabuffer)
{
	Camera *handle = (Camera *) hdl;
	CameraBuffer *cambuf = (CameraBuffer *)camerabuffer;
	struct v4l2_buffer buf;
	int i;

	CamDebug("qbuf cambuf index: %d, viraddr: %p,len: %x, phyaddr: %p\n", \
						cambuf->index, cambuf->viraddr[0], cambuf->length[0], \
						cambuf->phyaddr[0]);

	memset(&buf, 0, sizeof(struct v4l2_buffer));
	buf.index = cambuf->index;
	if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		buf.length = MAX_PLANES;
		buf.m.planes = (struct v4l2_plane *)calloc(buf.length,
						sizeof(struct v4l2_plane));
		for (i = 0; i < MAX_PLANES; i++) {
			buf.m.planes[i].m.mem_offset = (unsigned int)cambuf->phyaddr[i];
		}
	}
	else
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	/**/
	if (ioctl(handle->fd, VIDIOC_QBUF, &buf) != 0) {
		CamErr("[%s] camera[%d] vidioc_qbuf failed! errno(%d)\n",
				__func__, handle->index, errno);
		if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
			free(buf.m.planes);
		return -1;
	} else {
		CamDebug("camera[%d] q buf[%d]success \n", handle->index, buf.index);
		if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
			free(buf.m.planes);
	}

	return 0;
}

int CameraGetConfig(void *hdl)
{
	Camera *handle = (Camera *) hdl;
	char str[KEY_LENGTH];
	char tempstr[KEY_LENGTH];
	int flag = 1;

	memset(str, 0, KEY_LENGTH);
	if (readKey(handle->index, KCAMERA_REQBUF_COUNT, str)) {
		if (atoi(str) >= 3)
			handle->reqbufcount = atoi(str);
		else
			handle->reqbufcount = 3;
	}

	memset(str, 0, KEY_LENGTH);
	if (readKey(handle->index, KCAMERA_FORMAT, str)) {
		if (strlen(str)) {
			while (SensorSupport[format_index].format) {
				if (SensorSupport[format_index].format == FormatStrToUsr(str)) {
					flag = 0;
					break;
				}
				format_index++;
			}
			if (flag)
				format_index = 0;
		} else
			format_index = 0;

		handle->format = SensorSupport[format_index].format;
	}

	memset(str, 0, KEY_LENGTH);
	if (readKey(handle->index, KCAMERA_SIZE, str)) {
		if (strlen(str)) {
			while (SensorSupport[format_index].size[size_index].width) {
				memset(tempstr, 0, KEY_LENGTH);
				sprintf(tempstr, "%dx%d", \
						SensorSupport[format_index].size[size_index].width, \
						SensorSupport[format_index].size[size_index].height);
				if (strncmp(tempstr, str, strlen(str)) == 0)
					break;
				size_index++;
			}
			if (SensorSupport[format_index].size[size_index].width == 0)
				size_index = 0;
		} else
			size_index = 0;

		handle->width = SensorSupport[format_index].size[size_index].width;
		handle->height = SensorSupport[format_index].size[size_index].height;
	}

		if (readKey(handle->index, KCAMERA_FRAMERATE, str)) {
		  if (strlen(str)) {
			while (SensorSupport[format_index]
					   .size[size_index]
					   .framerate[framerate_index]) {
			  if (atoi(str) == SensorSupport[format_index]
								.size[size_index]
								.framerate[framerate_index])
				break;
			  framerate_index++;
			}
			if (SensorSupport[format_index]
					.size[size_index]
					.framerate[framerate_index] == 0)
			  framerate_index = 0;
		  } else
			framerate_index = 0;

		  handle->framerate = SensorSupport[format_index]
								.size[size_index]
								.framerate[framerate_index];
		}

		return 0;
}

int CameraSetConfig(void *hdl)
{
	Camera *handle = (Camera *) hdl;
	char str[KEY_LENGTH];
	/* set reqbuf count */
	memset(str, 0, KEY_LENGTH);
	sprintf(str, "%d", handle->reqbufcount);
	setKey(handle->index, KCAMERA_REQBUF_COUNT, str);
	/* set width and height */
	memset(str, 0, KEY_LENGTH);
	sprintf(str, "%dx%d", handle->width, handle->height);
	setKey(handle->index, KCAMERA_SIZE, str);
	/* set format */
	memset(str, 0, KEY_LENGTH);
	setKey(handle->index, KCAMERA_FORMAT, FormatUsrToStr(handle->format));
	/* set framerate */
	memset(str, 0, KEY_LENGTH);
	sprintf(str, "%d", handle->framerate);
	setKey(handle->index, KCAMERA_FRAMERATE, str);

	return 0;
}

int CameraUpdateSupportConfig(void *hdl)
{
	Camera *handle = (Camera *) hdl;
	char str[KEY_LENGTH];
	int index;
	unsigned int offset;

	memset(str, 0, KEY_LENGTH);
	index = 0;
	offset = 0;
	while (SensorSupport[index].format) {
		offset += sprintf(str + offset, "%s ", \
						FormatUsrToStr(SensorSupport[index].format));
		index++;
	}
	setKey(handle->index, KCAMERA_SUPPORTED_FORMAT, str);

	memset(str, 0, KEY_LENGTH);
	index = 0;
	offset = 0;
	while (SensorSupport[format_index].size[index].width) {
		offset += sprintf(str + offset, "%dx%d ", \
							SensorSupport[format_index].size[index].width, \
							SensorSupport[format_index].size[index].height);
		index++;
	}
	setKey(handle->index, KCAMERA_SUPPORTED_SIZE, str);

	memset(str, 0, KEY_LENGTH);
	index = 0;
	offset = 0;
	while (SensorSupport[format_index].size[size_index].framerate[index]) {
		offset += sprintf(str + offset, "%d ", \
				SensorSupport[format_index].size[size_index].framerate[index]);
		index++;
	}
	setKey(handle->index, KCAMERA_SUPPORTED_FRAMERATE, str);
}
