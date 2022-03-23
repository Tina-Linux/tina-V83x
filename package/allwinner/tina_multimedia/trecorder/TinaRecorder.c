#include "TinaRecorder.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <CdxQueue.h>
#include <AwPool.h>
#include <CdxBinary.h>
#include "memoryAdapter.h"
#include "awencoder.h"
#include "RecorderWriter.h"
#include "TParseConfig.h"
#include "TRcommon.h"
#include "TRlog.h"

#define SAVE_DISP_SRC (0)
#define SAVE_AUDIO_SRC (0)

#define MAX_ERR_NUM 3
#define DEQUEUE_MAX_ERR_NUM 5

static int muxerThreadNum = 0;
static int dirty_background_bytes = 0;
static int dirty_background_ratio = 0;

typedef struct DemoRecoderContext
{
    AwEncoder*       mAwEncoder;
    int encoderInitOkFlag;
    int encoderEnable;
    VideoEncodeConfig videoConfig;
    AudioEncodeConfig audioConfig;
    CdxMuxerT* pMuxer;
    int muxType;
    CdxWriterT* pStream;
    unsigned char* extractDataBuff;
    unsigned int extractDataLength;
    pthread_t encoderThreadId;
    pthread_t muxerThreadId;
	EncDataCallBackOps *ops;
}DemoRecoderContext;

#if SAVE_DISP_SRC
typedef int (*ConverFunc)(void *rgbData,void *yuvData,int width,int height);

int NV12ToRGB24(void *RGB24,void *NV12,int width,int height)
{
    unsigned char *src_y = (unsigned char *)NV12;
    unsigned char *src_v = (unsigned char *)NV12 + width * height + 1;
    unsigned char *src_u = (unsigned char *)NV12 + width * height;

    unsigned char *dst_RGB = (unsigned char *)RGB24;

    int temp[3];

    if(RGB24 == NULL || NV12 == NULL || width <= 0 || height <= 0)
    {
        printf(" NV12ToRGB24 incorrect input parameter!\n");
        return -1;
    }

    for(int y = 0;y < height;y ++)
    {
        for(int x = 0;x < width;x ++)
        {
            int Y = y*width + x;
            int U = ( (y >> 1)*(width >>1) + (x >> 1) ) * 2;
            int V = U;

            temp[0] = src_y[Y] + ((7289 * src_u[U])>>12) - 228;  //b
            temp[1] = src_y[Y] - ((1415 * src_u[U])>>12) - ((2936 * src_v[V])>>12) + 136;  //g
            temp[2] = src_y[Y] + ((5765 * src_v[V])>>12) - 180;  //r

            dst_RGB[3*Y] = (temp[0]<0? 0: temp[0]>255? 255: temp[0]);
            dst_RGB[3*Y+1] = (temp[1]<0? 0: temp[1]>255? 255: temp[1]);
            dst_RGB[3*Y+2] = (temp[2]<0? 0: temp[2]>255? 255: temp[2]);
        }
    }

    return 0;

}

static int NV21ToRGB24(void *RGB24,void *NV21,int width,int height)
{
    unsigned char *src_y = (unsigned char *)NV21;
    unsigned char *src_v = (unsigned char *)NV21 + width * height;
    unsigned char *src_u = (unsigned char *)NV21 + width * height + 1;

    unsigned char *dst_RGB = (unsigned char *)RGB24;

    int temp[3];

    if(RGB24 == NULL || NV21 == NULL || width <= 0 || height <= 0){
        TRerr(" NV21ToRGB24 incorrect input parameter\n");
        return -1;
    }

    for(int y = 0;y < height;y ++){
        for(int x = 0;x < width;x ++){
            int Y = y*width + x;
            int U = ( (y >> 1)*(width >>1) + (x >> 1) )<<1;
            int V = U;

            temp[0] = src_y[Y] + ((7289 * src_u[U])>>12) - 228;  //b
            temp[1] = src_y[Y] - ((1415 * src_u[U])>>12) - ((2936 * src_v[V])>>12) + 136;  //g
            temp[2] = src_y[Y] + ((5765 * src_v[V])>>12) - 180;  //r

            dst_RGB[3*Y] = (temp[0]<0? 0: temp[0]>255? 255: temp[0]);
            dst_RGB[3*Y+1] = (temp[1]<0? 0: temp[1]>255? 255: temp[1]);
            dst_RGB[3*Y+2] = (temp[2]<0? 0: temp[2]>255? 255: temp[2]);
        }
    }

    return 0;
}

int YUYVToRGB24(void *RGB24,void *YUYV,int width,int height)
{
    unsigned char *src_y = (unsigned char *)YUYV;
    unsigned char *src_u = (unsigned char *)YUYV + 1;
    unsigned char *src_v = (unsigned char *)YUYV + 3;

    unsigned char *dst_RGB = (unsigned char *)RGB24;

    int temp[3];

    if(RGB24 == NULL || YUYV == NULL || width <= 0 || height <= 0)
    {
        printf(" YUYVToRGB24 incorrect input parameter!\n");
        return -1;
    }

    for(int i = 0;i < width*height;i ++)
    {
            int Y = 2 * i;
            int U = (i >> 1) * 4;
            int V = U;

            temp[0] = src_y[Y] + ((7289 * src_u[U])>>12) - 228;  //b
            temp[1] = src_y[Y] - ((1415 * src_u[U])>>12) - ((2936 * src_v[V])>>12) + 136;  //g
            temp[2] = src_y[Y] + ((5765 * src_v[V])>>12) - 180;  //r

            dst_RGB[3*i] = (temp[0]<0? 0: temp[0]>255? 255: temp[0]);
            dst_RGB[3*i+1] = (temp[1]<0? 0: temp[1]>255? 255: temp[1]);
            dst_RGB[3*i+2] = (temp[2]<0? 0: temp[2]>255? 255: temp[2]);

    }
    return 0;

}

static int YUVToBMP(const char *bmp_path,char *yuv_data,ConverFunc func,int width,int height)
{
    unsigned char *rgb_24 = NULL;
    FILE *fp = NULL;

#define WORD  unsigned short
#define DWORD unsigned int
#define LONG  unsigned int

/* Bitmap header */
typedef struct tagBITMAPFILEHEADER {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
}__attribute__((packed)) BITMAPFILEHEADER;

/* Bitmap info header */
typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
}__attribute__((packed)) BITMAPINFOHEADER;

    BITMAPFILEHEADER BmpFileHeader;
    BITMAPINFOHEADER BmpInfoHeader;

    if(bmp_path == NULL || yuv_data == NULL || func == NULL || width <= 0 || height <= 0){
        printf(" YUVToBMP incorrect input parameter!\n");
        return -1;
    }

   /* Fill header information */
   BmpFileHeader.bfType = 0x4d42;
   BmpFileHeader.bfSize = width*height*3 + sizeof(BmpFileHeader) + sizeof(BmpInfoHeader);
   BmpFileHeader.bfReserved1 = 0;
   BmpFileHeader.bfReserved2 = 0;
   BmpFileHeader.bfOffBits = sizeof(BmpFileHeader) + sizeof(BmpInfoHeader);

   BmpInfoHeader.biSize = sizeof(BmpInfoHeader);
   BmpInfoHeader.biWidth = width;
   BmpInfoHeader.biHeight = -height;
   BmpInfoHeader.biPlanes = 0x01;
   BmpInfoHeader.biBitCount = 24;
   BmpInfoHeader.biCompression = 0;
   BmpInfoHeader.biSizeImage = 0;
   //BmpInfoHeader.biXPelsPerMeter = 0;
   //BmpInfoHeader.biYPelsPerMeter = 0;
   BmpInfoHeader.biClrUsed = 0;
   BmpInfoHeader.biClrImportant = 0;

    rgb_24 = (unsigned char *)malloc(width*height*3);
    if(rgb_24 == NULL){
       printf(" YUVToBMP alloc failed!\n");
       return -1;
    }

    func(rgb_24,yuv_data,width,height);

    /* Create bmp file */
    fp = fopen(bmp_path,"wb+");
    if(!fp){
        printf(" Create bmp file:%s faled!\n", bmp_path);
        free(rgb_24);
        return -1;
    }

    fwrite(&BmpFileHeader,sizeof(BmpFileHeader),1,fp);

    fwrite(&BmpInfoHeader,sizeof(BmpInfoHeader),1,fp);

    fwrite(rgb_24,width*height*3,1,fp);

    free(rgb_24);

    fclose(fp);

    return 0;
}

#endif

static int captureJpegPhoto(TinaRecorder *hdl,char *savePath,struct MediaPacket *yuvDataParam);

static int initStorePktInfo(TinaRecorder *hdlTina)
{
    if(!hdlTina)
        return -1;

    hdlTina->mTotalAudioPktBufSize = MUXER_AUDIO_QUEUE_KbSIZE*1024;
    hdlTina->mAudioPktBufSizeCount = 0;
    hdlTina->mNeedAudioPktBufSpace = 0;
    pthread_mutex_init(&hdlTina->mAudioPktBufLock, NULL);
    sem_init(&hdlTina->mAudioPktBufSem, 0, 0);

    if(hdlTina->encodeWidth >= 1920)
        hdlTina->mTotalVideoPktBufSize = MUXER_MoreVIDEO1080P_QUEUE_MbSIZE*1024*1024;
    else
        hdlTina->mTotalVideoPktBufSize = MUXER_LessVIDEO1080P_QUEUE_MbSIZE*1024*1024;
    hdlTina->mVideoPktBufSizeCount = 0;
    hdlTina->mNeedVideoPktBufSpace = 0;
    pthread_mutex_init(&hdlTina->mVideoPktBufLock, NULL);
    sem_init(&hdlTina->mVideoPktBufSem, 0, 0);

    TRlog(TR_LOG, "video index %d audio PktBufSize %d video PktBufSize %d\n",
            hdlTina->vport.mCameraIndex, hdlTina->mTotalAudioPktBufSize, hdlTina->mTotalVideoPktBufSize);

    return 0;
}

static int checkStorePktInfo(TinaRecorder *hdlTina, CdxMuxerPacketT *databuf)
{
    if(!hdlTina || !databuf)
        return -1;

    if(databuf->type == CDX_MEDIA_AUDIO){
        pthread_mutex_lock(&hdlTina->mAudioPktBufLock);
        if(hdlTina->mNeedAudioPktBufSpace > 0){
            hdlTina->mNeedAudioPktBufSpace -= databuf->buflen;
            if(hdlTina->mNeedAudioPktBufSpace <= 0)
                sem_post(&hdlTina->mAudioPktBufSem);
        }
        hdlTina->mAudioPktBufSizeCount -= databuf->buflen;
        pthread_mutex_unlock(&hdlTina->mAudioPktBufLock);
    }else if(databuf->type == CDX_MEDIA_VIDEO){
        pthread_mutex_lock(&hdlTina->mVideoPktBufLock);
        if(hdlTina->mNeedVideoPktBufSpace > 0){
            hdlTina->mNeedVideoPktBufSpace -= databuf->buflen;
            if(hdlTina->mNeedVideoPktBufSpace <= 0)
                sem_post(&hdlTina->mVideoPktBufSem);
        }
        hdlTina->mVideoPktBufSizeCount -= databuf->buflen;
        pthread_mutex_unlock(&hdlTina->mVideoPktBufLock);
    }

    return 0;
}

static int destroyStorePktInfo(TinaRecorder *hdlTina)
{
    if(!hdlTina)
        return -1;

    sem_destroy(&hdlTina->mVideoPktBufSem);
    pthread_mutex_destroy(&hdlTina->mVideoPktBufLock);
    sem_destroy(&hdlTina->mAudioPktBufSem);
    pthread_mutex_destroy(&hdlTina->mAudioPktBufLock);

    return 0;
}

static void TinaReadSetDirty(void)
{
    int dirtyfd = -1;
    char str[32];
    int ret = -1;

    /* dirty_background_bytes */
    if(access("/proc/sys/vm/dirty_background_bytes", F_OK | R_OK | W_OK) == 0){
        dirtyfd = open("/proc/sys/vm/dirty_background_bytes", O_RDWR, 0);
        if(dirtyfd){
            memset(str, 0, sizeof(str));
            read(dirtyfd, str, sizeof(str));
            close(dirtyfd);
            dirtyfd = 0;
            dirty_background_bytes = atoi(str);
        }
    }

    /* dirty_background_ratio */
    if(access("/proc/sys/vm/dirty_background_ratio", F_OK | R_OK | W_OK) == 0){
        dirtyfd = open("/proc/sys/vm/dirty_background_ratio", O_RDWR, 0);
        if(dirtyfd){
            memset(str, 0, sizeof(str));
            read(dirtyfd, str, sizeof(str));
            close(dirtyfd);
            dirtyfd = 0;
            dirty_background_ratio = atoi(str);
        }
    }

    /* setting dirty_background_bytes */
    if(access("/proc/sys/vm/dirty_background_bytes", F_OK | R_OK | W_OK) == 0){
        dirtyfd = open("/proc/sys/vm/dirty_background_bytes", O_RDWR, 0);
        if(dirtyfd){
            memset(str, 0, sizeof(str));
            sprintf(str, "%d", DIRTY_BACKGROUND_BYTES);
            ret = write(dirtyfd, str, strlen(str));
            if(ret < 0){
                TRerr("[%s] set /proc/sys/vm/dirty_background_bytes %d error\n",
                                            __func__, DIRTY_BACKGROUND_BYTES);
            }
            close(dirtyfd);
            dirtyfd = 0;
        }
    }
}

static void TinaResetDirty(void)
{
    int dirtyfd = -1;
    char str[32];
    int ret = -1;

    TRlog(TR_LOG, "[%s] reset dirty_background\n", __func__);

    /* reset dirty */
    if(dirty_background_bytes){
        dirtyfd = open("/proc/sys/vm/dirty_background_bytes", O_RDWR | O_TRUNC, 0);
        if(dirtyfd){
            memset(str, 0, sizeof(str));
            sprintf(str, "%d", dirty_background_bytes);
            ret = write(dirtyfd, str, strlen(str));
            if(ret < 0){
                TRerr("[%s] reset /proc/sys/vm/dirty_background_bytes %d error,errno(%d)\n",
                                            __func__, dirty_background_bytes, errno);
            }
            //lseek(dirtyfd, 0, SEEK_SET);
            //memset(str, 0, sizeof(str));
            //read(dirtyfd, str, sizeof(str));
            //TRprompt("[%s] set dirty_background_bytes %d,read %s\n",
            //                                    __func__, dirty_background_bytes, str);
            close(dirtyfd);
            dirtyfd = 0;
        }else
            TRerr("[%s] open /proc/sys/vm/dirty_background_bytes error,errno(%d)\n",
                                                __func__, errno);
    }else if(dirty_background_ratio){
        dirtyfd = open("/proc/sys/vm/dirty_background_ratio", O_RDWR | O_TRUNC, 0);
        if(dirtyfd){
            memset(str, 0, sizeof(str));
            sprintf(str, "%d", dirty_background_ratio);
            ret = write(dirtyfd, str, strlen(str));
            if(ret < 0){
                TRerr("[%s] reset /proc/sys/vm/dirty_background_ratio %d error,errno(%d)\n",
                                            __func__, dirty_background_ratio, errno);
            }
            //lseek(dirtyfd, 0, SEEK_SET);
            //memset(str, 0, sizeof(str));
            //read(dirtyfd, str, sizeof(str));
            //TRprompt("[%s] set dirty_background_ratio %d,read %s\n",
            //                                    __func__, dirty_background_ratio, str);
            close(dirtyfd);
            dirtyfd = 0;
        }else
            TRerr("[%s] open /proc/sys/vm/dirty_background_ratio error,errno(%d)\n",
                                                __func__, errno);
    }else
        TRerr("[%s] no dirty_background_* reset\n", __func__);
}

