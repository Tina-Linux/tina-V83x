#include "display.h"
#include "di.h"
#include "ion_mem_alloc.h"

int cameraInit(camera_hdl *hdl)
{
	int ret;
	int n_buffers = 0;
	unsigned int srcVirAddr, srcPhyAddr;
    struct v4l2_input inp;
    struct v4l2_streamparm parms;
    struct v4l2_format fmt;
    struct v4l2_requestbuffers req;
    struct v4l2_buffer buf;

    if(hdl == NULL)
    {
        printf(" cameraInit hdl == NULL\n");
        return -1;
    }

    hdl->videofd = open("/dev/video4", O_RDWR, 0);
    if(hdl->videofd <= 0)
    {
        printf(" open /dev/video4 fail!\n");
        return -1;
    }

	hdl->difd = open("/dev/deinterlace", O_RDWR, 0);
    if(hdl->difd <= 0)
    {
        printf(" open dev fail!\n");
        return -1;
    }

    /* set the data format */
    memset(&fmt, 0, sizeof(struct v4l2_format));
    fmt.type= V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width= hdl->video_width;
    fmt.fmt.pix.height= hdl->video_height;
    fmt.fmt.pix.pixelformat= hdl->pixelformat;
    fmt.fmt.pix.field= V4L2_FIELD_NONE;

	// tvd检测到信号，才往下跑，这里VIDIOC_G_FMT两次，是因为存在tvd检测到信号，但是还没有区分制式的情况
    while (ioctl(hdl->videofd, VIDIOC_G_FMT, &fmt)) {
        usleep(1000);
    }
	usleep(5000);
	while (ioctl(hdl->videofd, VIDIOC_G_FMT, &fmt)) {
        usleep(1000);
    }

    if (ioctl(hdl->videofd, VIDIOC_S_FMT, &fmt) < 0)
    {
        printf(" setting the data format failed!\n");

        return -1;
    }
	else
	{
       printf(" VIDIOC_S_FMT succeed\n");
       printf(" fmt.type = %d\n",fmt.type);
       printf(" fmt.fmt.pix.width = %d\n",fmt.fmt.pix.width);
       printf(" fmt.fmt.pix.height = %d\n",fmt.fmt.pix.height);
       printf(" fmt.fmt.pix.pixelformat = 0x%x\n",fmt.fmt.pix.pixelformat);
       printf(" fmt.fmt.pix.field = %d\n",fmt.fmt.pix.field);
	}

	// 实际的分辨率，由摄像头决定
	hdl->video_width = fmt.fmt.pix.width;
    hdl->video_height = fmt.fmt.pix.height;
    hdl->pixelformat = fmt.fmt.pix.pixelformat;
	printf("video_width=%d, video_height=%d, pixelformat=0x%x\n", (int)hdl->video_width, \
		(int)hdl->video_height, (int)hdl->pixelformat);

	/* set streaming parameters */
    parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parms.parm.capture.timeperframe.numerator = 1;
    parms.parm.capture.timeperframe.denominator = hdl->fps;

    if(ioctl(hdl->videofd,VIDIOC_G_PARM,&parms) < 0)
    {
        printf(" Setting streaming parameters\n");
        return -1;
    }
	printf("numerator=%d, denominator=%d\n", parms.parm.capture.timeperframe.numerator, parms.parm.capture.timeperframe.denominator);
	hdl->fps = parms.parm.capture.timeperframe.denominator;

    /* Initiate Memory Mapping or User Pointer I/O */
    memset(&req, 0, sizeof(struct v4l2_requestbuffers));
    req.count  = 5;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (ioctl(hdl->videofd, VIDIOC_REQBUFS, &req) < 0)
    {
        printf(" VIDIOC_REQBUFS failed\n");

        return -1;
    }

    /* Query the status of a buffers */
    hdl->num_buffer = req.count;
    hdl->buffers = calloc(req.count, sizeof(struct buffer));
    for (n_buffers= 0; n_buffers < req.count; ++n_buffers) {
        memset(&buf, 0, sizeof(struct v4l2_buffer));
        buf.type= V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory= V4L2_MEMORY_MMAP;
        buf.index= n_buffers;

        if (ioctl(hdl->videofd, VIDIOC_QUERYBUF, &buf) == -1)
        {
            printf(" VIDIOC_QUERYBUF error\n");

            return -1;
        }

        hdl->buffers[n_buffers].length= buf.length;
        hdl->buffers[n_buffers].start = mmap(NULL , buf.length,
                                            PROT_READ | PROT_WRITE, \
                                            MAP_SHARED , hdl->videofd, \
                                            buf.m.offset);
       printf("map buffer index: %d, mem: 0x%x, len: %x, offset: %x\n", \
                n_buffers,(int)hdl->buffers[n_buffers].start,buf.length,buf.m.offset);
    }

    /* Exchange a buffer with the driver */
    for(n_buffers = 0; n_buffers < req.count; n_buffers++) {
        memset(&buf, 0, sizeof(struct v4l2_buffer));
        buf.type= V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory= V4L2_MEMORY_MMAP;
        buf.index= n_buffers;

        if (ioctl(hdl->videofd, VIDIOC_QBUF, &buf) == -1) {
            printf(" VIDIOC_QBUF error\n");

            return -1;
        }
    }

	#if 0
    /* select the current video input */
    inp.index = 0;
    inp.type = V4L2_INPUT_TYPE_CAMERA;
    if(ioctl(hdl->videofd,VIDIOC_S_INPUT,&inp) < 0)
    {
        printf(" VIDIOC_S_INPUT failed! s_input: %d\n",inp.index);

        return -1;
    }
	#endif

	memset(&hdl->data_info, 0, sizeof(struct datainfo));
	hdl->ionfd = GetMemAdapterOpsS();
	ret = hdl->ionfd->open();
	if(ret != 0)
	{
		printf("open err ret=%d\n", ret);
		return -1;
	}

	srcVirAddr = hdl->ionfd->palloc(hdl->video_width*hdl->video_height*3/2);
	srcPhyAddr = hdl->ionfd->cpu_get_phyaddr(srcVirAddr);
	printf("srcVirAddr=0x%x, srcPhyAddr=0x%x\n", srcVirAddr, srcPhyAddr);
	memset(srcVirAddr, 0x80, hdl->video_width*hdl->video_height*3/2);
	hdl->data_info.start = (void *)srcVirAddr;
	hdl->data_info.phaddr = (unsigned long)srcPhyAddr;

    /* disable camera output */
    hdl->output_enable = 0;

    /* init sem */
    sem_init(&hdl->dqfinSem, 0, 0);
    sem_init(&hdl->qbufSem, 0, 0);

    return 0;
}

