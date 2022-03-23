/*
 * Copyright (C) 2017 Allwinnertech
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "TRlog.h"
#include "Video_input_port.h"
#include "TParseConfig.h"
#include "TRcommon.h"

static pixelFormat cameraFormat[] = {
    TR_PIXEL_YVU420SP,
    TR_PIXEL_H264,
    TR_PIXEL_MJPEG,
    TR_PIXEL_YUYV422,
};

static int detectCameraFormat(void *hdl)
{
    vInPort *handle = (vInPort *)hdl;
    struct v4l2_fmtdesc fmtdesc;     /* Enumerate image formats */
    struct v4l2_format fmt;
    struct v4l2_frmsizeenum frmsize;
    int formatIndex = 0;
    int maxWidth = 0;
    int maxHeight = 0;
    char str[8];
    int ret = -1;

    fmtdesc.index = 0;
    if(handle->driveType == other_drive)
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    else
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    while(ioctl(handle->fd,VIDIOC_ENUM_FMT,&fmtdesc) == 0)
    {
        if(fmtdesc.pixelformat == GetCaptureFormat(handle->format)){
			TRlog(TR_LOG_VIDEO, " /dev/video%d support %s\n",
					handle->mCameraIndex, ReturnVideoTRFormatText(handle->format));
			break;
		}

        fmtdesc.index++;
    }

    memset(&fmt, 0, sizeof(struct v4l2_format));
    /* decect mainchannel format */
    if(handle->driveType == other_drive){
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = handle->MainWidth;
        fmt.fmt.pix.height = handle->MainHeight;
        fmt.fmt.pix.pixelformat = GetCaptureFormat(handle->format);
        fmt.fmt.pix.field = V4L2_FIELD_NONE;
    }else{
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        fmt.fmt.pix_mp.width = handle->MainWidth;
        fmt.fmt.pix_mp.height = handle->MainHeight;
        fmt.fmt.pix_mp.pixelformat = GetCaptureFormat(handle->format);
        fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
    }

    ret = ioctl(handle->fd, VIDIOC_TRY_FMT, &fmt);
    if(ret == 0){
        if(fmt.type == V4L2_BUF_TYPE_VIDEO_CAPTURE &&
            GetCaptureFormat(handle->format) == fmt.fmt.pix.pixelformat){
            if(handle->MainWidth != fmt.fmt.pix.width
                        || handle->MainHeight != fmt.fmt.pix.height){
                TRwarn(" /dev/video%d supports %s but does not support size %d*%d,detect size\n",
                            handle->mCameraIndex, ReturnVideoTRFormatText(handle->format),
                            handle->MainWidth, handle->MainHeight);
                goto DETECT_SIZE;
            }else
                return 0;
        }else{
            if(GetCaptureFormat(handle->format) == fmt.fmt.pix_mp.pixelformat){
                if(handle->MainWidth != fmt.fmt.pix_mp.width
                        || handle->MainHeight != fmt.fmt.pix_mp.height){
                    TRwarn(" /dev/video%d supports %s but does not support size %d*%d,detect size\n",
                            handle->mCameraIndex, ReturnVideoTRFormatText(handle->format),
                            handle->MainWidth, handle->MainHeight);
                    goto DETECT_SIZE;
                }else
                    return 0;
            }
        }
    }
    TRwarn(" detect /dev/video%d resolution and format error:format %s size %d*%d errno(%d)\n",
                            handle->mCameraIndex, ReturnVideoTRFormatText(handle->format),
                            handle->MainWidth, handle->MainHeight, errno);

    TRwarn(" will automatically detect the format\n");
    /* detect format */
    for(formatIndex = 0; formatIndex < (sizeof(cameraFormat)/sizeof(cameraFormat[0])); formatIndex++){

        memset(&fmt, 0, sizeof(struct v4l2_format));
        if(handle->driveType == other_drive){
            fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            fmt.fmt.pix.width = handle->MainWidth;
            fmt.fmt.pix.height = handle->MainHeight;
            fmt.fmt.pix.pixelformat = GetCaptureFormat(cameraFormat[formatIndex]);
            fmt.fmt.pix.field = V4L2_FIELD_NONE;
        }else{
            fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
            fmt.fmt.pix_mp.width = handle->MainWidth;
            fmt.fmt.pix_mp.height = handle->MainHeight;
            fmt.fmt.pix_mp.pixelformat = GetCaptureFormat(cameraFormat[formatIndex]);
            fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
        }

        ret = ioctl(handle->fd, VIDIOC_TRY_FMT, &fmt);
        if(ret == 0 && ((fmt.type == V4L2_BUF_TYPE_VIDEO_CAPTURE &&
                    GetCaptureFormat(cameraFormat[formatIndex]) == fmt.fmt.pix.pixelformat)
                || (fmt.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE &&
                    GetCaptureFormat(cameraFormat[formatIndex]) == fmt.fmt.pix_mp.pixelformat)))
            break;
    }

    if(formatIndex >= (sizeof(cameraFormat)/sizeof(cameraFormat[0]))){
        TRerr("[%s] /dev/video%d format probe failed\n", __func__, handle->mCameraIndex);
        return -1;
    }else{
        /* save format,modify recorder.cfg configuration */
        handle->format = cameraFormat[formatIndex];
        setKey(handle->mCameraIndex, kVIDEO_FORMAT, ReturnVideoTRFormatText(handle->format));
    }