static void NotifyCallbackForAwEncorder(void* pUserData, int msg, void* param)
{
	TinaRecorder *hdlTina = (TinaRecorder *)pUserData;
	if(!hdlTina)
		return;

    int ret = 0;
    int bufId = 0;
    struct modulePacket *mPacket = NULL;
    struct modulePacket *OutputPacket = NULL;
    struct MediaPacket *outputbuf = NULL;
    VencThumbInfo thumbAddr;
    VencThumbInfo *pthumbAddr;
    DemoRecoderContext* demoRecoder = (DemoRecoderContext*)hdlTina->EncoderContext;
	renderBuf rBuf;
	int scaleWidth,scaleHeight;
    int index = 0;
    int errorCount = 0;

    switch(msg)
    {
        case AWENCODER_VIDEO_ENCODER_NOTIFY_RETURN_BUFFER:
        {
            bufId = *((int*)param);

callback_next:
            pthread_mutex_lock(&hdlTina->mEncoderPacketLock);
            for(index = 0; index < ENCODER_SAVE_NUM; index++){
                mPacket = hdlTina->encoderpacket[index];
                if(mPacket){
                    outputbuf = (struct MediaPacket *)(mPacket->buf);
                    if(outputbuf && (bufId == outputbuf->buf_index))
                        break;
                }
            }

            if(index < ENCODER_SAVE_NUM){
                packetDestroy(hdlTina->encoderpacket[index]);
                hdlTina->encoderpacket[index] = NULL;
                pthread_mutex_unlock(&hdlTina->mEncoderPacketLock);
            }else{
                pthread_mutex_unlock(&hdlTina->mEncoderPacketLock);
                TRerr("[%s] encoder %d unknow id %d, sleep wait...\n",
                                                __func__, hdlTina->vport.mCameraIndex, bufId);
                errorCount++;
                if(errorCount > MAX_ERR_NUM){
                    TRerr("[%s] encoder %d unknow id %d error count more than 3,encoder error exit\n",
                                                __func__, hdlTina->vport.mCameraIndex, bufId);
                    modulePort_SetEnable(&hdlTina->encoderPort, 0);
                    demoRecoder->encoderEnable = 0;
                    break;
                }

                usleep(500);
                goto callback_next;
            }
            break;
        }
		case AWENCODER_VIDEO_ENCODER_NOTIFY_RETURN_THUMBNAIL:
            if(hdlTina->dispport.enable == 0)
                break;

            pthread_mutex_lock(&hdlTina->mScaleCallbackLock);

			pthumbAddr = (VencThumbInfo *)param;

            if(pthumbAddr->pThumbBuf != (unsigned char *)hdlTina->rBuf.vir_addr){
                TRerr("[%s] encoder %d return thumbnail vir addr error\n",
                                                    __func__, hdlTina->vport.mCameraIndex);

                pthread_mutex_unlock(&hdlTina->mScaleCallbackLock);
                return;
            }

            scaleWidth = ((hdlTina->encodeWidth + 15)/16*16)/hdlTina->scaleDownRatio;
            scaleHeight = ((hdlTina->encodeHeight + 15)/16*16)/hdlTina->scaleDownRatio;

            /* create new packet */
            OutputPacket = (struct modulePacket *)packetCreate(sizeof(struct MediaPacket));
            if(!OutputPacket){
                TRerr("[%s] create packet error\n", __func__);

                pthread_mutex_unlock(&hdlTina->mScaleCallbackLock);
                return ;
            }
            /* fill new packet buf */
            outputbuf = (struct MediaPacket *)OutputPacket->buf;

            /* only push display, ignore buf_index and nPts */
            outputbuf->Vir_Y_addr = (unsigned char *)pthumbAddr->pThumbBuf;
            outputbuf->Vir_C_addr = (unsigned char *)pthumbAddr->pThumbBuf + scaleWidth*scaleHeight;
            outputbuf->Phy_Y_addr = (unsigned char *)hdlTina->rBuf.phy_addr;
            outputbuf->Phy_C_addr = (unsigned char *)hdlTina->rBuf.phy_addr + scaleWidth*scaleHeight;
            outputbuf->buf_index = 0;
            outputbuf->width = scaleWidth;
            outputbuf->height = scaleHeight;
            outputbuf->nPts = 0;
            outputbuf->data_len = scaleWidth*scaleHeight*3/2;
            outputbuf->bytes_used = scaleWidth*scaleHeight*3/2;
            outputbuf->format = TR_PIXEL_YUV420SP;

			/* push new packet to next queue */
            OutputPacket->OnlyMemFlag = 1;
			OutputPacket->packet_type = SCALE_YUV;
			ret = module_push(&hdlTina->encoderPort, OutputPacket);
            if(ret <= 0){
                TRerr("[%s] scale push data error\n", __func__);
                packetDestroy(OutputPacket);
            }
            OutputPacket = NULL;

            pthread_mutex_unlock(&hdlTina->mScaleCallbackLock);

			break;
		case AWENCODER_VIDEO_ENCODER_NOTIFY_THUMBNAIL_GETBUFFER:
            if(hdlTina->dispport.enable == 0)
                break;

            pthread_mutex_lock(&hdlTina->mScaleCallbackLock);

			scaleWidth = ((hdlTina->encodeWidth + 15)/16*16)/hdlTina->scaleDownRatio;
            scaleHeight = ((hdlTina->encodeHeight + 15)/16*16)/hdlTina->scaleDownRatio;

            memset(&rBuf, 0, sizeof(renderBuf));
            ret = hdlTina->dispport.dequeue(&hdlTina->dispport,&rBuf);
            if(ret < 0){
                TRerr("[%s] %d dispport dequeue return error\n",
                                                __func__, hdlTina->vport.mCameraIndex);

                pthread_mutex_unlock(&hdlTina->mScaleCallbackLock);
                return;
            }
            /* save disp physical address */
            memcpy(&hdlTina->rBuf, &rBuf, sizeof(renderBuf));

            thumbAddr.pThumbBuf = (unsigned char *)rBuf.vir_addr;
            thumbAddr.nThumbSize = scaleWidth*scaleHeight*3/2;

			AwEncoderSetParamete(demoRecoder->mAwEncoder,AwEncoder_SetThumbNailAddr,&thumbAddr);

            pthread_mutex_unlock(&hdlTina->mScaleCallbackLock);
			break;
		case AWENCODER_VIDEO_ENCODER_NOTIFY_ERROR:
            TRerr("[%s] error callback from AwRecorder %d.\n",
                                                __func__, hdlTina->vport.mCameraIndex);
			break;
		default:
			TRerr("[%s] unknown callback from AwRecorder %d.\n",
                                                __func__, hdlTina->vport.mCameraIndex);
			break;
	}

	return ;
}

static int EncoderfreePacket(void *packet)
{
	struct modulePacket *mPacket = (struct modulePacket *)packet;
	CdxMuxerPacketT *databuf = (CdxMuxerPacketT *)mPacket->buf;

	free(databuf->buf);

	return 0;
}

int onVideoDataEnc(void *app,CdxMuxerPacketT *buff)
{
	TinaRecorder *hdlTina = (TinaRecorder *)app;

	if(!hdlTina || !buff)
		return -1;

    struct modulePacket *mPacket = NULL;
    CdxMuxerPacketT *outputbuf = NULL;
    int ret;

	if (hdlTina->outport.enable){
        if((hdlTina->mNeedMuxerVideoFlag == MUXER_OK_KEYFRAME)
                    || (hdlTina->mNeedMuxerVideoFlag == MUXER_NEED_KEYFRAME && buff->keyFrameFlag == 1)){
            if(hdlTina->mNeedMuxerVideoFlag == MUXER_NEED_KEYFRAME && buff->keyFrameFlag == 1){
                hdlTina->mNeedMuxerVideoFlag = MUXER_OK_KEYFRAME;
                hdlTina->mBaseVideoPts = buff->pts;
            }

            pthread_mutex_lock(&hdlTina->mVideoPktBufLock);
            hdlTina->mNeedVideoPktBufSpace = buff->buflen-(hdlTina->mTotalVideoPktBufSize-hdlTina->mVideoPktBufSizeCount);
            if(hdlTina->mNeedVideoPktBufSpace > 0){
                TRwarn("[%s] video index %d mNeedVideoPktBufSpace = %d,muxer queue packet num = %d,waiting\n",
                                __func__, hdlTina->vport.mCameraIndex,
                                hdlTina->mNeedVideoPktBufSpace, queueCountRead(&hdlTina->muxerPort));
                pthread_mutex_unlock(&hdlTina->mVideoPktBufLock);
                sem_wait(&hdlTina->mVideoPktBufSem);
            }else{
                pthread_mutex_unlock(&hdlTina->mVideoPktBufLock);
            }
        }else{
            TRlog(TR_LOG_ENCODER, " wait key frame\n");
            return 0;
        }
    }else if(module_NoOutputPort(&hdlTina->encoderPort)){
        return 0;
    }

    mPacket = packetCreate(sizeof(CdxMuxerPacketT));
    if(!mPacket){
        TRerr("[%s] create packet error\n", __func__);
        return -1;
    }

    outputbuf = (CdxMuxerPacketT *)mPacket->buf;

	outputbuf->buflen = buff->buflen;
	outputbuf->length = buff->length;
	outputbuf->buf = malloc(buff->buflen);
    if(outputbuf->buf == NULL){
        TRerr("[%s] malloc encoder callback data buf error\n", __func__);
        free(mPacket->buf);
        free(mPacket);
        return -1;
    }

	memcpy(outputbuf->buf, buff->buf, buff->buflen);
	outputbuf->pts = buff->pts - hdlTina->mBaseVideoPts;
	outputbuf->type = buff->type;
	outputbuf->streamIndex  = buff->streamIndex;
    outputbuf->keyFrameFlag  = buff->keyFrameFlag;

    mPacket->OnlyMemFlag = 1;
    mPacket->mode.free.freePacketHdl = hdlTina;
    mPacket->mode.free.freeFunc = EncoderfreePacket;
    mPacket->packet_type = VIDEO_ENC;

    if(hdlTina->outport.enable){
        pthread_mutex_lock(&hdlTina->mVideoPktBufLock);
        hdlTina->mVideoPktBufSizeCount += outputbuf->buflen;
        pthread_mutex_unlock(&hdlTina->mVideoPktBufLock);
    }

	/* push encode video data packet into queue */
	ret = module_push(&hdlTina->encoderPort, mPacket);
    if(ret <= 0){
        if(hdlTina->outport.enable){
            pthread_mutex_lock(&hdlTina->mVideoPktBufLock);
            hdlTina->mVideoPktBufSizeCount -= outputbuf->buflen;
            pthread_mutex_unlock(&hdlTina->mVideoPktBufLock);
        }

        packetDestroy(mPacket);
    }

	return 0;
}

int onAudioDataEnc(void *app,CdxMuxerPacketT *buff)
{
	TinaRecorder *hdlTina = (TinaRecorder *)app;

	if(!hdlTina || !buff)
		return -1;

    struct modulePacket *mPacket = NULL;
    CdxMuxerPacketT *outputbuf = NULL;
    int ret;

	if (hdlTina->outport.enable){
        pthread_mutex_lock(&hdlTina->mAudioPktBufLock);
        hdlTina->mNeedAudioPktBufSpace = buff->buflen-(hdlTina->mTotalAudioPktBufSize-hdlTina->mAudioPktBufSizeCount);
        if(hdlTina->mNeedAudioPktBufSpace > 0){
            TRwarn("[%s] video index %d mNeedAudioPktBufSpace = %d,muxer queue packet num = %d,waiting\n",
                                        __func__, hdlTina->vport.mCameraIndex,
                                        hdlTina->mNeedAudioPktBufSpace, queueCountRead(&hdlTina->muxerPort));
            pthread_mutex_unlock(&hdlTina->mAudioPktBufLock);
            sem_wait(&hdlTina->mAudioPktBufSem);
        }else{
            pthread_mutex_unlock(&hdlTina->mAudioPktBufLock);
        }
    }else if(module_NoOutputPort(&hdlTina->encoderPort)){
        return 0;
    }

    mPacket = packetCreate(sizeof(CdxMuxerPacketT));
    if(!mPacket){
        TRerr("[%s] create packet error\n", __func__);
        return -1;
    }

    outputbuf = (CdxMuxerPacketT *)mPacket->buf;

	outputbuf->buflen = buff->buflen;
	outputbuf->length = buff->length;
	outputbuf->buf = malloc(buff->buflen);
    if(outputbuf->buf == NULL){
        TRerr("[%s] malloc encoder callback data buf error\n", __func__);
        free(mPacket->buf);
        free(mPacket);

        return -1;
    }
	memcpy(outputbuf->buf, buff->buf, buff->buflen);
	outputbuf->pts = buff->pts;
	outputbuf->type = buff->type;
	outputbuf->streamIndex = buff->streamIndex;
	outputbuf->duration = buff->duration;

    mPacket->OnlyMemFlag = 1;
    mPacket->mode.free.freePacketHdl = hdlTina;
    mPacket->mode.free.freeFunc = EncoderfreePacket;
    mPacket->packet_type = AUDIO_ENC;

    if (hdlTina->outport.enable){
        pthread_mutex_lock(&hdlTina->mAudioPktBufLock);
        hdlTina->mAudioPktBufSizeCount += outputbuf->buflen;
        pthread_mutex_unlock(&hdlTina->mAudioPktBufLock);
    }

	/* push encode audio data packet into queue */
	ret = module_push(&hdlTina->encoderPort, mPacket);
    if(ret <= 0){
        if (hdlTina->outport.enable){
            pthread_mutex_lock(&hdlTina->mAudioPktBufLock);
            hdlTina->mAudioPktBufSizeCount -= outputbuf->buflen;
            pthread_mutex_unlock(&hdlTina->mAudioPktBufLock);
        }

        packetDestroy(mPacket);
    }

	return 0;
}

void *TRDecoderThread(void *param)
{
    int i;
    TinaRecorder *hdlTina = (TinaRecorder *)param;
	if(!hdlTina)
		return NULL;

	decoder_handle *dec = &hdlTina->decoderport;
	struct modulePacket *mPacket = NULL;
	struct modulePacket *mPacket_new = NULL;
	struct MediaPacket *inputbuf = NULL;
	struct MediaPacket *outputbuf = NULL;
	int ret = -1;

    TRlog(TR_LOG_DECODER, " decoder video src %d thread start : pid %lu\n",
                                        hdlTina->vport.mCameraIndex, syscall(SYS_gettid));
    while(hdlTina->decoderport.enable){
        /* wait data in queue and pop it */
        module_waitReceiveSem(&hdlTina->decoderPort);

        /* pop packet from queue */
		mPacket = (struct modulePacket *)module_pop(&hdlTina->decoderPort);
        if(!mPacket || !mPacket->buf)
            continue;

        /* process packet buf */
        inputbuf = (struct MediaPacket *)mPacket->buf;

		if(!inputbuf->Vir_Y_addr || inputbuf->bytes_used <= 0){
            TRerr("[%s] decoder %d pop data error,vir_addr %p bytes_used %d\n",
                __func__, hdlTina->vport.mCameraIndex,
                inputbuf->Vir_Y_addr, inputbuf->bytes_used);
            packetDestroy(mPacket);
            mPacket = NULL;
            TRerr(" decoder %d destroy error packet ok\n", hdlTina->vport.mCameraIndex);
            continue;
        }

        ret = DecoderDequeue(dec,
                            (unsigned char *)inputbuf->Vir_Y_addr,
                            inputbuf->nPts,
                            inputbuf->bytes_used);

		/* if decode error we can continue decode or break loop */
		if(ret < 0) {
			/* we should destroy mPacket before continue loop */
			packetDestroy(mPacket);
			continue;
		}
		/* destroy packet when not use it again */
		packetDestroy(mPacket);
        inputbuf = NULL;
        mPacket = NULL;

		/* create new packet */
		mPacket_new = packetCreate(sizeof(struct MediaPacket));
        if(!mPacket_new){
            TRerr("[%s] create packet error\n", __func__);
            continue;
        }

		/* fill new packet buf */
		outputbuf = (struct MediaPacket *)mPacket_new->buf;
        outputbuf->Vir_Y_addr = (unsigned char *)dec->Vir_Y_Addr;
        outputbuf->Vir_C_addr = (unsigned char *)dec->Vir_U_Addr;
		outputbuf->Phy_Y_addr = (unsigned char *)dec->Phy_Y_Addr;
		outputbuf->Phy_C_addr = (unsigned char *)dec->Phy_C_Addr;
		outputbuf->buf_index = dec->bufId;
        outputbuf->width = dec->mWidth;
        outputbuf->height = dec->mHeight;
		outputbuf->nPts = dec->nPts;
		outputbuf->data_len = dec->mYuvSize;
        outputbuf->bytes_used = dec->mYuvSize;
        outputbuf->format = TR_PIXEL_YVU420SP;

		if(hdlTina->vport.cameraType == usb_camera
            && (hdlTina->vport.format == TR_PIXEL_MJPEG || hdlTina->vport.format == TR_PIXEL_H264)){
			/* set chain func to do something such as add wm */
			if (hdlTina->use_wm)
				hdlTina->vport.addWM(&hdlTina->vport,
									dec->mWidth,
									dec->mHeight,
									(void *)dec->Vir_Y_Addr,
									(void *)dec->Vir_U_Addr,
									hdlTina->wm_width,
									hdlTina->wm_height,
									NULL);
			/**/
			if (hdlTina->mCaptureFlag) {
				/*set hdlTina->param*/
				ret = captureJpegPhoto(hdlTina,hdlTina->mCaptureConfig.capturePath,
							outputbuf);
                if(ret == 0)
                    TRlog(TR_LOG_DECODER, " video %d capture photo ok\n", hdlTina->vport.mCameraIndex);
                else
                    TRlog(TR_LOG_DECODER, " video %d capture photo error\n", hdlTina->vport.mCameraIndex);

				hdlTina->mCaptureFlag = 0;
			}
		}

        mPacket_new->OnlyMemFlag = 0;
		/* set packet notify callback */
        mPacket_new->mode.notify.notifySrcHdl = &hdlTina->decoderPort;
        mPacket_new->mode.notify.notifySrcFunc = module_postReturnSem;

		/* push new packet to next queue */
		mPacket_new->packet_type = VIDEO_YUV;
		ret = module_push(&hdlTina->decoderPort, mPacket_new);
        for(i = 0; i < ret; i++)
            module_waitReturnSem(&hdlTina->decoderPort);

        DecoderaEnqueue(&hdlTina->decoderport, dec->videoPicture);
        free(mPacket_new->buf);
        free(mPacket_new);
        mPacket_new = NULL;
    }

    TRlog(TR_LOG_DECODER, "Decoder video src %d thread exit normal\n", hdlTina->vport.mCameraIndex);
    return NULL;
}

