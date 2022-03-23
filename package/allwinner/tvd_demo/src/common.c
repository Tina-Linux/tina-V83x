/*************************************************************************
> File Name: common.c
> Author:
> Mail:
> Created Time:
************************************************************************/

#include "common.h"

long long secs_to_msecs(long long secs,long long usecs)
{
    long long msecs;

    msecs = usecs/1000 + secs* 1000;

    return msecs;
}

int save_frame_to_file(void *str,void *start,int length)
{
    FILE *fp = NULL;

    fp = fopen(str, "wb+"); //save more frames
    if(!fp)
    {
        printf(" Open file error\n");

        return -1;
    }

    if(fwrite(start, length, 1, fp))
    {
        //printf(" Write finish!\n");
        fclose(fp);

        return 0;
    }
    else
    {
        printf(" Write file fail (%s)\n",strerror(errno));
        fclose(fp);

        return -1;
    }

    return 0;
}

/****************************************************************************
*Measure the frame rate according to the interval of dqbuf.
*Take the last three times the average.
***************************************************************************/
int measure_fps(int fd)
{
    struct v4l2_buffer buf;
    struct timeval tv;
    long long timestamp = 0;
    fd_set testfds;
    long long timestamp_now,timestamp_save;
    int np = 0;
    int ret = 0;
    int timeval = 0;

    FD_ZERO(&testfds);
    FD_SET(fd,&testfds);

    memset(&buf,0,sizeof(struct v4l2_buffer));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    while(np < 5)
    {
        /* wait for sensor capture data */
        tv.tv_sec  = 1;
        tv.tv_usec = 0;
        ret = select(fd + 1, &testfds, NULL, NULL, &tv);
        if (ret == -1)
            printf(" select error\n");
        else if (ret == 0)
            printf(" select timeout\n");

        /* dqbuf */
        ret = ioctl(fd, VIDIOC_DQBUF, &buf);
        if(ret != 0)
            printf("****DQBUF FAIL*****\n");

        timestamp_now = secs_to_msecs((long long)(buf.timestamp.tv_sec), (long long)(buf.timestamp.tv_usec));
        if (np == 0)
            timestamp_save = timestamp_now;

        if(np != 1)
            timeval  += (timestamp_now - timestamp_save);

        timestamp_save = timestamp_now;

        /* qbuf */
        if (ioctl(fd, VIDIOC_QBUF, &buf) != 0)
            printf("****QBUF FAIL*****\n");

        np++;
    }

    return 1000/(timeval / 3);
}