int cameraStartCapture(camera_hdl *hdl)
{
    enum v4l2_buf_type type;

    if(hdl == NULL)
    {
        printf(" cameraStartCapture hdl = NULL\n");
        return -1;
    }

    /* stream on */
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(hdl->videofd,VIDIOC_STREAMON,&type) < 0)
    {
        printf(" VIDIOC_STREAMON error\n");
        return -1;
    }

    /* enable camera */
    hdl->output_enable = 1;

    return 0;
}

int cameraStopCapture(camera_hdl *hdl)
{
    enum v4l2_buf_type type;

    if(hdl == NULL)
    {
        printf(" cameraStopCapture hal = NULL\n");
        return -1;
    }

    /* stream off */
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(hdl->videofd, VIDIOC_STREAMOFF, &type) < 0)
    {
        printf(" VIDIOC_STREAMOFF error\n");
        return -1;
    }

    /* disable camera */
    hdl->output_enable = 0;

    return 0;
}

int cameraRelease(camera_hdl *hdl)
{
    if(hdl ==NULL)
        return -1;

    for(int i=0; i<hdl->num_buffer; i++)
    {
        munmap(hdl->buffers[i].start,hdl->buffers[i].length);
    }

	hdl->ionfd->pfree(hdl->data_info.start);
	hdl->ionfd->close();

    sem_destroy(&hdl->dqfinSem);
    sem_destroy(&hdl->qbufSem);

    free(hdl->buffers);
    close(hdl->videofd);

    return 0;
}

