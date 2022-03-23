#include "camerademo.h"

int camera_dbg_en = 0;
static int support_tinatest = 0;

static struct vfe_format fmt_fourcc[] = {
    {
        .name = "YUV422P",
        .fourcc = V4L2_PIX_FMT_YUV422P,
        .ToRGB24Func = YUV422PToRGB24,
    },
    {
        .name = "YUV420",
        .fourcc = V4L2_PIX_FMT_YUV420,
        .ToRGB24Func = YUV420ToRGB24,
    },
    {
        .name = "YVU420",
        .fourcc = V4L2_PIX_FMT_YVU420,
        .ToRGB24Func = YVU420ToRGB24,
    },
    {
        .name = "NV16",
        .fourcc = V4L2_PIX_FMT_NV16,
        .ToRGB24Func = NV16ToRGB24,
    },
    {
        .name = "NV12",
        .fourcc = V4L2_PIX_FMT_NV12,
        .ToRGB24Func = NV12ToRGB24,
    },
    {
        .name = "NV61",
        .fourcc = V4L2_PIX_FMT_NV61,
        .ToRGB24Func = NV61ToRGB24,
    },
    {
        .name = "NV21",
        .fourcc = V4L2_PIX_FMT_NV21,
        .ToRGB24Func = NV21ToRGB24,
    },
    {
        .name = "HM12",
        .fourcc = V4L2_PIX_FMT_HM12,
        .ToRGB24Func = NULL,
    },
    {
        .name = "YUYV",
        .fourcc = V4L2_PIX_FMT_YUYV,
        .ToRGB24Func = YUYVToRGB24,
    },
    {
        .name = "YVYU",
        .fourcc = V4L2_PIX_FMT_YVYU,
        .ToRGB24Func = YVYUToRGB24,
    },
    {
        .name = "UYVY",
        .fourcc = V4L2_PIX_FMT_UYVY,
        .ToRGB24Func = UYVYToRGB24,
    },
    {
        .name = "VYUY",
        .fourcc = V4L2_PIX_FMT_VYUY,
        .ToRGB24Func = VYUYToRGB24,
    },
    {
        .name = "BA81",
        .fourcc = V4L2_PIX_FMT_SBGGR8,
        .ToRGB24Func = NULL,
    },
    {
        .name = "GBRG",
        .fourcc = V4L2_PIX_FMT_SGBRG8,
        .ToRGB24Func = NULL,
    },
    {
        .name = "GRBG",
        .fourcc = V4L2_PIX_FMT_SGRBG8,
        .ToRGB24Func = NULL,
    },
    {
        .name = "RGGB",
        .fourcc = V4L2_PIX_FMT_SRGGB8,
        .ToRGB24Func = NULL,
    },
    {
        .name = "BG10",
        .fourcc = V4L2_PIX_FMT_SBGGR10,
        .ToRGB24Func = NULL,
    },
    {
        .name = "GB10",
        .fourcc = V4L2_PIX_FMT_SGBRG10,
        .ToRGB24Func = NULL,
    },
    {
        .name = "BA10",
        .fourcc = V4L2_PIX_FMT_SGRBG10,
        .ToRGB24Func = NULL,
    },
    {
        .name = "RG10",
        .fourcc = V4L2_PIX_FMT_SRGGB10,
        .ToRGB24Func = NULL,
    },
    {
        .name = "BG12",
        .fourcc = V4L2_PIX_FMT_SBGGR12,
        .ToRGB24Func = NULL,
    },
    {
        .name = "GB12",
        .fourcc = V4L2_PIX_FMT_SGBRG12,
        .ToRGB24Func = NULL,
    },
    {
        .name = "BA12",
        .fourcc = V4L2_PIX_FMT_SGRBG12,
        .ToRGB24Func = NULL,
    },
    {
        .name = "RG12",
        .fourcc = V4L2_PIX_FMT_SRGGB12,
        .ToRGB24Func = NULL,
    },
    {
        .name = "MJPEG",
        .fourcc = V4L2_PIX_FMT_MJPEG,
        .ToRGB24Func = NULL,
    },
    {
        .name = "H264",
        .fourcc = V4L2_PIX_FMT_H264,
        .ToRGB24Func = NULL,
    },
};

#define FOURCC_NUM  (sizeof(fmt_fourcc)/sizeof(fmt_fourcc[0]))

#define NUM_SUPPORT_FMT 30
static int formatNum = 0;
static struct vfe_format support_fmt[NUM_SUPPORT_FMT];
static struct vfe_format sensor_formats[NUM_SUPPORT_FMT];

char *get_format_name(unsigned int format)
{
    if(format == V4L2_PIX_FMT_YUV422P)
        return "YUV422P";
    else if(format == V4L2_PIX_FMT_YUV420)
        return "YUV420";
    else if(format == V4L2_PIX_FMT_YVU420)
        return "YVU420";
    else if(format == V4L2_PIX_FMT_NV16)
        return "NV16";
    else if(format == V4L2_PIX_FMT_NV12)
        return "NV12";
    else if(format == V4L2_PIX_FMT_NV61)
        return "NV61";
    else if(format == V4L2_PIX_FMT_NV21)
        return "NV21";
    else if(format == V4L2_PIX_FMT_HM12)
        return "MB YUV420";
    else if(format == V4L2_PIX_FMT_YUYV)
        return "YUYV";
    else if(format == V4L2_PIX_FMT_YVYU)
        return "YVYU";
    else if(format == V4L2_PIX_FMT_UYVY)
        return "UYVY";
    else if(format == V4L2_PIX_FMT_VYUY)
        return "VYUY";
    else if(format == V4L2_PIX_FMT_MJPEG)
        return "MJPEG";
    else if(format == V4L2_PIX_FMT_H264)
        return "H264";
    else
        return NULL;
}

int return_format(char *short_name)
{
    if (strcmp(short_name, "YUV422P") == 0)
        return V4L2_PIX_FMT_YUV422P;
    else if (strcmp(short_name, "YUV420") == 0)
        return V4L2_PIX_FMT_YUV420;
    else if (strcmp(short_name, "YVU420") == 0)
        return V4L2_PIX_FMT_YVU420;
    else if (strcmp(short_name, "NV16") == 0)
        return V4L2_PIX_FMT_NV16;
    else if (strcmp(short_name, "NV12") == 0)
        return V4L2_PIX_FMT_NV12;
    else if (strcmp(short_name, "NV61") == 0)
        return V4L2_PIX_FMT_NV61;
    else if (strcmp(short_name, "NV21") == 0)
        return V4L2_PIX_FMT_NV21;
    else if (strcmp(short_name, "YUYV") == 0)
        return V4L2_PIX_FMT_YUYV;
    else if (strcmp(short_name, "YVYU") == 0)
        return V4L2_PIX_FMT_YVYU;
    else if (strcmp(short_name, "UYVY") == 0)
        return V4L2_PIX_FMT_UYVY;
    else if (strcmp(short_name, "VYUY") == 0)
        return V4L2_PIX_FMT_VYUY;
    else if (strcmp(short_name, "MJPEG") == 0)
        return V4L2_PIX_FMT_MJPEG;
    else if (strcmp(short_name, "H264") == 0)
        return V4L2_PIX_FMT_H264;
    else
        return -1;
}