DETECT_SIZE:
    TRwarn(" will automatically detect the max resolution\n");
    memset(&frmsize,0,sizeof(frmsize));
    frmsize.index = 0;
    frmsize.pixel_format = GetCaptureFormat(handle->format);
    while(ioctl(handle->fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0){
        if(handle->driveType == other_drive){
            if(maxWidth <= frmsize.discrete.width && maxHeight <= frmsize.discrete.height){
                maxWidth = frmsize.discrete.width;
                maxHeight = frmsize.discrete.height;
            }
        }else{
            if(maxWidth <= frmsize.stepwise.max_width && maxHeight <= frmsize.stepwise.max_height){
                maxWidth = frmsize.stepwise.max_width;
                maxHeight = frmsize.stepwise.max_height;
            }
        }

        frmsize.index++;
    }

    /* size probe succeeded, modify recorder.cfg configuration */
	handle->MainWidth = maxWidth;
	handle->MainHeight = maxHeight;

    TRwarn(" detect success, /dev/video%d format and resolution:format %s size %d*%d\n",
                            handle->mCameraIndex, ReturnVideoTRFormatText(handle->format),
                            handle->MainWidth, handle->MainHeight);

    memset(str, 0, sizeof(str));
    sprintf(str, "%d", handle->MainWidth);
    setKey(handle->mCameraIndex, kVIDEO_WIDTH, str);
    memset(str, 0, sizeof(str));
    sprintf(str, "%d", handle->MainHeight);
    setKey(handle->mCameraIndex, kVIDEO_HEIGHT, str);
    TRwarn(" write back %s success\n", CAMERA_KEY_CONFIG_PATH);

    return 0;
}

#ifdef __USE_VIN_ISP__
static int getSensorType(int fd)
{
    int ret = -1;
    struct v4l2_control ctrl;
    struct v4l2_queryctrl qc_ctrl;

    if (fd == 0)
        return 0xFF000000;

    memset(&qc_ctrl, 0, sizeof(struct v4l2_queryctrl));
    qc_ctrl.id = V4L2_CID_SENSOR_TYPE;
    ret = ioctl (fd, VIDIOC_QUERYCTRL, &qc_ctrl);
    if (ret < 0){
        TRerr("[%s] query sensor type ctrl failed\n", __func__);
        return ret;
    }

    memset(&ctrl, 0, sizeof(struct v4l2_control));
    ctrl.id = V4L2_CID_SENSOR_TYPE;
    ret = ioctl(fd, VIDIOC_G_CTRL, &ctrl);
    if(ret < 0){
        TRerr("[%s] get sensor type failed,errno(%d)\n", __func__, errno);
        return ret;
    }

    return ctrl.value;
}
#endif

int VportOpen(void *hdl, int index)
{
	vInPort *handle = (vInPort *) hdl;
	char dev_name[30];

	if (handle->state != CAMERA_STATE_NONE) {
		TRerr("[%s] Open /dev/video%d error\n", __func__, index);
		return -1;
	}
    handle->mCameraIndex = index;
	snprintf(dev_name, sizeof(dev_name), "/dev/video%d", index);
    handle->fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
	if (handle->fd < 0) {
		TRerr("[%s] Open /dev/video%d error\n", __func__, handle->mCameraIndex);
		return -1;
	}else
	    TRlog(TR_LOG_VIDEO, " open %s ok\n", dev_name);

	handle->state = CAMERA_STATE_CREATED;
	return 0;
}

int VportClose(void *hdl)
{
	vInPort *handle = (vInPort *) hdl;
	int ret;
	TRlog(TR_LOG_VIDEO, "enter Vport %d close,state:%d fd:%d handle:%p\n",
                            handle->mCameraIndex, handle->state, handle->fd, hdl);
	if (handle->state != CAMERA_STATE_CREATED) {
		TRerr("[%s] Camera %d state error\n", __func__, handle->mCameraIndex);
		return -1;
	}

    ret = close(handle->fd);
	if (ret != 0) {
		TRerr("[%s] Close /dev/video%d error\n", __func__, handle->mCameraIndex);
		return ret;
	}

	handle->state = CAMERA_STATE_NONE;
	return 0;
}

