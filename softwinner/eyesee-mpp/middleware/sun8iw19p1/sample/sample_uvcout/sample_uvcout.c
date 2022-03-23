#define LOG_TAG "sample_uvcout"
#include <utils/plat_log.h>

#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <getopt.h>
#include <signal.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <linux/usb/ch9.h>

#include "linux/videodev2.h"
#include "./include/video.h"
#include "./include/uvc.h"
#include "media/mm_comm_vi.h"
#include "media/mpi_vi.h"
#include "media/mpi_isp.h"
#include "media/mpi_venc.h"
#include "media/mpi_sys.h"
#include "mm_common.h"
#include "mm_comm_venc.h"
#include "mm_comm_rc.h"
#include <confparser.h>
#include <mpi_videoformat_conversion.h>
#include <cdx_list.h>

//#include "aw_ai_eve_type.h"
//#include "aw_ai_eve_event_interface.h"

#include "sample_uvcout.h"
#include "sample_uvcout_config.h"

#define TIME_TEST
#ifdef TIME_TEST
#include "SystemBase.h"
#endif

//#define USE_EVE_FACE

static int g_bWaitVencDone = 0;
static int g_bSampleExit   = 0;

#define ARRAY_SIZE(a)   ((sizeof(a) / sizeof(a[0])))

static int OpenUVCDevice(SampleUVCDevice *pstUVCDev)
{
    struct v4l2_capability stCap;
    int iRet;
    int iFd;
    char pcDevName[256];

    sprintf(pcDevName, "/dev/video%d", pstUVCDev->iDev);
    iFd = open(pcDevName, O_RDWR | O_NONBLOCK);
    if (iFd < 0) {
        aloge("open video device failed: device[%s] %s\n", pcDevName, strerror(errno));
        goto open_err;
    }
    iRet = ioctl(iFd, VIDIOC_QUERYCAP, &stCap);
    if (iRet < 0) {
        aloge("unable to query device: %s (%d)\n", strerror(errno), errno);
        goto query_cap_err;
    }
    alogd("device is %s on bus %s\n", stCap.card, stCap.bus_info);

    pstUVCDev->iFd = iFd;

    return 0;

query_cap_err:
    close(iFd); 
open_err:
    return -1;
}

static void CloseUVCDevice(SampleUVCDevice *pUVCDev)
{
    close(pUVCDev->iFd);
}

ERRORTYPE VencStreamCallBack(void *cookie, MPP_CHN_S *pChn, MPP_EVENT_TYPE event, void *pEventData)
{
    VENC_STREAM_S *pFrame = (VENC_STREAM_S *)pEventData;

    switch (event) {
        case MPP_EVENT_RELEASE_VIDEO_BUFFER:
            if (pFrame != NULL) {
                g_bWaitVencDone = 1;
            }
            break;

        default:
            break;
    }

    return SUCCESS;
}

static void EveFaceCallBackFunc(void* pUsr){

}

static inline int DoUVCVideoBufProcess(SampleUVCDevice *pstUVCDev)
{
    SampleUVCContext *pConfig = (SampleUVCContext *)pstUVCDev->pPrivite;
    SampleUVCInfo *pUVCInfo = &pConfig->mUVCInfo;
    struct v4l2_buffer stBuf;
    int iRet;

    pthread_mutex_lock(&pUVCInfo->mFrmLock);
    if (list_empty(&pUVCInfo->mValidFrm)) {
//        aloge("valid frame is empty!!\n");
        iRet = -1;
        goto frm_empty;
    }

    SampleVencBuf *pstVencBuf;
    pstVencBuf = list_first_entry(&pUVCInfo->mValidFrm, SampleVencBuf, mList);

    memset(&stBuf, 0, sizeof(struct v4l2_buffer));
    stBuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    stBuf.memory = V4L2_MEMORY_MMAP;
    iRet = ioctl(pstUVCDev->iFd, VIDIOC_DQBUF, &stBuf);
    if (iRet < 0) {
        aloge("Unable to dequeue buffer: %s (%d).\n", strerror(errno), errno);
        goto dqbuf_err;
    }

    /* fill the v4l2 buffer */
    memcpy(pstUVCDev->pstFrames[stBuf.index].pVirAddr, pstVencBuf->pcData, pstVencBuf->iDataSize0);
    stBuf.bytesused = pstVencBuf->iDataSize0;

    iRet = ioctl(pstUVCDev->iFd, VIDIOC_QBUF, &stBuf);
    if (iRet < 0) {
        aloge("Unable to requeue buffer: %s (%d).\n", strerror(errno), errno);
        goto qbuf_err;
    }
qbuf_err:
dqbuf_err:
    list_move_tail(&pstVencBuf->mList, &pUVCInfo->mIdleFrm);
//  AW_MPI_VENC_ReleaseStream(pUVCInfo->iVencChn, &pstVencBuf->stStream);
frm_empty:
    pthread_mutex_unlock(&pUVCInfo->mFrmLock);
    return iRet;
}

#ifndef MAX
    #define MAX(a,b) ((a) < (b) ? (b) : (a))
#endif

#ifndef MIN
    #define MIN(a,b) ((a) > (b) ? (b) : (a))
#endif
void draw_rectangle_nv21(unsigned char *pNV21_y, unsigned char *pNV21_vu, int width, int height, int linewidth, int sx, int sy, int ex, int ey)
{
    int i, j;
    unsigned char *pY  = pNV21_y;
    unsigned char *pVU = pNV21_vu;
    unsigned char *pSrc, *pSrc1;
    int halfline = (linewidth >> 1);
    int sty, endy, stx, endx;
    int colory = 81, coloru = 90, colorv = 239;

    //top line
    sty  = MAX(sy - halfline, 0);
    endy = MIN(sy + halfline, height - 1);
    for(i = sty; i <= endy; i++)
    {
        pSrc = (unsigned char *)(pVU + (i / 2)  * width);
        pSrc1 = (unsigned char *)(pY + i * width);
        for(j = sx; j <= ex; j ++)
        {
            pSrc1[j] = colory;
            pSrc[2*(j >> 1)] = coloru;
            pSrc[2*(j >> 1) + 1] = colorv;
        }
    }

    //bottom line
    sty  = MAX(ey - halfline, 0);
    endy = MIN(ey + halfline, height - 1);
    for(i = sty; i <= endy; i++)
    {
        pSrc = (unsigned char *)(pVU + (i / 2)  * width);
        pSrc1 = (unsigned char *)(pY + i * width);
        for(j = sx; j <= ex; j ++)
        {
            pSrc1[j] = colory;
            pSrc[2*(j >> 1)] = coloru;
            pSrc[2*(j >> 1) + 1] = colorv;
        }
    }

    //left line
    stx  = MAX(sx - halfline, 0);
    endx = MIN(sx + halfline, width - 1);
    sty  = MAX(sy - halfline, 0);
    endy = MIN(ey + halfline, height - 1);
    for(i = sty; i <= endy; i++)
    {
        pSrc = (unsigned char *)(pVU + (i / 2)  * width);
        pSrc1 = (unsigned char *)(pY + i * width);
        for(j = stx; j <= endx; j ++)
        {
            pSrc1[j] = colory;
            pSrc[2*(j >> 1)] = coloru;
            pSrc[2*(j >> 1) + 1] = colorv;
        }
    }

    //right line
    stx  = MAX(ex - halfline, 0);
    endx = MIN(ex + halfline, width - 1);

    for(i = sty; i <= endy; i++)
    {
        pSrc = (unsigned char *)(pVU + (i / 2)  * width);
        pSrc1 = (unsigned char *)(pY + i * width);
        for(j = stx; j <= endx; j ++)
        {
            pSrc1[j] = colory;
            pSrc[2*(j >> 1)] = coloru;
            pSrc[2*(j >> 1) + 1] = colorv;
        }
    }
}