static int input_stdin(char *input_type, void *param)
{
    while(scanf(input_type, (char *)param) == 0){
        int c;
        camera_err(" error input, please re-enter!\n");

        while((c = getchar()) != '\n' && c != EOF);
    }

    return 0;
}

static void show_help(void)
{
    camera_print("******************** camerademo help *********************\n");
    camera_print(" This program is a test camera.\n");
    camera_print(" It will query the sensor to support the resolution, output format and test frame rate.\n");
    camera_print(" At the same time you can modify the data to save the path and get the number of photos.\n");
    camera_print(" When the last parameter is debug, the output will be more detailed information\n");
    camera_print(" There are eight ways to run:\n");
    camera_print("    1.camerademo --- use the default parameters.\n");
    camera_print("    2.camerademo debug --- use the default parameters and output debug information.\n");
    camera_print("    3.camerademo setting --- can choose the resolution and data format.\n");
    camera_print("    4.camerademo setting debug --- setting and output debug information.\n");
    camera_print("    5.camerademo NV21 640 480 0 bmp /tmp 5 --- param input mode,can save bmp or yuv.\n");
    camera_print("    6.camerademo NV21 640 480 0 bmp /tmp 5 debug --- output debug information.\n");
    camera_print("    7.camerademo NV21 640 480 0 bmp /tmp 5 Num --- /dev/videoNum param input mode,can save bmp or yuv.\n");
    camera_print("    8.camerademo NV21 640 480 0 bmp /tmp 5 Num debug --- /dev/videoNum output debug information.\n");
    camera_print("**********************************************************\n");
}

#ifdef __USE_VIN_ISP__
static int getSensorType(int fd)
{
    int ret = -1;
    struct v4l2_control ctrl;
    struct v4l2_queryctrl qc_ctrl;

    if (fd == 0)
        return 0xFF000000;

    memset(&ctrl, 0, sizeof(struct v4l2_control));
    memset(&qc_ctrl, 0, sizeof(struct v4l2_queryctrl));
    ctrl.id = V4L2_CID_SENSOR_TYPE;
    qc_ctrl.id = V4L2_CID_SENSOR_TYPE;

    if (-1 == ioctl (fd, VIDIOC_QUERYCTRL, &qc_ctrl)){
        camera_err(" query sensor type ctrl failed\n");
        return -1;
    }

    ret = ioctl(fd, VIDIOC_G_CTRL, &ctrl);
    if(ret < 0){
        camera_err(" get sensor type failed,errno(%d)\n", errno);
        return ret;
    }

    return ctrl.value;
}
#endif

/*
 * Test the time from opening to getting photos
 */
static long long test_time(camera_handle *camera)
{
    unsigned char camera_path[16];
    struct v4l2_format fmt;          /* try a format */
    struct v4l2_input inp;           /* select the current video input */
    struct v4l2_streamparm parms;    /* set streaming parameters */
    struct v4l2_requestbuffers req;  /* Initiate Memory Mapping or User Pointer I/O */
    struct v4l2_buffer buf;          /* Query the status of a buffer */
    enum v4l2_buf_type type;

    struct timeval tv;
    fd_set fds;
    int n_buffers = 0;
    long long openTime = 0, dqbufTime = 0;
    long long result = -1;
    int ret = -1;

    /* get open videoX time */
    gettimeofday(&tv, NULL);
    openTime = secs_to_msecs(tv.tv_sec, tv.tv_usec);

    /*open video */
    memset(camera_path, 0, sizeof(camera_path));
    sprintf(camera_path, "%s%d", "/dev/video", camera->camera_index);
    camera->videofd = open(camera_path, O_RDWR, 0);

    /* set input */
    inp.index = 0;
    inp.type = V4L2_INPUT_TYPE_CAMERA;
    if(ioctl(camera->videofd,VIDIOC_S_INPUT,&inp) < 0){
        camera_err(" VIDIOC_S_INPUT failed! s_input: %d\n",inp.index);
        close(camera->videofd);
        return result;
    }

    /* set streaming parameters */
    memset(&parms, 0, sizeof(struct v4l2_streamparm));
    if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    else
        parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parms.parm.capture.timeperframe.numerator = 1;
    parms.parm.capture.timeperframe.denominator = 30;
    if(ioctl(camera->videofd, VIDIOC_S_PARM, &parms) < 0){
        camera_err(" Setting streaming parameters failed, numerator:%d denominator:%d\n",
                                                    parms.parm.capture.timeperframe.numerator,
                                                    parms.parm.capture.timeperframe.denominator);
        close(camera->videofd);
        return result;
    }

    /* get streaming parameters */
    memset(&parms,0,sizeof(struct v4l2_streamparm));
    if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    else
        parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(camera->videofd, VIDIOC_G_PARM, &parms) < 0){
        camera_err(" Get streaming parameters failed!\n");
    }

    /* set the data format */
    memset(&fmt, 0, sizeof(struct v4l2_format));
    if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE){
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        fmt.fmt.pix_mp.width = camera->win_width;
        fmt.fmt.pix_mp.height = camera->win_height;
        fmt.fmt.pix_mp.pixelformat = camera->pixelformat;
        fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
    }else{
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = camera->win_width;
        fmt.fmt.pix.height = camera->win_height;
        fmt.fmt.pix.pixelformat = camera->pixelformat;
        fmt.fmt.pix.field = V4L2_FIELD_NONE;
    }

    if (ioctl(camera->videofd, VIDIOC_S_FMT, &fmt) < 0){
        camera_err(" setting the data format failed!\n");
        close(camera->videofd);
        return result;
    }

    if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE){
        camera->win_width = fmt.fmt.pix_mp.width;
        camera->win_height = fmt.fmt.pix_mp.height;

        if (ioctl(camera->videofd, VIDIOC_G_FMT, &fmt) < 0)
            camera_err(" get the data format failed!\n");

        camera->nplanes = fmt.fmt.pix_mp.num_planes;
    }else{
        camera->win_width = fmt.fmt.pix.width;
        camera->win_height = fmt.fmt.pix.height;
    }

    /* Initiate Memory Mapping or User Pointer I/O */
    memset(&req, 0, sizeof(struct v4l2_requestbuffers));
    req.count = 3;
    if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    else
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if(ioctl(camera->videofd, VIDIOC_REQBUFS, &req) < 0){
        camera_err(" VIDIOC_REQBUFS failed\n");
        close(camera->videofd);
        return result;
    }

    /* Query the status of a buffers */
    camera->buf_count = req.count;

    camera->buffers = calloc(req.count, sizeof(struct buffer));
    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        memset(&buf, 0, sizeof(struct v4l2_buffer));
        if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        else
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;
        if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE){
            buf.length = camera->nplanes;
            buf.m.planes =  (struct v4l2_plane *)calloc(buf.length, sizeof(struct v4l2_plane));
        }

        if (ioctl(camera->videofd, VIDIOC_QUERYBUF, &buf) == -1) {
            camera_err(" VIDIOC_QUERYBUF error\n");

            if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
                free(buf.m.planes);
            free(camera->buffers);

            close(camera->videofd);

            return result;
        }

        if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE){
            for(int i = 0; i < camera->nplanes; i++){
                camera->buffers[n_buffers].length[i] = buf.m.planes[i].length;
                camera->buffers[n_buffers].start[i] = mmap(NULL , buf.m.planes[i].length,
                                                   PROT_READ | PROT_WRITE, \
                                                   MAP_SHARED , camera->videofd, \
                                                   buf.m.planes[i].m.mem_offset);
            }
            free(buf.m.planes);
        }else{
            camera->buffers[n_buffers].length[0] = buf.length;
            camera->buffers[n_buffers].start[0] = mmap(NULL , buf.length,
                                                PROT_READ | PROT_WRITE, \
                                                MAP_SHARED , camera->videofd, \
                                                buf.m.offset);
        }
    }

    /* Exchange a buffer with the driver */
    for(n_buffers = 0; n_buffers < req.count; n_buffers++) {
        memset(&buf, 0, sizeof(struct v4l2_buffer));
        if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        else
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory= V4L2_MEMORY_MMAP;
        buf.index= n_buffers;
        if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE){
            buf.length = camera->nplanes;
            buf.m.planes =  (struct v4l2_plane *)calloc(buf.length, sizeof(struct v4l2_plane));
        }

        if (ioctl(camera->videofd, VIDIOC_QBUF, &buf) == -1) {
            camera_err(" VIDIOC_QBUF error\n");

            if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
                free(buf.m.planes);
            free(camera->buffers);

            close(camera->videofd);
            return result;
        }
        if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
            free(buf.m.planes);
    }

    /* streamon */
    if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    else
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(camera->videofd, VIDIOC_STREAMON, &type) == -1) {
        camera_err(" VIDIOC_STREAMON error! %s\n",strerror(errno));
        goto TEST_EXIT;
    }

    FD_ZERO(&fds);
    FD_SET(camera->videofd, &fds);

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

    /* wait for sensor capture data */
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    ret = select(camera->videofd+1,&fds,NULL,NULL,&tv);
    if (ret == -1){
        camera_err(" select error\n");
        goto TEST_EXIT;
    }else if (ret == 0){
        camera_err(" select timeout,end capture thread!\n");
        goto TEST_EXIT;
    }

    gettimeofday(&tv, NULL);
    dqbufTime = secs_to_msecs(tv.tv_sec, tv.tv_usec);

    result = dqbufTime - openTime;

