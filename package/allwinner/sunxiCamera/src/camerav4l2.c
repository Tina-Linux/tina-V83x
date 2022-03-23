#include "camerav4l2.h"

#define camerav4l2_err(x,arg...) do{ \
								printf("[ERROR]" x,##arg); \
                            }while(0)

#ifdef __USE_VIN_ISP__

#include "AWIspApi.h"
#include "sunxi_camera_v2.h"

typedef struct ispInfo {
	int sensor_type;
	int isp_id;
	AWIspApi *isp_port;
} isp_info;

static int getSensorType(int fd)
{
	int ret = -1;
	struct v4l2_control ctrl;
	struct v4l2_queryctrl qc_ctrl;

	if (fd == 0)
		return 0xFF000000;

	memset(&qc_ctrl, 0, sizeof(struct v4l2_queryctrl));
	qc_ctrl.id = V4L2_CID_SENSOR_TYPE;
	ret = ioctl(fd, VIDIOC_QUERYCTRL, &qc_ctrl);
	if (ret < 0) {
		camerav4l2_err("[%s] query sensor type ctrl failed, errno(%d)\n", __func__, errno);
		return ret;
	}

	memset(&ctrl, 0, sizeof(struct v4l2_control));
	ctrl.id = V4L2_CID_SENSOR_TYPE;
	ret = ioctl(fd, VIDIOC_G_CTRL, &ctrl);
	if (ret < 0) {
		camerav4l2_err("[%s] get sensor type failed, errno(%d)\n", __func__, errno);
		return ret;
	}

	return ctrl.value;
}
#endif

int camerainit(camera_hal *camera)
{
	char camera_path[16];
	struct v4l2_capability cap;      /* Query device capabilities */
	struct v4l2_input inp;           /* select the current video input */
	struct v4l2_streamparm parms;    /* set streaming parameters */

	memset(camera_path, 0, sizeof(camera_path));
	sprintf(camera_path, "/dev/video%d", camera->video_index);
	camera->videofd = open(camera_path, O_RDWR | O_NONBLOCK, 0);
	if (camera->videofd < 0) {
		camerav4l2_err(" open /dev/video%d fail, errno(%d)\n", camera->video_index, errno);
		return -1;
	}

	/* Query device capabilities */
	memset(&cap, 0, sizeof(cap));
	if (ioctl(camera->videofd, VIDIOC_QUERYCAP, &cap) < 0) {
		camerav4l2_err(" Query device capabilities fail, errno(%d)\n", errno);
		close(camera->videofd);
		return -1;
	}

	if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
		camera->driver_type = V4L2_CAP_VIDEO_CAPTURE_MPLANE;
	} else if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
		camera->driver_type = V4L2_CAP_VIDEO_CAPTURE;
	} else {
		camerav4l2_err(" %s is not a capture device, cap.capabilities 0x%x\n", camera_path, cap.capabilities);
		close(camera->videofd);
		return -1;
	}

	/* select the current video input */
	memset(&inp, 0, sizeof(inp));
	inp.index = 0;
	inp.type = V4L2_INPUT_TYPE_CAMERA;
	if (ioctl(camera->videofd, VIDIOC_S_INPUT, &inp) < 0) {
		camerav4l2_err(" VIDIOC_S_INPUT failed, s_input: %d, errno(%d)\n", inp.index, errno);
		close(camera->videofd);
		return -1;
	}

#ifdef __USE_VIN_ISP__
	{
		isp_info *isp;

		camera->private_info = calloc(1, sizeof(isp_info));
		if (!camera->private_info) {
			camerav4l2_err(" calloc camera->private_info fail,close device\n");
			close(camera->videofd);
			return -1;
		}

		isp = (isp_info *)camera->private_info;
		isp->sensor_type = -1;
		isp->isp_id = -1;

		/* detect sensor type */
		isp->sensor_type = getSensorType(camera->videofd);
		if (isp->sensor_type == V4L2_SENSOR_TYPE_RAW)
			isp->isp_port = CreateAWIspApi();
	}