static void *Vipp2VencThread(void *pArg)
{
    SampleUVCContext *pConfig = (SampleUVCContext *)pArg;
    SampleUVCInfo *pUVCInfo = &pConfig->mUVCInfo;
    int iVippDev = pUVCInfo->iVippDev;
    int iVippChn = pUVCInfo->iVippChn;

    int iRet = 0;
    int iEncChn = pConfig->mUVCInfo.iVencChn;
    VENC_STREAM_S stVencStream;
    VENC_PACK_S stPack;

    stVencStream.mPackCount = 1;
    stVencStream.mpPack     = &stPack;

    alogd("begin to receive venc frames, vencchn[%d], vippdev[%d]\n", iEncChn, iVippDev);
    int i = 0, j = 0, sx = 0, sy = 0, ex = 0, ey = 0;

    VIDEO_FRAME_INFO_S stFrameInfo;

    while(1) {
//      pthread_testcancel();
        if (g_bSampleExit) {
            break;
        }

        iRet = AW_MPI_VENC_GetStream(iEncChn, &stVencStream, 4000);
        if (iRet < 0) {
            aloge("get first frame failed!!\n");
            goto get_stream_err;
        }

        pthread_mutex_lock(&pUVCInfo->mFrmLock);
        SampleVencBuf *pstVencBufTmp;
        if (list_empty(&pUVCInfo->mIdleFrm)) {
            goto idle_list_empty;
        }
        pstVencBufTmp = list_first_entry(&pUVCInfo->mIdleFrm, SampleVencBuf, mList);

        if(stVencStream.mpPack != NULL && stVencStream.mpPack->mLen0) {
            pstVencBufTmp->iDataSize0 = stVencStream.mpPack->mLen0;
            memcpy(pstVencBufTmp->pcData, stVencStream.mpPack->mpAddr0, stVencStream.mpPack->mLen0);
        }
        if(stVencStream.mpPack != NULL && stVencStream.mpPack->mLen1) {
            pstVencBufTmp->iDataSize1 = stVencStream.mpPack->mLen1;
            memcpy(pstVencBufTmp->pcData, stVencStream.mpPack->mpAddr1, stVencStream.mpPack->mLen1);
        }

        list_move_tail(&pstVencBufTmp->mList, &pUVCInfo->mValidFrm);

idle_list_empty:
        pthread_mutex_unlock(&pUVCInfo->mFrmLock);
        iRet = AW_MPI_VENC_ReleaseStream(iEncChn, &stVencStream);
        if (iRet < 0) {
            aloge("release stream failed!!\n");
        }
get_stream_err:
process:
        iRet = iRet;
    }

    alogd("Vipp2VencThread exit\n");
    pthread_exit(NULL);
}

#if (SUPPORT_EVE!=0)
static void *Vipp2EveFaceThread(void *pArg)
{
    int iRet = 0;
    SampleUVCContext *pConfig = (SampleUVCContext *)pArg;
    SampleUVCInfo *pUVCInfo = &pConfig->mUVCInfo;
    int iFaceVippDev = pUVCInfo->iFaceVippDev;
    int iFaceVippChn = pUVCInfo->iFaceVippChn;

    alogd("begin to process eve face thread, vipp[%d]\n", iFaceVippDev);
    int i = 0;

    VIDEO_FRAME_INFO_S stFrameInfo;
    AW_IMAGE_S         stEveImage;
    AW_STATUS_E        eStatus;
    AW_AI_EVE_EVENT_RESULT_S *pstEveResult = &pUVCInfo->stEveRes;
    AW_HANDLE pEveHd = pUVCInfo->pEveHd;
    int iCurPts = 0;
    while(1) {
//      pthread_testcancel();
        if (g_bSampleExit) {
            break;
        }

        iRet = AW_MPI_VI_GetFrame(iFaceVippDev, iFaceVippChn, &stFrameInfo, 60);
        if (iRet < 0) {
            aloge("Vipp2EveFaceThread get one frame failed!!\n");
            goto process;
        }

        /* set image attribution */
        stEveImage.mPixelFormat = stFrameInfo.VFrame.mPixelFormat;
        stEveImage.mWidth       = stFrameInfo.VFrame.mWidth;
        stEveImage.mHeight      = stFrameInfo.VFrame.mHeight;
        stEveImage.mStride[0]   = stFrameInfo.VFrame.mWidth;
        stEveImage.mpVirAddr[0] = stFrameInfo.VFrame.mpVirAddr[0];
        stEveImage.mpVirAddr[1] = stFrameInfo.VFrame.mpVirAddr[1];
        stEveImage.mpVirAddr[2] = stFrameInfo.VFrame.mpVirAddr[2];
        stEveImage.mPhyAddr[0]  = stFrameInfo.VFrame.mPhyAddr[0];
        stEveImage.mPhyAddr[1]  = stFrameInfo.VFrame.mPhyAddr[1];
        stEveImage.mPhyAddr[2]  = stFrameInfo.VFrame.mPhyAddr[2];
//        stFrameInfo.VFrame.mpts;
        iCurPts += (1*1000*1000) / 25;
        iCurPts = 0;

        eStatus = AW_AI_EVE_Event_SetEveSourceAddress(pEveHd, (void *)stEveImage.mPhyAddr[0]);
        if (AW_STATUS_OK != eStatus) {
            aloge("Do AW_AI_EVE_Event_SetEveSourceAddress fail!  ret:0x%x\n", eStatus);
            goto eve_set_source_err;
        }

//        pthread_mutex_lock(&pUVCInfo->mEveFaceResLock);
        eStatus = AW_AI_EVE_Event_Process(pEveHd, &stEveImage, iCurPts, pstEveResult);
        if (AW_STATUS_OK != eStatus) {
            aloge("Do AW_AI_EVE_Event_Process fail!  ret:0x%x\n", eStatus);
            goto eve_process_err;
        }

eve_process_err:
//        pthread_mutex_unlock(&pUVCInfo->mEveFaceResLock);
eve_set_source_err:
        AW_MPI_VI_ReleaseFrame(iFaceVippDev, iFaceVippChn, &stFrameInfo);
process:
        iRet = iRet;
    }

    alogd("Vipp2EveFaceThread exit\n");
    pthread_exit(NULL);
}
#endif

static int InitFrameList(int iFrmNum, SampleUVCContext *pConfig)
{
    SampleUVCInfo *pInfo = &pConfig->mUVCInfo;
    unsigned int iFrameSize = pConfig->iCapWidth * pConfig->iCapHeight * 3 / 2;

    alogd("begin to alloc frame list.\n");
    if (iFrmNum <= 0) {
        aloge("frame list number must bigger than 0!!\n");
        return -1;
    }

    INIT_LIST_HEAD(&pInfo->mIdleFrm);
    INIT_LIST_HEAD(&pInfo->mValidFrm);
    INIT_LIST_HEAD(&pInfo->mUsedFrm);

    SampleVencBuf *pBufTmp;
    pthread_mutex_init(&pInfo->mFrmLock, NULL);
    for (int i = 0; i < iFrmNum; i++) {
        pBufTmp = malloc(sizeof(SampleVencBuf));
        pBufTmp->pcData = malloc(iFrameSize);
        list_add_tail(&pBufTmp->mList, &pInfo->mIdleFrm);
    }
  #if (SUPPORT_EVE!=0)
    pthread_mutex_init(&pInfo->mEveFaceResLock, NULL);
  #endif

    return 0;
}

static void DeinitFrameList(SampleUVCContext *pConfig)
{
    SampleUVCInfo *pInfo = &pConfig->mUVCInfo;
  #if (SUPPORT_EVE!=0)
    pthread_mutex_destroy(&pInfo->mEveFaceResLock);
  #endif

    alogd("begin to free frame list.\n");
    SampleVencBuf *pBufTmp;
    SampleVencBuf *pBufTmpNext;
    list_for_each_entry_safe(pBufTmp, pBufTmpNext, &pInfo->mUsedFrm, mList)
    {
        list_del(&pBufTmp->mList);
        free(pBufTmp->pcData);
        free(pBufTmp);
    }
    list_for_each_entry_safe(pBufTmp, pBufTmpNext, &pInfo->mValidFrm, mList)
    {
        list_del(&pBufTmp->mList);
        free(pBufTmp->pcData);
        free(pBufTmp);
    }
    list_for_each_entry_safe(pBufTmp, pBufTmpNext, &pInfo->mIdleFrm, mList)
    {
        list_del(&pBufTmp->mList);
        free(pBufTmp->pcData);
        free(pBufTmp);
    }

    pthread_mutex_destroy(&pInfo->mFrmLock);
}