static void *TRDispThread(void *param)
{
	videoParam paramDisp;
	struct modulePacket *mPacket = NULL;
	struct MediaPacket *inputbuf = NULL;
	renderBuf rBuf;
    int ret = 0;

	/* check handle */
	TinaRecorder *hdlTina = (TinaRecorder *)param;
	if(!hdlTina)
		return NULL;

	TRlog(TR_LOG_DISPLAY, " display video src %d thread start : pid %lu\n",
                                                hdlTina->vport.mCameraIndex, syscall(SYS_gettid));

    /* wait for data to open again */
    module_waitReceiveSem(&hdlTina->screenPort);
    hdlTina->dispport.setEnable(&hdlTina->dispport, 1);
    hdlTina->dispport.setRect(&hdlTina->dispport, &hdlTina->dispport.rect);
    hdlTina->dispport.SetZorder(&hdlTina->dispport, hdlTina->zorder);
    /* data is not used,post receive sem */
    module_postReceiveSem(&hdlTina->screenPort);

	while(hdlTina->dispport.enable){
		/* wait data in queue and pop it */
        module_waitReceiveSem(&hdlTina->screenPort);

        /* pop packet from queue */
		mPacket = (struct modulePacket *)module_pop(&hdlTina->screenPort);
        if(!mPacket || !mPacket->buf)
            continue;

		inputbuf = (struct MediaPacket *)mPacket->buf;

		if(inputbuf->width <= 0 || inputbuf->height <= 0
                                            || !inputbuf->Vir_Y_addr){
            TRerr("[%s] disp %d pop data error,width %u height %u phy_addr %p vir_addr %p\n",
                __func__, hdlTina->vport.mCameraIndex,
                inputbuf->width, inputbuf->height,
                inputbuf->Phy_Y_addr, inputbuf->Vir_Y_addr);
            packetDestroy(mPacket);
            mPacket = NULL;
            TRerr(" disp %d destroy error packet ok\n", hdlTina->vport.mCameraIndex);
            continue;
        }

#if SAVE_DISP_SRC
        {
            static int index = 0;

            index++;
            if((index < 5) && (index%1) == 0){
                char bmp_path[64];
                ConverFunc ToRGB24;

                memset(bmp_path, 0, sizeof(bmp_path));
                sprintf(bmp_path, "/tmp/disp%dTest%d.bmp",
                                                    hdlTina->vport.mCameraIndex, index);
                if(inputbuf->format == TR_PIXEL_YUV420SP)
                    ToRGB24 = NV12ToRGB24;
                else if(inputbuf->format == TR_PIXEL_YVU420SP)
                    ToRGB24 = NV21ToRGB24;
                else if(inputbuf->format == TR_PIXEL_YUYV422)
                    ToRGB24 = YUYVToRGB24;
                else
                    ToRGB24 = NULL;

                YUVToBMP(bmp_path, (char *)inputbuf->Vir_Y_addr, ToRGB24,
                                                inputbuf->width, inputbuf->height);
            }
        }
#endif

		/* use pop data display */
		paramDisp.isPhy = 0;
		paramDisp.srcInfo.w = inputbuf->width;
		paramDisp.srcInfo.h = inputbuf->height;
		paramDisp.srcInfo.crop_x = 0;
		paramDisp.srcInfo.crop_y = 0;
		paramDisp.srcInfo.crop_w = inputbuf->width;
		paramDisp.srcInfo.crop_h = inputbuf->height;
        paramDisp.srcInfo.format = GetDisplayFormat(inputbuf->format);

		hdlTina->dispport.setRotateAngel(&hdlTina->dispport,
                                                    ROTATION_ANGLE_0);

        /* give priority to physical address display */
        if(inputbuf->Phy_Y_addr && inputbuf->Phy_C_addr && inputbuf->Vir_Y_addr){
            /* physical address display */
            rBuf.isExtPhy = 0;
            rBuf.phy_addr = (unsigned long)inputbuf->Phy_Y_addr;
            rBuf.vir_addr = (unsigned long)inputbuf->Vir_Y_addr;
            rBuf.y_phaddr = (unsigned long)inputbuf->Phy_Y_addr;
            rBuf.u_phaddr = (unsigned long)inputbuf->Phy_C_addr;
            rBuf.v_phaddr = (unsigned long)inputbuf->Phy_C_addr;

            ret = hdlTina->dispport.queueToDisplay(&hdlTina->dispport,
                                            inputbuf->bytes_used,
                                            &paramDisp, &rBuf);
        }else if(inputbuf->Vir_Y_addr && (!inputbuf->Phy_Y_addr || !inputbuf->Phy_C_addr)){
            /* virtual address display */
		    ret = hdlTina->dispport.writeData(&hdlTina->dispport,
									inputbuf->Vir_Y_addr,
									inputbuf->bytes_used,
									&paramDisp);
        }

        if(ret < 0)
            TRerr("[%s] disp %d write error ret %d:width %u height %u length %d bytes_used %d\n",
                                                __func__, hdlTina->vport.mCameraIndex, ret,
                                                inputbuf->width, inputbuf->height,
                                                inputbuf->data_len, inputbuf->bytes_used);

		/* destroy packet when not use packet again */
		packetDestroy(mPacket);
        mPacket = NULL;
	}

    TRlog(TR_LOG_DISPLAY, "Display video src %d thread exit normal\n", hdlTina->vport.mCameraIndex);
	return NULL;
}

void* TREncoderThread(void *param)
{
	TinaRecorder *hdlTina = (TinaRecorder *)param;
	if(!hdlTina)
		return NULL;
    DemoRecoderContext* p = (DemoRecoderContext*)hdlTina->EncoderContext;
    AudioInputBuffer *audioInputBuffer;
    VideoInputBuffer videoInputBuffer;
	struct modulePacket *mPacket = NULL;
	struct MediaPacket *inputbuf = NULL;
	int ret = 0;
	int err_num = 0;
    int index = 0;

	TRlog(TR_LOG_ENCODER, " encoder video src %d thread start : pid %lu\n",
                                                hdlTina->vport.mCameraIndex, syscall(SYS_gettid));
	while(p->encoderEnable) {
        module_waitReceiveSem(&hdlTina->encoderPort);

        /* pop packet frome queue */
		mPacket = (struct modulePacket *)module_pop(&hdlTina->encoderPort);
        if(!mPacket || !mPacket->buf)
            continue;

        /* packet may be two types */
		if (mPacket->packet_type & AUDIO_PCM){
            audioInputBuffer = (AudioInputBuffer *)mPacket->buf;

WritePcmData_Again:
            /* process packet */
            ret = AwEncoderWritePCMdata(p->mAwEncoder, audioInputBuffer);
            if(ret < 0){
                if(++err_num < MAX_ERR_NUM) {
					TRerr("[%s] encoder %d write PCM data error [%d] times, \
							Vir_PCM_addr %p length %d pts %lld\n",
						__func__, hdlTina->vport.mCameraIndex,
						err_num,
						audioInputBuffer->pData,
						audioInputBuffer->nLen,
						audioInputBuffer->nPts);
					usleep(10*1000);
					goto WritePcmData_Again;
				}
				/* clean err_num */
				err_num = 0;
				/* destroy packet when encode pcm data error */
				packetDestroy(mPacket);
                mPacket = NULL;
				continue;
            }
            /* destroy packet when not use packet again */
			packetDestroy(mPacket);
            mPacket = NULL;
		}else if(mPacket->packet_type & VIDEO_YUV) {
            inputbuf = (struct MediaPacket *)mPacket->buf;

            if(hdlTina->encodeUsePhy && (!inputbuf->Vir_Y_addr || !inputbuf->Phy_Y_addr
                                    || inputbuf->bytes_used <= 0 || inputbuf->buf_index < 0)){
                TRerr("[%s] encoder %d pop data error,vir_addr %p phy_addr %p len %d buf index %u\n",
                        __func__, hdlTina->vport.mCameraIndex,
                        inputbuf->Vir_Y_addr, inputbuf->Phy_Y_addr,
                        inputbuf->bytes_used, inputbuf->buf_index);
                packetDestroy(mPacket);
                TRerr(" encoder %d destroy error packet ok\n", hdlTina->vport.mCameraIndex);
                continue;
            }

            memset(&videoInputBuffer, 0, sizeof(videoInputBuffer));
            videoInputBuffer.nID = inputbuf->buf_index;
            videoInputBuffer.nPts = inputbuf->nPts;
            videoInputBuffer.pData = (unsigned char *)inputbuf->Vir_Y_addr;
            videoInputBuffer.pAddrPhyY = (unsigned char *)inputbuf->Phy_Y_addr;
            videoInputBuffer.pAddrPhyC = (unsigned char *)inputbuf->Phy_C_addr;
            videoInputBuffer.nLen = inputbuf->bytes_used;

            /* physical address encoding needs to save packet */
            if(hdlTina->encodeUsePhy){
EncoderSavePacket:
                pthread_mutex_lock(&hdlTina->mEncoderPacketLock);
                for(index = 0; index < ENCODER_SAVE_NUM; index++){
                    if(hdlTina->encoderpacket[index] == NULL)
                        break;
                }

                if(index < ENCODER_SAVE_NUM){
                    hdlTina->encoderpacket[index] = mPacket;
                    pthread_mutex_unlock(&hdlTina->mEncoderPacketLock);
                }else if(index >= ENCODER_SAVE_NUM){
                    pthread_mutex_unlock(&hdlTina->mEncoderPacketLock);

                    TRerr("[%s] encoder %d no encoder packet,sleep wait ...\n",
                                                        __func__, hdlTina->vport.mCameraIndex);
                    usleep(5*1000);
                    goto EncoderSavePacket;
                }
            }

WriteYuvData_Again:
            /* process packet */
			ret = AwEncoderWriteYUVdata(p->mAwEncoder, &videoInputBuffer);
			if(ret < 0) {
				if(++err_num < MAX_ERR_NUM) {
					TRerr("[%s] encoder %d write YUV data error [%d] times, \
							Vir_Y_addr %p Phy_Y_addr %p length %d\n",
							__func__, hdlTina->vport.mCameraIndex,
							err_num,
							videoInputBuffer.pData,
							videoInputBuffer.pAddrPhyY,
							videoInputBuffer.nLen);
					usleep(10*1000);
					goto WriteYuvData_Again;
				}
				/* clean err_num */
				err_num = 0;
				/* destroy packet when encode yuv data error */
				packetDestroy(mPacket);
                mPacket = NULL;
                pthread_mutex_lock(&hdlTina->mEncoderPacketLock);
                if(index < ENCODER_SAVE_NUM)
                    hdlTina->encoderpacket[index] = NULL;
                pthread_mutex_unlock(&hdlTina->mEncoderPacketLock);
				continue;
			}

            /* virtual address encoding release packet */
            if(hdlTina->encodeUsePhy == 0)
                packetDestroy(mPacket);

            mPacket = NULL;
        }else{
            TRerr("[%s] encoder %d pop packet type is not right,mPacket %p type =0x%x\n",
                                            __func__, hdlTina->vport.mCameraIndex,
                                            mPacket, mPacket->packet_type);

			/* destroy packet when not use packet again */
			packetDestroy(mPacket);
            mPacket = NULL;
        }
    }

    TRlog(TR_LOG_ENCODER, "Encoder thread exit normal\n");
    return NULL;
}

static void *TRMuxerThread(void *param)
{
	TinaRecorder *hdlTina = (TinaRecorder *)param;
	if(hdlTina == NULL)
		return NULL;
	DemoRecoderContext *p = (DemoRecoderContext*)hdlTina->EncoderContext;
	RecoderWriterT *rw;
	int ret;
	int fs_cache_size;
	CdxMuxerMediaInfoT mediainfo;
	struct modulePacket *mPacket = NULL;
	CdxMuxerPacketT *databuf = NULL;
    struct modulePacket *lastPacket = NULL;
    int writePacketFailFlag = 0;
    long long baseAudioPts = -1;
    long long baseVideoPts = -1;
    long long audioDurationCount = 0;
    long long videoDurationCount = 0;
    unsigned int keyFramExitFlag = 0;
    int aheadOfVideoTime = 200;//200ms
    int oneOriginFrameSize = 0;
    int timeOfOneFrame = 0;
    int aheadOfAudioTime = 0;
    int reopenFlag = 0;
    int file_count = 0;
    long long each_file_duration = hdlTina->maxRecordTime;//ms

    TRlog(TR_LOG_MUXER, " muxer %d thread start : pid %lu\n",
                                    hdlTina->vport.mCameraIndex, syscall(SYS_gettid));

    if(hdlTina->aport.enable){
        if(p->audioConfig.nType == AUDIO_ENCODE_MP3_TYPE){
            if(hdlTina->aport.getAudioSampleRate(&hdlTina->aport) >= 0 && hdlTina->aport.getAudioSampleRate(&hdlTina->aport) <= 24000)
                oneOriginFrameSize = 576*2*2;
            else
                oneOriginFrameSize = 1152*2*2;
        }else{
            oneOriginFrameSize = 1024 * hdlTina->aport.getAudioChannels(&hdlTina->aport) * 2;
        }
        timeOfOneFrame = oneOriginFrameSize*1000/(hdlTina->aport.getAudioSampleRate(&hdlTina->aport)*hdlTina->aport.getAudioChannels(&hdlTina->aport)*2);
        aheadOfAudioTime = timeOfOneFrame;//128ms
    }

    if(each_file_duration == 0)
        each_file_duration = 60*1000;//if not set the record time,set it to 1 min

    TRlog(TR_LOG_MUXER, " each file duration %lld ms\n", each_file_duration);

    muxerThreadNum++;
    if(muxerThreadNum == 1){
        /* setting dirty_background_bytes */
        TinaReadSetDirty();
    }

REOPEN:
    //first:we should callback to app,which can let the app set the output path
    if(hdlTina->callback){
        if(p->muxType != CDX_MUXER_AAC)
            hdlTina->callback(hdlTina->pUserData,TINA_RECORD_ONE_FILE_COMPLETE,NULL);
        else
            hdlTina->callback(hdlTina->pUserData,TINA_RECORD_ONE_AAC_FILE_COMPLETE,NULL);
    }

	/* create writer with OutputPath and create muxer*/
	if (hdlTina->mOutputPath) {
		if ((p->pStream = CdxWriterCreat()) == NULL) {
			TRerr("[%s] CdxWriterCreat Failed:%s\n", __func__, hdlTina->mOutputPath);
            goto CreatWriterErr;
		}
		rw = (RecoderWriterT*)p->pStream;
		rw->file_mode = FP_FILE_MODE;
		strncpy(rw->file_path, hdlTina->mOutputPath, FILE_PATH_MAX_LEN);
        TRlog(TR_LOG_MUXER, "rw->file_path: %s\n", rw->file_path);
		ret = RWOpen(p->pStream);
		if (ret < 0) {
			TRerr("[%s] RWOpen Failed:%s\n", __func__, rw->file_path);
		    goto RWOpenErr;
		}

		p->pMuxer = CdxMuxerCreate(p->muxType, p->pStream);
		if (!p->pMuxer) {
			TRerr("[%s] CdxMuxerCreate failed\n", __func__);
			goto MuxerCreateErr;
		}
	}

	/* set muxer mediaInfo when use one or both of video and audio */
	memset(&mediainfo, 0, sizeof(CdxMuxerMediaInfoT));
	// set audio port mediainfo
	if (hdlTina->aport.enable) {
		mediainfo.audioNum = 1;
        mediainfo.audio.eCodecFormat = ReturnMediaAudioType(p->audioConfig.nType);
		mediainfo.audio.nAvgBitrate = p->audioConfig.nBitrate;
		mediainfo.audio.nBitsPerSample = p->audioConfig.nSamplerBits;
		mediainfo.audio.nChannelNum = p->audioConfig.nOutChan;
		mediainfo.audio.nMaxBitRate = p->audioConfig.nBitrate;
		mediainfo.audio.nSampleRate = p->audioConfig.nOutSamplerate;
		mediainfo.audio.nSampleCntPerFrame = 1024;
	}
	// set video port mediainfo
	if (hdlTina->vport.enable
                    && (p->muxType == CDX_MUXER_MOV || p->muxType == CDX_MUXER_TS)) {
		mediainfo.videoNum = 1;
        mediainfo.video.eCodeType = ReturnMediaVideoType(p->videoConfig.nType);
		mediainfo.video.nWidth    =  p->videoConfig.nOutWidth;
		mediainfo.video.nHeight   =  p->videoConfig.nOutHeight;
		mediainfo.video.nFrameRate = p->videoConfig.nFrameRate;
	}

    if(p->pMuxer) {
        TRdebugMuxerMediaInfo(&mediainfo);
		CdxMuxerSetMediaInfo(p->pMuxer, &mediainfo);

#if FS_WRITER
		// control muxer write mode
		fs_cache_size = 8 * 1024;
		CdxMuxerControl(p->pMuxer, SET_FS_SIMPLE_CACHE_SIZE, &fs_cache_size);
		int fs_mode = FSWRITEMODE_SIMPLECACHE;
		CdxMuxerControl(p->pMuxer, SET_FS_WRITE_MODE, &fs_mode);
#endif
	}

	/* muxer write extradata */
	if (p->extractDataLength > 0 && p->pMuxer)
        CdxMuxerWriteExtraData(p->pMuxer, p->extractDataBuff, p->extractDataLength, 0);

	/* muxer write header */
	if (p->pMuxer)
		CdxMuxerWriteHeader(p->pMuxer);

    /* if it is reopened buf, buf will be written */
    if(reopenFlag == 1 && lastPacket && hdlTina->outport.enable){
        databuf = (CdxMuxerPacketT *)lastPacket->buf;

        if(databuf->type == CDX_MEDIA_AUDIO)
            audioDurationCount = databuf->pts - baseAudioPts;
        else if(databuf->type == CDX_MEDIA_VIDEO)
            videoDurationCount = databuf->pts - baseVideoPts;

        // write packet
        if (CdxMuxerWritePacket(p->pMuxer, databuf) < 0) {
           TRerr("[%s] muxer write pakcet failed\n", __func__);
           modulePort_SetEnable(&hdlTina->muxerPort, 0);
           packetDestroy(mPacket);
           writePacketFailFlag = 1;
           hdlTina->outport.enable = 0;
           reopenFlag = 0;
           TRerr("[%s] muxer thread writing error leads to exit\n", __func__);
        }

        checkStorePktInfo(hdlTina, databuf);

        packetDestroy(lastPacket);
    }

    /* reset flag */
    lastPacket = NULL;
    reopenFlag = 0;

    /* record file count */
    file_count++;
    keyFramExitFlag = 0;

	/* muxer write packet */
	while (hdlTina->outport.enable && reopenFlag == 0) {
        module_waitReceiveSem(&hdlTina->muxerPort);

		mPacket = (struct modulePacket *)module_pop(&hdlTina->muxerPort);
        if(!mPacket || !mPacket->buf)
            continue;

        databuf = (CdxMuxerPacketT *)mPacket->buf;

        // save pts info
        if(databuf->type == CDX_MEDIA_AUDIO){
            if(baseAudioPts == -1)
                baseAudioPts = databuf->pts;
            audioDurationCount = databuf->pts - baseAudioPts;
        }else if(databuf->type == CDX_MEDIA_VIDEO){
            if(baseVideoPts == -1)
                baseVideoPts = databuf->pts;
            videoDurationCount = databuf->pts - baseVideoPts;

            if((keyFramExitFlag == 1) && (databuf->keyFrameFlag == 1)){
                lastPacket = mPacket;
                reopenFlag = 1;
                break;
            }
        }

		// write packet
		if (CdxMuxerWritePacket(p->pMuxer, databuf) < 0) {
           TRerr("[%s] muxer write pakcet failed\n", __func__);
           modulePort_SetEnable(&hdlTina->muxerPort, 0);
           packetDestroy(mPacket);
           writePacketFailFlag = 1;
           hdlTina->outport.enable = 0;
           reopenFlag = 0;
           TRerr("[%s] muxer thread writing error leads to exit\n", __func__);
           break;
		}

        checkStorePktInfo(hdlTina, databuf);

        packetDestroy(mPacket);

        if(hdlTina->vport.enable && hdlTina->aport.enable
                && (p->muxType == CDX_MUXER_MOV || p->muxType == CDX_MUXER_TS)){
            if((audioDurationCount >= (file_count*each_file_duration-aheadOfAudioTime))
                    && (videoDurationCount >= (file_count*each_file_duration-aheadOfVideoTime))){
                keyFramExitFlag = 1;
                //TRlog(TR_LOG_MUXER, " waiting key frame\n");
            }
        }else if(hdlTina->vport.enable && (hdlTina->aport.enable == 0)){
            if(videoDurationCount >= (file_count*each_file_duration-aheadOfVideoTime)){
                keyFramExitFlag = 1;
                //TRlog(TR_LOG_MUXER, " waiting key frame\n");
            }
        }else if((hdlTina->vport.enable == 0) && hdlTina->aport.enable){
            if(audioDurationCount >= (file_count*each_file_duration-aheadOfAudioTime)){
                /* audio do not need key frame */
                reopenFlag = 1;
                break;
            }
        }
	}

	/* muxer write trailer */
	if (p->pMuxer && writePacketFailFlag == 0)
		CdxMuxerWriteTrailer(p->pMuxer);

	/* close muxer and destroy writer */
	if (p->pMuxer) {
		CdxMuxerClose(p->pMuxer);
		p->pMuxer = NULL;
	}
	if (p->pStream) {
		RWClose(p->pStream);
		CdxWriterDestroy(p->pStream);
		p->pStream = NULL;
		rw = NULL;
	}

	/* remux when loop record */
	if(reopenFlag == 1 && hdlTina->outport.enable){
        TRlog(TR_LOG_MUXER, " %s record is completed, the next record...\n",
                                                            hdlTina->mOutputPath);
        memset(hdlTina->mOutputPath, 0x00, MAX_PATH_LEN);
		goto REOPEN;
    }

    /* release queue resources */
    modulePort_SetEnable(&hdlTina->muxerPort, 0);
    while(!module_InputQueueEmpty(&hdlTina->muxerPort)) {
		mPacket = (struct modulePacket *)module_pop(&hdlTina->muxerPort);
        if(!mPacket)
            continue;

        packetDestroy(mPacket);
    }

    if(hdlTina->mNeedVideoPktBufSpace > 0)
        sem_post(&hdlTina->mVideoPktBufSem);
    if(hdlTina->mNeedAudioPktBufSpace > 0)
        sem_post(&hdlTina->mAudioPktBufSem);

    /* reset dirty_background */
    muxerThreadNum--;
    if(muxerThreadNum == 0){
        TinaResetDirty();
    }

	TRlog(TR_LOG_MUXER, "Muxer thread exit normal\n");
	return NULL;


MuxerCreateErr:
    if (p->pStream)
        RWClose(p->pStream);

RWOpenErr:
    CdxWriterDestroy(p->pStream);

CreatWriterErr:
    modulePort_SetEnable(&hdlTina->muxerPort, 0);
    hdlTina->outport.enable = 0;

    /* release queue resources */
    while(!module_InputQueueEmpty(&hdlTina->muxerPort)) {
		mPacket = (struct modulePacket *)module_pop(&hdlTina->muxerPort);
        if(!mPacket)
            continue;

        packetDestroy(mPacket);
    }

    if(hdlTina->mNeedVideoPktBufSpace > 0)
        sem_post(&hdlTina->mVideoPktBufSem);
    if(hdlTina->mNeedAudioPktBufSpace > 0)
        sem_post(&hdlTina->mAudioPktBufSem);

    /* reset dirty_background */
    muxerThreadNum--;
    if(muxerThreadNum == 0){
        TinaResetDirty();
    }

    TRerr("[%s] muxer %d thread writing error leads to exit\n",
                                        __func__, hdlTina->vport.mCameraIndex);
	return NULL;
}