TEST_EXIT:
    /* munmap camera->buffers */
    if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE){
        for (int i = 0; i < camera->buf_count; ++i)
            for (int j = 0; j < camera->nplanes; j++)
                 munmap(camera->buffers[i].start[j], camera->buffers[i].length[j]);
    }else{
        for(int i=0; i<camera->buf_count; i++)
            munmap(camera->buffers[i].start[0],camera->buffers[i].length[0]);
    }

    if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        free(buf.m.planes);
    free(camera->buffers);

    close(camera->videofd);

    return result;
}

static int capture_photo(camera_handle *camera)
{
    enum v4l2_buf_type type;
    struct timeval tv;
    fd_set fds;
    struct v4l2_buffer buf;
    int ret = 0;
    int np = 0;
    long long streamon_time;
    long long timestamp_now, timestamp_save;
    char source_data_path[64];
    char bmp_data_path[64];
    int i;
    int width,height;
    FILE *fp = NULL;

    /* streamon */
    if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    else
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(camera->videofd, VIDIOC_STREAMON, &type) == -1) {
        camera_err(" VIDIOC_STREAMON error! %s\n",strerror(errno));
        ret = -1;
        goto EXIT;
    }else
        camera_print(" stream on succeed\n");

#ifdef __USE_VIN_ISP__
    /* setting ISP */
    if(camera->sensor_type == V4L2_SENSOR_TYPE_RAW){
        camera->ispId = -1;
        camera->ispId = camera->ispPort->ispGetIspId(camera->camera_index);
        if(camera->ispId >= 0)
            camera->ispPort->ispStart(camera->ispId);
        /* if support tinatest, reserve the ISP to work normally and save the data */
        if(support_tinatest == 0xff)
            camera->photo_num = 20;
    }