static int CreateVi2VencThread(SampleUVCContext *pConfig)
{
    int iRet;

    MPP_SYS_CONF_S mSysConf;
    mSysConf.nAlignWidth = 32;
    AW_MPI_SYS_SetConf(&mSysConf);
    iRet = AW_MPI_SYS_Init();
    if (iRet < 0) {
        aloge("AW_MPI_SYS_Init failed!");
        goto sys_init_err;
    }

    /********************************/
    /*  create vipp device channel  */
    /********************************/
    VI_ATTR_S stVippAttr;
    int iVippDev;
    iVippDev = pConfig->iCapDev;
    /*Set VI Channel Attribute*/
    memset(&stVippAttr, 0, sizeof(VI_ATTR_S));
    stVippAttr.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    stVippAttr.memtype = V4L2_MEMORY_MMAP;
    stVippAttr.format.pixelformat = map_PIXEL_FORMAT_E_to_V4L2_PIX_FMT(pConfig->eCapFormat);
    stVippAttr.format.field = V4L2_FIELD_NONE;
    stVippAttr.format.width = pConfig->iCapWidth;
    stVippAttr.format.height = pConfig->iCapHeight;
    stVippAttr.fps = pConfig->iCapFrameRate;
    /* update configuration anyway, do not use current configuration */
    stVippAttr.use_current_win = 0;
    stVippAttr.wdr_mode = 0; // do not use wdr mode
    stVippAttr.nbufs = 5;
    stVippAttr.nplanes = 2;
    iRet = AW_MPI_VI_CreateVipp(iVippDev);
    if (iRet < 0) {
        aloge("create vipp device failed!! vipp_dev[%d]\n", iVippDev);
        goto create_vipp_err;
    }
    iRet = AW_MPI_VI_SetVippAttr(iVippDev, &stVippAttr);
    if (iRet < 0) {
        aloge("set vipp device attribution failed!! vipp_dev[%d]\n", iVippDev);
        goto set_vipp_attr_err;
    }
    iRet = AW_MPI_VI_EnableVipp(iVippDev);
    if (iRet < 0) {
        aloge("enable vipp device attribution failed!! vipp_dev[%d]\n", iVippDev);
        goto enable_err;
    }
    int iVippChn = 0;
    iRet = AW_MPI_VI_CreateVirChn(iVippDev, iVippChn, NULL);
    if (iRet < 0) {
        aloge("create vipp channle failed!! vipp_dev[%d],vipp_chn[%d]\n", iVippDev, iVippChn);
        goto create_vipp_chn_err;
    }

    /* open isp */
    int iIspDev = 0;
//    if (iVippDev == 0 || iVippDev == 2) {
//        iIspDev = 1;
//    } else if (iVippDev == 1 || iVippDev == 3) {
//        iIspDev = 0;
//    }

    /* MPP components */
    int iVencChn = 0;
    VENC_CHN_ATTR_S stVEncChnAttr;
    memset(&stVEncChnAttr, 0, sizeof(VENC_CHN_ATTR_S));
    stVEncChnAttr.VeAttr.Type         = pConfig->eEncoderType;
    stVEncChnAttr.VeAttr.SrcPicWidth  = pConfig->iCapWidth;
    stVEncChnAttr.VeAttr.SrcPicHeight = pConfig->iCapHeight;
    stVEncChnAttr.VeAttr.Field        = VIDEO_FIELD_FRAME;
    stVEncChnAttr.VeAttr.PixelFormat  = pConfig->eCapFormat;
    stVEncChnAttr.VeAttr.Rotate       = ROTATE_NONE;
    if(PT_MJPEG == stVEncChnAttr.VeAttr.Type)
    {
        stVEncChnAttr.VeAttr.AttrMjpeg.mbByFrame  = TRUE;
        stVEncChnAttr.VeAttr.AttrMjpeg.mPicWidth  = stVEncChnAttr.VeAttr.SrcPicWidth;
        stVEncChnAttr.VeAttr.AttrMjpeg.mPicHeight = stVEncChnAttr.VeAttr.SrcPicHeight;
        stVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_MJPEGCBR;
        stVEncChnAttr.RcAttr.mAttrMjpegeCbr.mSrcFrmRate = pConfig->iCapFrameRate;
        stVEncChnAttr.RcAttr.mAttrMjpegeCbr.fr32DstFrmRate = pConfig->iCapFrameRate;
        stVEncChnAttr.RcAttr.mAttrMjpegeCbr.mBitRate = pConfig->iEncBitRate;
    } else if (PT_JPEG == stVEncChnAttr.VeAttr.Type) {
        stVEncChnAttr.VeAttr.AttrJpeg.bByFrame   = TRUE;
        stVEncChnAttr.VeAttr.AttrJpeg.PicWidth   = pConfig->iCapWidth;;
        stVEncChnAttr.VeAttr.AttrJpeg.PicHeight  = pConfig->iCapHeight;
        //stVEncChnAttr.VeAttr.AttrJpeg.BufSize    = ((((pConfig->iCapWidth * pConfig->iCapHeight * 3) >> 2) + 1023) >> 10) << 10;
        stVEncChnAttr.VeAttr.AttrJpeg.BufSize    = 16 * 1024 * 1024;
        stVEncChnAttr.VeAttr.AttrJpeg.MaxPicWidth  = 0;
        stVEncChnAttr.VeAttr.AttrJpeg.MaxPicHeight = 0;
        stVEncChnAttr.VeAttr.AttrJpeg.bSupportDCF  = FALSE;
#if 1
        stVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_MJPEGFIXQP;
        stVEncChnAttr.RcAttr.mAttrMjpegeFixQp.mSrcFrmRate = pConfig->iCapFrameRate;
        stVEncChnAttr.RcAttr.mAttrMjpegeFixQp.mQfactor    = 99;
        stVEncChnAttr.RcAttr.mAttrMjpegeFixQp.fr32DstFrmRate = pConfig->iEncBitRate;
#endif
    } else {
        aloge("you should use JPEG encoder!!\n");
        goto unknow_enc_type;
    }

    iRet = AW_MPI_VENC_CreateChn(iVencChn, &stVEncChnAttr);
    if (iRet < 0) {
        aloge("create venc channle failed!! venc_chn[%d]\n", iVencChn);
        goto create_venc_chn_err;
    }
    VENC_FRAME_RATE_S stVencFrameRate;
    stVencFrameRate.SrcFrmRate = pConfig->iCapFrameRate;
    stVencFrameRate.DstFrmRate = pConfig->iEncFrameRate;
    iRet = AW_MPI_VENC_SetFrameRate(iVencChn, &stVencFrameRate);
    if (iRet < 0) {
        aloge("set venc channle frame rate failed!! venc_chn[%d]\n", iVencChn);
        goto set_venc_chn_attr_err;
    }
    if(PT_JPEG == stVEncChnAttr.VeAttr.Type) {
        VENC_PARAM_JPEG_S stJpegParam;
        memset(&stJpegParam, 0, sizeof(VENC_PARAM_JPEG_S));
        stJpegParam.Qfactor = pConfig->iEncQuality;
        AW_MPI_VENC_SetJpegParam(iVencChn, &stJpegParam);
    }

    MPP_CHN_S stVippChn = {MOD_ID_VIU, iVippDev, iVippChn};
    MPP_CHN_S stVencChn = {MOD_ID_VENC, 0, iVencChn};
    if (pConfig->bUseEve) {
        MPPCallbackInfo stVencCallback;
        stVencCallback.callback = VencStreamCallBack;
        stVencCallback.cookie   = (void *)&pConfig;
        AW_MPI_VENC_RegisterCallback(iVencChn, &stVencCallback);
    } else {
        /********************************/
        /*  start vipp&venc  streaming  */
        /********************************/
        iRet = AW_MPI_SYS_Bind(&stVippChn,&stVencChn);
        if(iRet != SUCCESS)
        {
            aloge("bind vipp_chn[%d]&venc_chn[%d] failed!!\n", iVippDev, iVencChn);
            goto bind_vipp_venc_err;
        }
    }

    pConfig->mUVCInfo.iVippDev = iVippDev;
    pConfig->mUVCInfo.iVippChn = iVippChn;
    pConfig->mUVCInfo.iVencChn = iVencChn;
    pConfig->mUVCInfo.iIspDev  = iIspDev;
    alogd("initialize vi2venc success, videv[%d],vichn[%d],vencchn[%d],ispdev[%d]\n",
        iVippDev, iVippChn, iVencChn, iIspDev);

    return 0;

    if (!pConfig->bUseEve) {
        AW_MPI_SYS_UnBind(&stVippChn,&stVencChn);
    }
bind_vipp_venc_err:
set_venc_chn_attr_err:
    AW_MPI_VENC_DestroyChn(iVencChn);
create_venc_chn_err:
unknow_enc_type:
    AW_MPI_VI_DestoryVirChn(iVippDev, iVippChn);
create_vipp_chn_err:
    AW_MPI_VI_DisableVipp(iVippDev);
enable_err:
set_vipp_attr_err:
    AW_MPI_VI_DestoryVipp(iVippDev);
create_vipp_err:
    AW_MPI_SYS_Exit();
sys_init_err:
    return iRet;
}

static int DestroyVi2VencThread(SampleUVCContext *pConfig)
{
    int iRet;

    if (pConfig->bUseEve) {
        MPP_CHN_S stVippChn = {MOD_ID_VIU, pConfig->mUVCInfo.iVippDev, pConfig->mUVCInfo.iVippChn};
        MPP_CHN_S stVencChn = {MOD_ID_VENC, 0, pConfig->mUVCInfo.iVencChn};
        AW_MPI_SYS_UnBind(&stVippChn,&stVencChn);
    }
    AW_MPI_VENC_DestroyChn(pConfig->mUVCInfo.iVencChn);
    AW_MPI_VI_DestoryVirChn(pConfig->mUVCInfo.iVippDev, pConfig->mUVCInfo.iVippChn);
    AW_MPI_VI_DisableVipp(pConfig->mUVCInfo.iVippDev);
    AW_MPI_VI_DestoryVipp(pConfig->mUVCInfo.iVippDev);

    iRet = AW_MPI_SYS_Exit();
    if (iRet < 0) {
        aloge("AW_MPI_SYS_Exit failed!!\n");
    }
    return iRet;
}