#endif

	/* set streaming parameters */
	memset(&parms, 0, sizeof(parms));
	if (camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
		parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	else
		parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	parms.parm.capture.timeperframe.numerator = 1;
	parms.parm.capture.timeperframe.denominator = camera->fps;
	if (ioctl(camera->videofd, VIDIOC_S_PARM, &parms) < 0) {
		camerav4l2_err(" Setting streaming parameters failed, numerator:%d denominator:%d, errno(%d)\n",
				parms.parm.capture.timeperframe.numerator,
				parms.parm.capture.timeperframe.denominator,
				errno);
		close(camera->videofd);
		return -1;
	}

	return 0;
}

int setformat(camera_hal *camera)
{
	struct v4l2_format fmt;

	/* set the data format */
	memset(&fmt, 0, sizeof(struct v4l2_format));

	if (camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		fmt.fmt.pix_mp.width = camera->width;
		fmt.fmt.pix_mp.height = camera->height;
		fmt.fmt.pix_mp.pixelformat = camera->format;
		fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
	} else {
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmt.fmt.pix.width = camera->width;
		fmt.fmt.pix.height = camera->height;
		fmt.fmt.pix.pixelformat = camera->format;
		fmt.fmt.pix.field = V4L2_FIELD_NONE;
	}

	if (ioctl(camera->videofd, VIDIOC_S_FMT, &fmt) < 0) {
		camerav4l2_err(" setting the data format failed, errno(%d)\n", errno);
		return -1;
	}

	if (camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
		if (camera->width != fmt.fmt.pix_mp.width || camera->height != fmt.fmt.pix_mp.height)
			camerav4l2_err(" does not support %u * %u\n", camera->width, camera->height);

		camera->width = fmt.fmt.pix_mp.width;
		camera->height = fmt.fmt.pix_mp.height;

		if (ioctl(camera->videofd, VIDIOC_G_FMT, &fmt) < 0)
			camerav4l2_err(" get the data format failed!\n");

		camera->nplanes = fmt.fmt.pix_mp.num_planes;
	} else {
		if (camera->width != fmt.fmt.pix.width || camera->height != fmt.fmt.pix.height)
			camerav4l2_err(" does not support %u * %u\n", camera->width, camera->height);

		camera->width = fmt.fmt.pix.width;
		camera->height = fmt.fmt.pix.height;
	}

	return 0;
}