#endif

    /* get stream on time */
    gettimeofday(&tv, NULL);
    streamon_time = secs_to_msecs(tv.tv_sec, tv.tv_usec);

    FD_ZERO(&fds);
    FD_SET(camera->videofd, &fds);

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

    while (np < camera->photo_num)
    {
        camera_print(" capture num is [%d]\n",np);

        /* wait for sensor capture data */
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        ret = select(camera->videofd+1,&fds,NULL,NULL,&tv);
        if (ret == -1){
            camera_err(" select error\n");
            continue;
        }else if (ret == 0){
            camera_err(" select timeout,end capture thread!\n");
            ret = -1;
            break;
        }

        /* dqbuf */
        ret = ioctl(camera->videofd, VIDIOC_DQBUF, &buf);
        if (ret == 0)
            camera_dbg("*****DQBUF[%d] FINISH*****\n",buf.index);
        else
            camera_err("****DQBUF FAIL*****\n");

        gettimeofday(&tv, NULL);
        timestamp_now = secs_to_msecs(tv.tv_sec, tv.tv_usec);
        if (np == 0){
            camera_prompt(" the time interval from the start to the first frame is %lld ms\n", timestamp_now-streamon_time);
            timestamp_save = timestamp_now;
        }
        camera_dbg(" the interval of two frames is %lld ms\n", timestamp_now - timestamp_save);
        timestamp_save = timestamp_now;

        /* check the data buf for byte alignment */
        if(camera->pixelformat == V4L2_PIX_FMT_YUV422P || camera->pixelformat == V4L2_PIX_FMT_YUYV
            || camera->pixelformat == V4L2_PIX_FMT_YVYU || camera->pixelformat == V4L2_PIX_FMT_UYVY
            || camera->pixelformat == V4L2_PIX_FMT_VYUY){
            if(camera->win_width*camera->win_height*2 < buf.bytesused){
                width = ALIGN_16B(camera->win_width);
                height = ALIGN_16B(camera->win_height);
            }else{
                width = camera->win_width;
                height = camera->win_height;
            }
        }else if(camera->pixelformat == V4L2_PIX_FMT_YUV420 || camera->pixelformat == V4L2_PIX_FMT_YVU420
            || camera->pixelformat == V4L2_PIX_FMT_NV16 || camera->pixelformat == V4L2_PIX_FMT_NV61
            || camera->pixelformat == V4L2_PIX_FMT_NV12 || camera->pixelformat == V4L2_PIX_FMT_NV21){
            if(camera->win_width*camera->win_height*1.5 < buf.bytesused){
                width = ALIGN_16B(camera->win_width);
                height = ALIGN_16B(camera->win_height);
            }else{
                width = camera->win_width;
                height = camera->win_height;
            }
        }else{
            width = camera->win_width;
            height = camera->win_height;
        }

        /* if support tinatest, just save the last one */
        if(support_tinatest == 0xff && np != camera->photo_num-1)
            goto TINATEST_SKIP;

        /* YUV data */
        if(camera->pixelformat != V4L2_PIX_FMT_MJPEG && camera->pixelformat != V4L2_PIX_FMT_H264){
            /* add water mark */
            if(camera->use_wm)
                AddWM(&camera->WM_info, width, height, camera->buffers[buf.index].start[0],
                        camera->buffers[buf.index].start[0]+width*height, camera->win_width-470, camera->win_height-40, NULL);

            if((camera->photo_type == ALL_TYPE) || (camera->photo_type == YUV_TYPE)){
                sprintf(source_data_path, "%s/%s%s_%d%s", camera->save_path, "source_", get_format_name(camera->pixelformat), np+1, ".yuv");
                save_frame_to_file(source_data_path, camera->buffers[buf.index].start[0], camera->buffers[buf.index].length[0]);
            }

            if( ((camera->photo_type == ALL_TYPE) || (camera->photo_type == BMP_TYPE)) && (camera->ToRGB24 != NULL) ){
                sprintf(bmp_data_path,"%s/%s%s_%d%s",camera->save_path, "bmp_", get_format_name(camera->pixelformat), np+1,".bmp");
                YUVToBMP(bmp_data_path,camera->buffers[buf.index].start[0], camera->ToRGB24, width, height);
            }
        }else{ /* MJPEG or H264 */
            sprintf(source_data_path, "%s/%s%d.%s", camera->save_path, "source_data", np+1, get_format_name(camera->pixelformat));
            save_frame_to_file(source_data_path, camera->buffers[buf.index].start[0], buf.bytesused);
        }

TINATEST_SKIP:
        /* qbuf */
        if (ioctl(camera->videofd, VIDIOC_QBUF, &buf) == 0)
            camera_dbg("************QBUF[%d] FINISH**************\n",buf.index);
        else
            camera_err("*****QBUF FAIL*****\n");

        np++;
    }

    camera_print(" Capture thread finish\n");

    camera_dbg("***************************************************************\n");
    camera_dbg(" Query the actual frame rate.\n");
    camera_dbg(" camera fps = %.1f.\n",measure_fps(camera->videofd));
    camera_dbg("***************************************************************\n");

    /* streamoff */
    if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    else
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(camera->videofd, VIDIOC_STREAMOFF, &type) == -1)
        camera_err(" VIDIOC_STREAMOFF error! %s\n",strerror(errno));

#ifdef __USE_VIN_ISP__
    /* stop ISP */
    if(camera->sensor_type == V4L2_SENSOR_TYPE_RAW
                                    && camera->ispId >= 0){
        camera->ispPort->ispStop(camera->ispId);
        DestroyAWIspApi(camera->ispPort);
        camera->ispPort = NULL;
    }
#endif

EXIT:
    /* munmap camera->buffers */
    if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE){
        for (int i = 0; i < camera->buf_count; ++i)
            for (int j = 0; j < camera->nplanes; j++)
                 munmap(camera->buffers[i].start[j], camera->buffers[i].length[j]);
    }else{
        for(int i=0; i<camera->buf_count; i++)
            munmap(camera->buffers[i].start[0],camera->buffers[i].length[0]);
    }

    /* free camera->buffers and close camera->videofd */
    camera_print(" close /dev/video%d\n", camera->camera_index);

    if(camera->driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        free(buf.m.planes);
    free(camera->buffers);
    if(camera->use_wm)
        WMRelease(&camera->WM_info);
    close(camera->videofd);

    return ret;
}