#if (SUPPORT_EVE!=0)
static int CreateVi2EveFaceThread(SampleUVCContext *pConfig)
{
    int iRet;

    MPP_SYS_CONF_S mSysConf;
    mSysConf.nAlignWidth = 32;
    AW_MPI_SYS_SetConf(&mSysConf);
    iRet = AW_MPI_SYS_Init();
    if (iRet < 0) {
        aloge("AW_MPI_SYS_Init failed!");
        goto sys_init_err;
    }

    /********************************/
    /*  create vipp device channel  */
    /********************************/
    VI_ATTR_S stVippAttr;
    int iVippDev;
    iVippDev = (pConfig->iCapDev + 2) % 4; /* vipp0<->vipp2, vipp1<->vipp3 */
    /*Set VI Channel Attribute*/
    memset(&stVippAttr, 0, sizeof(VI_ATTR_S));
    stVippAttr.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    stVippAttr.memtype = V4L2_MEMORY_MMAP;
    stVippAttr.format.pixelformat = map_PIXEL_FORMAT_E_to_V4L2_PIX_FMT(pConfig->eCapFormat);
    stVippAttr.format.field = V4L2_FIELD_NONE;
    stVippAttr.format.width = 640;         /* for eve face */
    stVippAttr.format.height = 360;
    stVippAttr.fps = pConfig->iCapFrameRate;
    /* update configuration anyway, do not use current configuration */
    stVippAttr.use_current_win = 0;
    stVippAttr.wdr_mode = 0; // do not use wdr mode
    stVippAttr.nbufs = 5;
    stVippAttr.nplanes = 2;
    iRet = AW_MPI_VI_CreateVipp(iVippDev);
    if (iRet < 0) {
        aloge("create vipp device failed!! vipp_dev[%d]\n", iVippDev);
        goto create_vipp_err;
    }
    iRet = AW_MPI_VI_SetVippAttr(iVippDev, &stVippAttr);
    if (iRet < 0) {
        aloge("set vipp device attribution failed!! vipp_dev[%d]\n", iVippDev);
        goto set_vipp_attr_err;
    }
    iRet = AW_MPI_VI_EnableVipp(iVippDev);
    if (iRet < 0) {
        aloge("enable vipp device attribution failed!! vipp_dev[%d]\n", iVippDev);
        goto enable_err;
    }
    int iVippChn = 0;
    iRet = AW_MPI_VI_CreateVirChn(iVippDev, iVippChn, NULL);
    if (iRet < 0) {
        aloge("create vipp channle failed!! vipp_dev[%d],vipp_chn[%d]\n", iVippDev, iVippChn);
        goto create_vipp_chn_err;
    }

    /* open isp */
    int iIspDev = 0;
    if (iVippDev == 0 || iVippDev == 2) {
        iIspDev = 1;
    } else if (iVippDev == 1 || iVippDev == 3) {
        iIspDev = 0;
    }

    /* Initialize eve face module */
    AW_AI_EVE_CTRL_S stEVECtrl;
    memset(&stEVECtrl, 0, sizeof(AW_AI_EVE_CTRL_S));
    stEVECtrl.addrInputType  = AW_AI_EVE_ADDR_INPUT_PHY; //ÊäÈëÎïÀíµØÖ·
    stEVECtrl.scale_factor   = 1;
    stEVECtrl.mScanStageNo   = 10;
    stEVECtrl.yStep          = 3;
    stEVECtrl.xStep0         = 1;
    stEVECtrl.xStep1         = 4;
    stEVECtrl.mMidRltStageNo = 10;
    stEVECtrl.mMidRltNum     = 0;
    stEVECtrl.mRltNum        = AW_AI_EVE_MAX_RESULT_NUM;
    stEVECtrl.rltType        = AW_AI_EVE_RLT_OUTPUT_DETAIL;

    stEVECtrl.mDmaOut.s16Width              = 640;
    stEVECtrl.mDmaOut.s16Height             = 360;
    stEVECtrl.mPyramidLowestLayel.s16Width  = 640;
    stEVECtrl.mPyramidLowestLayel.s16Height = 360;
    stEVECtrl.dmaSrcSize.s16Width           = 640;
    stEVECtrl.dmaSrcSize.s16Height          = 360;
    stEVECtrl.dmaDesSize.s16Width           = 640;
    stEVECtrl.dmaDesSize.s16Height          = 360;
    stEVECtrl.dmaRoi.s16X                   = 0;
    stEVECtrl.dmaRoi.s16Y                   = 0;
    stEVECtrl.dmaRoi.s16Width               = stEVECtrl.dmaDesSize.s16Width;
    stEVECtrl.dmaRoi.s16Height              = stEVECtrl.dmaDesSize.s16Height;

    FILE *fEveConf;
    int iTemp;
    static AW_S8    *awKeyNew = (AW_S8*)"1111111111111111"; //key
    fEveConf = fopen("/usr/share/eve.conf", "r");///usr/share/eve.conf  /mnt/extsd/
    if(fEveConf == NULL) {
        aloge("EVE cfg file is not exist! So use defualt config param.\n");

        stEVECtrl.classifierNum = 8;
        stEVECtrl.classifierPath[0].path = (AW_S8*)"/usr/share/classifier/frontface.ld";
        stEVECtrl.classifierPath[0].key  = (AW_U8*)awKeyNew;
        stEVECtrl.classifierPath[1].path = (AW_S8*)"/usr/share/classifier/fullprofleftface.ld";
        stEVECtrl.classifierPath[1].key  = (AW_U8*)awKeyNew;
        stEVECtrl.classifierPath[2].path = (AW_S8*)"/usr/share/classifier/fullprofrightface.ld";
        stEVECtrl.classifierPath[2].key  = (AW_U8*)awKeyNew;
        stEVECtrl.classifierPath[3].path = (AW_S8*)"/usr/share/classifier/halfdownface.ld";
        stEVECtrl.classifierPath[3].key  = (AW_U8*)awKeyNew;
        stEVECtrl.classifierPath[4].path = (AW_S8*)"/usr/share/classifier/profileface.ld";
        stEVECtrl.classifierPath[4].key  = (AW_U8*)awKeyNew;
        stEVECtrl.classifierPath[5].path = (AW_S8*)"/usr/share/classifier/rotleftface.ld";
        stEVECtrl.classifierPath[5].key  = (AW_U8*)awKeyNew;
        stEVECtrl.classifierPath[6].path = (AW_S8*)"/usr/share/classifier/rotrightface.ld";
        stEVECtrl.classifierPath[6].key  = (AW_U8*)awKeyNew;
        stEVECtrl.classifierPath[7].path = (AW_S8*)"/usr/share/classifier/smallface.ld";
        stEVECtrl.classifierPath[7].key  = (AW_U8*)awKeyNew;

    } else {
        fscanf(fEveConf, "%d", (int *)&stEVECtrl.classifierNum);
        alogd("face clasifier num = %d\n", stEVECtrl.classifierNum);
        for(int i = 0; i < stEVECtrl.classifierNum; i++)
        {
            stEVECtrl.classifierPath[i].path = malloc(256);
            stEVECtrl.classifierPath[i].key  = (AW_U8*)awKeyNew;
            fscanf(fEveConf, "%s", stEVECtrl.classifierPath[i].path);
            alogd("classifier : %s\n", stEVECtrl.classifierPath[i].path);
        }
        fscanf(fEveConf, "%d\n", &iTemp);
        stEVECtrl.scale_factor = iTemp;
        fscanf(fEveConf, "%d\n", &iTemp);
        stEVECtrl.mScanStageNo = iTemp;
        fscanf(fEveConf, "%d\n", &iTemp);
        stEVECtrl.yStep = iTemp;
        fscanf(fEveConf, "%d\n", &iTemp);
        stEVECtrl.xStep0 = iTemp;
        fscanf(fEveConf, "%d\n", &iTemp);
        stEVECtrl.xStep1 = iTemp;
        fscanf(fEveConf, "%d\n", &iTemp);
        stEVECtrl.mMidRltNum = iTemp;
        fscanf(fEveConf, "%d\n", &iTemp);
        stEVECtrl.mMidRltStageNo = iTemp;

        alogd("scale_factor = %d, mScanStageNo = %d, yStep = %d, xStep0 = %d, xStep1 = %d, mMidRltNum = %d, mMidRltStageNo = %d\n",
                stEVECtrl.scale_factor, stEVECtrl.mScanStageNo,stEVECtrl.yStep,stEVECtrl.xStep0,
                stEVECtrl.xStep1, stEVECtrl.mMidRltNum, stEVECtrl.mMidRltStageNo);
    }

    stEVECtrl.dmaCallBackFunc   = &EveFaceCallBackFunc;
    pConfig->mUVCInfo.pEveHd = AW_AI_EVE_Event_Init(&stEVECtrl);
    iRet = AW_AI_EVE_Event_SetEveDMAExecuteMode(pConfig->mUVCInfo.pEveHd, AW_AI_EVE_DMA_EXECUTE_SYNC);
    if (AW_STATUS_OK != iRet)
    {
        aloge("Do AW_AI_EVE_Event_SetEveDMAExecuteMode fail! status:%d \n", iRet);
        goto eve_set_dmamode_err;
    }

    AW_AI_EVE_EVENT_FACEDET_PARAM_S stFaceParam;
    memset(&stFaceParam, 0, sizeof(AW_AI_EVE_EVENT_FACEDET_PARAM_S));
    stFaceParam.s32ClassifyFlag           = 0;
    stFaceParam.sRoiSet.s32RoiNum         = 1;
    stFaceParam.sRoiSet.sID[0]            = 1;
    stFaceParam.sRoiSet.sRoi[0].s16Top    = 0;
    stFaceParam.sRoiSet.sRoi[0].s16Bottom = 360  - 5;
    stFaceParam.sRoiSet.sRoi[0].s16Left   = 0;
    stFaceParam.sRoiSet.sRoi[0].s16Right  = 640  - 5;
    stFaceParam.s32ClassifyFlag           = 0; //close
    stFaceParam.s32MinFaceSize            = 20;
    stFaceParam.s32MaxFaceNum             = 16;
    stFaceParam.s32OverLapCoeff           = 20;
    stFaceParam.s32MergeThreshold         = 3;
    stFaceParam.s8Cfgfile                 = AW_NULL;//"/usr/share/classifier/face_classifier_24X24.cfg";
    stFaceParam.s8Weightfile              = AW_NULL;//"/usr/share/classifier/face_classifier_24X24.cweights";

    AW_STATUS_E eStatus;
    eStatus = AW_AI_EVE_Event_SetEventParam(pConfig->mUVCInfo.pEveHd, AW_AI_EVE_EVENT_FACEDETECT, (void *)&stFaceParam);
    if (AW_STATUS_OK != eStatus)
    {
        aloge("Do AW_AI_EVE_Event_SetEventParam fail! status:%d \n", eStatus);
        goto eve_set_eventparam_err;
    }

    pConfig->mUVCInfo.iFaceVippDev = iVippDev;
    pConfig->mUVCInfo.iFaceVippChn = iVippChn;
    pConfig->mUVCInfo.iFaceIspDev  = iIspDev;
    alogd("create vi2eveface success, videv[%d],vichn[%d],vencchn[%d],ispdev[%d]\n",
        iVippDev, iVippChn, iIspDev);

    return 0;

eve_set_eventparam_err:
eve_set_dmamode_err:
//	AW_AI_EVE_Event_UnInit(pConfig->mUVCInfo.pEveHd);
    AW_MPI_VI_DestoryVirChn(iVippDev, iVippChn);
create_vipp_chn_err:
    AW_MPI_VI_DisableVipp(iVippDev);
enable_err:
set_vipp_attr_err:
    AW_MPI_VI_DestoryVipp(iVippDev);
create_vipp_err:
    AW_MPI_SYS_Exit();
sys_init_err:
    return iRet;
}