int VportInit(void *hdl, int enable, int format, int framerate, int width,
	      int height, int subwidth, int subheight,int ispEnable, int rot_angle, int use_wm)
{
	vInPort *handle = (vInPort *) hdl;
	int ret;
    struct v4l2_capability cap;      /* Query device capabilities */
	struct v4l2_input inp;
	struct v4l2_streamparm parms;
	struct v4l2_format fmt;
	struct v4l2_pix_format sub_fmt;

	if (handle->state != CAMERA_STATE_CREATED) {
		TRerr("[%s] Camera %d state error.state:%d\n", __func__, handle->mCameraIndex, handle->state);
		return -1;
	}

    /* Query device capabilities */
    if(ioctl(handle->fd, VIDIOC_QUERYCAP, &cap) < 0){
        TRerr("[%s] Query /dev/video%d capabilities fail!!! errno(%d)\n",
                                        __func__, handle->mCameraIndex, errno);
    }
    if((strcmp((char *)cap.driver,"sunxi-vfe") == 0) || (strcmp((char *)cap.driver,"sunxi-vin") == 0)){
        handle->cameraType = csi_camera;
        /* vin underlying drive framework is not the same */
        if(strcmp((char *)cap.driver,"sunxi-vfe") == 0)
            handle->driveType = other_drive;
        else
            handle->driveType = vin_drive;
    }else if(strcmp((char *)cap.driver,"uvcvideo") == 0){
        handle->cameraType = usb_camera;
        handle->driveType = other_drive;
    }else{
        TRerr("[%s] unknow camera type is %s\n",  __func__, cap.driver);
    }

	handle->enable = enable;
	handle->format = format;
	handle->use_isp = ispEnable;
	handle->use_wm = use_wm;
	handle->framerate = framerate;
	handle->rot_angle = rot_angle;
	handle->MainWidth = width;
	handle->MainHeight = height;
	handle->SubWidth = subwidth;
	handle->SubHeight = subheight;

	/* set capture input */
	inp.index = 0;
	inp.type = V4L2_INPUT_TYPE_CAMERA;
	if ((ret = ioctl(handle->fd, VIDIOC_S_INPUT, &inp)) < 0) {
		TRerr("[%s] Set /dev/video%d input error:input index %d errno(%d)\n",
                                        __func__, handle->mCameraIndex, inp.index, errno);
		return ret;
	}

#ifdef __USE_VIN_ISP__
    handle->sensor_type = -1;
    handle->ispId = -1;

    /* detect sensor type */
    handle->sensor_type = getSensorType(handle->fd);
    if(handle->sensor_type == V4L2_SENSOR_TYPE_RAW){
        handle->ispPort = CreateAWIspApi();
        TRlog(TR_LOG_VIDEO, " raw sensor use vin isp\n");
    }else{
        TRwarn(" [%s] compile VIN isp,but sensor is not raw sensor, sensor_type:%d\n",
                                                            __func__, handle->sensor_type);
    }
#endif

#ifndef __USE_VIN_ISP__
    /* detect format and size */
    ret = detectCameraFormat(handle);
    if(ret < 0){
        TRerr("[%s] detect /dev/video%d format parameter error\n", __func__, handle->mCameraIndex);
		return ret;
    }
#endif

	/* set capture parms */
    memset(&parms, 0, sizeof(struct v4l2_streamparm));
    if(handle->driveType == other_drive){
        parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    }else{
        parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	}
    parms.parm.capture.timeperframe.numerator = 1;
	parms.parm.capture.timeperframe.denominator = handle->framerate;
	if ((ret = ioctl(handle->fd, VIDIOC_S_PARM, &parms)) < 0) {
		TRerr("[%s] Set /dev/video%d input error errno(%d)\n", __func__, handle->mCameraIndex, errno);
		return ret;
	}

	memset(&fmt, 0, sizeof(struct v4l2_format));
	/* set mainchannel format */
    if(handle->driveType == other_drive){
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = handle->MainWidth;
        fmt.fmt.pix.height = handle->MainHeight;
        fmt.fmt.pix.pixelformat = GetCaptureFormat(handle->format);
        fmt.fmt.pix.field = V4L2_FIELD_NONE;
    }else{
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        fmt.fmt.pix_mp.width = handle->MainWidth;
        fmt.fmt.pix_mp.height = handle->MainHeight;
        fmt.fmt.pix_mp.pixelformat = GetCaptureFormat(handle->format);
        fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
    }
	/* if use isp set subchannel format */
	if (handle->use_isp == 1 && handle->driveType == other_drive) {
		memset(&sub_fmt, 0, sizeof(struct v4l2_pix_format));
		fmt.fmt.pix.subchannel = &sub_fmt;
		sub_fmt.width = handle->SubWidth;
		sub_fmt.height = handle->SubHeight;
		sub_fmt.pixelformat = V4L2_PIX_FMT_NV21;
		sub_fmt.field = V4L2_FIELD_NONE;
		sub_fmt.rot_angle = handle->rot_angle;
	}
	if ((ret = ioctl(handle->fd, VIDIOC_S_FMT, &fmt)) < 0) {
		TRerr("[%s] Set /dev/video%d resolution and format error:format %s size %d*%d errno(%d)\n",
                            __func__, handle->mCameraIndex, get_format_name(fmt.fmt.pix.pixelformat),
                            handle->MainWidth, handle->MainHeight, errno);
		return ret;
	}else{
	    if(handle->driveType == other_drive){
            handle->MainWidth = fmt.fmt.pix.width;
            handle->MainHeight = fmt.fmt.pix.height;
            TRlog(TR_LOG_VIDEO, " /dev/video%d VIDIOC_S_FMT succeed\n", handle->mCameraIndex);
            TRlog(TR_LOG_VIDEO, " fmt.type = %d\n",fmt.type);
            TRlog(TR_LOG_VIDEO, " fmt.fmt.pix.width = %d\n",fmt.fmt.pix.width);
            TRlog(TR_LOG_VIDEO, " fmt.fmt.pix.height = %d\n",fmt.fmt.pix.height);
            TRlog(TR_LOG_VIDEO, " fmt.fmt.pix.pixelformat = %s\n",get_format_name(fmt.fmt.pix.pixelformat));
            TRlog(TR_LOG_VIDEO, " fmt.fmt.pix.field = %d\n",fmt.fmt.pix.field);
        }else{
            handle->MainWidth = fmt.fmt.pix_mp.width;
            handle->MainHeight = fmt.fmt.pix_mp.height;
            if (ioctl(handle->fd, VIDIOC_G_FMT, &fmt) < 0)
                TRerr("[%s] /dev/video%d VIDIOC_G_FMT error\n", __func__, handle->mCameraIndex);
            else
                handle->nplanes = fmt.fmt.pix_mp.num_planes;
            TRlog(TR_LOG_VIDEO, " /dev/video%d VIDIOC_S_FMT succeed\n", handle->mCameraIndex);
            TRlog(TR_LOG_VIDEO, " fmt.type = %d\n",fmt.type);
            TRlog(TR_LOG_VIDEO, " fmt.fmt.pix_mp.width = %d\n",fmt.fmt.pix_mp.width);
            TRlog(TR_LOG_VIDEO, " fmt.fmt.pix_mp.height = %d\n",fmt.fmt.pix_mp.height);
            TRlog(TR_LOG_VIDEO, " fmt.fmt.pix_mp.pixelformat = %s\n",get_format_name(fmt.fmt.pix_mp.pixelformat));
            TRlog(TR_LOG_VIDEO, " fmt.fmt.pix_mp.field = %d\n",fmt.fmt.pix_mp.field);
        }
    }

	/* init memops */
	handle->pMemops = GetMemAdapterOpsS();
	SunxiMemOpen(handle->pMemops);

	return 0;
}