void* AudioInputThread(void *param)
{
        int i;
	TinaRecorder *hdlTina = (TinaRecorder *)param;
	if(!hdlTina)
		return NULL;
	DemoRecoderContext* demoRecoder = (DemoRecoderContext*)hdlTina->EncoderContext;

	int readOnceSize;
    if(demoRecoder->audioConfig.nType == AUDIO_ENCODE_MP3_TYPE){
        if(hdlTina->aport.getAudioSampleRate(&hdlTina->aport) >= 0 && hdlTina->aport.getAudioSampleRate(&hdlTina->aport) <= 24000)
            readOnceSize = 576*2*2;
        else
            readOnceSize = 1152*2*2;
    }else{
        readOnceSize = 1024 * hdlTina->aport.getAudioChannels(&hdlTina->aport) * 2;
    }
	int readActual = 0;
	AudioInputBuffer audioInputBuffer;
    struct modulePacket *mPacket = NULL;
    int ret;

#if SAVE_AUDIO_SRC
    FILE *audioFp = NULL;
    char audio_path[64];

    memset(audio_path, 0, sizeof(audio_path));
    sprintf(audio_path, "/tmp/TRaudio%d_test.pcm", hdlTina->vport.mCameraIndex);

    /* create audio test file */
    audioFp = fopen(audio_path, "wb+");
#endif

	memset(&audioInputBuffer, 0x00, sizeof(AudioInputBuffer));
	audioInputBuffer.nPts = -1;
	audioInputBuffer.nLen = readOnceSize;
	audioInputBuffer.pData = (char *)malloc(audioInputBuffer.nLen);

    TRlog(TR_LOG_AUDIO, " audio %d thread start : pid %lu\n",
                                        hdlTina->vport.mCameraIndex, syscall(SYS_gettid));

	while (hdlTina->aport.enable) {
		readActual = hdlTina->aport.readData(&hdlTina->aport,audioInputBuffer.pData,readOnceSize);
		audioInputBuffer.nLen = readActual;
		if (readActual != 0) {
			if (readActual != readOnceSize)
				TRwarn("[%s] Index%d:Actual=%d, OnceSize=%d\n", __func__, hdlTina->mCameraIndex, readActual, readOnceSize);

			if (audioInputBuffer.nPts == -1)
				audioInputBuffer.nPts = 0;
			else
				audioInputBuffer.nPts = hdlTina->aport.getpts(&hdlTina->aport);

            /* if the app set audio Mute,we should set the data 0 */
			if(hdlTina->mAudioMuteFlag == 1) {
				memset(audioInputBuffer.pData, 0, readActual);
			}

#if SAVE_AUDIO_SRC
            fwrite(audioInputBuffer.pData, audioInputBuffer.nLen, 1, audioFp);
#endif

            if(hdlTina->outport.enable){
				/* set packet */
				mPacket = packetCreate(0);
                if(!mPacket){
                    TRerr("[%s] create packet error\n", __func__);
                    continue;
                }

				mPacket->buf = &audioInputBuffer;
				mPacket->packet_type = AUDIO_PCM;
                mPacket->OnlyMemFlag = 0;
                mPacket->mode.notify.notifySrcHdl = &hdlTina->audioPort;
                mPacket->mode.notify.notifySrcFunc = module_postReturnSem;
                ret = module_push(&hdlTina->audioPort, mPacket);
                for(i = 0; i < ret; i++)
                    module_waitReturnSem(&hdlTina->audioPort);

                free(mPacket);
                mPacket = NULL;
            }
		}
	}

#if SAVE_AUDIO_SRC
    fclose(audioFp);
#endif

	if (audioInputBuffer.pData)
		free(audioInputBuffer.pData);

	hdlTina->aport.clear(&hdlTina->aport);

    TRlog(TR_LOG_AUDIO, "Audio thread exit normal\n");
	return 0;
}

static void *VideoInputThread(void *param)
{
        int i;
	TinaRecorder *hdlTina = (TinaRecorder *)param;
	if(!hdlTina)
		return NULL;

	int ret = -1;
    int err_num = 0;
    struct modulePacket *mPacket = NULL;
    struct MediaPacket *outputbuf = NULL;

    /* start capture */
    ret = hdlTina->vport.startcapture(&hdlTina->vport);
    if(ret < 0){
        TRerr("[%s] camera srart capture error\n", __func__);
        return NULL;
    }

	TRlog(TR_LOG_VIDEO, " camera %d thread start : pid %lu\n",
                                    hdlTina->vport.mCameraIndex, syscall(SYS_gettid));
	while (hdlTina->vport.enable) {
		/* dequeue from buffer queue */
		if (hdlTina->vport.dequeue(&hdlTina->vport,&hdlTina->param) != 0) {
			TRerr("[%s] Dequeue error!\n", __func__);
            err_num++;
            if(err_num > DEQUEUE_MAX_ERR_NUM){
                TRerr("[%s] dequeue failed to reach the maximum number of DEQUEUE_MAX_ERR_NUM\n",
                                                                                        __func__);
                break;
            }
			continue;
		}
        /* reset error num */
        err_num = 0;

		/* create packet */
		mPacket = packetCreate(sizeof(struct MediaPacket));
        if(!mPacket){
            TRerr("[%s] create packet error\n", __func__);
            hdlTina->vport.enqueue(&hdlTina->vport, hdlTina->param.bufferId);
            continue;
        }
		/* fill packet buf */
		outputbuf = (struct MediaPacket *)mPacket->buf;

        outputbuf->Vir_Y_addr = (unsigned char *)hdlTina->param.MainYVirAddr;
        outputbuf->Vir_C_addr = (unsigned char *)hdlTina->param.MainCVirAddr;
		outputbuf->Phy_Y_addr = (unsigned char *)hdlTina->param.MainYPhyAddr;
		outputbuf->Phy_C_addr = (unsigned char *)hdlTina->param.MainCPhyAddr;
		outputbuf->buf_index = hdlTina->param.bufferId;
        outputbuf->width = hdlTina->vport.MainWidth;
        outputbuf->height = hdlTina->vport.MainHeight;
		outputbuf->nPts = hdlTina->param.pts;
		outputbuf->data_len = hdlTina->param.MainSize;
        outputbuf->bytes_used = hdlTina->param.BytesUsed;
        outputbuf->format = hdlTina->vport.format;

        /* uvc camera output has no consecutive physical address */
        if(hdlTina->vport.cameraType == usb_camera){
            outputbuf->Phy_Y_addr = NULL;
            outputbuf->Phy_C_addr = NULL;
        }

		if(hdlTina->vport.format != TR_PIXEL_MJPEG && hdlTina->vport.format != TR_PIXEL_H264){
			/* set chain func to do something such as add wm */
			if (hdlTina->use_wm)
				hdlTina->vport.addWM(&hdlTina->vport,
									hdlTina->vport.MainWidth,
									hdlTina->vport.MainHeight,
									hdlTina->param.MainYVirAddr,
									hdlTina->param.MainCVirAddr,
									hdlTina->wm_width,
									hdlTina->wm_height,
									NULL);
			/**/
			if (hdlTina->mCaptureFlag) {
				ret = captureJpegPhoto(hdlTina,hdlTina->mCaptureConfig.capturePath,
							outputbuf);
                if(ret == 0)
                    TRlog(TR_LOG_VIDEO, " video %d capture photo ok\n", hdlTina->vport.mCameraIndex);
                else
                    TRlog(TR_LOG_VIDEO, " video %d capture photo error\n", hdlTina->vport.mCameraIndex);
				hdlTina->mCaptureFlag = 0;
			}
		}

        mPacket->OnlyMemFlag = 0;
		/* set packet notify callback */
        mPacket->mode.notify.notifySrcHdl = &hdlTina->cameraPort;
        mPacket->mode.notify.notifySrcFunc = module_postReturnSem;

        mPacket->packet_type = VIDEO_YUV;

        /* uvc format decode */
        if((hdlTina->vport.format == TR_PIXEL_MJPEG)
                || (hdlTina->vport.format == TR_PIXEL_H264))
            mPacket->packet_type = VIDEO_ENC;

		/* push packet into queue */
		ret = module_push(&hdlTina->cameraPort, mPacket);
        for(i = 0; i < ret; i++)
            module_waitReturnSem(&hdlTina->cameraPort);

        ret = hdlTina->vport.enqueue(&hdlTina->vport, hdlTina->param.bufferId);
        if(ret < 0)
            TRerr("[%s] video enqueue error,ret %d buf index %u\n",
                                        __func__, ret, hdlTina->param.bufferId);
        free(mPacket->buf);
        free(mPacket);
        mPacket = NULL;
    }

    TRlog(TR_LOG_VIDEO, "Video thread exit normal\n");
	return NULL;
}

static void TinaModuleDefInit(void *hdl)
{
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;

	if(!hdlTina || hdlTina->modulePortInitFlag)
		return;

    memset(&hdlTina->cameraPort, 0x00, sizeof(struct moduleAttach));
    memset(&hdlTina->audioPort, 0x00, sizeof(struct moduleAttach));
    memset(&hdlTina->screenPort, 0x00, sizeof(struct moduleAttach));
    memset(&hdlTina->decoderPort, 0x00, sizeof(struct moduleAttach));
    memset(&hdlTina->encoderPort, 0x00, sizeof(struct moduleAttach));
    memset(&hdlTina->muxerPort, 0x00, sizeof(struct moduleAttach));

    ModuleSetNotifyCallback(&hdlTina->cameraPort, NotifyCallbackToSink, &hdlTina->cameraPort);
    ModuleSetNotifyCallback(&hdlTina->audioPort, NotifyCallbackToSink, &hdlTina->audioPort);
    ModuleSetNotifyCallback(&hdlTina->screenPort, NotifyCallbackToSink, &hdlTina->screenPort);
    ModuleSetNotifyCallback(&hdlTina->decoderPort, NotifyCallbackToSink, &hdlTina->decoderPort);
    ModuleSetNotifyCallback(&hdlTina->encoderPort, NotifyCallbackToSink, &hdlTina->encoderPort);
    ModuleSetNotifyCallback(&hdlTina->muxerPort, NotifyCallbackToSink, &hdlTina->muxerPort);

    hdlTina->audioPort.name = AUDIO;
    hdlTina->cameraPort.name = CAMERA;
	hdlTina->decoderPort.name = DECODER;
    hdlTina->screenPort.name = SCREEN;
    hdlTina->encoderPort.name = ENCODER;
	hdlTina->muxerPort.name = MUXER;

    hdlTina->audioPort.outputTyte = AUDIO_PCM;

    if(hdlTina->vport.cameraType == usb_camera)
        hdlTina->cameraPort.outputTyte = VIDEO_YUV | VIDEO_ENC;
    else if(hdlTina->vport.cameraType == csi_camera)
        hdlTina->cameraPort.outputTyte = VIDEO_YUV;
    else if(hdlTina->vport.cameraType == raw_camera)
        hdlTina->cameraPort.outputTyte = VIDEO_RAW;

    hdlTina->decoderPort.inputTyte = VIDEO_ENC | AUDIO_ENC;
    hdlTina->decoderPort.outputTyte = VIDEO_YUV | AUDIO_PCM;

    hdlTina->screenPort.inputTyte = VIDEO_YUV | SCALE_YUV;

    hdlTina->encoderPort.inputTyte = AUDIO_PCM | VIDEO_YUV | SCALE_YUV;
    hdlTina->encoderPort.outputTyte = AUDIO_ENC | VIDEO_ENC | SCALE_YUV;

    hdlTina->muxerPort.inputTyte = AUDIO_ENC | VIDEO_ENC;

    /* module port init ok */
    hdlTina->modulePortInitFlag = 1;
}