static int DestroyVi2EveFaceThread(SampleUVCContext *pConfig)
{
    int iRet = 0;

//	AW_AI_EVE_Event_UnInit(pConfig->mUVCInfo.pEveHd);
    AW_MPI_VI_DestoryVirChn(pConfig->mUVCInfo.iFaceVippDev, pConfig->mUVCInfo.iFaceVippChn);
    AW_MPI_VI_DisableVipp(pConfig->mUVCInfo.iFaceVippDev);
    AW_MPI_VI_DestoryVipp(pConfig->mUVCInfo.iFaceVippDev);
#if 0
	/* do not exit MPI_SYS here, but in DestroyVi2VencThread */
    iRet = AW_MPI_SYS_Exit();
    if (iRet < 0) {
        aloge("AW_MPI_SYS_Exit failed!!\n");
    }
#endif
    return iRet;
}
#endif

static int StartProcessing(SampleUVCContext *pConfig)
{
    int iRet = 0;

    AW_MPI_ISP_Init();
    AW_MPI_ISP_Run(pConfig->mUVCInfo.iIspDev);
    AW_MPI_VI_EnableVirChn(pConfig->mUVCInfo.iVippDev, pConfig->mUVCInfo.iVippChn);
    AW_MPI_VENC_StartRecvPic(pConfig->mUVCInfo.iVippChn);
    iRet = pthread_create(&pConfig->mUVCInfo.tEncTrd, NULL, Vipp2VencThread, pConfig);
    if (iRet < 0) {
        aloge("caeate Vipp2VencThread failed!!\n");
        goto create_vipp2venc_err;
    }

create_vipp2venc_err:
create_vipp2eveface_err:
    return iRet;
}

static int StopProcessing(SampleUVCContext *pConfig)
{
    pthread_join(pConfig->mUVCInfo.tEncTrd, NULL);
    pthread_join(pConfig->mUVCInfo.tFaceTrd, NULL);

	alogd("all thread has exit\n");
    AW_MPI_VENC_StopRecvPic(pConfig->mUVCInfo.iVencChn);
    AW_MPI_VI_DisableVirChn(pConfig->mUVCInfo.iVippDev, pConfig->mUVCInfo.iVippChn);
    AW_MPI_VENC_ResetChn(pConfig->mUVCInfo.iVencChn);
  #if (SUPPORT_EVE!=0)
    AW_MPI_VI_DisableVirChn(pConfig->mUVCInfo.iFaceVippDev, pConfig->mUVCInfo.iFaceVippChn);
  #endif

    AW_MPI_ISP_Stop(pConfig->mUVCInfo.iIspDev);
  #if (SUPPORT_EVE!=0)
    AW_MPI_ISP_Stop(pConfig->mUVCInfo.iFaceIspDev);
  #endif
    AW_MPI_ISP_Exit();

    return 0;
}

#if 0
static int CreateUVCThread(SampleUVCContext *pConfig)
{
    int iRet;

    iRet = pthread_create(&pConfig->mUVCInfo.tUVCTrd, NULL, SendPic2UVCThread, pConfig);
    if (iRet < 0) {
        aloge("create UVC thread failed!!");
        goto create_uvctrd_err;
    }

create_uvctrd_err:
    return iRet;
}

static void DestroyUVCThread(SampleUVCContext *pConfig)
{
//  pthread_cancel(pConfig->mUVCInfo.tUVCTrd);
    pthread_join(pConfig->mUVCInfo.tUVCTrd, NULL);
}
#endif

static SampleUVCFormat g_pstFormat[] = {
    {
        .iWidth  = 1920,
        .iHeight = 1080,
        .iFormat = V4L2_PIX_FMT_MJPEG,
        .iInterval = 333333, //units: 100ns
    },
    {
        .iWidth  = 1920,
        .iHeight = 1080,
        .iFormat = V4L2_PIX_FMT_MJPEG,
        .iInterval = 400000, //units: 100ns
    },
    {
        .iWidth  = 1280,
        .iHeight = 720,
        .iFormat = V4L2_PIX_FMT_MJPEG,
        .iInterval = 333333, //units: 100ns
    },
    {
        .iWidth  = 1280,
        .iHeight = 720,
        .iFormat = V4L2_PIX_FMT_MJPEG,
        .iInterval = 400000, //units: 100ns
    },
};

static int UVCVideoSetFormat(SampleUVCDevice *pstUVCDev)
{
    SampleUVCContext *pstConfig = (SampleUVCContext*)pstUVCDev->pPrivite;
    int iRet;
#if 0
    alogd("iCapFrameRate=[%d]\n", pstConfig->iCapFrameRate);
    struct v4l2_streamparm stParam;
    memset(&stParam, 0, sizeof(struct v4l2_streamparm));
    stParam.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    stParam.parm.output.timeperframe.numerator = 1;
    stParam.parm.output.timeperframe.denominator = pstConfig->iCapFrameRate;
    stParam.parm.output.outputmode = V4L2_MODE_IMAGE;
    stParam.parm.output.reserved[0] = 0;
    iRet = ioctl(pstUVCDev->iFd, VIDIOC_S_PARM, &stParam);
    if (iRet < 0) {
        aloge("VIDIOC_S_PARAM failed!! %s (%d).\n", strerror(errno), errno);
    }
#endif
    alogd("iWidth=[%d],iHeight=[%d],iFormat=[0x%08x]\n",
        pstUVCDev->iWidth, pstUVCDev->iHeight, pstUVCDev->iFormat);
    struct v4l2_format stFormat;
    memset(&stFormat, 0, sizeof(struct v4l2_format));
    stFormat.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    stFormat.fmt.pix.pixelformat = pstUVCDev->iFormat;
    stFormat.fmt.pix.width       = pstUVCDev->iWidth;
    stFormat.fmt.pix.height      = pstUVCDev->iHeight;
    stFormat.fmt.pix.field       = V4L2_FIELD_NONE;
    stFormat.fmt.pix.sizeimage   = pstUVCDev->iWidth * pstUVCDev->iHeight * 1.5;
    iRet = ioctl(pstUVCDev->iFd, VIDIOC_S_FMT, &stFormat);
    if (iRet < 0) {
        aloge("VIDIOC_S_FMT failed!! %s (%d).\n", strerror(errno), errno);
    }

    iRet = ioctl(pstUVCDev->iFd, VIDIOC_G_FMT, &stFormat);
    alogd("width=[%d],height=[%d],sizeimage=[%d]\n",
        stFormat.fmt.pix.width, stFormat.fmt.pix.height, stFormat.fmt.pix.sizeimage);

    return iRet;
}