int VportRequestbuf(void *hdl, int count)
{
	vInPort *handle = (vInPort *) hdl;
	int ret;
	int i;
        int planes;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_buffer buf;

	TRlog(TR_LOG_VIDEO, " /dev/video%d requeset buf:%d\n",handle->mCameraIndex, handle->state);

	if (handle->state != CAMERA_STATE_CREATED) {
		TRerr("[%s] Camera %d state error", __func__, handle->mCameraIndex);
		return -1;
	}

	/* reqbufs */
	memset(&reqbuf, 0, sizeof(struct v4l2_requestbuffers));
	reqbuf.count = count;
    if(handle->driveType == other_drive){
	    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    }else{
        reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	}
    reqbuf.memory = V4L2_MEMORY_MMAP;
	if ((ret = ioctl(handle->fd, VIDIOC_REQBUFS, &reqbuf)) < 0) {
		TRerr("[%s] /dev/video%d request %d buffer error, ret %d errno(%d)\n",
                            __func__, handle->mCameraIndex, count, ret, errno);
		return ret;
	}

	/* querybuf and mmap */
    handle->buf_num = reqbuf.count;
	handle->buffers = calloc(count, sizeof(buffer));
	for (i = 0; i < handle->buf_num; i++) {
		memset(&buf, 0, sizeof(struct v4l2_buffer));
        if(handle->driveType == other_drive)
		    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        else
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
        if(handle->driveType == vin_drive){
            buf.length = handle->nplanes;
            buf.m.planes =  (struct v4l2_plane *)calloc(buf.length, sizeof(struct v4l2_plane));
        }
		if ((ret = ioctl(handle->fd, VIDIOC_QUERYBUF, &buf)) < 0) {
			TRerr("[%s] /dev/video%d QUERYBUF error errno(%d)\n", __func__, handle->mCameraIndex, errno);
            if(handle->driveType == vin_drive)
                free(buf.m.planes);
            free(handle->buffers);
			return ret;
		}
        if(handle->driveType == other_drive){
		handle->buffers[i].length[0] = buf.length;
		handle->buffers[i].start[0] = mmap(NULL, buf.length,
						PROT_READ | PROT_WRITE,
						MAP_SHARED,
						handle->fd, buf.m.offset);
		if (handle->buffers[i].start[0] == MAP_FAILED) {
			TRerr("[%s] /dev/video%d MMAP error", __func__, handle->mCameraIndex);
                free(handle->buffers);
			return -1;
		}
		TRlog(TR_LOG_VIDEO, " /dev/video%d map buffer index: %d, start: %p, len: %u, offset: 0x%x\n",
			handle->mCameraIndex, i, handle->buffers[i].start[0], buf.length, buf.m.offset);
        }else{
            for(planes = 0; planes < handle->nplanes; planes++){
                handle->buffers[i].length[planes] = buf.m.planes[planes].length;
                handle->buffers[i].start[planes] = mmap(NULL , buf.m.planes[planes].length,
                                                   PROT_READ | PROT_WRITE, \
                                                   MAP_SHARED , handle->fd, \
                                                   buf.m.planes[planes].m.mem_offset);
                TRlog(TR_LOG_VIDEO, " /dev/video%d  map buffer index: %d, mem: %p, len: %u, offset: 0x%x\n",
                   handle->mCameraIndex, i, handle->buffers[i].start[planes],buf.m.planes[planes].length,
                   buf.m.planes[planes].m.mem_offset);
            }
            free(buf.m.planes);
        }
	}

	/* enqbuf for ready */
	for (i = 0; i < handle->buf_num; i++) {
		memset(&buf, 0, sizeof(struct v4l2_buffer));
        if(handle->driveType == other_drive)
		    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        else
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
        if(handle->driveType == vin_drive){
            buf.length = handle->nplanes;
            buf.m.planes =  (struct v4l2_plane *)calloc(buf.length, sizeof(struct v4l2_plane));
        }

        ret = ioctl(handle->fd, VIDIOC_QBUF, &buf);
		if (ret < 0) {
			TRerr("[%s] /dev/video%d QBUF error errno(%d)\n", __func__, handle->mCameraIndex, errno);
            if(handle->driveType == vin_drive)
                free(buf.m.planes);
			return ret;
		}
        if(handle->driveType == vin_drive)
            free(buf.m.planes);
	}

	handle->state = CAMERA_STATE_PREPARED;
	return 0;
}

int VportReleasebuf(void *hdl, int count)
{
	vInPort *handle = (vInPort *) hdl;
	int i;
        int j;

	if (handle->state != CAMERA_STATE_PREPARED) {
		TRerr("[%s] Camera %d state error\n", __func__, handle->mCameraIndex);
		return -1;
	}

    if(handle->driveType == other_drive){
	for (i = 0; i < count; i++)
		munmap(handle->buffers[i].start[0], handle->buffers[i].length[0]);
    }else{
        for (i = 0; i < count; ++i)
            for (j = 0; j < handle->nplanes; j++)
                munmap(handle->buffers[i].start[j], handle->buffers[i].length[j]);
    }
	free(handle->buffers);

	/* close memops */
	SunxiMemClose(handle->pMemops);

	handle->state = CAMERA_STATE_CREATED;
	return 0;
}

int VportStartcapture(void *hdl)
{
	vInPort *handle = (vInPort *) hdl;
	enum v4l2_buf_type type;
	int ret;

	if (handle->state != CAMERA_STATE_PREPARED) {
		TRerr("[%s] Camera %d state error\n", __func__, handle->mCameraIndex);
		return -1;
	}

    if(handle->driveType == other_drive)
	    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    else
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

    ret = ioctl(handle->fd, VIDIOC_STREAMON, &type);
	if (ret < 0) {
		TRerr("[%s] /dev/video%d Streamon error errno(%d)\n", __func__, handle->mCameraIndex, errno);
		return ret;
	}

#ifdef __USE_VIN_ISP__
    /* setting ISP */
    if(handle->sensor_type == V4L2_SENSOR_TYPE_RAW){
        handle->ispId = -1;
        handle->ispId = handle->ispPort->ispGetIspId(handle->mCameraIndex);
        if(handle->ispId >= 0)
            handle->ispPort->ispStart(handle->ispId);
    }
#endif

    handle->lastFarmeTime = 0;
    handle->intervalTime = (1000/handle->framerate)*5/4;

	handle->state = CAMERA_STATE_CAPTURING;
	return 0;
}