static int TinaModuleDefLink(void *hdl)
{
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;

	if(hdlTina == NULL)
		return -1;

    TinaModuleDefInit(hdlTina);

    /* initialization decoding when uvc camera is in MJPEG or H264 */
	if(hdlTina->vport.enable && hdlTina->vport.cameraType == usb_camera
            && (hdlTina->vport.format == TR_PIXEL_MJPEG || hdlTina->vport.format == TR_PIXEL_H264)){
		modules_link(&hdlTina->cameraPort,&hdlTina->decoderPort,NULL);
		hdlTina->videoPort = &hdlTina->decoderPort;
		TRlog(TR_LOG, " link camera decoder\n");
	}else if(hdlTina->vport.enable){
		hdlTina->videoPort = &hdlTina->cameraPort;
	}

	/* if need preview link camera and display modules*/
	if(hdlTina->vport.enable && hdlTina->dispport.enable){
		modules_link(hdlTina->videoPort, &hdlTina->screenPort, NULL);
		TRlog(TR_LOG, " link camera display\n");
	}

	/* if need output link encoder muxer modules */
	if(hdlTina->outport.enable){
		modules_link(&hdlTina->encoderPort, &hdlTina->muxerPort, NULL);
		TRlog(TR_LOG, " link encoder muxer\n");
	}

    /* if use vport link video and encoder */
	if(hdlTina->vport.enable && hdlTina->outport.enable) {
		modules_link(hdlTina->videoPort, &hdlTina->encoderPort, NULL);
		TRlog(TR_LOG, " link video encoder\n");
	}

    if(hdlTina->dispport.enable && hdlTina->scaleDownRatio != 0){
        modules_unlink(hdlTina->videoPort, &hdlTina->screenPort, NULL);

        modules_link(&hdlTina->encoderPort, &hdlTina->screenPort, NULL);
        TRlog(TR_LOG, " link encoder display\n");
    }

    /* if use aport link audio and encoder */
	if(hdlTina->aport.enable && hdlTina->outport.enable) {
		modules_link(&hdlTina->audioPort, &hdlTina->encoderPort, NULL);
		TRlog(TR_LOG, " link audio encoder\n");
	}

    return 0;
}

static int TinamoduleUnlink(void *hdl)
{
    TinaRecorder *hdlTina = (TinaRecorder *)hdl;

	if(!hdlTina)
		return -1;

    /* output */
    if(module_detectLink(&hdlTina->encoderPort, &hdlTina->muxerPort)){
	TRlog(TR_LOG, "output unlink\n");
        modules_unlink(&hdlTina->encoderPort, &hdlTina->muxerPort, NULL);
    }

    if(module_detectLink(hdlTina->videoPort, &hdlTina->encoderPort)){
	TRlog(TR_LOG, "video output unlink\n");
        modules_unlink(hdlTina->videoPort, &hdlTina->encoderPort, NULL);
    }

    if(module_detectLink(&hdlTina->audioPort, &hdlTina->encoderPort)){
	TRlog(TR_LOG, "audio output unlink\n");
        modules_unlink(&hdlTina->audioPort, &hdlTina->encoderPort, NULL);
    }

    /* display */
    if(module_detectLink(hdlTina->videoPort, &hdlTina->screenPort)){
	TRlog(TR_LOG, "display unlink\n");
        modules_unlink(hdlTina->videoPort, &hdlTina->screenPort, NULL);
    }

    /* enable camera and camera type is uvc camera */
    if(module_detectLink(&hdlTina->cameraPort, &hdlTina->decoderPort)){
	TRlog(TR_LOG, "uvc unlink\n");
        modules_unlink(&hdlTina->cameraPort, &hdlTina->decoderPort, NULL);
    }

    return 0;
}

static int TinaPrintModulelink(void *hdl)
{
    int index = 0;
    struct outputSrc *outputinfo;
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;

	if(!hdlTina)
		return -1;

    TRlog(TR_LOG, "trecorder module link info:\n");
    /*  camera  */
    index = 0;
    outputinfo = hdlTina->cameraPort.output;
    TRlog(TR_LOG, "camera enable %u name 0x%x, supported output type 0x%x, self queue %p, ouput info:\n",
                hdlTina->cameraPort.moduleEnable, hdlTina->cameraPort.name,
                hdlTina->cameraPort.outputTyte, hdlTina->cameraPort.sinkQueue);
    while(outputinfo){
        TRlog(TR_LOG, "  %d. enable %u supported input type 0x%x, queue addr %p\n",
            index, *(outputinfo->moduleEnable), *(outputinfo->sinkInputType), outputinfo->srcQueue);
        index++;
        outputinfo = outputinfo->next;
    }
    /*  decoder  */
    index = 0;
    outputinfo = hdlTina->decoderPort.output;
    TRlog(TR_LOG, "decoder enable %u name 0x%x, supported output type 0x%x, self queue %p ouput info:\n",
                hdlTina->decoderPort.moduleEnable, hdlTina->decoderPort.name,
                hdlTina->decoderPort.outputTyte, hdlTina->decoderPort.sinkQueue);
    while(outputinfo){
        TRlog(TR_LOG, "  %d. enable %u supported input type 0x%x, queue addr %p\n",
            index, *(outputinfo->moduleEnable), *(outputinfo->sinkInputType), outputinfo->srcQueue);
        index++;
        outputinfo = outputinfo->next;
    }
    /*  screen  */
    index = 0;
    outputinfo = hdlTina->screenPort.output;
    TRlog(TR_LOG, "screen enable %u name 0x%x, supported output type 0x%x, self queue %p ouput info:\n",
                hdlTina->screenPort.moduleEnable, hdlTina->screenPort.name,
                hdlTina->screenPort.outputTyte, hdlTina->screenPort.sinkQueue);
    while(outputinfo){
        TRlog(TR_LOG, "  %d. enable %u supported input type 0x%x, queue addr %p\n",
            index, *(outputinfo->moduleEnable), *(outputinfo->sinkInputType), outputinfo->srcQueue);
        index++;
        outputinfo = outputinfo->next;
    }
    /*  encoder  */
    index = 0;
    outputinfo = hdlTina->encoderPort.output;
    TRlog(TR_LOG, "encoder enable %u name 0x%x, supported output type 0x%x, self queue %p ouput info:\n",
                hdlTina->encoderPort.moduleEnable, hdlTina->encoderPort.name,
                hdlTina->encoderPort.outputTyte, hdlTina->encoderPort.sinkQueue);
    while(outputinfo){
        TRlog(TR_LOG, "  %d. enable %u supported input type 0x%x, queue addr %p\n",
            index, *(outputinfo->moduleEnable), *(outputinfo->sinkInputType), outputinfo->srcQueue);
        index++;
        outputinfo = outputinfo->next;
    }
    /*  muxer  */
    index = 0;
    outputinfo = hdlTina->muxerPort.output;
    TRlog(TR_LOG, "muxer enable %u name 0x%x, supported output type 0x%x, self queue %p ouput info:\n",
                hdlTina->muxerPort.moduleEnable, hdlTina->muxerPort.name,
                hdlTina->muxerPort.outputTyte, hdlTina->muxerPort.sinkQueue);
    while(outputinfo){
        TRlog(TR_LOG, "  %d. enable %u supported input type 0x%x, queue addr %p\n",
            index, *(outputinfo->moduleEnable), *(outputinfo->sinkInputType), outputinfo->srcQueue);
        index++;
        outputinfo = outputinfo->next;
    }

    return 0;
}

static void clearConfigure(TinaRecorder *hdl)
{
	hdl->encodeWidth = 0;
	hdl->encodeHeight = 0;
	hdl->encodeBitrate = 0;
	hdl->encodeFramerate = 0;
	hdl->outputFormat = -1;
	hdl->callback = NULL;
	hdl->encodeVFormat = -1;
	hdl->encodeAFormat = -1;
	hdl->maxRecordTime = 0;
	hdl->EncoderContext = NULL;
#if 0
	memset(&hdl->vport, 0, sizeof(vInPort));
	memset(&hdl->aport, 0, sizeof(aInPort));
	memset(&hdl->dispport, 0, sizeof(dispOutPort));
	memset(&hdl->outport, 0, sizeof(rOutPort));
#endif
}

static void InitJpegExif(TinaRecorder *hdl,EXIFInfo *exifinfo)
{
    if(hdl==NULL || exifinfo==NULL)
        TRerr("[%s] hdl or exifinfo is null,InitJpegExif() fail\n", __func__);

    exifinfo->ThumbWidth = hdl->vport.MainWidth/8;
    exifinfo->ThumbHeight = hdl->vport.MainHeight/8;

    TRlog(TR_LOG_VIDEO, "exifinfo->ThumbWidth = %d\n",exifinfo->ThumbWidth);
    TRlog(TR_LOG_VIDEO, "exifinfo->ThumbHeight = %d\n",exifinfo->ThumbHeight);

    strcpy((char*)exifinfo->CameraMake,        "allwinner make test");
    strcpy((char*)exifinfo->CameraModel,        "allwinner model test");
    strcpy((char*)exifinfo->DateTime,         "2014:02:21 10:54:05");
    strcpy((char*)exifinfo->gpsProcessingMethod,  "allwinner gps");

    exifinfo->Orientation = 0;

    exifinfo->ExposureTime.num = 2;
    exifinfo->ExposureTime.den = 1000;

    exifinfo->FNumber.num = 20;
    exifinfo->FNumber.den = 10;
    exifinfo->ISOSpeed = 50;

    exifinfo->ExposureBiasValue.num= -4;
    exifinfo->ExposureBiasValue.den= 1;

    exifinfo->MeteringMode = 1;
    exifinfo->FlashUsed = 0;

    exifinfo->FocalLength.num = 1400;
    exifinfo->FocalLength.den = 100;

    exifinfo->DigitalZoomRatio.num = 4;
    exifinfo->DigitalZoomRatio.den = 1;

    exifinfo->WhiteBalance = 1;
    exifinfo->ExposureMode = 1;

    exifinfo->enableGpsInfo = 1;

    exifinfo->gps_latitude = 23.2368;
    exifinfo->gps_longitude = 24.3244;
    exifinfo->gps_altitude = 1234.5;

    exifinfo->gps_timestamp = (long)time(NULL);

    strcpy((char*)exifinfo->CameraSerialNum,  "123456789");
    strcpy((char*)exifinfo->ImageName,  "exif-name-test");
    strcpy((char*)exifinfo->ImageDescription,  "exif-descriptor-test");
}

static int saveJpeg(char *savePath,VencOutputBuffer *jpegData)
{
    FILE* savaJpegFd = fopen(savePath, "wb+");
    TRlog(TR_LOG_VIDEO, "save jpeg path:%s\n",savePath);

    if(savaJpegFd==NULL){
	    TRerr("[%s] fopen %s  fail****\n",__func__,savePath);
	    TRerr("[%s] err str: %s\n",__func__,strerror(errno));
        return -1;
    }else{
        fseek(savaJpegFd, 0, SEEK_SET);

        fwrite(jpegData->pData0, 1, jpegData->nSize0, savaJpegFd);
        if(jpegData->nSize1)
            fwrite(jpegData->pData1, 1, jpegData->nSize1, savaJpegFd);

        fflush(savaJpegFd);
        fsync(fileno(savaJpegFd));

	fclose(savaJpegFd);
	savaJpegFd = NULL;
    }

    return 0;
}

static int captureJpegPhoto(TinaRecorder *hdl,char *savePath,struct MediaPacket *yuvDataParam)
{
    if(hdl == NULL || savePath == NULL || yuvDataParam == NULL){
        TRerr("[%s] input parameter error\n", __func__);
        return -1;
    }

    VencBaseConfig baseConfig;
    VencInputBuffer inputBuffer;
    VencOutputBuffer outputBuffer;
    VideoEncoder* pVideoEnc = NULL;
    EXIFInfo exifinfo;

    InitJpegExif(hdl,&exifinfo);

    memset(&baseConfig, 0 ,sizeof(VencBaseConfig));
    memset(&inputBuffer, 0x00, sizeof(VencInputBuffer));
    memset(&outputBuffer, 0x00, sizeof(VencOutputBuffer));

    baseConfig.eInputFormat = GetJpegSourceFormat(hdl->vport.format);
    if(baseConfig.eInputFormat < 0){
        TRerr("[%s] capture photo source format error\n", __func__);
        return -1;
    }
    baseConfig.nInputWidth = hdl->vport.MainWidth;
    baseConfig.nInputHeight = hdl->vport.MainHeight;
    baseConfig.nStride = hdl->vport.MainWidth;

    baseConfig.nDstWidth = hdl->mCaptureConfig.captureWidth;
    baseConfig.nDstHeight = hdl->mCaptureConfig.captureHeight;
    if(baseConfig.nDstWidth/32 > exifinfo.ThumbWidth || baseConfig.nDstHeight/32 > exifinfo.ThumbHeight){
        TRwarn("[%s] the thumb width(%d) or height(%d) is too small,set them to %d * %d\n",
            __func__,exifinfo.ThumbWidth,exifinfo.ThumbHeight,baseConfig.nDstWidth/32,baseConfig.nDstHeight/32);
        exifinfo.ThumbWidth = baseConfig.nDstWidth/32;
        exifinfo.ThumbHeight = baseConfig.nDstHeight/32;
    }

    TRlog(TR_LOG, "[%s] capture photo input width %d height %d input format %d dstWidth %d dstHeight %d\n",
                            __func__, baseConfig.nInputWidth, baseConfig.nInputHeight, baseConfig.eInputFormat,
                            baseConfig.nDstWidth, baseConfig.nDstHeight);

    pVideoEnc = VideoEncCreate(VENC_CODEC_JPEG);
    if(pVideoEnc == NULL){
        TRerr("[%s] VideoEncCreate fail\n", __func__);
		return -1;
    }

    int quality = 80;
    int jpeg_mode = 0;
    int vbvSize = 750*1024;
    int vbvThreshold = 0;//that means all the vbv buffer can be used
    if(baseConfig.nDstWidth<1920){
        vbvSize = 512*1024;
    }

    VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegExifInfo, &exifinfo);
    VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegQuality, &quality);
    VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegEncMode, &jpeg_mode);
    VideoEncSetParameter(pVideoEnc, VENC_IndexParamSetVbvSize, &vbvSize);
    VideoEncSetParameter(pVideoEnc, VENC_IndexParamSetFrameLenThreshold, &vbvThreshold);

    int vencRet = -1;

    vencRet = VideoEncInit(pVideoEnc, &baseConfig);

    if(vencRet == -1){
        TRerr("[%s] VideoEncInit fail\n", __func__);
        goto handle_err;
    }

    inputBuffer.nID = yuvDataParam->buf_index;
    inputBuffer.nPts = yuvDataParam->fps;
    inputBuffer.nFlag = 0;
    inputBuffer.pAddrPhyY = yuvDataParam->Phy_Y_addr;
    inputBuffer.pAddrPhyC = yuvDataParam->Phy_C_addr;

    inputBuffer.bEnableCorp = 0;
    inputBuffer.sCropInfo.nLeft =  240;
    inputBuffer.sCropInfo.nTop  =  240;
    inputBuffer.sCropInfo.nWidth  =  240;
    inputBuffer.sCropInfo.nHeight =  240;

    vencRet = AddOneInputBuffer(pVideoEnc, &inputBuffer);
    if(vencRet == -1){
        TRerr("[%s] AddOneInputBuffer fail\n", __func__);
        goto handle_err;
    }

    vencRet = VideoEncodeOneFrame(pVideoEnc);
    if(vencRet == -1){
        TRerr("[%s] VideoEncodeOneFrame fail\n", __func__);
        goto handle_err;
    }

    vencRet = GetOneBitstreamFrame(pVideoEnc, &outputBuffer);
    if(vencRet == -1){
        TRerr("[%s] GetOneBitstreamFrame fail\n", __func__);
        goto handle_err;
    }

    saveJpeg(savePath,&outputBuffer);

    vencRet = FreeOneBitStreamFrame(pVideoEnc, &outputBuffer);
    if(vencRet == -1){
        TRerr("[%s] FreeOneBitStreamFrame fail\n", __func__);
        goto handle_err;
    }

    if(pVideoEnc)
    {
        VideoEncDestroy(pVideoEnc);
    }

    return vencRet;

 handle_err:
    if(pVideoEnc)
    {
        VideoEncDestroy(pVideoEnc);
    }

    return -1;
}

int DestroyTinaRecorder(TinaRecorder *hdl)
{
	if(hdl == NULL)
		return -1;

	if(hdl->status == TINA_RECORD_STATUS_RELEASED)
	{
		free(hdl);
	}
	else
		return -1;
	return 0;
}

int TinasetVideoInPort(void *hdl,vInPort *vPort)
{
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;
	if(hdlTina == NULL)
		return -1;
	if (hdlTina->status == TINA_RECORD_STATUS_INIT ||
		hdlTina->status == TINA_RECORD_STATUS_INITIALIZED) {
		if(vPort != NULL) {
			memcpy(&hdlTina->vport, vPort, sizeof(vInPort));
            hdlTina->vport.enable = 1;
		} else
			hdlTina->vport.enable = 0;

		hdlTina->status = TINA_RECORD_STATUS_INITIALIZED;
	} else
		return -1;

	return 0;
}