int requestbuf(camera_hal *camera)
{
	int n_buffers;
	struct v4l2_requestbuffers req;  /* Initiate Memory Mapping or User Pointer I/O */
	struct v4l2_buffer buf;          /* Query the status of a buffer */

	/* Initiate Memory Mapping or User Pointer I/O */
	memset(&req, 0, sizeof(struct v4l2_requestbuffers));
	req.count = V4L2_BUF_NUM;
	if (camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
		req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	else
		req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	if (ioctl(camera->videofd, VIDIOC_REQBUFS, &req) < 0) {
		camerav4l2_err(" VIDIOC_REQBUFS failed, errno(%d)\n", errno);
		return -1;
	}

	/* Query the status of a buffers */
	camera->buf_count = req.count;
	camera->buffers = (struct buffer *)calloc(req.count, sizeof(struct buffer));
	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		memset(&buf, 0, sizeof(struct v4l2_buffer));
		if (camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		else
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n_buffers;
		if (camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
			buf.length = camera->nplanes;
			buf.m.planes = (struct v4l2_plane *)calloc(buf.length, sizeof(struct v4l2_plane));
		}

		if (ioctl(camera->videofd, VIDIOC_QUERYBUF, &buf) == -1) {
			camerav4l2_err(" VIDIOC_QUERYBUF error, buf.index %d errno(%d)\n", buf.index, errno);

			if (camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
				free(buf.m.planes);
			free(camera->buffers);
			return -1;
		}

		if (camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
			for (int i = 0; i < camera->nplanes; i++) {
				camera->buffers[n_buffers].length[i] = buf.m.planes[i].length;
				camera->buffers[n_buffers].start[i] = mmap(NULL , buf.m.planes[i].length,
								      PROT_READ | PROT_WRITE, \
								      MAP_SHARED , camera->videofd, \
								      buf.m.planes[i].m.mem_offset);
			}
			free(buf.m.planes);
			buf.m.planes = NULL;
		} else {
			camera->buffers[n_buffers].length[0] = buf.length;
			camera->buffers[n_buffers].start[0] = mmap(NULL , buf.length,
							      PROT_READ | PROT_WRITE, \
							      MAP_SHARED , camera->videofd, \
							      buf.m.offset);
		}
	}

	/* Exchange a buffer with the driver */
	for (n_buffers = 0; n_buffers < req.count; n_buffers++) {
		memset(&buf, 0, sizeof(struct v4l2_buffer));
		if (camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		else
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n_buffers;
		if (camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
			buf.length = camera->nplanes;
			buf.m.planes = (struct v4l2_plane *)calloc(buf.length, sizeof(struct v4l2_plane));
		}

		if (ioctl(camera->videofd, VIDIOC_QBUF, &buf) == -1) {
			camerav4l2_err(" VIDIOC_QBUF error, buf.index %d errno(%d)\n", buf.index, errno);

			if (camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
				free(buf.m.planes);
			free(camera->buffers);
			return -1;
		}

		if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
		    free(buf.m.planes);
			buf.m.planes = NULL;
		}
	}

	return 0;
}

int releasebuf(camera_hal *camera)
{
	/* munmap camera->buffers */
	if (camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
		for (int i = 0; i < camera->buf_count; ++i)
			for (int j = 0; j < camera->nplanes; j++)
				munmap(camera->buffers[i].start[j], camera->buffers[i].length[j]);
	} else {
		for (int i = 0; i < camera->buf_count; i++)
			munmap(camera->buffers[i].start[0], camera->buffers[i].length[0]);
	}

	/* free camera->buffers */
	free(camera->buffers);
	camera->buffers = NULL;
}

int streamon(camera_hal *camera)
{
	int ret = 0;
	enum v4l2_buf_type type;

	if (camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	else
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(camera->videofd, VIDIOC_STREAMON, &type);
	if (ret < 0) {
		camerav4l2_err("[%s] /dev/video%d streamon error errno(%d)\n", __func__, camera->video_index, errno);
		return ret;
	}

#ifdef __USE_VIN_ISP__
	{
		isp_info *isp;

		isp = (isp_info *)camera->private_info;
		/* setting ISP */
		if (isp && isp->sensor_type == V4L2_SENSOR_TYPE_RAW) {
			isp->isp_id = -1;
			isp->isp_id = isp->isp_port->ispGetIspId(camera->video_index);
			if (isp->isp_id >= 0)
				isp->isp_port->ispStart(isp->isp_id);
		}
	}
#endif

	return 0;
}

int streamoff(camera_hal *camera)
{
	int ret = 0;
	enum v4l2_buf_type type;

	if (camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	else
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(camera->videofd, VIDIOC_STREAMOFF, &type);
	if (ret < 0) {
		camerav4l2_err("[%s] /dev/video%d streamoff error, errno(%d)\n", __func__, camera->video_index, errno);
		return ret;
	}

#ifdef __USE_VIN_ISP__
	{
		isp_info *isp;

		isp = (isp_info *)camera->private_info;
		/* stop ISP */
		if (isp && isp->sensor_type == V4L2_SENSOR_TYPE_RAW
		    && isp->isp_id >= 0) {
			isp->isp_port->ispStop(isp->isp_id);
			isp->isp_id = -1;
		}
	}
#endif

	return 0;
}

int releasecamera(camera_hal *camera)
{
	int ret = -1;

	ret = close(camera->videofd);
	if (ret < 0) {
		camerav4l2_err("[%s] /dev/video%d close error, errno(%d)\n", __func__, camera->video_index, errno);
	}

#ifdef __USE_VIN_ISP__
	{
		isp_info *isp;

		isp = (isp_info *)camera->private_info;
		DestroyAWIspApi(isp->isp_port);
		isp->isp_port = NULL;
		free(isp);
		camera->private_info = NULL;
	}
#endif

	return 0;
}

int resetsync(camera_hal *camera)
{
	int ret;

	ret = ioctl(camera->videofd, VIDIOC_VIN_RESET_MCUSYNC, &ret);
	if (ret != 0) {
		camerav4l2_err("[%s] /dev/video%d ioctl reset mcusync errno(%d)\n",
                                        __func__, camera->video_index, errno);
		return ret;
	}

	return 0;
}

#define MAX_WAITING_TIME 3
int waitingbuf(camera_hal *camera)
{
	int ret = 0;
	fd_set fds;
	struct timeval tv;

	FD_ZERO(&fds);
	FD_SET(camera->videofd, &fds);

	tv.tv_sec = MAX_WAITING_TIME;
	tv.tv_usec = 0;

    ret = select(camera->videofd + 1, &fds, NULL, NULL, &tv);
    if(ret == -1){
        camerav4l2_err("[%s] /dev/video%d input select error, errno(%d)\n", __func__, camera->video_index, errno);
        return -1;
    }else if(ret == 0){
        camerav4l2_err("[%s] /dev/video%d input select timeout\n", __func__, camera->video_index);
        return -1;
    }

	return 0;
}

int dqbuf(camera_hal *camera)
{
	int ret = 0;
	struct v4l2_buffer buf;

	memset(&buf, 0, sizeof(struct v4l2_buffer));
	if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	else
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE){
		buf.length = camera->nplanes;
		buf.m.planes = (struct v4l2_plane *)calloc(camera->nplanes, sizeof(struct v4l2_plane));
	}

	ret = ioctl(camera->videofd, VIDIOC_DQBUF, &buf);
	if (ret != 0) {
		/* EAGAIN: nonblocking and no buffers to dequeue */
		if(errno != EAGAIN)
			camerav4l2_err("[%s] /dev/video%d ioctl v4l2 buf dequeue error, ret %d errno(%d)\n",
		                                    __func__, camera->video_index, ret, errno);
        if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
            free(buf.m.planes);
		return -1;
	}

	camera->buffers[buf.index].image_id = buf.reserved;
	camera->buffers[buf.index].lost_image_num = buf.sequence;
	camera->buffers[buf.index].exp_time = buf.reserved2;
	camera->buffers[buf.index].image_timestamp = buf.timestamp.tv_sec*1000000 + buf.timestamp.tv_usec;

    if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
        free(buf.m.planes);
		buf.m.planes = NULL;
	}

	return buf.index;
}

int qbuf(camera_hal *camera, int index)
{
	int ret;
	struct v4l2_buffer buf;

	memset(&buf, 0, sizeof(struct v4l2_buffer));
	if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	else
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE){
		buf.length = camera->nplanes;
		buf.m.planes = (struct v4l2_plane *)calloc(camera->nplanes, sizeof(struct v4l2_plane));
	}
    buf.index = index;
	ret = ioctl(camera->videofd, VIDIOC_QBUF, &buf);
	if (ret != 0) {
		camerav4l2_err("[%s] /dev/video%d ioctl v4l2 buf enqueue error, buf index %u ret %d errno(%d)\n",
                                        __func__, camera->video_index, buf.index, ret, errno);
        if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
            free(buf.m.planes);
		return ret;
	}

    if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
        free(buf.m.planes);
		buf.m.planes = NULL;
	}

	return 0;
}