int VportStopcapture(void *hdl)
{
	vInPort *handle = (vInPort *) hdl;
	enum v4l2_buf_type type;
	int ret;

	if (handle->state != CAMERA_STATE_CAPTURING) {
		TRerr("[%s] Camera %d state error\n", __func__, handle->mCameraIndex);
		return -1;
	}

    if(handle->driveType == other_drive){
	    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    }else{
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	}

    ret = ioctl(handle->fd, VIDIOC_STREAMOFF, &type);
    if (ret < 0) {
		TRerr("[%s] /dev/video%d Streamoff error errno(%d)\n", __func__, handle->mCameraIndex, errno);
		return ret;
	}

#ifdef __USE_VIN_ISP__
    /* stop ISP */
    if(handle->sensor_type == V4L2_SENSOR_TYPE_RAW
                                    && handle->ispId >= 0){
        handle->ispPort->ispStop(handle->ispId);
        DestroyAWIspApi(handle->ispPort);
        handle->ispPort = NULL;
    }
#endif

	handle->state = CAMERA_STATE_PREPARED;
	return 0;
}

int VportDequeue(void *hdl, dataParam *param)
{
	int ret;
	unsigned int MainYSize;
	unsigned int SubYSize;
	fd_set fds;
	struct timeval tv;
	struct v4l2_buffer buf;
	vInPort *handle = (vInPort *) hdl;

	if (handle->state != CAMERA_STATE_CAPTURING) {
		TRerr("[%s] Camera %d state error,state:%d\n", __func__, handle->mCameraIndex, handle->state);
		return -1;
	}

	FD_ZERO(&fds);
	FD_SET(handle->fd, &fds);

	tv.tv_sec = CSI_SELECT_FRM_TIMEOUT;
	tv.tv_usec = 0;

    ret = select(handle->fd + 1, &fds, NULL, NULL, &tv);
    if(ret == -1){
        TRerr("[%s] /dev/video%d input Select error\n", __func__, handle->mCameraIndex);
        return -1;
    }else if(ret == 0){
        TRerr("[%s] /dev/video%d input Select timeout\n", __func__, handle->mCameraIndex);
        return -1;
    }

	memset(&buf, 0, sizeof(struct v4l2_buffer));
    if(handle->driveType == other_drive){
	    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    }else{
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	}
    buf.memory = V4L2_MEMORY_MMAP;
    if(handle->driveType == vin_drive){
        buf.length = handle->nplanes;
        buf.m.planes = (struct v4l2_plane *)calloc(handle->nplanes, sizeof(struct v4l2_plane));
    }
	ret = ioctl(handle->fd, VIDIOC_DQBUF, &buf);
	if (ret != 0) {
		TRerr("[%s] /dev/video%d ioctl v4l2 buf dequeue error,ret %d errno(%d)\n",
                                        __func__, handle->mCameraIndex, ret, errno);
        if(handle->driveType == vin_drive)
            free(buf.m.planes);
		return ret;
	}

	MainYSize = ALIGN_16B(handle->MainWidth) * ALIGN_16B(handle->MainHeight);
	SubYSize = ALIGN_16B(handle->SubWidth) * ALIGN_16B(handle->SubHeight);
	param->MainSize = ALIGN_4K(ALIGN_16B(handle->MainWidth) *
			  ALIGN_16B(handle->MainHeight)*3 >> 1);
	param->SubSize = ALIGN_4K(ALIGN_16B(handle->SubWidth) *
			 ALIGN_16B(handle->SubHeight)*3 >> 1);
	param->RotSize = param->SubSize;
	param->BytesUsed = buf.bytesused;
	if(param->BytesUsed == 0)
		param->BytesUsed = buf.length;
	if (handle->driveType == vin_drive)
		param->BytesUsed = buf.m.planes[0].bytesused;

	param->bufferId = buf.index;
	//param->pts = (long long)(buf.timestamp.tv_sec) * 1000 +
	//    (long long)(buf.timestamp.tv_usec) / 1000;
    param->pts = GetNowMs();

    handle->lastFarmeTime = param->pts;

	param->MainYVirAddr = handle->buffers[buf.index].start[0];
    if(handle->driveType == other_drive){
	    param->MainYPhyAddr = (void *)buf.m.offset;
    }else{
        param->MainYPhyAddr = (void *)buf.m.planes[0].m.mem_offset;
	}
    param->MainCVirAddr = param->MainYVirAddr + MainYSize;
	param->MainCPhyAddr = param->MainYPhyAddr + MainYSize;

	param->SubYVirAddr = param->MainYVirAddr + param->MainSize;
	param->SubYPhyAddr = param->MainYPhyAddr + param->MainSize;
	param->SubCVirAddr = param->SubYVirAddr + SubYSize;
	param->SubCPhyAddr = param->SubYPhyAddr + SubYSize;

	if (handle->rot_angle) {
		param->RotYVirAddr = param->SubYVirAddr + param->SubSize;
		param->RotYPhyAddr = param->SubYPhyAddr + param->SubSize;
		param->RotCVirAddr = param->RotYVirAddr + SubYSize;
		param->RotCPhyAddr = param->RotYPhyAddr + SubYSize;
	}

	if(handle->driveType == vin_drive)
		free(buf.m.planes);

	return 0;
}

int VportEnqueue(void *hdl, unsigned int index)
{
	int ret;
	struct v4l2_buffer buf;
	vInPort *handle = (vInPort *) hdl;

	if (handle->state != CAMERA_STATE_CAPTURING) {
		TRerr("[%s] Camera %d state error\n", __func__, handle->mCameraIndex);
		return -1;
	}

    if(index > handle->buf_num){
        TRerr("[%s] /dev/video%d buf index %u exceeds buf count %d\n",
                                        __func__, handle->mCameraIndex, index, handle->buf_num);
        return -1;
    }

	memset(&buf, 0, sizeof(struct v4l2_buffer));
    if(handle->driveType == other_drive){
	    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    }else{
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	}
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = index;
    if(handle->driveType == vin_drive){
        buf.length = handle->nplanes;
        buf.m.planes = (struct v4l2_plane *)calloc(handle->nplanes, sizeof(struct v4l2_plane));
    }
	ret = ioctl(handle->fd, VIDIOC_QBUF, &buf);
	if (ret != 0) {
		TRerr("[%s] /dev/video%d ioctl v4l2 buf enqueue error,buf index %u ret %d errno(%d)\n",
                                        __func__, handle->mCameraIndex, buf.index, ret, errno);
        if(handle->driveType == vin_drive)
            free(buf.m.planes);
		return ret;
	}

    if(handle->driveType == vin_drive){
        free(buf.m.planes);
    }

	return 0;
}