int TinasetAudioInPort(void *hdl,aInPort *aPort)
{
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;
	if(hdlTina == NULL)
		return -1;

	if(hdlTina->status == TINA_RECORD_STATUS_INIT || hdlTina->status == TINA_RECORD_STATUS_INITIALIZED)
	{
		if(aPort != NULL){
			memcpy(&hdlTina->aport, aPort, sizeof(aInPort));
            hdlTina->aport.enable = 1;
            hdlTina->mAudioUserSetting = 1;
		}else{
			hdlTina->aport.enable = 0;
            hdlTina->mAudioUserSetting = 0;
        }
		hdlTina->status = TINA_RECORD_STATUS_INITIALIZED;
	}
	else
		return -1;

	return 0;
}

int TinasetDispOutPort(void *hdl,dispOutPort *dispPort)
{
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;
	if(hdlTina == NULL)
		return -1;

	if(hdlTina->status == TINA_RECORD_STATUS_INIT
        || hdlTina->status == TINA_RECORD_STATUS_INITIALIZED
        || hdlTina->status == TINA_RECORD_STATUS_DATA_SOURCE_CONFIGURED)
	{
		if(dispPort != NULL){
			memcpy(&hdlTina->dispport,dispPort,sizeof(dispOutPort));
            hdlTina->dispport.enable = 1;
		}else
			hdlTina->dispport.enable = 0;

	hdlTina->dispport.disp_fd = 0;

        hdlTina->status = TINA_RECORD_STATUS_DATA_SOURCE_CONFIGURED;
	}
	else
		return -1;

	return 0;
}

int TinasetOutput(void *hdl,rOutPort *output)
{
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;
	if(hdlTina == NULL)
		return -1;

	if (hdlTina->status == TINA_RECORD_STATUS_INIT
            || hdlTina->status == TINA_RECORD_STATUS_INITIALIZED
            || hdlTina->status == TINA_RECORD_STATUS_DATA_SOURCE_CONFIGURED) {
		if(output != NULL) {
			memcpy(&hdlTina->outport,output,sizeof(rOutPort));
            hdlTina->outport.enable = 1;
		} else
			hdlTina->outport.enable = 0;

		hdlTina->status = TINA_RECORD_STATUS_DATA_SOURCE_CONFIGURED;
	} else {
		TRerr("[%s] handle status error\n", __func__);
		return -1;
	}
	return 0;
}

int TinasetEncodeSize(void *hdl,int width,int height)
{
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;
	if(hdlTina == NULL)
		return -1;
	if(hdlTina->status == TINA_RECORD_STATUS_DATA_SOURCE_CONFIGURED)
	{
		hdlTina->encodeWidth = width;
		hdlTina->encodeHeight = height;
	}
	else
		return -1;
	return 0;
}

int TinasetEncodeBitRate(void *hdl,int bitrate)
{
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;
	if(hdlTina == NULL)
		return -1;
	if(hdlTina->status == TINA_RECORD_STATUS_DATA_SOURCE_CONFIGURED)
		hdlTina->encodeBitrate = bitrate;
	else
		return -1;
	return 0;
}

int TinasetOutputFormat(void *hdl,int format)
{
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;
	if(hdlTina == NULL)
		return -1;
	if(hdlTina->status == TINA_RECORD_STATUS_DATA_SOURCE_CONFIGURED)
		hdlTina->outputFormat = format;
	else
		return -1;
	return 0;
}
int TinasetCallback(void *hdl,TinaRecorderCallback callback,void* pUserData)
{
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;
	if(hdlTina == NULL)
		return -1;
    TRlog(TR_LOG, "TinasetCallback:hdlTina->status = %d\n",hdlTina->status);
	if(hdlTina->status == TINA_RECORD_STATUS_DATA_SOURCE_CONFIGURED){
		hdlTina->callback = callback;
                hdlTina->pUserData = pUserData;
       }
	else
		return -1;
	return 0;
}


int TinasetEncodeVideoFormat(void *hdl,int format)
{
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;
	if(hdlTina == NULL)
		return -1;
	if(hdlTina->status == TINA_RECORD_STATUS_DATA_SOURCE_CONFIGURED)
		hdlTina->encodeVFormat = format;
	else
		return -1;
	return 0;
}

int TinasetEncodeAudioFormat(void *hdl,int format)
{
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;
	if(hdlTina == NULL)
		return -1;
	if(hdlTina->status == TINA_RECORD_STATUS_DATA_SOURCE_CONFIGURED)
		hdlTina->encodeAFormat = format;
	else
		return -1;
	return 0;
}

int TinagetRecorderTime(void *hdl)
{
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;
	if(hdlTina == NULL)
		return -1;

	if(hdlTina->status == TINA_RECORD_STATUS_RECORDING)
	{
		//fixme here
		return 111;
	}
	else
		return -1;
}

int TinacaptureCurrent(void *hdl,captureConfig *capConfig)
{
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;
	if(hdlTina == NULL)
		return -1;

	TRlog(TR_LOG_VIDEO, "------------------TinacaptureCurrent\n");
	if(hdlTina->status == TINA_RECORD_STATUS_RECORDING)
	{

	    TRlog(TR_LOG_VIDEO, "------------------capture status right\n");

		//fixme here,do something
        strcpy(hdlTina->mCaptureConfig.capturePath,capConfig->capturePath);
        hdlTina->mCaptureConfig.capFormat = capConfig->capFormat;
        if(capConfig->captureWidth>8*hdlTina->vport.MainWidth){
            TRwarn("[%s] warnning:the capture width(%d) is 8 times larger than source width(%d),set it to%d\n",
                __func__,capConfig->captureWidth,hdlTina->vport.MainWidth,hdlTina->vport.MainWidth*8);

            capConfig->captureWidth = hdlTina->vport.MainWidth*8;
        }
        if(capConfig->captureHeight>8*hdlTina->vport.MainHeight){
            TRwarn("[%s] warnning:the capture height(%d) is 8 times larger than source height(%d),set it to%d\n",
                __func__,capConfig->captureHeight,hdlTina->vport.MainHeight,hdlTina->vport.MainHeight*8);

            capConfig->captureHeight = hdlTina->vport.MainHeight*8;
        }
        hdlTina->mCaptureConfig.captureWidth= capConfig->captureWidth;
        hdlTina->mCaptureConfig.captureHeight= capConfig->captureHeight;
        hdlTina->mCaptureFlag = 1;

		return 0;
	}
	else
		return -1;
}

int TinasetMaxRecordTime(void *hdl,int ms)
{
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;
	if(hdlTina == NULL)
		return -1;
	//if(hdlTina->status == TINA_RECORD_STATUS_DATA_SOURCE_CONFIGURED)
		hdlTina->maxRecordTime = ms;
	//else
		//return -1;
	return 0;
}

static int TinaScaleDownInit(void *hdl)
{
    int enable = 0;
    int ratio = 0;
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;

	if(!hdlTina)
		return -1;
    DemoRecoderContext *demoRecoder = (DemoRecoderContext*)hdlTina->EncoderContext;

    pthread_mutex_init(&hdlTina->mScaleCallbackLock, NULL);

	//close data copy from VE
	//fixme,to do here
	enable = 0;
	AwEncoderSetParamete(demoRecoder->mAwEncoder,AwEncoder_SetThumbNailEnable,&enable);

	//set new scale down ratio to VE
	//fixme,to do here
	if(hdlTina->scaleDownRatio == 2)
		ratio = VENC_ISP_SCALER_HALF;
	else if(hdlTina->scaleDownRatio == 4)
		ratio = VENC_ISP_SCALER_QUARTER;
	else if(hdlTina->scaleDownRatio == 8)
		ratio = VENC_ISP_SCALER_EIGHTH;
	else
		ratio = VENC_ISP_SCALER_0;

	AwEncoderSetParamete(demoRecoder->mAwEncoder,AwEncoder_SetScaleDownRatio,&ratio);

	//start copy ve buffer to scaledown buffer
	//fixme,to do here
	enable = 1;
	AwEncoderSetParamete(demoRecoder->mAwEncoder,AwEncoder_SetThumbNailEnable,&enable);

    return 0;
}

int TinasetParameter(void *hdl,int cmd,int parameter)
{
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;
	if(hdlTina == NULL)
		return -1;
	if(hdlTina->status == TINA_RECORD_STATUS_DATA_SOURCE_CONFIGURED)
	{
		//fixme,do something here
	}
	else
		return -1;
	return 0;
}

int Tinaprepare(void *hdl)
{
    int i;
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;
	if(!hdlTina)
		return -1;

    if(hdlTina->status == TINA_RECORD_STATUS_DATA_SOURCE_CONFIGURED
                    && !hdlTina->vport.enable && !hdlTina->aport.enable){
        TRprint(" video and audio disable,encoder can not use,prepare ok\n");

        hdlTina->status = TINA_RECORD_STATUS_PREPARED;
        return 0;
    }

    pthread_mutex_init(&hdlTina->mEncoderPacketLock, NULL);
    for(i = 0; i < ENCODER_SAVE_NUM; i++){
        hdlTina->encoderpacket[i] = NULL;
    }

    /* initialization decoding when uvc camera is in MJPEG or H264 */
	if(hdlTina->vport.cameraType == usb_camera
            && (hdlTina->vport.format == TR_PIXEL_MJPEG || hdlTina->vport.format == TR_PIXEL_H264)){
		hdlTina->decoderport.Input_Fmt = GetDecoderVideoSourceFormat(hdlTina->vport.format);
		hdlTina->decoderport.Output_Fmt = PIXEL_FORMAT_NV21;
		DecoderInit(&hdlTina->decoderport);
	}

	// need rewrite
	EncDataCallBackOps *mEncDataCallBackOps = (EncDataCallBackOps *)malloc(sizeof(EncDataCallBackOps));
	mEncDataCallBackOps->onAudioDataEnc = onAudioDataEnc;
	mEncDataCallBackOps->onVideoDataEnc = onVideoDataEnc;
	hdlTina->rotateAngel = ROTATION_ANGLE_0;

	if(hdlTina->status == TINA_RECORD_STATUS_DATA_SOURCE_CONFIGURED)
	{
		// create the demoRecoder.
		DemoRecoderContext *demoRecoder = (DemoRecoderContext *)malloc(sizeof(DemoRecoderContext));
		if(demoRecoder == NULL)
			goto error_release;

		memset(demoRecoder, 0, sizeof(DemoRecoderContext));
		demoRecoder->ops = mEncDataCallBackOps;
		hdlTina->EncoderContext = (void *)demoRecoder;

        if(hdlTina->vport.enable || hdlTina->aport.enable){
		/* set muxType  */
		switch(hdlTina->outputFormat)
		{
			case TR_OUTPUT_TS:
                    if(hdlTina->vport.enable == 0){
                        TRerr("[%s] vport not enable when record TS!\n", __func__);
                        goto error_release;
                    }
                    if(hdlTina->aport.enable == 0)
                        TRwarn("[%s] warnning:aport not enable when record TS!\n", __func__);
				demoRecoder->muxType = CDX_MUXER_TS;
				break;
			case TR_OUTPUT_AAC:
				if(hdlTina->vport.enable != 0)
				{
					TRerr("[%s] vport not disable when record AAC!\n", __func__);
					goto error_release;
				}
				demoRecoder->muxType = CDX_MUXER_AAC;
				break;
			case TR_OUTPUT_MP3:
				if(hdlTina->vport.enable != 0)
				{
					TRerr("[%s] vport not disable when record MP3!\n", __func__);
					goto error_release;
				}
				demoRecoder->muxType = CDX_MUXER_MP3;
				break;
			case TR_OUTPUT_MOV:
                    if(hdlTina->vport.enable == 0){
                        TRerr("[%s] vport not enable when record MOV!\n", __func__);
                        goto error_release;
                    }
                    if(hdlTina->aport.enable == 0)
                        TRwarn("[%s] warnning:aport not enable when record MOV!\n", __func__);
	            demoRecoder->muxType = CDX_MUXER_MOV;
				break;
			case TR_OUTPUT_JPG:
				if(hdlTina->aport.enable != 0)
				{
					TRerr("[%s] aport not disable when taking picture!\n", __func__);
					goto error_release;
				}
				break;
			default:
				TRerr("[%s] output format not supported!\n", __func__);
				goto error_release;
				break;
		}
        }

		/* if vport enable and enc video */
		if(hdlTina->vport.enable && (demoRecoder->muxType == CDX_MUXER_MOV ||
									demoRecoder->muxType == CDX_MUXER_TS))
		{
			/* config VideoEncodeConfig */
			memset(&demoRecoder->videoConfig, 0x00, sizeof(VideoEncodeConfig));
            demoRecoder->videoConfig.nType = ReturnEncoderVideoType(hdlTina->encodeVFormat);
            if(demoRecoder->videoConfig.nType < 0){
                TRerr("[%s] video encoder type error:type %d!\n", __func__, hdlTina->outputFormat);
                goto error_release;
            }
            demoRecoder->videoConfig.nInputYuvFormat = GetEncoderVideoSourceFormat(hdlTina->vport.format);
            if(demoRecoder->videoConfig.nInputYuvFormat < 0){
                TRerr("[%s] video encoder source format %d not support!\n",
                                            __func__, hdlTina->encodeVinputFormat);
                goto error_release;
            }
            hdlTina->vport.getSize(&hdlTina->vport,
                                                &demoRecoder->videoConfig.nSrcWidth,
                                                &demoRecoder->videoConfig.nSrcHeight);
            demoRecoder->videoConfig.nOutWidth = hdlTina->encodeWidth;
            demoRecoder->videoConfig.nOutHeight = hdlTina->encodeHeight;
            demoRecoder->videoConfig.nBitRate = hdlTina->encodeBitrate;
            demoRecoder->videoConfig.nFrameRate = hdlTina->encodeFramerate;
            /* uvc camera does not decode without continuous physical address */
            if(hdlTina->vport.cameraType == usb_camera
                    && hdlTina->vport.format != TR_PIXEL_MJPEG
                    && hdlTina->vport.format != TR_PIXEL_H264){
                demoRecoder->videoConfig.bUsePhyBuf = 0;
                hdlTina->encodeUsePhy = 0;
            }else{
                demoRecoder->videoConfig.bUsePhyBuf = 1;
                hdlTina->encodeUsePhy = 1;
            }

			if(hdlTina->scaleDownRatio == 2)
				demoRecoder->videoConfig.ratio = VENC_ISP_SCALER_HALF;
			else if(hdlTina->scaleDownRatio == 4)
				demoRecoder->videoConfig.ratio = VENC_ISP_SCALER_QUARTER;
			else if(hdlTina->scaleDownRatio == 8)
				demoRecoder->videoConfig.ratio = VENC_ISP_SCALER_EIGHTH;
			else
				demoRecoder->videoConfig.ratio = VENC_ISP_SCALER_0;
		}

		/* if aport enable and enc audio */
		if (hdlTina->aport.enable) {
			//config AudioEncodeConfig
			memset(&demoRecoder->audioConfig, 0x00, sizeof(AudioEncodeConfig));
            demoRecoder->audioConfig.nType = GetEncoderAudioSourceFormat(hdlTina->encodeAFormat);
			demoRecoder->audioConfig.nInChan = hdlTina->aport.getAudioChannels(&hdlTina->aport);
			demoRecoder->audioConfig.nInSamplerate = hdlTina->aport.getAudioSampleRate(&hdlTina->aport);
            demoRecoder->audioConfig.nSamplerBits = hdlTina->aport.getAudioSampleBits(&hdlTina->aport);
			demoRecoder->audioConfig.nOutChan = demoRecoder->audioConfig.nInChan;
			demoRecoder->audioConfig.nOutSamplerate = hdlTina->aport.getAudioSampleRate(&hdlTina->aport);

			if (demoRecoder->muxType == CDX_MUXER_TS &&
				demoRecoder->audioConfig.nType == AUDIO_ENCODE_PCM_TYPE) {
				demoRecoder->audioConfig.nFrameStyle = 2;
			}
			if (demoRecoder->muxType == CDX_MUXER_TS &&
				demoRecoder->audioConfig.nType == AUDIO_ENCODE_AAC_TYPE) {
				//not add head when encode aac,because use ts muxer
				demoRecoder->audioConfig.nFrameStyle = 1;
			}
			if(demoRecoder->muxType == CDX_MUXER_AAC) {
				demoRecoder->audioConfig.nType = AUDIO_ENCODE_AAC_TYPE;
				//add head when encode aac
				demoRecoder->audioConfig.nFrameStyle = 0;
			}
			if(demoRecoder->muxType == CDX_MUXER_MP3) {
				demoRecoder->audioConfig.nType = AUDIO_ENCODE_MP3_TYPE;
			}
		}

		if(hdlTina->outport.enable)
		{
		    //store the muxer file path
		    strcpy(hdlTina->mOutputPath,hdlTina->outport.url);
		}

		demoRecoder->mAwEncoder = AwEncoderCreate(hdl);
		if(!demoRecoder->mAwEncoder) {
			TRerr("[%s] can not create AwRecorder, quit.\n", __func__);
			goto error_release;
		}

        /* set callback to recoder,if the encoder has used the buf ,it will callback to app */
        AwEncoderSetNotifyCallback(demoRecoder->mAwEncoder,NotifyCallbackForAwEncorder,hdl);

		if(hdlTina->aport.enable && (demoRecoder->muxType == CDX_MUXER_AAC ||
									demoRecoder->muxType == CDX_MUXER_MP3)) {
			AwEncoderInit(demoRecoder->mAwEncoder,
							NULL,
							&demoRecoder->audioConfig,
							mEncDataCallBackOps);
		} else if(hdlTina->vport.enable && !hdlTina->aport.enable &&
				(demoRecoder->muxType == CDX_MUXER_MOV ||
				 demoRecoder->muxType == CDX_MUXER_TS)) {
			AwEncoderInit(demoRecoder->mAwEncoder,
							&demoRecoder->videoConfig,
							NULL,
							mEncDataCallBackOps);
		}
		else if(hdlTina->vport.enable && hdlTina->aport.enable &&
				(demoRecoder->muxType == CDX_MUXER_MOV ||
				 demoRecoder->muxType == CDX_MUXER_TS)) {
			AwEncoderInit(demoRecoder->mAwEncoder,
							&demoRecoder->videoConfig,
							&demoRecoder->audioConfig,
							mEncDataCallBackOps);
		}

        /* print encoder info */
        TRlog(TR_LOG, "encoder init ok, muxtype %d\n", demoRecoder->muxType);
        TRdebugEncoderVideoInfo(&demoRecoder->videoConfig);
        TRdebugEncoderAudioInfo(&demoRecoder->audioConfig);
        TRlog(TR_LOG, "*****************    end     *****************\n");
	} else {
		/* status not equal TINA_RECORD_STATUS_DATA_SOURCE_CONFIGURED */
		TRerr("[%s] status error:%d\n", __func__, hdlTina->status);
		if(mEncDataCallBackOps){
			free(mEncDataCallBackOps);
			mEncDataCallBackOps = NULL;
		}
		return -1;
	}

	TRprint(" prepare ok\n");

	hdlTina->status = TINA_RECORD_STATUS_PREPARED;
	return 0;

error_release:
    if(mEncDataCallBackOps)
        free(mEncDataCallBackOps);
    if(hdlTina->EncoderContext){
        free(hdlTina->EncoderContext);
        hdlTina->EncoderContext = NULL;
    }

	TRerr("[%s] Failed\n", __func__);
	return -1;
}