static void UVCFillStreamingControl(SampleUVCDevice *pstUVCDev, struct uvc_streaming_control *pstCtrl, int iFmtIndex, int iFrmIndex)
{
    SampleUVCContext *pConfig = (SampleUVCContext *)pstUVCDev->pPrivite;

    /* 0: interval fixed
     * 1: keyframe rate fixed
     * 2: Pframe rate fixed
     */
    pstCtrl->bmHint = 0;
    pstCtrl->bFormatIndex    = iFmtIndex + 1;
    pstCtrl->bFrameIndex     = iFrmIndex + 1;
    pstCtrl->dwFrameInterval = g_pstFormat[iFmtIndex].iInterval;
    pstCtrl->wDelay = 0;
    pstCtrl->dwMaxVideoFrameSize = g_pstFormat[iFmtIndex].iWidth * g_pstFormat[iFmtIndex].iHeight;
    pstCtrl->dwMaxPayloadTransferSize = g_pstFormat[iFmtIndex].iWidth * g_pstFormat[iFmtIndex].iHeight;
//  pstCtrl->dwClockFrequency    = ;
    pstCtrl->bmFramingInfo = 3; //ignore in JPEG or MJPEG format
    pstCtrl->bPreferedVersion = 1;
    pstCtrl->bMinVersion = 1;
    pstCtrl->bMaxVersion = 1;
}