int VportgetEnable(void *hdl)
{
	vInPort *handle = (vInPort *) hdl;

	return handle->enable;
}

int VportsetEnable(void *hdl, int enable)
{
	vInPort *handle = (vInPort *) hdl;

	handle->enable = enable;

	return 0;
}

int VportgetFrameRate(void *hdl)
{
	vInPort *handle = (vInPort *) hdl;

	return handle->framerate;
}

int VportsetFrameRate(void *hdl, int framerate)
{
	vInPort *handle = (vInPort *) hdl;
	struct v4l2_streamparm parms;
	int ret;

	if (handle->state != CAMERA_STATE_CREATED) {
		TRerr("[%s] Camera state error\n", __func__);
		return -1;
	}

    memset(&parms, 0, sizeof(struct v4l2_streamparm));
    if(handle->driveType == other_drive){
	    parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    }else{
        parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	}
    parms.parm.capture.timeperframe.numerator = 1;
	parms.parm.capture.timeperframe.denominator = framerate;

    ret = ioctl(handle->fd, VIDIOC_S_PARM, &parms);
	if (ret < 0) {
		TRerr("[%s] Set csi input framerate error\n", __func__);
		return ret;
	}

    handle->framerate = framerate;

	return 0;
}

int VportgetFormat(void *hdl)
{
	vInPort *handle = (vInPort *) hdl;

	return handle->format;
}

int VportsetFormat(void *hdl, int format)
{
	vInPort *handle = (vInPort *) hdl;
	struct v4l2_format fmt;
	struct v4l2_pix_format sub_fmt;
	int ret;

	if (handle->state != CAMERA_STATE_CREATED) {
		TRerr("[%s] Camera state error\n", __func__);
		return -1;
	}

	/* set mainchannel format */
	memset(&fmt, 0, sizeof(struct v4l2_format));
	/* set mainchannel format */
    if(handle->driveType == other_drive){
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = handle->MainWidth;
        fmt.fmt.pix.height = handle->MainHeight;
        fmt.fmt.pix.pixelformat = GetCaptureFormat(format);
        fmt.fmt.pix.field = V4L2_FIELD_NONE;
    }else{
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        fmt.fmt.pix_mp.width = handle->MainWidth;
        fmt.fmt.pix_mp.height = handle->MainHeight;
        fmt.fmt.pix_mp.pixelformat = GetCaptureFormat(format);
        fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
    }
	/* if use isp set subchannel format */
	if (handle->use_isp == 1 && handle->driveType == other_drive) {
		memset(&sub_fmt, 0, sizeof(struct v4l2_pix_format));
		fmt.fmt.pix.subchannel = &sub_fmt;
		sub_fmt.width = handle->SubWidth;
		sub_fmt.height = handle->SubHeight;
		sub_fmt.pixelformat = V4L2_PIX_FMT_NV21;
		sub_fmt.field = V4L2_FIELD_NONE;
		sub_fmt.rot_angle = handle->rot_angle;
	}

    ret = ioctl(handle->fd, VIDIOC_S_FMT, &fmt);
	if (ret < 0) {
		TRerr("[%s]  VIDIOC_S_FMT error\n", __func__);
		return ret;
	}

    handle->format = format;

	return 0;
}

int VportgetSize(void *hdl, int *width, int *height)
{
	vInPort *handle = (vInPort *) hdl;
	if (handle->MainWidth == 0 || handle->MainHeight == 0) {
		TRerr("[%s] width and height is NULL\n", __func__);
		return -1;
	}

	*width = handle->MainWidth;
	*height = handle->MainHeight;

	return 0;
}

int VportsetSize(void *hdl, int width, int height)
{
	vInPort *handle = (vInPort *) hdl;
	struct v4l2_format fmt;
	struct v4l2_pix_format sub_fmt;
	int ret;

	if (handle->state != CAMERA_STATE_CREATED) {
		TRerr("[%s] Camera state error\n", __func__);
		return -1;
	}

	handle->format = width;
	handle->MainHeight = height;

	memset(&fmt, 0, sizeof(struct v4l2_format));
	/* set mainchannel format */
    if(handle->driveType == other_drive){
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = handle->MainWidth;
        fmt.fmt.pix.height = handle->MainHeight;
        fmt.fmt.pix.pixelformat = GetCaptureFormat(handle->format);
        fmt.fmt.pix.field = V4L2_FIELD_NONE;
    }else{
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        fmt.fmt.pix_mp.width = handle->MainWidth;
        fmt.fmt.pix_mp.height = handle->MainHeight;
        fmt.fmt.pix_mp.pixelformat = GetCaptureFormat(handle->format);
        fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
    }
	/* if use isp set subchannel format */
	if (handle->use_isp == 1 && handle->driveType == other_drive) {
		memset(&sub_fmt, 0, sizeof(struct v4l2_pix_format));
		fmt.fmt.pix.subchannel = &sub_fmt;
		sub_fmt.width = handle->SubWidth;
		sub_fmt.height = handle->SubHeight;
		sub_fmt.pixelformat = V4L2_PIX_FMT_NV21;
		sub_fmt.field = V4L2_FIELD_NONE;
		sub_fmt.rot_angle = handle->rot_angle;
	}

	if (ret = ioctl(handle->fd, VIDIOC_S_FMT, &fmt) < 0) {
		TRerr("[%s]  VIDIOC_S_FMT error\n", __func__);
		return ret;
	}

	return 0;
}

int VportGetIndex(void *hdl, int *index){
    vInPort *handle = (vInPort *) hdl;
    *index = handle->mCameraIndex;
    return 1;
}