int Tinastart(void *hdl,int flags)
{
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;
	videoParam param;
    int ratio;
    int enable;
    int ret = 0;

	if(!hdlTina)
		return -1;

	DemoRecoderContext *demoRecoder = (DemoRecoderContext *)hdlTina->EncoderContext;

	if(hdlTina->status == TINA_RECORD_STATUS_PREPARED){
        TinaPrintModulelink(hdlTina);

		if(flags == FLAGS_RECORDING){
			if(hdlTina->outport.enable == 0)
				return -1;
			hdlTina->dispport.enable = 0;
            modulePort_SetEnable(&hdlTina->screenPort, 0);
			hdlTina->startFlags = FLAGS_RECORDING;
		}else if(flags == FLAGS_PREVIEW){
			hdlTina->outport.enable = 0;
			hdlTina->aport.enable = 0;
            modulePort_SetEnable(&hdlTina->muxerPort, 0);
            modulePort_SetEnable(&hdlTina->encoderPort, 0);
			hdlTina->startFlags = FLAGS_PREVIEW;
		}else
			hdlTina->startFlags = FLAGS_ALL;

		/* create module thread when use it */

		if(hdlTina->vport.enable) {
            if(hdlTina->vport.cameraType == usb_camera
                    && (hdlTina->vport.format == TR_PIXEL_MJPEG || hdlTina->vport.format == TR_PIXEL_H264))
                hdlTina->decoderport.enable = 1;

			pthread_create(&hdlTina->videoId, NULL, VideoInputThread, hdl);
	    }

		if(hdlTina->decoderport.enable) {
			pthread_create(&hdlTina->decoderId, NULL, TRDecoderThread, hdl);

            modulePort_SetEnable(&hdlTina->decoderPort, 1);
		}

		if(hdlTina->dispport.enable){
			/* display */
            /* csi camera display use csi ion buf */
            if(hdlTina->vport.cameraType != csi_camera){
                if(hdlTina->scaleDownRatio == 0){
                    param.srcInfo.w = hdlTina->vport.MainWidth;
                    param.srcInfo.h = hdlTina->vport.MainHeight;
                }else{
                    param.srcInfo.w = ((hdlTina->encodeWidth + 15)/16*16)/hdlTina->scaleDownRatio;
                    param.srcInfo.h = ((hdlTina->encodeHeight + 15)/16*16)/hdlTina->scaleDownRatio;
                }
                if(param.srcInfo.w == 0 || param.srcInfo.h == 0){
                    TRlog(TR_LOG_DISPLAY, "video source is not set, use configuration parameters\n");
                    param.srcInfo.w = hdlTina->dispSrcWidth;
                    param.srcInfo.h = hdlTina->dispSrcHeight;
                }
                param.srcInfo.format = GetDisplayFormat(hdlTina->vport.format);
                TRlog(TR_LOG_DISPLAY, "disp allocateVideoMem width %d height %d format %s\n",
                                                        param.srcInfo.w, param.srcInfo.h,
                                                        ReturnVideoTRFormatText(hdlTina->vport.format));
                ret = hdlTina->dispport.allocateVideoMem(&hdlTina->dispport,&param);
                if(ret < 0){
                    TRerr("[%s] disp alloc mem error, param.srcInfo.w %u param.srcInfo.h %u\n",
                                                            __func__, param.srcInfo.w, param.srcInfo.h);
                    return -1;
                }
            }

            hdlTina->dispport.enable = 1;
			pthread_create(&hdlTina->dirdispId, NULL, TRDispThread, hdl);

            modulePort_SetEnable(&hdlTina->screenPort, 1);

            /* scale display */
            if(hdlTina->vport.enable && (hdlTina->scaleDownRatio != 0)){
                AwEncoderStart(demoRecoder->mAwEncoder);
                AwEncoderGetExtradata(demoRecoder->mAwEncoder,
                                        &demoRecoder->extractDataBuff,
                                        &demoRecoder->extractDataLength);
                demoRecoder->encoderInitOkFlag = 1;

                /* scale down init */
                TinaScaleDownInit(hdlTina);

                demoRecoder->encoderEnable = 1;
                pthread_create(&demoRecoder->encoderThreadId, NULL, TREncoderThread, hdl);

                modulePort_SetEnable(&hdlTina->encoderPort, 1);
            }
		}

		if(hdlTina->outport.enable){
            if(demoRecoder->encoderInitOkFlag == 0){
                AwEncoderStart(demoRecoder->mAwEncoder);
                AwEncoderGetExtradata(demoRecoder->mAwEncoder,
                                        &demoRecoder->extractDataBuff,
                                        &demoRecoder->extractDataLength);
                demoRecoder->encoderInitOkFlag = 1;
            }

            /* only recorder */
            if(hdlTina->vport.enable && (hdlTina->dispport.enable == 0)){
                enable = 0;
                AwEncoderSetParamete(demoRecoder->mAwEncoder,
                                                AwEncoder_SetThumbNailEnable,&enable);
            }

            initStorePktInfo(hdlTina);

            hdlTina->outport.enable = 1;
			pthread_create(&demoRecoder->muxerThreadId, NULL, TRMuxerThread, hdl);

            modulePort_SetEnable(&hdlTina->muxerPort, 1);
            hdlTina->mNeedMuxerVideoFlag = MUXER_NEED_KEYFRAME;

            demoRecoder->encoderEnable = 1;
            pthread_create(&demoRecoder->encoderThreadId, NULL, TREncoderThread, hdl);
            modulePort_SetEnable(&hdlTina->encoderPort, 1);
        }

        /* user sets up audio and enables it */
		if(hdlTina->mAudioUserSetting && hdlTina->aport.enable)
			pthread_create(&hdlTina->audioId, NULL, AudioInputThread, hdl);

		hdlTina->status = TINA_RECORD_STATUS_RECORDING;
		return 0;
	} else if(hdlTina->status == TINA_RECORD_STATUS_RECORDING) {
		if(hdlTina->startFlags == FLAGS_ALL || flags == hdlTina->startFlags)
		{
			TRprompt("[%s] return do nothing!\n", __func__);
			return 0;
		}

		if(flags == FLAGS_ALL && hdlTina->startFlags == FLAGS_PREVIEW)
			flags = FLAGS_RECORDING;
		else if(flags == FLAGS_ALL && hdlTina->startFlags == FLAGS_RECORDING)
			flags = FLAGS_PREVIEW;
		TRprompt("[%s] need to start:%d\n", __func__, flags);

		if(flags == FLAGS_PREVIEW){
            if(hdlTina->scaleDownRatio == 0){
			    param.srcInfo.w = hdlTina->vport.MainWidth;
			    param.srcInfo.h = hdlTina->vport.MainHeight;
            }else{
			    param.srcInfo.w = ((hdlTina->encodeWidth + 15)/16*16)/hdlTina->scaleDownRatio;
				param.srcInfo.h = ((hdlTina->encodeHeight + 15)/16*16)/hdlTina->scaleDownRatio;

                if(hdlTina->scaleDownRatio == 2)
					ratio = VENC_ISP_SCALER_HALF;
				else if(hdlTina->scaleDownRatio == 4)
					ratio = VENC_ISP_SCALER_QUARTER;
				else if(hdlTina->scaleDownRatio == 8)
					ratio = VENC_ISP_SCALER_EIGHTH;
				else
					ratio = VENC_ISP_SCALER_0;
				AwEncoderSetParamete(demoRecoder->mAwEncoder,AwEncoder_SetScaleDownRatio,&ratio);
            }

            /* csi camera display use csi ion buf */
            if(hdlTina->vport.cameraType != csi_camera){
                param.srcInfo.format = GetDisplayFormat(hdlTina->vport.format);
                TRlog(TR_LOG_DISPLAY, "disp allocateVideoMem width %d height %d format %s\n",
                                                        param.srcInfo.w, param.srcInfo.h,
                                                        ReturnVideoTRFormatText(hdlTina->vport.format));

                ret = hdlTina->dispport.allocateVideoMem(&hdlTina->dispport,&param);
                if(ret < 0){
                    TRerr("[%s] disp alloc mem error, param.srcInfo.w %u param.srcInfo.h %u\n",
                                                            __func__, param.srcInfo.w, param.srcInfo.h);
                    return -1;
                }
            }

            hdlTina->dispport.enable = 1;
            pthread_create(&hdlTina->dirdispId, NULL, TRDispThread, hdl);

            modulePort_SetEnable(&hdlTina->screenPort, 1);

            if(hdlTina->scaleDownRatio != 0){
                enable = 1;
                AwEncoderSetParamete(demoRecoder->mAwEncoder,AwEncoder_SetThumbNailEnable,&enable);
            }

			hdlTina->startFlags = FLAGS_ALL;
       } else if(flags == FLAGS_RECORDING) {
            if(demoRecoder->encoderInitOkFlag == 0){
                AwEncoderStart(demoRecoder->mAwEncoder);
                AwEncoderGetExtradata(demoRecoder->mAwEncoder,
                                        &demoRecoder->extractDataBuff,
                                        &demoRecoder->extractDataLength);
                demoRecoder->encoderInitOkFlag = 1;
            }

            initStorePktInfo(hdlTina);

            hdlTina->outport.enable = 1;
            pthread_create(&demoRecoder->muxerThreadId, NULL, TRMuxerThread, hdl);

            modulePort_SetEnable(&hdlTina->muxerPort, 1);
            hdlTina->mNeedMuxerVideoFlag = MUXER_NEED_KEYFRAME;

            if(demoRecoder->encoderThreadId == 0 || demoRecoder->encoderEnable == 0){
                demoRecoder->encoderEnable = 1;
                pthread_create(&demoRecoder->encoderThreadId, NULL, TREncoderThread, hdl);
                modulePort_SetEnable(&hdlTina->encoderPort, 1);
            }

            /* user sets up audio and enables it */
            if(hdlTina->mAudioUserSetting
                            && (hdlTina->audioId == 0 || hdlTina->aport.enable == 0)){
                hdlTina->aport.enable = 1;

                pthread_create(&hdlTina->audioId, NULL, AudioInputThread, hdl);
            }

			hdlTina->startFlags = FLAGS_ALL;
	    }
        return 0;
	}
    return -1;
}

int Tinarelease(void *hdl)
{
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;
	if(hdlTina == NULL)
		return -1;

	if(hdlTina->status == TINA_RECORD_STATUS_INIT)
		hdlTina->status = TINA_RECORD_STATUS_RELEASED;
	else
		return -1;

	return 0;
}

int Tinareset(void *hdl)
{
    struct modulePacket *mPacket = NULL;

	TinaRecorder *hdlTina = (TinaRecorder *)hdl;
    int needStopVideo = 0;
    int enable;
	if(hdlTina == NULL)
		return -1;
	DemoRecoderContext *demoRecoder = (DemoRecoderContext *)hdlTina->EncoderContext;
	AwEncoder *w = NULL;
	if(demoRecoder != NULL)
		w = demoRecoder->mAwEncoder;

    TRlog(TR_LOG, "%s: hdlTina %p reset state %d\n", __func__, hdlTina, hdlTina->status);

	if(hdlTina->status == TINA_RECORD_STATUS_INITIALIZED){
		memset(&hdlTina->vport, 0, sizeof(vInPort));
		memset(&hdlTina->aport, 0, sizeof(aInPort));
	}else if(hdlTina->status == TINA_RECORD_STATUS_DATA_SOURCE_CONFIGURED)
		clearConfigure(hdlTina);
	else if(hdlTina->status == TINA_RECORD_STATUS_PREPARED){
		if(w != NULL)
			AwEncoderDestory(w);

        pthread_mutex_destroy(&hdlTina->mEncoderPacketLock);

		free(demoRecoder->ops);
		free(demoRecoder);
		hdlTina->EncoderContext = NULL;
		clearConfigure(hdlTina);
	}else if(hdlTina->status == TINA_RECORD_STATUS_RECORDING){
	    /* stop push data */
	    modulePort_SetEnable(&hdlTina->screenPort, 0);
        modulePort_SetEnable(&hdlTina->encoderPort, 0);

		if(hdlTina->vport.enable){
            hdlTina->vport.enable = 0;
            needStopVideo = 1;
            if(hdlTina->videoId)
                pthread_join(hdlTina->videoId, NULL);
            hdlTina->videoId = 0;
		}

        if(hdlTina->aport.enable){
		hdlTina->aport.enable = 0;
            if(hdlTina->audioId)
                pthread_join(hdlTina->audioId, NULL);
            hdlTina->audioId = 0;
        }

        /* decoder */
        if(hdlTina->decoderport.enable == 1){
            hdlTina->decoderport.enable = 0;
            module_postReceiveSem(&hdlTina->decoderPort);

            if(hdlTina->decoderId)
                pthread_join(hdlTina->decoderId, NULL);
            hdlTina->decoderId = 0;
        }

        if(hdlTina->dispport.enable){
            hdlTina->dispport.enable = 0;
            module_postReceiveSem(&hdlTina->screenPort);

            hdlTina->dispport.setEnable(&hdlTina->dispport, 0);

            if(hdlTina->scaleDownRatio != 0){
                /* stop encoder thumb callback */
                enable = 0;
                AwEncoderSetParamete(demoRecoder->mAwEncoder,AwEncoder_SetThumbNailEnable,&enable);

                pthread_mutex_destroy(&hdlTina->mScaleCallbackLock);
            }
            if(hdlTina->dirdispId)
                pthread_join(hdlTina->dirdispId, NULL);
            hdlTina->dirdispId = 0;

            while(!module_InputQueueEmpty(&hdlTina->screenPort)) {
                mPacket = (struct modulePacket *)module_pop(&hdlTina->screenPort);
                if(!mPacket)
                    continue;

                packetDestroy(mPacket);
            }

            if(hdlTina->vport.cameraType != csi_camera)
                hdlTina->dispport.freeVideoMem(&hdlTina->dispport);
        }

        demoRecoder->encoderEnable = 0;
        module_postReceiveSem(&hdlTina->encoderPort);

        TRlog(TR_LOG_ENCODER, "wait for encoder thread to quit, queue packet num %d\n",
                                                        queueCountRead(&hdlTina->encoderPort));
        if(demoRecoder->encoderThreadId)
            pthread_join(demoRecoder->encoderThreadId, NULL);
        demoRecoder->encoderThreadId = 0;
        while(!module_InputQueueEmpty(&hdlTina->encoderPort)) {
            mPacket = (struct modulePacket *)module_pop(&hdlTina->encoderPort);
            if(!mPacket)
                continue;

            packetDestroy(mPacket);
        }

        /* stop video */
        if(needStopVideo)
            hdlTina->vport.stopcapture(&hdlTina->vport);

        hdlTina->outport.enable = 0;
        modulePort_SetEnable(&hdlTina->muxerPort, 0);
        module_postReceiveSem(&hdlTina->muxerPort);
        TRlog(TR_LOG_MUXER, "wait for muxer thread to quit, queue packet num %d\n",
                                                        queueCountRead(&hdlTina->muxerPort));
        if(demoRecoder->muxerThreadId)
            pthread_join(demoRecoder->muxerThreadId, NULL);
        demoRecoder->muxerThreadId = 0;

        hdlTina->mNeedMuxerVideoFlag = 0;

        //clear muxer related resources
        while(!module_InputQueueEmpty(&hdlTina->muxerPort)) {
            mPacket = (struct modulePacket *)module_pop(&hdlTina->muxerPort);
            if(!mPacket)
                continue;

            packetDestroy(mPacket);
        }

        destroyStorePktInfo(hdlTina);

		if(w != NULL){
			AwEncoderStop(w);
			AwEncoderDestory(w);
		}

        pthread_mutex_destroy(&hdlTina->mEncoderPacketLock);

		free(demoRecoder->ops);
		free(demoRecoder);
		hdlTina->EncoderContext = NULL;

		clearConfigure(hdlTina);
	}

	hdlTina->status = TINA_RECORD_STATUS_INIT;
	return 0;
}