int main(int argc,char *argv[])
{
    camera_handle camera;
    unsigned char camera_path[16];
    struct v4l2_capability cap;      /* Query device capabilities */
    struct v4l2_fmtdesc fmtdesc;     /* Enumerate image formats */
    struct v4l2_frmsizeenum frmsize; /* Enumerate frame sizes */
	struct v4l2_frmivalenum ival;	/* Enumerate fps */
    struct v4l2_format fmt;          /* try a format */
    struct v4l2_input inp;           /* select the current video input */
    struct v4l2_streamparm parms;    /* set streaming parameters */
    struct v4l2_requestbuffers req;  /* Initiate Memory Mapping or User Pointer I/O */
    struct v4l2_buffer buf;          /* Query the status of a buffer */

    int n_buffers = 0;
    int index = 0;

    /* default settings */
    memset(&camera, 0, sizeof(camera_handle));
    memset(camera_path, 0, sizeof(camera_path));
    camera.camera_index = 0;
    camera.isDefault = true;
    camera.sensor_type = -1;
    camera.ispId = -1;
    camera.photo_type = BMP_TYPE;
    camera.win_width = 640;
    camera.win_height = 480;
    camera.photo_num = 5;
    camera.pixelformat = V4L2_PIX_FMT_NV21;
    memcpy(camera.save_path,"/tmp",sizeof("/tmp"));
    camera.ToRGB24 = NV21ToRGB24;
    camera.use_wm = 0;

    camera_print("**********************************************************\n");
    camera_print("*                                                        *\n");
    camera_print("*              this is camera test.                      *\n");
    camera_print("*                                                        *\n");
    camera_print("**********************************************************\n");

    /* parsed input parameters */
    if((argc == 2) && (strcmp(argv[1],"setting") == 0)){
        camera.isDefault = false;

        camera_print(" Please input the setting parameters according to the prompts.\n");
    }else if ((argc == 2) && (strcmp(argv[1],"help") == 0)){
        show_help();
        return 0;
    }else if ((argc == 2) && (strcmp(argv[1],"debug") == 0)){
        camera_dbg_en = 1;
    }else if ((argc == 4) && (strcmp(argv[1],"tinatest") == 0)){
        support_tinatest = 0xff;
        camera.camera_index = atoi(argv[2]);
        camera.photo_type = BMP_TYPE;
        memset(camera.save_path, 0, sizeof(camera.save_path));
        memcpy(camera.save_path, argv[3], sizeof(camera.save_path));
    }else if ((argc == 3) && (strcmp(argv[1],"setting") == 0) && (strcmp(argv[2],"debug") == 0)){
        camera_dbg_en = 1;
        camera.isDefault = false;
        camera_print(" Please input the setting parameters according to the prompts.\n");
    }else if(argc == 8){
        if(return_format(argv[1]) < 0){
            camera_err(" unsupport input format %s\n", argv[1]);
            camera_print(" using format parameters NV21\n");
        }else{
            int i = 0;
            camera.pixelformat = return_format(argv[1]);
            while(camera.pixelformat != fmt_fourcc[i].fourcc)
                i++;
            camera.ToRGB24 = fmt_fourcc[i].ToRGB24Func;
        }

        camera.win_width = atoi(argv[2]);
        camera.win_height = atoi(argv[3]);
        camera.use_wm = atoi(argv[4]);
        camera.photo_type = atoi(argv[5]);
        if(strcmp(argv[5],"all") == 0){
            camera.photo_type = ALL_TYPE;
        }else if(strcmp(argv[5],"bmp") == 0){
            camera.photo_type = BMP_TYPE;
        }else if(strcmp(argv[5],"yuv") == 0){
            camera.photo_type = YUV_TYPE;
        }else{
            camera_err(" input %s error\n", argv[5]);
            camera_print(" using default parameters:save BMP and YUV format\n");
            camera.photo_type = ALL_TYPE;
        }

        memset(camera.save_path, 0, sizeof(camera.save_path));
        memcpy(camera.save_path, argv[6], sizeof(camera.save_path));
        camera.photo_num = atoi(argv[7]);
        if(camera.photo_num <= 0)
            camera_err(" input photo number is %d\n", camera.photo_num);
    }else if((argc == 9) && (strcmp(argv[8],"debug") == 0)){
        if(return_format(argv[1]) < 0){
            camera_err(" unsupport input format %s\n", argv[1]);
            camera_print(" using format parameters NV21\n");
        }else{
            int i = 0;
            camera.pixelformat = return_format(argv[1]);
            while(camera.pixelformat != fmt_fourcc[i].fourcc)
                i++;
            camera.ToRGB24 = fmt_fourcc[i].ToRGB24Func;
        }

        camera_dbg_en = 1;
        camera.win_width = atoi(argv[2]);
        camera.win_height = atoi(argv[3]);
        camera.use_wm = atoi(argv[4]);
        camera.photo_type = atoi(argv[5]);
        if(strcmp(argv[5],"all") == 0){
            camera.photo_type = ALL_TYPE;
        }else if(strcmp(argv[5],"bmp") == 0){
            camera.photo_type = BMP_TYPE;
        }else if(strcmp(argv[5],"yuv") == 0){
            camera.photo_type = YUV_TYPE;
        }else{
            camera_err(" input %s error\n", argv[5]);
            camera_print(" using default parameters:save BMP and YUV format\n");
            camera.photo_type = ALL_TYPE;
        }

        memset(camera.save_path, 0, sizeof(camera.save_path));
        memcpy(camera.save_path, argv[6], sizeof(camera.save_path));
        camera.photo_num = atoi(argv[7]);
        if(camera.photo_num <= 0)
            camera_err(" input photo number is %d\n", camera.photo_num);
    }else if(argc == 9){
            if(return_format(argv[1]) < 0){
                camera_err(" unsupport input format %s\n", argv[1]);
                camera_print(" using format parameters NV21\n");
            }else{
                int i = 0;
                camera.pixelformat = return_format(argv[1]);
                while(camera.pixelformat != fmt_fourcc[i].fourcc)
                    i++;
                camera.ToRGB24 = fmt_fourcc[i].ToRGB24Func;
            }

            camera.win_width = atoi(argv[2]);
            camera.win_height = atoi(argv[3]);
            camera.use_wm = atoi(argv[4]);
            camera.photo_type = atoi(argv[5]);
            if(strcmp(argv[5],"all") == 0){
                camera.photo_type = ALL_TYPE;
            }else if(strcmp(argv[5],"bmp") == 0){
                camera.photo_type = BMP_TYPE;
            }else if(strcmp(argv[5],"yuv") == 0){
                camera.photo_type = YUV_TYPE;
            }else{
                camera_err(" input %s error\n", argv[5]);
                camera_print(" using default parameters:save BMP and YUV format\n");
                camera.photo_type = ALL_TYPE;
            }

            memset(camera.save_path, 0, sizeof(camera.save_path));
            memcpy(camera.save_path, argv[6], sizeof(camera.save_path));
            camera.photo_num = atoi(argv[7]);
            if(camera.photo_num <= 0)
                camera_err(" input photo number is %d\n", camera.photo_num);

            camera.camera_index = atoi(argv[8]);
        }else if((argc == 10) && (strcmp(argv[9],"debug") == 0)){
            if(return_format(argv[1]) < 0){
                camera_err(" unsupport input format %s\n", argv[1]);
                camera_print(" using format parameters NV21\n");
            }else{
                int i = 0;
                camera.pixelformat = return_format(argv[1]);
                while(camera.pixelformat != fmt_fourcc[i].fourcc)
                    i++;
                camera.ToRGB24 = fmt_fourcc[i].ToRGB24Func;
            }

            camera_dbg_en = 1;
            camera.win_width = atoi(argv[2]);
            camera.win_height = atoi(argv[3]);
            camera.use_wm = atoi(argv[4]);
            camera.photo_type = atoi(argv[5]);
            if(strcmp(argv[5],"all") == 0){
                camera.photo_type = ALL_TYPE;
            }else if(strcmp(argv[5],"bmp") == 0){
                camera.photo_type = BMP_TYPE;
            }else if(strcmp(argv[5],"yuv") == 0){
                camera.photo_type = YUV_TYPE;
            }else{
                camera_err(" input %s error\n", argv[5]);
                camera_print(" using default parameters:save BMP and YUV format\n");
                camera.photo_type = ALL_TYPE;
            }

            memset(camera.save_path, 0, sizeof(camera.save_path));
            memcpy(camera.save_path, argv[6], sizeof(camera.save_path));
            camera.photo_num = atoi(argv[7]);
            if(camera.photo_num <= 0)
                camera_err(" input photo number is %d\n", camera.photo_num);

            camera.camera_index = atoi(argv[8]);
        }else if(argc != 1){
            show_help();
            return -1;
        }

    /* 1.open /dev/videoX node */
    sprintf(camera_path, "%s%d", "/dev/video", camera.camera_index);
    camera_print("**********************************************************\n");
    camera_print(" open %s!\n", camera_path);
    camera_print("**********************************************************\n");

    camera.videofd = open(camera_path, O_RDWR, 0);
    if(camera.videofd < 0){
        camera_err(" open %s fail!!!\n", camera_path);

        return -1;
    }

    /* 2.Query device capabilities */
    memset(&cap, 0, sizeof(cap));
    if(ioctl(camera.videofd,VIDIOC_QUERYCAP,&cap) < 0){
        camera_err(" Query device capabilities fail!!!\n");
    }else{
        camera_dbg(" Querey device capabilities succeed\n");
        camera_dbg(" cap.driver=%s\n",cap.driver);
        camera_dbg(" cap.card=%s\n",cap.card);
        camera_dbg(" cap.bus_info=%s\n",cap.bus_info);
        camera_dbg(" cap.version=0x%08x\n",cap.version);
        camera_dbg(" cap.capabilities=0x%08x\n",cap.capabilities);
    }

    if((cap.capabilities & (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_VIDEO_CAPTURE_MPLANE)) <= 0){
        camera_err(" The device is not supports the Video Capture interface!!!\n");
        close(camera.videofd);
        return -1;
    }

    if(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE){
        camera.driver_type = V4L2_CAP_VIDEO_CAPTURE_MPLANE;
    }else if(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE){
        camera.driver_type = V4L2_CAP_VIDEO_CAPTURE;
    }else{
        camera_err(" %s is not a capture device.\n",camera_path);
        close(camera.videofd);
        return -1;
    }

    if(!camera.isDefault){
        camera_print("**********************************************************\n");

        camera_print(" Please enter the data save path:\n");
        input_stdin("%s",camera.save_path);

        camera_print(" Please enter the number of captured photos:\n");
        input_stdin("%d",&camera.photo_num);

set_type:
        camera_print(" Please enter the data save type:\n");
        camera_print(" 0:save BMP and YUV formats\n");
        camera_print(" 1:save BMP format\n");
        camera_print(" 2:save YUV format\n");
        input_stdin("%d",&camera.photo_type);
        if(camera.photo_type > 2 || camera.photo_type < 0){
            camera_err(" input error\n");
            goto set_type;
        }

        camera_print(" Whether to add a watermark, enter 0 or 1, 0---NO, 1---YES:\n");
        input_stdin("%d",&camera.use_wm);
    }

    camera_print("**********************************************************\n");
    camera_print(" The path to data saving is %s.\n",camera.save_path);
    camera_print(" The number of captured photos is %d.\n",camera.photo_num);
    switch(camera.photo_type){
    case 0:
        camera_print(" save bmp and yuv format\n");
        break;
    case 1:
        camera_print(" save bmp format\n");
        break;
    case 2:
        camera_print(" save yuv format\n");
        break;
    default:
        break;
    }
    if(camera.use_wm == 0)
        camera_print(" do not use watermarks\n");
    else
        camera_print(" use watermarks\n");

    /* 3.select the current video input */
    inp.index = 0;
    inp.type = V4L2_INPUT_TYPE_CAMERA;
    if(ioctl(camera.videofd,VIDIOC_S_INPUT,&inp) < 0){
        camera_err(" VIDIOC_S_INPUT failed! s_input: %d\n",inp.index);
        close(camera.videofd);
        return -1;
    }

#ifdef __USE_VIN_ISP__
    /* detect sensor type */
    camera.sensor_type = getSensorType(camera.videofd);
    if(camera.sensor_type == V4L2_SENSOR_TYPE_RAW){
        camera.ispPort = CreateAWIspApi();
        camera_dbg(" raw sensor use vin isp\n");
    }
#endif

    /* 4.Enumerate image formats */
    camera_dbg("**********************************************************\n");
    camera_dbg(" enumerate image formats\n");
    memset(support_fmt, 0, sizeof(support_fmt));
    fmtdesc.index = 0;
    if(camera.driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    else
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    index = 0;
    while(ioctl(camera.videofd,VIDIOC_ENUM_FMT,&fmtdesc) == 0)
    {
        /* save image formats */
        if(get_format_name(fmtdesc.pixelformat) != NULL){
            memcpy(support_fmt[index].name, fmtdesc.description, sizeof(fmtdesc.description));
            support_fmt[index].fourcc = fmtdesc.pixelformat;

            camera_dbg(" format index = %d, name = %s\n", index, get_format_name(fmtdesc.pixelformat));
            index++;
        }
        fmtdesc.index++;
    }

    /* 5.try all format,here resolution is not important */
    int enumFmtIndex = 0;
    formatNum = 0;
    memset(&fmt,0,sizeof(fmt));
    if(camera.driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE){
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        fmt.fmt.pix_mp.width = camera.win_width;
        fmt.fmt.pix_mp.height = camera.win_height;
        fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
    }else{
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = camera.win_width;
        fmt.fmt.pix.height = camera.win_height;
        fmt.fmt.pix.field = V4L2_FIELD_NONE;
    }
    memset(sensor_formats,0,sizeof(sensor_formats));

    camera_dbg("*********************************************************\n");
    camera_dbg(" The sensor supports the following formats :\n");
    while(support_fmt[enumFmtIndex].fourcc != 0)
    {
        if(camera.driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
            fmt.fmt.pix_mp.pixelformat = support_fmt[enumFmtIndex].fourcc;
        else
            fmt.fmt.pix.pixelformat = support_fmt[enumFmtIndex].fourcc;

        if(ioctl(camera.videofd,VIDIOC_TRY_FMT,&fmt) == 0){
            if(camera.driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
                sensor_formats[formatNum].fourcc = fmt.fmt.pix_mp.pixelformat;
            else
                sensor_formats[formatNum].fourcc = fmt.fmt.pix.pixelformat;
            camera_dbg(" Index %d : %s.\n",formatNum,get_format_name(sensor_formats[formatNum].fourcc));
            formatNum++;
        }
        enumFmtIndex++;
    }

    /* 6.Enumerate frame sizes */
    enumFmtIndex = 0;

    while(sensor_formats[enumFmtIndex].fourcc != 0)
    {
        memset(&frmsize,0,sizeof(frmsize));
        frmsize.index = 0;
        frmsize.pixel_format = sensor_formats[enumFmtIndex].fourcc;
        camera_dbg("**********************************************************\n");
        camera_dbg(" The %s supports the following resolutions:\n",get_format_name(sensor_formats[enumFmtIndex].fourcc));
        while(ioctl(camera.videofd,VIDIOC_ENUM_FRAMESIZES,&frmsize) == 0){
            if(frmsize.type == V4L2_FRMSIZE_TYPE_CONTINUOUS){
                sensor_formats[enumFmtIndex].size[frmsize.index].width = frmsize.stepwise.max_width;
                sensor_formats[enumFmtIndex].size[frmsize.index].height = frmsize.stepwise.max_height;

                camera_dbg(" Index %d : %u * %u\n",frmsize.index,frmsize.stepwise.max_width,frmsize.stepwise.max_height);
            }else{
                sensor_formats[enumFmtIndex].size[frmsize.index].width = frmsize.discrete.width;
                sensor_formats[enumFmtIndex].size[frmsize.index].height = frmsize.discrete.height;

                camera_dbg(" Index %d : %u * %u\n",frmsize.index,frmsize.discrete.width,frmsize.discrete.height);
            }

			memset (&ival, 0, sizeof (struct v4l2_frmivalenum));
			ival.index = 0;
			ival.pixel_format = frmsize.pixel_format;
			ival.width = sensor_formats[enumFmtIndex].size[frmsize.index].width;
			ival.height = sensor_formats[enumFmtIndex].size[frmsize.index].height;

			if(ioctl(camera.videofd,VIDIOC_ENUM_FRAMEINTERVALS,&ival) < 0)
				goto enum_frameintervals_failed;

			if(ival.type == V4L2_FRMIVAL_TYPE_DISCRETE){
				do{
					camera_dbg("	adding discrete framerate:: %d/%d\n",
										ival.discrete.denominator,ival.discrete.numerator);
					ival.index++;
				}while(ioctl(camera.videofd,VIDIOC_ENUM_FRAMEINTERVALS,&ival) >= 0);
			}else if(ival.type == V4L2_FRMIVAL_TYPE_CONTINUOUS){
				camera_dbg("	continuous frame interval %d/%d to %d/%d\n",
									ival.stepwise.max.denominator,ival.stepwise.max.numerator,
									ival.stepwise.min.denominator,ival.stepwise.min.numerator);
			}

enum_frameintervals_failed:
            frmsize.index++;
            if(frmsize.index > N_WIN_SIZES-1){
                camera_err(" Beyond the maximum queryable range, please modify N_WIN_SIZES.\n");
                break;
            }
        }

        /* next frame */
        enumFmtIndex++;
    }

SET_FORMAT:
    if(!camera.isDefault){
        int i = 0;

        /* Select the format */
        camera_print("*********************************************************\n");
        camera_print(" The sensor supports the following formats :\n");
        while(sensor_formats[i].fourcc != 0)
        {
            camera_print(" index %d : %s\n",i, get_format_name(sensor_formats[i].fourcc));
            i++;
        }

        camera_print(" Please enter the serial number you need for pixelformat:\n");
        input_stdin("%d",&index);

        if( sensor_formats[index].fourcc == 0 ){
            camera_err(" Input ordinal corresponds to unsupported format, please re-enter!\n");

            goto SET_FORMAT;
        }else{
            camera_print(" The input value is %d.\n",index);
        }

        camera.pixelformat = sensor_formats[index].fourcc;
        for(i=0; i<FOURCC_NUM; i++)
        {
            if(camera.pixelformat == fmt_fourcc[i].fourcc){
                camera.ToRGB24 = fmt_fourcc[i].ToRGB24Func;
                camera_print(" camera pixelformat: %s\n",fmt_fourcc[i].name);
                break;
            }
        }
        if(i >= FOURCC_NUM)
            camera.ToRGB24 = NULL;

        /* select the size */
        for(enumFmtIndex=0; sensor_formats[enumFmtIndex].fourcc!=0; enumFmtIndex++)
            if(camera.pixelformat == sensor_formats[enumFmtIndex].fourcc)
                break;

        i = 0;
        camera_print("**********************************************************\n");
        camera_print(" The %s supports the following resolutions:\n",get_format_name(sensor_formats[enumFmtIndex].fourcc));
        while(sensor_formats[enumFmtIndex].size[i].width != 0)
        {
            camera_print(" Index %d : %u * %u\n", i, sensor_formats[enumFmtIndex].size[i].width, sensor_formats[enumFmtIndex].size[i].height);
            i++;
        }
        /* Gets the resolution index */
        while(1)
        {
            camera_print(" Please enter the serial number you need for windows size:\n");
            input_stdin("%d",&index);

            if( (index >= 0) && (index < i)){
                camera_print(" The input value is %d.\n",index);
                camera.win_width = sensor_formats[enumFmtIndex].size[index].width;
                camera.win_height = sensor_formats[enumFmtIndex].size[index].height;
                camera_print(" Resolution size : %u * %u\n",camera.win_width,camera.win_height);
                break;
            }

            camera_err(" The input value %d is not in the 0-%d range, please re-enter\n", index, i-1);
        }
    }else{
        int i;
        camera_print("**********************************************************\n");
        camera_print(" Using format parameters %s.\n", get_format_name(camera.pixelformat));
        for(i=0; sensor_formats[i].fourcc!=0; i++)
            if(camera.pixelformat == sensor_formats[i].fourcc)
                break;

        /* find support pixelformat */
        if(sensor_formats[i].fourcc == 0){
            camera_err(" sensor not support %s\n", get_format_name(camera.pixelformat));
            camera_print(" Use support for the first format and resolution\n");
            camera.pixelformat = sensor_formats[0].fourcc;
            camera.win_width = sensor_formats[0].size[0].width;
            camera.win_height = sensor_formats[0].size[0].height;

            /*find YUVToRGB func*/
            camera.ToRGB24 = NULL;
            for(int j=0; j<FOURCC_NUM; j++)
                if(camera.pixelformat == fmt_fourcc[j].fourcc)
                    camera.ToRGB24 = fmt_fourcc[j].ToRGB24Func;
        }else{
            int j,sizeindex;

            for(j=0; sensor_formats[j].fourcc!=0; j++)
                if(camera.pixelformat == sensor_formats[j].fourcc)
                    break;

            for(sizeindex=0;sensor_formats[j].size[sizeindex].width!=0; sizeindex++)
                if(camera.win_width == sensor_formats[j].size[sizeindex].width && camera.win_height == sensor_formats[j].size[sizeindex].height)
                    break;

            if(sensor_formats[j].size[sizeindex].width == 0){
                camera_err(" sensor not support %u * %u\n", camera.win_width, camera.win_height);
                camera_print(" use support for the first resolution\n");
                if(sensor_formats[j].size[0].width != 0 && sensor_formats[j].size[0].height != 0){
                    camera.win_width = sensor_formats[j].size[0].width;
                    camera.win_height = sensor_formats[j].size[0].height;
                }else{
                    /* mandatory settings */
                    camera.win_width = 640;
                    camera.win_height = 480;
                }
            }
        }

        camera_print(" camera pixelformat: %s\n",get_format_name(camera.pixelformat));
        camera_print(" Resolution size : %u * %u\n",camera.win_width,camera.win_height);
    }

    camera_print(" The photo save path is %s.\n",camera.save_path);
    camera_print(" The number of photos taken is %d.\n",camera.photo_num);

    /* water mark init */
    if(camera.use_wm > 0){
        if(WMInit(&camera.WM_info, "/etc/res/wm_540p_") < 0){
            camera.use_wm = 0;
            camera_err(" water mark init error\n");
        }
    }

    /* 7.set streaming parameters */
    memset(&parms, 0, sizeof(struct v4l2_streamparm));
    if(camera.driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    else
        parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parms.parm.capture.timeperframe.numerator = 1;
    parms.parm.capture.timeperframe.denominator = 30;
    if(ioctl(camera.videofd,VIDIOC_S_PARM,&parms) < 0){
        camera_err(" Setting streaming parameters failed, numerator:%d denominator:%d\n",
                                                    parms.parm.capture.timeperframe.numerator,
                                                    parms.parm.capture.timeperframe.denominator);
        close(camera.videofd);
        return -1;
    }

    /* 8.get streaming parameters */
    memset(&parms,0,sizeof(struct v4l2_streamparm));
    if(camera.driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    else
        parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(camera.videofd,VIDIOC_G_PARM,&parms) < 0){
        camera_err(" Get streaming parameters failed!\n");
    }else{
        camera_print(" Camera capture framerate is %u/%u\n",
                parms.parm.capture.timeperframe.denominator, \
                parms.parm.capture.timeperframe.numerator);
    }

    /* 9.set the data format */
    memset(&fmt, 0, sizeof(struct v4l2_format));
    if(camera.driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE){
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        fmt.fmt.pix_mp.width = camera.win_width;
        fmt.fmt.pix_mp.height = camera.win_height;
        fmt.fmt.pix_mp.pixelformat = camera.pixelformat;
        fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
    }else{
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = camera.win_width;
        fmt.fmt.pix.height = camera.win_height;
        fmt.fmt.pix.pixelformat = camera.pixelformat;
        fmt.fmt.pix.field = V4L2_FIELD_NONE;
    }

    if (ioctl(camera.videofd, VIDIOC_S_FMT, &fmt) < 0){
        camera_err(" setting the data format failed!\n");
        close(camera.videofd);
        return -1;
    }

    if(camera.driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE){
        if(camera.win_width != fmt.fmt.pix_mp.width || camera.win_height != fmt.fmt.pix_mp.height)
            camera_err(" does not support %u * %u\n", camera.win_width, camera.win_height);

        camera.win_width = fmt.fmt.pix_mp.width;
        camera.win_height = fmt.fmt.pix_mp.height;
        camera_print(" VIDIOC_S_FMT succeed\n");
        camera_print(" fmt.type = %d\n",fmt.type);
        camera_print(" fmt.fmt.pix_mp.width = %d\n",fmt.fmt.pix_mp.width);
        camera_print(" fmt.fmt.pix_mp.height = %d\n",fmt.fmt.pix_mp.height);
        camera_print(" fmt.fmt.pix_mp.pixelformat = %s\n",get_format_name(fmt.fmt.pix_mp.pixelformat));
        camera_print(" fmt.fmt.pix_mp.field = %d\n",fmt.fmt.pix_mp.field);

        if (ioctl(camera.videofd, VIDIOC_G_FMT, &fmt) < 0)
            camera_err(" get the data format failed!\n");

        camera.nplanes = fmt.fmt.pix_mp.num_planes;
    }else{
        if(camera.win_width != fmt.fmt.pix.width || camera.win_height != fmt.fmt.pix.height)
            camera_err(" does not support %u * %u\n", camera.win_width, camera.win_height);

        camera.win_width = fmt.fmt.pix.width;
        camera.win_height = fmt.fmt.pix.height;
        camera_print(" VIDIOC_S_FMT succeed\n");
        camera_print(" fmt.type = %d\n",fmt.type);
        camera_print(" fmt.fmt.pix.width = %d\n",fmt.fmt.pix.width);
        camera_print(" fmt.fmt.pix.height = %d\n",fmt.fmt.pix.height);
        camera_print(" fmt.fmt.pix.pixelformat = %s\n",get_format_name(fmt.fmt.pix.pixelformat));
        camera_print(" fmt.fmt.pix.field = %d\n",fmt.fmt.pix.field);
    }

    /* 10.Initiate Memory Mapping or User Pointer I/O */
    memset(&req, 0, sizeof(struct v4l2_requestbuffers));
    req.count = 3;
    if(camera.driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    else
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if(ioctl(camera.videofd, VIDIOC_REQBUFS, &req) < 0){
        camera_err(" VIDIOC_REQBUFS failed\n");
        close(camera.videofd);
        return -1;
    }

    /* Query the status of a buffers */
    camera.buf_count = req.count;
    camera_dbg(" reqbuf number is %d\n",camera.buf_count);

    camera.buffers = calloc(req.count, sizeof(struct buffer));
    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        memset(&buf, 0, sizeof(struct v4l2_buffer));
        if(camera.driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        else
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;
        if(camera.driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE){
            buf.length = camera.nplanes;
            buf.m.planes =  (struct v4l2_plane *)calloc(buf.length, sizeof(struct v4l2_plane));
        }

        if (ioctl(camera.videofd, VIDIOC_QUERYBUF, &buf) == -1) {
            camera_err(" VIDIOC_QUERYBUF error\n");

            if(camera.driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
                free(buf.m.planes);
            free(camera.buffers);

            close(camera.videofd);

            return -1;
        }

        if(camera.driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE){
            for(int i = 0; i < camera.nplanes; i++){
                camera.buffers[n_buffers].length[i] = buf.m.planes[i].length;
                camera.buffers[n_buffers].start[i] = mmap(NULL , buf.m.planes[i].length,
                                                   PROT_READ | PROT_WRITE, \
                                                   MAP_SHARED , camera.videofd, \
                                                   buf.m.planes[i].m.mem_offset);

                camera_dbg(" map buffer index: %d, mem: %p, len: %x, offset: %x\n",
                   n_buffers, camera.buffers[n_buffers].start[i],buf.m.planes[i].length,
                   buf.m.planes[i].m.mem_offset);
            }
            free(buf.m.planes);
        }else{
            camera.buffers[n_buffers].length[0] = buf.length;
            camera.buffers[n_buffers].start[0] = mmap(NULL , buf.length,
                                                PROT_READ | PROT_WRITE, \
                                                MAP_SHARED , camera.videofd, \
                                                buf.m.offset);
            camera_dbg(" map buffer index: %d, mem: %p, len: %x, offset: %x\n", \
                    n_buffers, camera.buffers[n_buffers].start[0],buf.length,buf.m.offset);
        }
    }

    /* 11.Exchange a buffer with the driver */
    for(n_buffers = 0; n_buffers < req.count; n_buffers++) {
        memset(&buf, 0, sizeof(struct v4l2_buffer));
        if(camera.driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        else
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory= V4L2_MEMORY_MMAP;
        buf.index= n_buffers;
        if(camera.driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE){
            buf.length = camera.nplanes;
            buf.m.planes =  (struct v4l2_plane *)calloc(buf.length, sizeof(struct v4l2_plane));
        }

        if (ioctl(camera.videofd, VIDIOC_QBUF, &buf) == -1) {
            camera_err(" VIDIOC_QBUF error\n");

            if(camera.driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
                free(buf.m.planes);
            free(camera.buffers);

            close(camera.videofd);
            return -1;
        }
        if(camera.driver_type == V4L2_CAP_VIDEO_CAPTURE_MPLANE)
            free(buf.m.planes);
    }

    /* 12.Capture photo */
    if(capture_photo(&camera) < 0){
        camera_err(" capture_photo return error\n");
        return -1;
    }

    camera_dbg("***************************************************************\n");
    camera_dbg(" Performance Testing---format:%s size:%u * %u\n", get_format_name(camera.pixelformat),
                                                                camera.win_width, camera.win_height);
    camera_dbg(" The interval from open to streaming is %lld ms.\n", test_time(&camera));
    camera_dbg("***************************************************************\n");

    return 0;
}