int VportGetContrast(void *hdl)
{
	struct v4l2_control control;
	vInPort *handle = (vInPort *) hdl;

	memset (&control, 0, sizeof (control));
	control.id = V4L2_CID_CONTRAST;
	if (0 == ioctl (handle->fd, VIDIOC_G_CTRL, &control)) {
		TRlog(TR_LOG_VIDEO, "Contrast value = %d\n", control.value);
	} else {
		TRerr("VIDIOC_G_CTRL(CONTRAST) error!\n");
		return -1;
	}
	return control.value;
}

int VportQueryContrast(void *hdl)
{
	struct v4l2_queryctrl queryctrl;
	vInPort *handle = (vInPort *) hdl;

	memset (&queryctrl, 0, sizeof (queryctrl));
	queryctrl.id = V4L2_CID_CONTRAST;
	if (0 == ioctl (handle->fd, VIDIOC_QUERYCTRL, &queryctrl))
	{
		TRlog(TR_LOG_VIDEO, "Contrast Range [%d]~[%d] and step = %d\n", queryctrl.minimum,
						queryctrl.maximum, queryctrl.step);
		return 0;
	} else {
		TRerr("VIDIOC_QUERYCTRL(CONTRAST) error\n");
		return -1;
	}
}

int VportSetContrast(void *hdl, int value)
{
	struct v4l2_control control;
	vInPort *handle = (vInPort *) hdl;

	memset (&control, 0, sizeof (control));
	control.id = V4L2_CID_CONTRAST;
	control.value = value;
	if (-1 == ioctl (handle->fd, VIDIOC_S_CTRL, &control))
	{
		TRerr("VIDIOC_S_CTRL(CONTRAST) error!\n");
		return -1;
	}
	return 0;
}

int VportGetWhiteBalance(void *hdl)
{
	struct v4l2_control control;
	vInPort *handle = (vInPort *) hdl;

	memset (&control, 0, sizeof (control));
	control.id = V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE;
	if (0 == ioctl (handle->fd, VIDIOC_G_CTRL, &control)) {
		TRlog(TR_LOG_VIDEO, "WhiteBalance value = %d\n", control.value);
	} else {
		TRerr("VIDIOC_G_CTRL(WHITE_BALACNE) error!\n");
		return -1;
	}
	return control.value;
}

int VportSetWhiteBalance(void *hdl, enum v4l2_auto_n_preset_white_balance value)
{
	struct v4l2_control control;
	vInPort *handle = (vInPort *) hdl;

	memset (&control, 0, sizeof (control));
	control.id = V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE;
	control.value = value;

	if (-1 == ioctl (handle->fd, VIDIOC_S_CTRL, &control)) {
		TRerr("VIDIOC_S_CTRL(WHITE_BALANCE) error!\n");
		return -1;
	}
	return 0;
}

int VportQueryExposureCompensation(void *hdl)
{
	struct v4l2_queryctrl queryctrl;
	vInPort *handle = (vInPort *) hdl;

	memset (&queryctrl, 0, sizeof (queryctrl));
	queryctrl.id = V4L2_CID_AUTO_EXPOSURE_BIAS;
	if (0 == ioctl (handle->fd, VIDIOC_QUERYCTRL, &queryctrl))
	{
		TRlog(TR_LOG_VIDEO, "ExposureCompensation Range [%d]~[%d] and step = %d\n", queryctrl.minimum,
						queryctrl.maximum, queryctrl.step);
		return 0;
	} else {
		TRerr("VIDIOC_QUERYCTRL(ExposureCompensation) error\n");
		return -1;
	}
}

int VportGetExposureCompensation(void *hdl)
{
	struct v4l2_control control;
	vInPort *handle = (vInPort *) hdl;

	memset (&control, 0, sizeof (control));
	control.id = V4L2_CID_CONTRAST;
	if (0 == ioctl (handle->fd, VIDIOC_G_CTRL, &control)) {
		TRlog(TR_LOG_VIDEO, "ExposureCompensation value = %d\n", control.value);
	} else {
		TRerr("VIDIOC_G_CTRL(ExposureCompensation) error!\n");
		return -1;
	}
	return control.value;
}

int VportSetExposureCompensation(void *hdl, int value)
{
	struct v4l2_control control;
	vInPort *handle = (vInPort *) hdl;

	memset (&control, 0, sizeof (control));
	control.id = V4L2_CID_AUTO_EXPOSURE_BIAS;
	control.value = value;

	if (-1 == ioctl (handle->fd, VIDIOC_S_CTRL, &control)) {
		TRerr("VIDIOC_S_CTRL(EXPOSURE_BIAS) error!\n");
		return -1;
	}
	return 0;
}

int VportWMInit(WaterMarkInfo *WM_info, char WMPath[30])
{
		int i;
		int watermark_pic_num = 13;
		char filename[64];
		FILE *icon_hdle = NULL;
		unsigned char *tmp_argb = NULL;
		int width = 0;
		int height = 0;

		/* init watermark pic info */
		for (i = 0; i < watermark_pic_num; i++) {
				sprintf(filename, "%s%d.bmp", WMPath, i);
				icon_hdle = fopen(filename, "r");
				if (icon_hdle == NULL) {
						TRerr("get wartermark %s error\n", filename);
						return -1;
				}
				/* get watermark picture size */
				fseek(icon_hdle, 18, SEEK_SET);
				fread(&width, 1, 4, icon_hdle);
				fread(&height, 1, 4, icon_hdle);
				fseek(icon_hdle, 54, SEEK_SET);

				if (WM_info->width == 0) {
						WM_info->width = width;
						WM_info->height = height * (-1);
				}

				WM_info->single_pic[i].id = i;
				WM_info->single_pic[i].y = (unsigned char *)malloc(WM_info->width * WM_info->height * 5 / 2);
				WM_info->single_pic[i].alph = WM_info->single_pic[i].y + WM_info->width * WM_info->height;
				WM_info->single_pic[i].c = WM_info->single_pic[i].alph + WM_info->width * WM_info->height;

				if (tmp_argb == NULL)
						tmp_argb = (unsigned char *)malloc(WM_info->width * WM_info->height * 4);

				fread(tmp_argb, WM_info->width * WM_info->height * 4, 1, icon_hdle);
				argb2yuv420sp(tmp_argb, WM_info->single_pic[i].alph, WM_info->width, WM_info->height,
								WM_info->single_pic[i].y, WM_info->single_pic[i].c);

						fclose(icon_hdle);
						icon_hdle = NULL;
		}
		WM_info->picture_number = i;

		if (tmp_argb != NULL)
			free(tmp_argb);

		return 0;
}