static void SubscribeUVCEvent(struct SampleUVCDevice *pstUVCDev)
{
    struct v4l2_event_subscription stSub;
#if 1
    UVCFillStreamingControl(pstUVCDev, &pstUVCDev->stProbe, 0, 0);
    UVCFillStreamingControl(pstUVCDev, &pstUVCDev->stCommit, 0, 0);
#endif
    /* subscribe events, for debug, subscribe all events */
    memset(&stSub, 0, sizeof stSub);
    stSub.type = UVC_EVENT_FIRST;
    ioctl(pstUVCDev->iFd, VIDIOC_SUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_CONNECT;
    ioctl(pstUVCDev->iFd, VIDIOC_SUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_DISCONNECT;
    ioctl(pstUVCDev->iFd, VIDIOC_SUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_STREAMON;
    ioctl(pstUVCDev->iFd, VIDIOC_SUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_STREAMOFF;
    ioctl(pstUVCDev->iFd, VIDIOC_SUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_SETUP;
    ioctl(pstUVCDev->iFd, VIDIOC_SUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_DATA;
    ioctl(pstUVCDev->iFd, VIDIOC_SUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_LAST;
    ioctl(pstUVCDev->iFd, VIDIOC_SUBSCRIBE_EVENT, &stSub);
}

static void UnSubscribeUVCEvent(struct SampleUVCDevice *pstUVCDev)
{
    struct v4l2_event_subscription stSub;
#if 0
    uvc_fill_streaming_control(pstUVCDev, &pstUVCDev->probe, 0, 0);
    uvc_fill_streaming_control(pstUVCDev, &pstUVCDev->commit, 0, 0);
#endif
    /* subscribe events, for debug, subscribe all events */
    memset(&stSub, 0, sizeof stSub);
    stSub.type = UVC_EVENT_FIRST;
    ioctl(pstUVCDev->iFd, VIDIOC_UNSUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_CONNECT;
    ioctl(pstUVCDev->iFd, VIDIOC_UNSUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_DISCONNECT;
    ioctl(pstUVCDev->iFd, VIDIOC_UNSUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_STREAMON;
    ioctl(pstUVCDev->iFd, VIDIOC_UNSUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_STREAMOFF;
    ioctl(pstUVCDev->iFd, VIDIOC_UNSUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_SETUP;
    ioctl(pstUVCDev->iFd, VIDIOC_UNSUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_DATA;
    ioctl(pstUVCDev->iFd, VIDIOC_UNSUBSCRIBE_EVENT, &stSub);
    stSub.type = UVC_EVENT_LAST;
    ioctl(pstUVCDev->iFd, VIDIOC_UNSUBSCRIBE_EVENT, &stSub);
}

static int DoUVCEventSetupClassStreaming(SampleUVCDevice *pstUVCDev, struct uvc_event *pstEvent, struct uvc_request_data *pstReq)
{
    struct uvc_streaming_control *pstCtrl;
    uint8_t ucReq = pstEvent->req.bRequest;
    uint8_t ucCtrlSet = pstEvent->req.wValue >> 8;

    alogd("streaming request (req %02x cs %02x)\n", ucReq, ucCtrlSet);

    if (ucCtrlSet != UVC_VS_PROBE_CONTROL && ucCtrlSet != UVC_VS_COMMIT_CONTROL)
        return 0;

    pstCtrl = (struct uvc_streaming_control *)&pstReq->data[0];
    pstReq->length = sizeof(struct uvc_streaming_control);

    switch (ucReq) {
    case UVC_SET_CUR:
        pstUVCDev->iCtrlSetCur = ucCtrlSet;
        pstReq->length = 34;
        break;

    case UVC_GET_CUR:
//        alogd("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$getcur\n");
        if (ucCtrlSet == UVC_VS_PROBE_CONTROL)
            memcpy(pstCtrl, &pstUVCDev->stProbe, sizeof(struct uvc_streaming_control));
        else
            memcpy(pstCtrl, &pstUVCDev->stCommit, sizeof(struct uvc_streaming_control));
        break;

    case UVC_GET_MIN:
    case UVC_GET_MAX:
    case UVC_GET_DEF:
//        alogd("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$getdef\n");
        UVCFillStreamingControl(pstUVCDev, pstCtrl, 0, 0);
        break;

    case UVC_GET_RES:
//        alogd("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$get res\n");
        memset(pstCtrl, 0, sizeof(struct uvc_streaming_control));
        break;

    case UVC_GET_LEN:
//        alogd("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$get len\n");
        pstReq->data[0] = 0x00;
        pstReq->data[1] = 0x22;
        pstReq->length = 2;
        break;

    case UVC_GET_INFO:
//        alogd("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$get info\n");
        pstReq->data[0] = 0x03;
        pstReq->length = 1;
        break;
    }

    return 0;
}

static int DoUVCEventSetupClass(SampleUVCDevice *pstUVCDev, struct uvc_event *pstEvent, struct uvc_request_data *pstReq)
{
    if ((pstEvent->req.bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE)
        return 0;

    switch (pstEvent->req.wIndex & 0xff) {
    case UVC_INTF_CONTROL:
//      alogd("*******************************intfctonrol\n");
//        uvc_events_process_control(dev, ctrl->bRequest, ctrl->wValue >> 8, resp);
        break;

    case UVC_INTF_STREAMING:
//        alogd("*******************************intfstreaming\n");
        DoUVCEventSetupClassStreaming(pstUVCDev, pstEvent, pstReq);
        break;

    default:
        break;
    }

    return 0;
}

static int DoUVCEventSetup(SampleUVCDevice *pstUVCDev, struct uvc_event *pstEvent, struct uvc_request_data *pstReq)
{
    switch(pstEvent->req.bRequestType & USB_TYPE_MASK) {
        /* USB_TYPE_STANDARD: kernel driver will process it */
        case USB_TYPE_STANDARD:
        case USB_TYPE_VENDOR:
            aloge("do not care\n");
            break;
        case USB_TYPE_CLASS:
            DoUVCEventSetupClass(pstUVCDev, pstEvent, pstReq);
            break;

        default: break;
    }

    return 0;
}

static int DoUVCEventData(SampleUVCDevice *pstUVCDev, struct uvc_request_data *pstReq)
{
    struct uvc_streaming_control *pstTarget;

    switch(pstUVCDev->iCtrlSetCur) {
        case UVC_VS_PROBE_CONTROL:
//            alogd("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&probecontrol\n");
            printf("setting probe control, length = %d\n", pstReq->length);
            pstTarget = &pstUVCDev->stProbe;
            break;

        case UVC_VS_COMMIT_CONTROL:
//            alogd("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&commitcontrol\n");
            printf("setting commit control, length = %d\n", pstReq->length);
            pstTarget = &pstUVCDev->stCommit;
            break;

        default:
            printf("setting unknown control, length = %d\n", pstReq->length);
            return 0;
    }

    struct uvc_streaming_control *pstCtrl;
    pstCtrl = (struct uvc_streaming_control*)&pstReq->data[0];

    memcpy(pstTarget, pstCtrl, sizeof(struct uvc_streaming_control));

    alogd("pstCtrl->bmHint[%d],pstCtrl->bFormatIndex[%d],pstCtrl->bFrameIndex[%d],pstCtrl->dwFrameInterval[%d],\
        pstCtrl->dwMaxVideoFrameSize[%d],pstCtrl->dwMaxPayloadTransferSize[%d]",
        pstCtrl->bmHint, pstCtrl->bFormatIndex, pstCtrl->bFrameIndex, pstCtrl->dwFrameInterval,
        pstCtrl->dwMaxVideoFrameSize, pstCtrl->dwMaxPayloadTransferSize);

    if (pstUVCDev->iCtrlSetCur == UVC_VS_COMMIT_CONTROL) {
        pstUVCDev->iWidth  = g_pstFormat[pstCtrl->bFormatIndex - 1].iWidth;
        pstUVCDev->iHeight = g_pstFormat[pstCtrl->bFormatIndex - 1].iHeight;
        pstUVCDev->iFormat = g_pstFormat[pstCtrl->bFormatIndex - 1].iFormat;
        UVCVideoSetFormat(pstUVCDev);
    }

    return 0;
}

static int DoUVDReqReleaseBufs(SampleUVCDevice *pstUVCDev,  int iBufsNum)
{
    int iRet;

    if (iBufsNum > 0) {
        struct v4l2_requestbuffers stReqBufs;
        memset(&stReqBufs, 0, sizeof(struct v4l2_requestbuffers));
        stReqBufs.count  = iBufsNum;
        stReqBufs.memory = V4L2_MEMORY_MMAP;
        stReqBufs.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        iRet = ioctl(pstUVCDev->iFd, VIDIOC_REQBUFS, &stReqBufs);
        if (iRet < 0) {
            aloge("VIDIOC_REQBUFS failed!!\n");
            goto reqbufs_err;
        }

        pstUVCDev->pstFrames = malloc(iBufsNum * sizeof(SampleUVCFrame));

        for (int i = 0; i < iBufsNum; i++) {
            struct v4l2_buffer stBuffer;
            memset(&stBuffer, 0, sizeof(struct v4l2_buffer));
            stBuffer.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
            stBuffer.memory = V4L2_MEMORY_MMAP;
            stBuffer.index  = i;
            iRet = ioctl(pstUVCDev->iFd, VIDIOC_QUERYBUF, &stBuffer);
            if (iRet < 0) {
                aloge("VIDIOC_QUERYBUF failed!!\n");
            }

            pstUVCDev->pstFrames[i].pVirAddr = mmap(0, stBuffer.length, 
                PROT_READ | PROT_WRITE, MAP_SHARED, pstUVCDev->iFd, stBuffer.m.offset);
            pstUVCDev->pstFrames[i].iBufLen  = stBuffer.length;

            ioctl(pstUVCDev->iFd, VIDIOC_QBUF, &stBuffer);
        }

        pstUVCDev->iBufsNum = stReqBufs.count;
        alogd("request [%d] buffers success\n", pstUVCDev->iBufsNum);
    } else {
        for (int i = 0; i < pstUVCDev->iBufsNum; i++) {
            munmap(pstUVCDev->pstFrames[i].pVirAddr, pstUVCDev->pstFrames[i].iBufLen);
        }

        free(pstUVCDev->pstFrames);
    }
reqbufs_err:
    return iRet;
}

static int DoUVCStreamOnOff(SampleUVCDevice *pstUVCDev, int iOn)
{
    int iRet;
    enum v4l2_buf_type stBufType;
    stBufType = V4L2_BUF_TYPE_VIDEO_OUTPUT;

    if (iOn) {
        iRet = ioctl(pstUVCDev->iFd, VIDIOC_STREAMON, &stBufType);
        if (iRet == 0) {
            alogd("begin to streaming\n");
            pstUVCDev->bIsStreaming = 1;
        }
    } else {
        iRet = ioctl(pstUVCDev->iFd, VIDIOC_STREAMOFF, &stBufType);
        if (iRet == 0) {
            alogd("stop to streaming\n");
            pstUVCDev->bIsStreaming = 0;
        }
    }

    return iRet;
}

static int DoUVCEventProcess(SampleUVCDevice *pstUVCDev)
{
    int iRet;
    struct v4l2_event stEvent;
    struct uvc_event *pstUVCEvent = (struct uvc_event*)&stEvent.u.data[0];
    struct uvc_request_data stUVCReq;
    memset(&stUVCReq, 0, sizeof(struct uvc_request_data));
    stUVCReq.length = -EL2HLT;

    iRet = ioctl(pstUVCDev->iFd, VIDIOC_DQEVENT, &stEvent);
    if (iRet < 0) {
//        aloge("queue event failed!!\n");
        goto qevent_err;
    }

    alogd("event is 0x%x\n", stEvent.type);
    switch (stEvent.type) {
        case UVC_EVENT_CONNECT:
            alogd("uvc event first.\n");
            break;
        case UVC_EVENT_DISCONNECT:
            alogd("uvc event disconnect.\n");
            break;
        case UVC_EVENT_STREAMON:
            alogd("uvc event stream on.\n");
            DoUVDReqReleaseBufs(pstUVCDev, 5);
            DoUVCStreamOnOff(pstUVCDev, 1);
            goto qevent_err;
        case UVC_EVENT_STREAMOFF:
            alogd("uvc event stream off.\n");
            DoUVDReqReleaseBufs(pstUVCDev, 0);
            DoUVCStreamOnOff(pstUVCDev, 0);
            goto qevent_err;
        case UVC_EVENT_SETUP:
            alogd("uvc event setup.\n");
            DoUVCEventSetup(pstUVCDev, pstUVCEvent, &stUVCReq);
            break;
        case UVC_EVENT_DATA:
            DoUVCEventData(pstUVCDev, &pstUVCEvent->data);
            alogd("uvc event data.\n");
            break;

        default: break;
    }

    ioctl(pstUVCDev->iFd, UVCIOC_SEND_RESPONSE, &stUVCReq);
qevent_err:
    return iRet;
}

static void usage(const char *argv0)
{
    printf(
        "\033[33m"
        "exec [-h|--help] [-p|--path]\n"
        "   <-h|--help>: print the help information\n"
        "   <-p|--path>       <args>: point to the configuration file path\n"
        "   <-x|--width>      <args>: set video picture width\n"
        "   <-y|--height>     <args>: set video picture height\n"
        "   <-f|--framerate>  <args>: set the video frame rate\n"
        "   <-b|--bulk>       <args>: Use bulk mode or not[0|1]\n"
        "   <-d|--device>     <args>: uvc video device number[0-3]\n"
        "   <-i|--image>      <args>: MJPEG image\n"
        "\033[0m\n");
}

static ERRORTYPE LoadSampleUVCConfig(SampleUVCContext *pConfig, const char *conf_path)
{
    int iRet;

    CONFPARSER_S stConfParser;
    iRet = createConfParser(conf_path, &stConfParser);
    if(iRet < 0)
    {
        alogd("user not set config file. use default test parameter!");
        pConfig->bUseEve = 0;
        pConfig->iUVCDev = 2;
        pConfig->iCapDev = 1;
        pConfig->iCapWidth  = 1920;
        pConfig->iCapHeight = 1080;
        pConfig->iCapFrameRate = 25;
        pConfig->eCapFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
        pConfig->eEncoderType = PT_JPEG;
        pConfig->iEncBitRate  = 4194304;
        pConfig->iEncWidth  = 1920;
        pConfig->iEncHeight = 1080;
        pConfig->iEncFrameRate = 25;
        pConfig->iEncQuality = 99;
        goto use_default_conf;
    }

    if (pConfig->bUseEve == -1) {
        pConfig->bUseEve = GetConfParaInt(&stConfParser, SAMPLE_UVC_KEY_UVC_USE_EVE, 0);
      #if (0 == SUPPORT_EVE)
        pConfig->bUseEve = 0;
      #endif
    }
    if (pConfig->iUVCDev == -1) {
        pConfig->iUVCDev = GetConfParaInt(&stConfParser, SAMPLE_UVC_KEY_UVC_DEVICE, 0);
    }
    if (pConfig->iCapDev == -1) {
        pConfig->iCapDev = GetConfParaInt(&stConfParser, SAMPLE_UVC_KEY_VIN_DEVICE, 0);
    }
    if (pConfig->iCapWidth == -1) {
        pConfig->iCapWidth = GetConfParaInt(&stConfParser, SAMPLE_UVC_KEY_CAP_WIDTH, 0);
    }
    if (pConfig->iCapHeight == -1) {
        pConfig->iCapHeight = GetConfParaInt(&stConfParser, SAMPLE_UVC_KEY_CAP_HEIGHT, 0);
    }
    if (pConfig->iCapFrameRate == -1) {
        pConfig->iCapFrameRate = GetConfParaInt(&stConfParser, SAMPLE_UVC_KEY_CAP_FRAMERATE, 0);
    }
    if (pConfig->iEncBitRate == -1) {
        pConfig->iEncBitRate = GetConfParaInt(&stConfParser, SAMPLE_UVC_KEY_ENC_BITRATE, 0);
    }
    if (pConfig->iEncQuality == -1) {
        pConfig->iEncQuality = GetConfParaInt(&stConfParser, SAMPLE_UVC_KEY_ENC_QUALITY, 0);
    }
    if (pConfig->iEncWidth == -1) {
        pConfig->iEncWidth = GetConfParaInt(&stConfParser, SAMPLE_UVC_KEY_ENC_WIDTH, 0);
    }
    if (pConfig->iEncHeight == -1) {
        pConfig->iEncHeight = GetConfParaInt(&stConfParser, SAMPLE_UVC_KEY_ENC_HEIGHT, 0);
    }
    if (pConfig->iEncFrameRate == -1) {
        pConfig->iEncFrameRate = GetConfParaInt(&stConfParser, SAMPLE_UVC_KEY_ENC_FRAMERATE, 0);
    }

    char *pcTmpPtr;
    pcTmpPtr = (char *)GetConfParaString(&stConfParser, SAMPLE_UVC_KEY_CAP_FMT, NULL);
    if (NULL != pcTmpPtr && pConfig->eCapFormat == -1) {
        if (!strcmp(pcTmpPtr, "yu12"))
        {
            pConfig->eCapFormat = MM_PIXEL_FORMAT_YUV_PLANAR_420;
        }
        else if (!strcmp(pcTmpPtr, "yv12"))
        {
            pConfig->eCapFormat = MM_PIXEL_FORMAT_YVU_PLANAR_420;
        }
        else if (!strcmp(pcTmpPtr, "nv21"))
        {
            pConfig->eCapFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
        }
        else if (!strcmp(pcTmpPtr, "nv12"))
        {
            pConfig->eCapFormat = MM_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        }
        else if (!strcmp(pcTmpPtr, "yuv422p"))
        {
            pConfig->eCapFormat = MM_PIXEL_FORMAT_YUV_PLANAR_422;
        }
        else if (!strcmp(pcTmpPtr, "yuv422sp"))
        {
            pConfig->eCapFormat = MM_PIXEL_FORMAT_YUV_SEMIPLANAR_422;
        }
        else
        {
            aloge("not support the pixfmt, use default configuration 'MM_PIXEL_FORMAT_YUV_SEMIPLANAR_420'!!");
            pConfig->eCapFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
        }
    }

    pcTmpPtr = (char *)GetConfParaString(&stConfParser, SAMPLE_UVC_KEY_ENCODEER_TYPE, NULL);
    if (NULL != pcTmpPtr && pConfig->eEncoderType == -1) {
        if (!strcmp(pcTmpPtr, "mjpeg")) {
            pConfig->eEncoderType = PT_MJPEG;
        } else if (!strcmp(pcTmpPtr, "jpeg")) {
            pConfig->eEncoderType = PT_JPEG;
        } else {
            aloge("unknow encoder type, fix to JPEG!!\n");
            pConfig->eEncoderType = PT_JPEG;
        }
    }

use_default_conf:
    alogd("UVCDev=%d,CapDev=%d,CapWidth=%d,CapHeight=%d,CapFrmRate=%d,EncBitRate=%d,\
        EncWidth=%d,EncHeight=%d,EncFrmRate=%d,quality=%d\n", 
        pConfig->iUVCDev, pConfig->iCapDev, pConfig->iCapWidth, pConfig->iCapHeight, pConfig->iCapFrameRate, 
        pConfig->iEncBitRate, pConfig->iEncWidth, pConfig->iEncHeight, pConfig->iCapFrameRate, pConfig->iEncQuality);

    destroyConfParser(&stConfParser);
    return SUCCESS;
}

static struct option pstLongOptions[] = {
   {"help",        no_argument,       0, 'h'},
   {"bulk",        required_argument, 0, 'b'},
   {"path",        required_argument, 0, 'p'},
   {"width",       required_argument, 0, 'x'},
   {"height",      required_argument, 0, 'y'},
   {"framerate",   required_argument, 0, 'f'},
   {"device",      required_argument, 0, 'd'},
   {0,             0,                 0,  0 }
};

static int ParseCmdLine(int argc, char **argv, SampleUVCContext *pCmdLinePara)
{
    int mRet;
    int iOptIndex = 0;

    memset(pCmdLinePara, -1, sizeof(SampleUVCContext));
    pCmdLinePara->mCmdLinePara.mConfigFilePath[0] = 0;
    while (1) {
        mRet = getopt_long(argc, argv, ":p:b:x:y:f:d:h", pstLongOptions, &iOptIndex);
        if (mRet == -1) {
            break;
        }

        switch (mRet) {
            /* let the "sampleXXX -path sampleXXX.conf" command to be compatible with
             * "sampleXXX -p sampleXXX.conf"
             */
            case 'p':
                if (strcmp("ath", optarg) == 0) {
                    if (NULL == argv[optind]) {
                        usage(argv[0]);
                        goto opt_need_arg;
                    }
                    alogd("path is [%s]\n", argv[optind]);
                    strncpy(pCmdLinePara->mCmdLinePara.mConfigFilePath, argv[optind], sizeof(pCmdLinePara->mCmdLinePara.mConfigFilePath));
                } else {
                    alogd("path is [%s]\n", optarg);
                    strncpy(pCmdLinePara->mCmdLinePara.mConfigFilePath, optarg, sizeof(pCmdLinePara->mCmdLinePara.mConfigFilePath));
                }
                break;
            case 'x':
                alogd("width is [%d]\n", atoi(optarg));
                pCmdLinePara->iCapWidth = atoi(optarg);
                break;
            case 'y':
                alogd("height is [%d]\n", atoi(optarg));
                pCmdLinePara->iCapHeight = atoi(optarg);
                break;
            case 'f':
                alogd("frame rate is [%d]\n", atoi(optarg));
                pCmdLinePara->iCapFrameRate = atoi(optarg);
                break;
            case 'b':
                alogd("bulk mode is [%d]\n", atoi(optarg));
                // fix
                break;
            case 'd':
                alogd("device is [%d]\n", atoi(optarg));
                pCmdLinePara->iUVCDev = atoi(optarg);
                break;
            case 'h':
                usage(argv[0]);
                goto print_help_exit;
                break;
            case ':':
                aloge("option \"%s\" need <arg>\n", argv[optind - 1]);
                goto opt_need_arg;
                break;
            case '?':
                if (optind > 2) {
                    break;
                }
                aloge("unknow option \"%s\"\n", argv[optind - 1]);
                usage(argv[0]);
                goto unknow_option;
                break;
            default:
                printf("?? why getopt_long returned character code 0%o ??\n", mRet);
                break;
        }
    }

    return 0;
opt_need_arg:
unknow_option:
print_help_exit:
    return -1;
}

void SignalHandle(int iArg)
{
    alogd("receive exit signal. \n");
    g_bSampleExit = 1;
}

int main(int argc, char *argv[])
{
    int iRet;

    alogd("sample_uvcout running!\n");
    SampleUVCContext stContext;
    memset(&stContext, 0, sizeof(SampleUVCContext));

    iRet = ParseCmdLine(argc, argv, &stContext);
    if (iRet < 0) {
//        aloge("parse cmdline error.\n");
        return -1;
    }

    /* parse config file. */
    if(LoadSampleUVCConfig(&stContext , stContext.mCmdLinePara.mConfigFilePath) != SUCCESS)
    {
        aloge("fatal error! no config file or parse conf file fail");
        iRet = -1;
        goto load_conf_err;
    }

#if 1
    SampleUVCDevice *pstUVCDev = &stContext.stUVCDev;
    pstUVCDev->iDev = stContext.iUVCDev;
    pstUVCDev->pPrivite = (void *)&stContext;
    iRet = OpenUVCDevice(pstUVCDev);
    if (iRet < 0) {
        aloge("open uvc video device failed!!\n");
        iRet = -1;
        goto uvc_open_err;
    }
    pstUVCDev->bIsStreaming = 0;

    SubscribeUVCEvent(pstUVCDev);
#endif

    iRet = InitFrameList(10, &stContext);
    if (iRet < 0) {
        aloge("initialize frame list failed!!\n");
        goto init_frm_err;
    }

    iRet = CreateVi2VencThread(&stContext);
    if (iRet < 0) {
        aloge("create vipp to venc thread failed!!\n");
        goto init_vi2venc_err;
    }

    iRet = StartProcessing(&stContext);
    if (iRet < 0) {
        aloge("start processing failed!!\n");
        goto start_processing_err;
    }
    signal(SIGINT, SignalHandle);
#if 1
    fd_set stFdSet;
    FD_ZERO(&stFdSet);
    FD_SET(pstUVCDev->iFd, &stFdSet);

    while (1) {
        fd_set stErSet = stFdSet;
        fd_set stWrSet = stFdSet;
        struct timeval stTimeVal;
        /* Wait up to five seconds. */
        stTimeVal.tv_sec = 0;
        stTimeVal.tv_usec = 10*1000;

        if (g_bSampleExit) {
            break;
        }

        iRet = select(pstUVCDev->iFd + 1, NULL, &stWrSet, &stErSet, &stTimeVal);
        if (FD_ISSET(pstUVCDev->iFd, &stErSet))
            DoUVCEventProcess(pstUVCDev);
        if (FD_ISSET(pstUVCDev->iFd, &stWrSet) && pstUVCDev->bIsStreaming) {
            DoUVCVideoBufProcess(pstUVCDev);
        }
    }
#else
    while(1) {
        if (g_bSampleExit) {
            break;
        }
        sleep(1);
    }
#endif
    iRet = 0;

init_uvc_err:
    StopProcessing(&stContext);
start_processing_err:
  #if (SUPPORT_EVE!=0)
    DestroyVi2EveFaceThread(&stContext);
  #endif
init_vi2evefave_err:
    DestroyVi2VencThread(&stContext);
init_vi2venc_err:
    DeinitFrameList(&stContext);
init_frm_err:
    UnSubscribeUVCEvent(pstUVCDev);
    CloseUVCDevice(pstUVCDev);
uvc_open_err:
load_conf_err:
    if (0 == iRet) {
        printf("sample_uvcout exit!\n");
    }
    return iRet;
}