static void cleanPreviewStatus(TinaRecorder *hdlTina)
{
	DemoRecoderContext *demoRecoder = (DemoRecoderContext *)hdlTina->EncoderContext;
    struct modulePacket *mPacket = NULL;
	int needStopCapture = 0;
    int enable;

    /* stop push screen queue */
    modulePort_SetEnable(&hdlTina->screenPort, 0);

	if(hdlTina->vport.enable)
		needStopCapture = 1;

    hdlTina->vport.enable = 0;
    if(hdlTina->videoId)
        pthread_join(hdlTina->videoId, NULL);
    hdlTina->videoId = 0;

    if(hdlTina->vport.cameraType == usb_camera
            && (hdlTina->vport.format == TR_PIXEL_MJPEG || hdlTina->vport.format == TR_PIXEL_H264)){
        hdlTina->decoderport.enable = 0;
        module_postReceiveSem(&hdlTina->decoderPort);
        if(hdlTina->decoderId)
            pthread_join(hdlTina->decoderId, NULL);
        hdlTina->decoderId = 0;
    }

	if(demoRecoder != NULL && demoRecoder->mAwEncoder != NULL){
        demoRecoder->encoderEnable = 0;
        module_postReceiveSem(&hdlTina->encoderPort);
        if(demoRecoder->encoderThreadId)
            pthread_join(demoRecoder->encoderThreadId, NULL);
        demoRecoder->encoderThreadId = 0;

        if(hdlTina->scaleDownRatio != 0){
            /* stop encoder thumb callback */
            enable = 0;
            AwEncoderSetParamete(demoRecoder->mAwEncoder,AwEncoder_SetThumbNailEnable,&enable);

            pthread_mutex_destroy(&hdlTina->mScaleCallbackLock);
        }

		AwEncoderStop(demoRecoder->mAwEncoder);
		AwEncoderDestory(demoRecoder->mAwEncoder);

        destroyStorePktInfo(hdlTina);

        pthread_mutex_destroy(&hdlTina->mEncoderPacketLock);

        free(demoRecoder->ops);
        free(demoRecoder);

        hdlTina->EncoderContext = NULL;
	}

	if(hdlTina->dispport.enable){

		hdlTina->dispport.enable = 0;
        module_postReceiveSem(&hdlTina->screenPort);
        hdlTina->dispport.setEnable(&hdlTina->dispport, 0);

        if(hdlTina->dirdispId)
            pthread_join(hdlTina->dirdispId, NULL);
        hdlTina->dirdispId = 0;

        if(hdlTina->vport.cameraType != csi_camera)
		    hdlTina->dispport.freeVideoMem(&hdlTina->dispport);

        while(!module_InputQueueEmpty(&hdlTina->screenPort)) {
            mPacket = (struct modulePacket *)module_pop(&hdlTina->screenPort);
            if(!mPacket)
                continue;

            packetDestroy(mPacket);
        }
	}

    /* stop video */
    if(needStopCapture)
		hdlTina->vport.stopcapture(&hdlTina->vport);

	clearConfigure(hdlTina);

	hdlTina->status = TINA_RECORD_STATUS_INIT;
}

static void cleanRecordStatus(TinaRecorder *hdlTina)
{
	DemoRecoderContext *demoRecoder = (DemoRecoderContext *)hdlTina->EncoderContext;
    int needStopCapture = 0;
    struct modulePacket *mPacket = NULL;

    /* just stop pushing data to encoder queue */
    modulePort_SetEnable(&hdlTina->encoderPort, 0);

    if(hdlTina->vport.enable){
        hdlTina->vport.enable = 0;
        needStopCapture = 1;
        if(hdlTina->videoId)
            pthread_join(hdlTina->videoId, NULL);
        hdlTina->videoId = 0;
    }

    if(hdlTina->aport.enable){
        hdlTina->aport.enable = 0;
        if(hdlTina->audioId)
            pthread_join(hdlTina->audioId, NULL);
        hdlTina->audioId = 0;
    }

    if(hdlTina->vport.cameraType == usb_camera
                && (hdlTina->vport.format == TR_PIXEL_MJPEG || hdlTina->vport.format == TR_PIXEL_H264)){
        hdlTina->decoderport.enable = 0;
        module_postReceiveSem(&hdlTina->decoderPort);
        if(hdlTina->decoderId)
            pthread_join(hdlTina->decoderId, NULL);
        hdlTina->decoderId = 0;
    }

	demoRecoder->encoderEnable = 0;
    module_postReceiveSem(&hdlTina->encoderPort);
    TRlog(TR_LOG_ENCODER, "wait for encoder thread to quit, queue packet num %d\n",
                                                        queueCountRead(&hdlTina->encoderPort));
    if(demoRecoder->encoderThreadId)
        pthread_join(demoRecoder->encoderThreadId, NULL);
    demoRecoder->encoderThreadId = 0;
    while(!module_InputQueueEmpty(&hdlTina->encoderPort)) {
        mPacket = (struct modulePacket *)module_pop(&hdlTina->encoderPort);
        if(!mPacket)
            continue;

        packetDestroy(mPacket);
    }

    /* stop video */
    if(needStopCapture)
		hdlTina->vport.stopcapture(&hdlTina->vport);

    hdlTina->outport.enable = 0;
    module_postReceiveSem(&hdlTina->muxerPort);
    TRlog(TR_LOG_MUXER, "wait for muxer thread to quit, queue packet num %d\n",
                                                        queueCountRead(&hdlTina->muxerPort));
	if(demoRecoder->muxerThreadId)
		pthread_join(demoRecoder->muxerThreadId, NULL);
    demoRecoder->muxerThreadId = 0;

    //clear muxer related resources
	modulePort_SetEnable(&hdlTina->muxerPort, 0);
    while(!module_InputQueueEmpty(&hdlTina->muxerPort)) {
        mPacket = (struct modulePacket *)module_pop(&hdlTina->muxerPort);
        if(!mPacket)
            continue;

        packetDestroy(mPacket);
    }

    hdlTina->mNeedMuxerVideoFlag = 0;

    destroyStorePktInfo(hdlTina);

    pthread_mutex_destroy(&hdlTina->mEncoderPacketLock);

	free(demoRecoder->ops);
	free(demoRecoder);
	hdlTina->EncoderContext = NULL;

	clearConfigure(hdlTina);

	hdlTina->status = TINA_RECORD_STATUS_INIT;
}

int Tinastop(void *hdl,int flags)
{
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;
    if(!hdlTina)
		return -1;

    int enable;
	DemoRecoderContext *demoRecoder = (DemoRecoderContext *)hdlTina->EncoderContext;
	struct modulePacket *mPacket = NULL;

	if(hdlTina->status != TINA_RECORD_STATUS_RECORDING)
		return -1;
	else{
		if(flags == FLAGS_ALL && hdlTina->startFlags == FLAGS_ALL){
			TRlog(TR_LOG, "reset all route\n");
			Tinareset(hdlTina);

            if(hdlTina->moduleLinkFlag == 0)
                TinamoduleUnlink(hdl);

			return 0;
		}else if((flags == FLAGS_ALL && hdlTina->startFlags == FLAGS_PREVIEW)
		            || (flags == FLAGS_PREVIEW && hdlTina->startFlags == FLAGS_PREVIEW)){
		    TRlog(TR_LOG, "stop preview\n");
			cleanPreviewStatus(hdlTina);

		    if(hdlTina->moduleLinkFlag == 0)
                TinamoduleUnlink(hdlTina);

			return 0;
		}else if((flags == FLAGS_ALL && hdlTina->startFlags == FLAGS_RECORDING)
		            || (flags == FLAGS_RECORDING && hdlTina->startFlags == FLAGS_RECORDING)){
		    TRlog(TR_LOG, "stop recording\n");
			cleanRecordStatus(hdlTina);

		    if(hdlTina->moduleLinkFlag == 0){
		        TinamoduleUnlink(hdlTina);
            }

			return 0;
        }else if(flags == FLAGS_PREVIEW && hdlTina->startFlags == FLAGS_ALL){
            flags = FLAGS_PREVIEW;
        }else if(flags == FLAGS_RECORDING && hdlTina->startFlags == FLAGS_ALL){
            flags = FLAGS_RECORDING;
        }else
			return -1;

		TRprint("[%s] need to stop:%d\n", __func__, flags);

		if(flags == FLAGS_PREVIEW){
            if(hdlTina->scaleDownRatio != 0){
                /* stop encoder thumb callback */
                enable = 0;
                AwEncoderSetParamete(demoRecoder->mAwEncoder,AwEncoder_SetThumbNailEnable,&enable);
            }
            /* just stop pushing data to the queue */
            modulePort_SetEnable(&hdlTina->screenPort, 0);
            while(!module_InputQueueEmpty(&hdlTina->screenPort));

            hdlTina->dispport.setEnable(&hdlTina->dispport, 0);
            hdlTina->dispport.enable = 0;
            module_postReceiveSem(&hdlTina->screenPort);

            if(hdlTina->dirdispId)
                pthread_join(hdlTina->dirdispId, NULL);
            hdlTina->dirdispId = 0;

            if(hdlTina->vport.cameraType != csi_camera)
		        hdlTina->dispport.freeVideoMem(&hdlTina->dispport);

			hdlTina->startFlags = FLAGS_RECORDING;
		}else if(flags == FLAGS_RECORDING){
            /* just stop pushing data to muxer queue */
            modulePort_SetEnable(&hdlTina->muxerPort, 0);
            hdlTina->outport.enable = 0;

            /* if scale disable, close encoder and scalse enable,encoder run */
            if(hdlTina->scaleDownRatio == 0){
                modulePort_SetEnable(&hdlTina->encoderPort, 0);
                /* sleep waiting queue is empty */
                while(!module_InputQueueEmpty(&hdlTina->encoderPort))
                    usleep(10*1000);

                //close encoder thread
                demoRecoder->encoderEnable = 0;
                module_postReceiveSem(&hdlTina->encoderPort);
                if(demoRecoder->encoderThreadId)
                    pthread_join(demoRecoder->encoderThreadId, NULL);
                demoRecoder->encoderThreadId = 0;
            }

			//close muxer thread
			module_postReceiveSem(&hdlTina->muxerPort);
            TRlog(TR_LOG_MUXER, "wait for muxer thread to quit, queue packet num %d\n",
                                                        queueCountRead(&hdlTina->muxerPort));
			if(demoRecoder->muxerThreadId)
				pthread_join(demoRecoder->muxerThreadId, NULL);
            demoRecoder->muxerThreadId = 0;

            hdlTina->mNeedMuxerVideoFlag = 0;

			//clear muxer related resources
            while(!module_InputQueueEmpty(&hdlTina->muxerPort)) {
                mPacket = (struct modulePacket *)module_pop(&hdlTina->muxerPort);
                if(!mPacket)
                    continue;

                packetDestroy(mPacket);
            }

            /* stop audio thread */
            if(hdlTina->aport.enable){
                hdlTina->aport.enable = 0;
                if(hdlTina->audioId)
                    pthread_join(hdlTina->audioId, NULL);
                hdlTina->audioId = 0;
            }

            destroyStorePktInfo(hdlTina);

			hdlTina->startFlags = FLAGS_PREVIEW;
		}
	}

	return 0;
}

int TinasetEncodeFramerate(void *hdl,int framerate)
{
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;
	if(!hdl)
		return -1;

	if(hdlTina->status == TINA_RECORD_STATUS_DATA_SOURCE_CONFIGURED)
		hdlTina->encodeFramerate = framerate;
	else
		return -1;

    return 0;
}

int TinasetVEScaleDownRatio(void *hdl,int ratio)
{
	TinaRecorder *hdlTina = (TinaRecorder *)hdl;
    if(!hdlTina)
        return -1;

    DemoRecoderContext* demoRecoder = (DemoRecoderContext*)hdlTina->EncoderContext;
	videoParam param;
	int enable = 0;
	int ret = 0;

    if(hdlTina->status == TINA_RECORD_STATUS_DATA_SOURCE_CONFIGURED)
		hdlTina->scaleDownRatio = ratio;
	else if(hdlTina->status == TINA_RECORD_STATUS_RECORDING){
        if(hdlTina->scaleDownRatio == ratio){
            TRerr("[%s] scale down ratio %d has not changed\n", __func__, ratio);
            return 0;
        }

        //get new ratio from app
		hdlTina->scaleDownRatio = ratio;
		if(ratio != 2 && ratio != 4 && ratio != 8){
			TRerr("[%s] unsupported scale down ratio!\n", __func__);
			return -1;
		}

        /* csi camera display use csi ion buf */
        if(hdlTina->vport.cameraType == csi_camera)
            return 0;

        enable = 0;
        AwEncoderSetParamete(demoRecoder->mAwEncoder,AwEncoder_SetThumbNailEnable,&enable);

        /* free disp mem */
        modulePort_SetEnable(&hdlTina->screenPort, 0);
        hdlTina->dispport.freeVideoMem(&hdlTina->dispport);

		param.srcInfo.w = ((hdlTina->encodeWidth + 15)/16*16)/hdlTina->scaleDownRatio;
		param.srcInfo.h = ((hdlTina->encodeHeight + 15)/16*16)/hdlTina->scaleDownRatio;
		param.srcInfo.format = GetDisplayFormat(hdlTina->vport.format);
		TRlog(TR_LOG_DISPLAY, "disp allocateVideoMem width %d height %d format %s\n",
													param.srcInfo.w, param.srcInfo.h,
													ReturnVideoTRFormatText(hdlTina->vport.format));

        ret = hdlTina->dispport.allocateVideoMem(&hdlTina->dispport,&param);
        if(ret < 0){
            TRerr("[%s] disp allocateVideoMem error,width %d height %d\n",
                                            __func__, param.srcInfo.w, param.srcInfo.h);
            return -1;
        }
        modulePort_SetEnable(&hdlTina->screenPort, 1);

        TinaScaleDownInit(hdlTina);
	}else
		return -1;

	return 0;
}

int TinaChangeOutputPath(void *hdl, char* path){
    TinaRecorder *hdlTina = (TinaRecorder *)hdl;

    if(!hdlTina || !path){
        TRerr("[%s] fail\n", __func__);
        return -1;
    }

    if(sizeof(hdlTina->mOutputPath) < strlen(path)){
        TRerr("[%s] output path is too long,should be less than %d\n",
                                            __func__, sizeof(hdlTina->mOutputPath));
        return -1;
    }

    strncpy(hdlTina->mOutputPath, path, MAX_PATH_LEN);

    return 0;
}

int TinaSetAudioMute(void *hdl,int muteFlag){
    TinaRecorder *hdlTina = (TinaRecorder *)hdl;

    if(!hdlTina){
        TRerr("[%s] fail\n", __func__);
        return -1;
    }

    hdlTina->mAudioMuteFlag = muteFlag;

    return 0;
}


TinaRecorder *CreateTinaRecorder()
{
	TinaRecorder *tinaRec = (TinaRecorder *)malloc(sizeof(TinaRecorder));
	if(tinaRec == NULL)
		return NULL;
	memset(tinaRec, 0, sizeof(TinaRecorder));
	tinaRec->setVideoInPort			=	TinasetVideoInPort;
	tinaRec->setAudioInPort			=	TinasetAudioInPort;
	tinaRec->setDispOutPort			=	TinasetDispOutPort;
	tinaRec->setOutput				=	TinasetOutput;
	tinaRec->start					=	Tinastart;
	tinaRec->stop					=	Tinastop;
	tinaRec->release				=	Tinarelease;
	tinaRec->reset					=	Tinareset;
	tinaRec->prepare				=	Tinaprepare;
	tinaRec->deflink				=	TinaModuleDefLink;
    tinaRec->moduleDefInit          =   TinaModuleDefInit;
	tinaRec->setVideoEncodeSize		=	TinasetEncodeSize;
	tinaRec->setVideoEncodeBitRate	=	TinasetEncodeBitRate;
	tinaRec->setOutputFormat		=	TinasetOutputFormat;
	tinaRec->setCallback			=	TinasetCallback;
	tinaRec->setEncodeVideoFormat	=	TinasetEncodeVideoFormat;
	tinaRec->setEncodeAudioFormat   =	TinasetEncodeAudioFormat;
	tinaRec->getRecorderTime        =	TinagetRecorderTime;
	tinaRec->captureCurrent         =	TinacaptureCurrent;
	tinaRec->setMaxRecordTime       =	TinasetMaxRecordTime;
	tinaRec->setParameter           =	TinasetParameter;
	tinaRec->setEncodeFramerate     =	TinasetEncodeFramerate;
	tinaRec->setVEScaleDownRatio    =	TinasetVEScaleDownRatio;
	tinaRec->changeOutputPath		=	TinaChangeOutputPath;
    tinaRec->setAudioMute           =   TinaSetAudioMute;

	return tinaRec;
}