int VportWMRelease(void *hdl)
{
	int watermark_pic_num = 13;
	int i;

	vInPort *handle = (vInPort *) hdl;

	for (i = 0; i < watermark_pic_num; i++) {
		if (handle->WM_info.single_pic[i].y) {
			free(handle->WM_info.single_pic[i].y);
			handle->WM_info.single_pic[i].y = NULL;
		}
	}

	return 0;
}

int VportAddWM (void *hdl, unsigned int bg_width, unsigned int bg_height, void *bg_y_vir,
		void *bg_c_vir, unsigned int wm_pos_x, unsigned int wm_pos_y, struct tm *time_data)
{
    int ret = 0;
	time_t rawtime;
	struct tm *time_info;
	BackGroudLayerInfo BG_info;
	ShowWaterMarkParam WM_Param;

	vInPort *handle = (vInPort *) hdl;

	memset(&BG_info, 0, sizeof(BackGroudLayerInfo));
	/* init backgroud info */
	BG_info.width		= bg_width;
	BG_info.height		= bg_height;
	BG_info.y		= (unsigned char *)bg_y_vir;
	BG_info.c		= (unsigned char *)bg_c_vir;

	/* init watermark show para */
	WM_Param.pos.x = wm_pos_x;
	WM_Param.pos.y = wm_pos_y;

	if (time_data == NULL)
	{
		time(&rawtime);
		time_info = localtime(&rawtime);
	}
	else
	{
		time_info = time_data;
		time_info->tm_year = time_info->tm_year - 1900;
	}

	WM_Param.id_list[0] = (time_info->tm_year + 1900) / 1000;
	WM_Param.id_list[1] = ((time_info->tm_year + 1900) / 100) % 10;
	WM_Param.id_list[2] = ((time_info->tm_year + 1900) / 10) % 10;
	WM_Param.id_list[3] = (time_info->tm_year + 1900) % 10;
	WM_Param.id_list[4] = 11;
	WM_Param.id_list[5] = (1 + time_info->tm_mon) / 10 ;
	WM_Param.id_list[6] = (1 + time_info->tm_mon) % 10;
	WM_Param.id_list[7] = 11;
	WM_Param.id_list[8] = time_info->tm_mday / 10;
	WM_Param.id_list[9] = time_info->tm_mday % 10;
	WM_Param.id_list[10] = 10;
	WM_Param.id_list[11] = time_info->tm_hour / 10;
	WM_Param.id_list[12] = time_info->tm_hour % 10;
	WM_Param.id_list[13] = 12;
	WM_Param.id_list[14] = time_info->tm_min / 10;
	WM_Param.id_list[15] = time_info->tm_min % 10;
	WM_Param.id_list[16] = 12;
	WM_Param.id_list[17] = time_info->tm_sec / 10;
	WM_Param.id_list[18] = time_info->tm_sec % 10;
	WM_Param.number = 19;

	ret = watermark_blending(&BG_info, &handle->WM_info, &WM_Param);
    if(ret < 0){
        TRerr("[%s] watermark blending error\n", __func__);
        return ret;
    }

	SunxiMemFlushCache(handle->pMemops, BG_info.y + wm_pos_y * bg_width,
				handle->WM_info.height * bg_width);
	return 0;
}

vInPort *CreateTinaVinport()
{
	vInPort *tinaVin = (vInPort *)malloc(sizeof(vInPort));
	if (tinaVin == NULL)
		return NULL;

	memset(tinaVin, 0, sizeof(vInPort));
	tinaVin->open = VportOpen;
	tinaVin->close = VportClose;
	tinaVin->init = VportInit;
	tinaVin->requestbuf = VportRequestbuf;
	tinaVin->releasebuf = VportReleasebuf;
	tinaVin->getEnable = VportgetEnable;
	tinaVin->setEnable = VportsetEnable;
	tinaVin->getFrameRate = VportgetFrameRate;
	tinaVin->setFrameRate = VportsetFrameRate;
	tinaVin->getFormat = VportgetFormat;
	tinaVin->setFormat = VportsetFormat;
	tinaVin->getSize = VportgetSize;
	tinaVin->setSize = VportsetSize;
	tinaVin->startcapture = VportStartcapture;
	tinaVin->stopcapture = VportStopcapture;
	tinaVin->dequeue = VportDequeue;
	tinaVin->enqueue = VportEnqueue;
	tinaVin->getCameraIndex = VportGetIndex;
	tinaVin->queryContrast = VportQueryContrast;
	tinaVin->getContrast = VportGetContrast;
	tinaVin->setContrast = VportSetContrast;
	tinaVin->setWhiteBalance = VportSetWhiteBalance;
	tinaVin->getWhiteBalance = VportGetWhiteBalance;
	tinaVin->setExposureCompensation = VportSetExposureCompensation;
	tinaVin->getExposureCompensation = VportGetExposureCompensation;
	tinaVin->queryExposureCompensation = VportQueryExposureCompensation;
	tinaVin->WMInit = VportWMInit;
	tinaVin->WMRelease = VportWMRelease;
	tinaVin->addWM = VportAddWM;
	tinaVin->state = CAMERA_STATE_NONE;
	return tinaVin;
}

int DestroyTinaVinport(vInPort *hdl)
{
	vInPort *handle = (vInPort *)hdl;

	if (handle->state != CAMERA_STATE_NONE) {
		TRerr("[%s] Camera state error,status:%d", __func__,handle->state);
		return -1;
	}

	free(handle);

	return 0;
}