void *CameraThread(void *param)
{
    cameraplay *hdl = (cameraplay *)param;
    struct timeval tv;
    fd_set fds;
    struct v4l2_buffer buf;
    int ret = 0;
	char save_path[32];
	static unsigned int save_num = 0;
	struct __di_para_t di_para;

	if(hdl ==NULL)
        return NULL;

    FD_ZERO(&fds);
    FD_SET(hdl->capture.videofd,&fds);

    memset(&buf, 0, sizeof(struct v4l2_buffer));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

	hdl->capture.frist_frame = 1;
	memset(&di_para, 0, sizeof(struct __di_para_t));
    while(!hdl->quitFlag)
    {
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        ret = select(hdl->capture.videofd+1, &fds, NULL, NULL, &tv);
        if(ret == -1)
        {
            printf(" select error\n");
        }
        else if(ret == 0)
        {
            printf(" select timeout\n");
        }

        /* DQBUF */
        ret = ioctl(hdl->capture.videofd, VIDIOC_DQBUF, &buf);
        if(ret != 0)
        {
            printf(" DQBUF fail\n");
        }

		if(hdl->capture.frist_frame == 1)
		{
			hdl->capture.frist_frame = 0;
			hdl->capture.tvd_buf[0].index = buf.index;
	        hdl->capture.tvd_buf[0].m.offset = buf.m.offset;
	        hdl->capture.tvd_buf[0].bytesused = buf.bytesused;
	        hdl->capture.tvd_buf[0].timestamp.tv_sec = buf.timestamp.tv_sec;
	        hdl->capture.tvd_buf[0].timestamp.tv_usec = buf.timestamp.tv_usec;
			memcpy(&hdl->capture.tvd_buf[0], &buf, sizeof(struct v4l2_buffer));
			continue;
		}

		hdl->capture.tvd_buf[1].index = buf.index;
	    hdl->capture.tvd_buf[1].m.offset = buf.m.offset;
	    hdl->capture.tvd_buf[1].bytesused = buf.bytesused;
	    hdl->capture.tvd_buf[1].timestamp.tv_sec = buf.timestamp.tv_sec;
	    hdl->capture.tvd_buf[1].timestamp.tv_usec = buf.timestamp.tv_usec;
		memcpy(&hdl->capture.tvd_buf[1], &buf, sizeof(struct v4l2_buffer));

		// in
		di_para.pre_fb.addr[0] = (char *)hdl->capture.tvd_buf[0].m.offset;
		di_para.pre_fb.addr[1] = (char *)hdl->capture.tvd_buf[0].m.offset \
			+ (hdl->capture.video_width * hdl->capture.video_height);
		di_para.pre_fb.size.width = hdl->capture.video_width;
		di_para.pre_fb.size.height = hdl->capture.video_height;
		di_para.pre_fb.format = DI_FORMAT_NV12;

		di_para.input_fb.addr[0] = (char *)hdl->capture.tvd_buf[1].m.offset;
		di_para.input_fb.addr[1] = (char *)hdl->capture.tvd_buf[1].m.offset \
			+ (hdl->capture.video_width * hdl->capture.video_height);
		di_para.input_fb.size.width = hdl->capture.video_width;
		di_para.input_fb.size.height = hdl->capture.video_height;
		di_para.input_fb.format = DI_FORMAT_NV12;

		di_para.source_regn.width = hdl->capture.video_width;
		di_para.source_regn.height = hdl->capture.video_height;

		// out
		di_para.output_fb.addr[0] = (char *)hdl->capture.data_info.phaddr;
		di_para.output_fb.addr[1] = (char *)hdl->capture.data_info.phaddr + (hdl->capture.video_width * hdl->capture.video_height);
		di_para.output_fb.size.width = hdl->capture.video_width;
		di_para.output_fb.size.height = hdl->capture.video_height;
		di_para.output_fb.format = DI_FORMAT_NV12;

		di_para.out_regn.width = hdl->capture.video_width;
		di_para.out_regn.height = hdl->capture.video_height;

		// other
		di_para.field = 0;
		di_para.top_field_first = 0;

		// di deal
		//ioctl(hdl->capture.difd, DI_IOCSTART, &di_para);
		// not di deal
		memcpy(hdl->capture.data_info.start, hdl->capture.buffers[hdl->capture.tvd_buf[1].index].start, buf.bytesused);

        hdl->capture.data_info.index = buf.index;
        hdl->capture.data_info.length = buf.bytesused;
        hdl->capture.data_info.timestamp.tv_sec = buf.timestamp.tv_sec;
        hdl->capture.data_info.timestamp.tv_usec = buf.timestamp.tv_usec;

        sem_post(&hdl->capture.dqfinSem);

		if(hdl->saveframe_Flag == 1)
		{
			sprintf(save_path, "/tmp/bmp_data%d.bmp",save_num++);
			YUVToBMP(save_path, hdl->capture.data_info.start, NV12ToRGB24, hdl->capture.video_width, hdl->capture.video_height);
			/* reset flag */
			hdl->saveframe_Flag = 0;
		}

        sem_wait(&hdl->capture.qbufSem);

        /* QBUF */
        if(ioctl(hdl->capture.videofd, VIDIOC_QBUF, &hdl->capture.tvd_buf[0]))
        {
            printf(" QBUF fail\n");
        }

		memcpy(&hdl->capture.tvd_buf[0], &hdl->capture.tvd_buf[1], sizeof(struct v4l2_buffer));
    }

	/* Prevent program deadlock, release multiple times */
	sem_post(&hdl->capture.dqfinSem);

    /* stop Capture*/
    cameraStopCapture(&hdl->capture);

    /* free buffers and close fd */
    cameraRelease(&hdl->capture);

   printf(" capture thread end\n");
}
