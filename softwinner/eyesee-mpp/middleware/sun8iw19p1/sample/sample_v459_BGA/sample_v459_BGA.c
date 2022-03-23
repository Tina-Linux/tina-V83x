/******************************************************************************
  Copyright (C), 2001-2016, Allwinner Tech. Co., Ltd.
 ******************************************************************************
  File Name     :
  Version       : Initial Draft
  Author        : Allwinner BU3-PD2 Team
  Created       : 2016/11/4
  Last Modified :
  Description   :
  Function List :
  History       :
******************************************************************************/

#define LOG_NDEBUG 0
#define LOG_TAG "SampleV459BGA"

#include <unistd.h>
#include "plat_log.h"
#include <time.h>
#include <mm_common.h>

#include "sample_v459_BGA.h"
#include "sample_v459_BGA_conf.h"
#include <mm_comm_vo.h>
#include <mpi_vo.h>


#define DEFAULT_SRC_SIZE   1080
#define DEFAULT_DST_VIDEO_FILE      "/mnt/extsd/video/1080p.mp4"
#define DEFAULT_DST_DIR             "/mnt/extsd/video"
#define FILE_EXIST(PATH)   (access(PATH, F_OK) == 0)

#define DEFAULT_SRC_SIZE   1080

#define DEFAULT_MAX_DURATION  60*1000
#define DEFAULT_DST_VIDEO_FRAMERATE 30
#define DEFAULT_DST_VIDEO_BITRATE 12*1000*1000

#define DEFAULT_SRC_PIXFMT   MM_PIXEL_FORMAT_YUV_PLANAR_420
#define DEFAULT_DST_PIXFMT   MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420
#define DEFAULT_ENCODER   PT_H264

#define DEFAULT_SIMPLE_CACHE_SIZE_VFS       (4*1024)

static SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData_main;
static SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData_sub;

static int setOutputFileSync(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData, char* path, int64_t fallocateLength, int muxerId);


static ERRORTYPE InitVi2Venc2MuxerData(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData)
{
    pVi2Venc2MuxerData->mConfigPara.srcSize = pVi2Venc2MuxerData->mConfigPara.dstSize = DEFAULT_SRC_SIZE;
    if (pVi2Venc2MuxerData->mConfigPara.srcSize == 3840)
    {
        pVi2Venc2MuxerData->mConfigPara.srcWidth = 3840;
        pVi2Venc2MuxerData->mConfigPara.srcHeight = 2160;
    }
    else if (pVi2Venc2MuxerData->mConfigPara.srcSize == 1080)
    {
        pVi2Venc2MuxerData->mConfigPara.srcWidth = 1920;
        pVi2Venc2MuxerData->mConfigPara.srcHeight = 1080;
    }
    else if (pVi2Venc2MuxerData->mConfigPara.srcSize == 720)
    {
        pVi2Venc2MuxerData->mConfigPara.srcWidth = 1280;
        pVi2Venc2MuxerData->mConfigPara.srcHeight = 720;
    }
    else if (pVi2Venc2MuxerData->mConfigPara.srcSize == 640)
    {
        pVi2Venc2MuxerData->mConfigPara.srcWidth = 640;
        pVi2Venc2MuxerData->mConfigPara.srcHeight = 360;
    }

    if (pVi2Venc2MuxerData->mConfigPara.dstSize == 3840)
    {
        pVi2Venc2MuxerData->mConfigPara.dstWidth = 3840;
        pVi2Venc2MuxerData->mConfigPara.dstHeight = 2160;
    }
    else if (pVi2Venc2MuxerData->mConfigPara.dstSize == 1080)
    {
        pVi2Venc2MuxerData->mConfigPara.dstWidth = 1920;
        pVi2Venc2MuxerData->mConfigPara.dstHeight = 1080;
    }
    else if (pVi2Venc2MuxerData->mConfigPara.dstSize == 720)
    {
        pVi2Venc2MuxerData->mConfigPara.dstWidth = 1280;
        pVi2Venc2MuxerData->mConfigPara.dstHeight = 720;
    }    
    else if (pVi2Venc2MuxerData->mConfigPara.srcSize == 640)
    {
        pVi2Venc2MuxerData->mConfigPara.srcWidth = 640;
        pVi2Venc2MuxerData->mConfigPara.srcHeight = 360;
    }

    pVi2Venc2MuxerData->mConfigPara.mMaxFileDuration = DEFAULT_MAX_DURATION;
    pVi2Venc2MuxerData->mConfigPara.mVideoFrameRate = DEFAULT_DST_VIDEO_FRAMERATE;
    pVi2Venc2MuxerData->mConfigPara.mVideoBitRate = DEFAULT_DST_VIDEO_BITRATE;

    pVi2Venc2MuxerData->mConfigPara.srcPixFmt = DEFAULT_SRC_PIXFMT;
    pVi2Venc2MuxerData->mConfigPara.dstPixFmt = DEFAULT_DST_PIXFMT;
    pVi2Venc2MuxerData->mConfigPara.mVideoEncoderFmt = DEFAULT_ENCODER;
    pVi2Venc2MuxerData->mConfigPara.mField = VIDEO_FIELD_FRAME;

    pVi2Venc2MuxerData->mConfigPara.mColor2Grey = FALSE;
    pVi2Venc2MuxerData->mConfigPara.m3DNR = FALSE;

    pVi2Venc2MuxerData->mMuxGrp = MM_INVALID_CHN;
    pVi2Venc2MuxerData->mVeChn = MM_INVALID_CHN;
    pVi2Venc2MuxerData->mViChn = MM_INVALID_CHN;
    pVi2Venc2MuxerData->mViDev = MM_INVALID_DEV;

    strcpy(pVi2Venc2MuxerData->mConfigPara.dstVideoFile, DEFAULT_DST_VIDEO_FILE);

    pVi2Venc2MuxerData->mCurrentState = REC_NOT_PREPARED;
    pVi2Venc2MuxerData->loop_file_cnt = 5;

    return SUCCESS;
}

static ERRORTYPE parseCmdLine(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData, int argc, char** argv)
{
    ERRORTYPE ret = FAILURE;

    while (*argv)
    {        
       if (!strcmp(*argv, "-path"))
       {
          argv++;
          if (*argv)
          {
              ret = SUCCESS;
              if (strlen(*argv) >= MAX_FILE_PATH_LEN)
              {
                 aloge("fatal error! file path[%s] too long:!", *argv);
              }

              strncpy(pVi2Venc2MuxerData->mCmdLinePara.mConfigFilePath, *argv, MAX_FILE_PATH_LEN-1);
              pVi2Venc2MuxerData->mCmdLinePara.mConfigFilePath[MAX_FILE_PATH_LEN-1] = '\0';
          }
       }
       else if(!strcmp(*argv, "-h"))
       {
            printf("CmdLine param:\n"
                "\t-path /home/sample_v459_BGA.conf\n");
            break;
       }
       else if (*argv)
       {
          argv++;
       }
    }

    return ret;
}

static ERRORTYPE loadConfigPara(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData, const char *conf_path, int vi_node)
{
    int ret;
    char *ptr;
    CONFPARSER_S mConf;

    ret = createConfParser(conf_path, &mConf);

    if (ret < 0)
    {
        aloge("load conf fail");
        return FAILURE;
    }

    if(vi_node == 0){
        pVi2Venc2MuxerData->mConfigPara.mDevNo = GetConfParaInt(&mConf, CFG_SRC_DEV_NODE_MAIN, 0);    
        pVi2Venc2MuxerData->mConfigPara.srcSize = GetConfParaInt(&mConf, CFG_SRC_SIZE_MAIN, 0);    
        pVi2Venc2MuxerData->mConfigPara.dstSize = GetConfParaInt(&mConf, CFG_DST_VIDEO_SIZE_MAIN, 0);
        aloge("%d,%d",pVi2Venc2MuxerData->mConfigPara.srcSize,GetConfParaInt(&mConf, CFG_SRC_SIZE_MAIN, 0));
        ptr = (char *)GetConfParaString(&mConf, CFG_DST_VIDEO_FILE_STR_MAIN, NULL);
        strcpy(pVi2Venc2MuxerData->mConfigPara.dstVideoFile, ptr);
        
        pVi2Venc2MuxerData->mConfigPara.mVideoFrameRate = GetConfParaInt(&mConf, CFG_DST_VIDEO_FRAMERATE_MAIN, 0);
        pVi2Venc2MuxerData->mConfigPara.mVideoBitRate = GetConfParaInt(&mConf, CFG_DST_VIDEO_BITRATE_MAIN, 0);
        pVi2Venc2MuxerData->mConfigPara.mMaxFileDuration = GetConfParaInt(&mConf, CFG_DST_VIDEO_DURATION_MAIN, 0);
        
        pVi2Venc2MuxerData->mConfigPara.mRcMode = GetConfParaInt(&mConf, CFG_RC_MODE_MAIN, 0);
        pVi2Venc2MuxerData->mConfigPara.mEnableFastEnc = GetConfParaInt(&mConf, CFG_FAST_ENC_MAIN, 0);
        pVi2Venc2MuxerData->mConfigPara.mEnableRoi = GetConfParaInt(&mConf, CFG_ROI_MAIN, 0);
        
        ptr = (char *)GetConfParaString(&mConf, CFG_DST_VIDEO_ENCODER_MAIN, NULL);
    }else if(vi_node == 1){
        pVi2Venc2MuxerData->mConfigPara.mDevNo = GetConfParaInt(&mConf, CFG_SRC_DEV_NODE_SUB, 0);        
        pVi2Venc2MuxerData->mConfigPara.srcSize = GetConfParaInt(&mConf, CFG_SRC_SIZE_SUB, 0);
        pVi2Venc2MuxerData->mConfigPara.dstSize = GetConfParaInt(&mConf, CFG_DST_VIDEO_SIZE_SUB, 0);
        
        ptr = (char *)GetConfParaString(&mConf, CFG_DST_VIDEO_FILE_STR_SUB, NULL);
        strcpy(pVi2Venc2MuxerData->mConfigPara.dstVideoFile, ptr);
        
        pVi2Venc2MuxerData->mConfigPara.mVideoFrameRate = GetConfParaInt(&mConf, CFG_DST_VIDEO_FRAMERATE_SUB, 0);
        pVi2Venc2MuxerData->mConfigPara.mVideoBitRate = GetConfParaInt(&mConf, CFG_DST_VIDEO_BITRATE_SUB, 0);
        pVi2Venc2MuxerData->mConfigPara.mMaxFileDuration = GetConfParaInt(&mConf, CFG_DST_VIDEO_DURATION_SUB, 0);
        
        pVi2Venc2MuxerData->mConfigPara.mRcMode = GetConfParaInt(&mConf, CFG_RC_MODE_SUB, 0);
        pVi2Venc2MuxerData->mConfigPara.mEnableFastEnc = GetConfParaInt(&mConf, CFG_FAST_ENC_SUB, 0);
        pVi2Venc2MuxerData->mConfigPara.mEnableRoi = GetConfParaInt(&mConf, CFG_ROI_SUB, 0);
        
        ptr = (char *)GetConfParaString(&mConf, CFG_DST_VIDEO_ENCODER_SUB, NULL);
    }
    if (pVi2Venc2MuxerData->mConfigPara.srcSize == 3840)
    {
        pVi2Venc2MuxerData->mConfigPara.srcWidth = 3840;
        pVi2Venc2MuxerData->mConfigPara.srcHeight = 2160;
    }
    else if(pVi2Venc2MuxerData->mConfigPara.srcSize == 2592){
        pVi2Venc2MuxerData->mConfigPara.srcWidth = 2592;
        pVi2Venc2MuxerData->mConfigPara.srcHeight = 1944;
    }
    else if (pVi2Venc2MuxerData->mConfigPara.srcSize == 1080)
    {
        pVi2Venc2MuxerData->mConfigPara.srcWidth = 1920;
        pVi2Venc2MuxerData->mConfigPara.srcHeight = 1080;
    }
    else if (pVi2Venc2MuxerData->mConfigPara.srcSize == 720)
    {
        pVi2Venc2MuxerData->mConfigPara.srcWidth = 1280;
        pVi2Venc2MuxerData->mConfigPara.srcHeight = 720;
    }    
    else if (pVi2Venc2MuxerData->mConfigPara.srcSize == 640)
    {
        pVi2Venc2MuxerData->mConfigPara.srcWidth = 640;
        pVi2Venc2MuxerData->mConfigPara.srcHeight = 360;
    }
    if (pVi2Venc2MuxerData->mConfigPara.dstSize == 3840)
    {
        pVi2Venc2MuxerData->mConfigPara.dstWidth = 3840;
        pVi2Venc2MuxerData->mConfigPara.dstHeight = 2160;
    }    
    else if(pVi2Venc2MuxerData->mConfigPara.dstSize == 2592){
        pVi2Venc2MuxerData->mConfigPara.dstWidth = 2592;
        pVi2Venc2MuxerData->mConfigPara.dstHeight = 1944;
    }
    else if (pVi2Venc2MuxerData->mConfigPara.dstSize == 1080)
    {
        pVi2Venc2MuxerData->mConfigPara.dstWidth = 1920;
        pVi2Venc2MuxerData->mConfigPara.dstHeight = 1080;
    }
    else if (pVi2Venc2MuxerData->mConfigPara.dstSize == 720)
    {
        pVi2Venc2MuxerData->mConfigPara.dstWidth = 1280;
        pVi2Venc2MuxerData->mConfigPara.dstHeight = 720;
    }
    else if (pVi2Venc2MuxerData->mConfigPara.dstSize == 640)
    {
        pVi2Venc2MuxerData->mConfigPara.dstWidth = 640;
        pVi2Venc2MuxerData->mConfigPara.dstHeight = 360;
    }
    if (!strcmp(ptr, "H.264"))
    {
        pVi2Venc2MuxerData->mConfigPara.mVideoEncoderFmt = PT_H264;
    }
    else if (!strcmp(ptr, "H.265"))
    {
        pVi2Venc2MuxerData->mConfigPara.mVideoEncoderFmt = PT_H265;
    }
    else if (!strcmp(ptr, "MJPEG"))
    {
        pVi2Venc2MuxerData->mConfigPara.mVideoEncoderFmt = PT_MJPEG;
    }
    else
    {
        aloge("error conf encoder type");
    }

    pVi2Venc2MuxerData->mConfigPara.mTestDuration = GetConfParaInt(&mConf, CFG_TEST_DURATION, 0);

    alogd("dev_node:%d, frame rate:%d, bitrate:%d, video_duration=%d, test_time=%d", pVi2Venc2MuxerData->mConfigPara.mDevNo,\
        pVi2Venc2MuxerData->mConfigPara.mVideoFrameRate, pVi2Venc2MuxerData->mConfigPara.mVideoBitRate,\
        pVi2Venc2MuxerData->mConfigPara.mMaxFileDuration, pVi2Venc2MuxerData->mConfigPara.mTestDuration);

    ptr	= (char *)GetConfParaString(&mConf, CFG_COLOR2GREY, NULL);
    if(!strcmp(ptr, "yes"))
    {
        pVi2Venc2MuxerData->mConfigPara.mColor2Grey = TRUE;
    }
    else
    {
        pVi2Venc2MuxerData->mConfigPara.mColor2Grey = FALSE;
    }

    ptr	= (char *)GetConfParaString(&mConf, CFG_3DNR, NULL);
    if(!strcmp(ptr, "yes"))
    {
        pVi2Venc2MuxerData->mConfigPara.m3DNR = TRUE;
    }
    else
    {
        pVi2Venc2MuxerData->mConfigPara.m3DNR = FALSE;
    }
    pVi2Venc2MuxerData->loop_file_cnt = GetConfParaInt(&mConf, CFG_TEST_LOOP_FILE_CNT, 5); 
    pVi2Venc2MuxerData->loop_file_cnt = 0;
    destroyConfParser(&mConf);
    return SUCCESS;
}

static unsigned long long GetNowTimeUs(void)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return now.tv_sec * 1000000 + now.tv_usec;
}

static int getFileNameByCurTime(char *pNameBuf, SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData)
{
    int len = strlen(pVi2Venc2MuxerData->mConfigPara.dstVideoFile);
    char *ptr = pVi2Venc2MuxerData->mConfigPara.dstVideoFile;
    char fileName[128] = {0};
    while (*(ptr+len-1) != '.')
    {
        len--;
    }
    strncpy(fileName, pVi2Venc2MuxerData->mConfigPara.dstVideoFile, len-1);
    sprintf(pNameBuf, "%s_%d.mp4", fileName, pVi2Venc2MuxerData->file_num);    
    pVi2Venc2MuxerData->file_num++;
    if(pVi2Venc2MuxerData->file_num == 6){
        pVi2Venc2MuxerData->file_num =0;
    }
    return 0;
}

static ERRORTYPE MPPCallbackWrapper(void *cookie, MPP_CHN_S *pChn, MPP_EVENT_TYPE event, void *pEventData)
{
    if(MOD_ID_VENC == pChn->mModId)
    {
        switch(event)
        {
            case MPP_EVENT_RELEASE_VIDEO_BUFFER:
            {
                break;
            }
            default:
            {
                break;
            }
        }
    }
    else if(MOD_ID_MUX == pChn->mModId)
    {
        switch(event)
        {
            case MPP_EVENT_RECORD_DONE:
            {
                int muxerId = *(int*)pEventData;
                alogd("file done, mux_id=%d", muxerId);
                break;
            }
            case MPP_EVENT_NEED_NEXT_FD:
            {
                int muxerId = *(int*)pEventData;
                SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData = (SAMPLE_VI2VENC2MUXER_S *)cookie;
                char fileName[128] = {0};
                getFileNameByCurTime(fileName, pVi2Venc2MuxerData);
                alogd("mux set next fd, filepath=%s", fileName);
                setOutputFileSync(pVi2Venc2MuxerData, fileName, 0, muxerId);
                break;
            }
            case MPP_EVENT_BSFRAME_AVAILABLE:
            {
                alogd("mux bs frame available");
                break;
            }
            default:
            {
                break;
            }
        }
    }

    return SUCCESS;
}

static ERRORTYPE VOCallbackWrapper(void *cookie, MPP_CHN_S *pChn, MPP_EVENT_TYPE event, void *pEventData)
{
    ERRORTYPE ret = SUCCESS;
    SAMPLE_VI2VENC2MUXER_S *pContext = (SAMPLE_VI2VENC2MUXER_S*)cookie;
    if(MOD_ID_VOU == pChn->mModId)
    {
        if(pChn->mChnId != pContext->mVOChn)
        {
            aloge("fatal error! VO chnId[%d]!=[%d]", pChn->mChnId, pContext->mVOChn);
        }
        switch(event)
        {
            case MPP_EVENT_RELEASE_VIDEO_BUFFER:
            {
                aloge("fatal error! sample_virvi2vo use binding mode!");
                break;
            }
            case MPP_EVENT_SET_VIDEO_SIZE:
            {
                SIZE_S *pDisplaySize = (SIZE_S*)pEventData;
                alogd("vo report video display size[%dx%d]", pDisplaySize->Width, pDisplaySize->Height);
                break;
            }
            case MPP_EVENT_RENDERING_START:
            {
                alogd("vo report rendering start");
                break;
            }
            default:
            {
                //postEventFromNative(this, event, 0, 0, pEventData);
                aloge("fatal error! unknown event[0x%x] from channel[0x%x][0x%x][0x%x]!", event, pChn->mModId, pChn->mDevId, pChn->mChnId);
                ret = ERR_VO_ILLEGAL_PARAM;
                break;
            }
        }
    }
    else
    {
        aloge("fatal error! why modId[0x%x]?", pChn->mModId);
        ret = FAILURE;
    }
    return ret;
}

static ERRORTYPE configMuxGrpAttr(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData)
{
    memset(&pVi2Venc2MuxerData->mMuxGrpAttr, 0, sizeof(MUX_GRP_ATTR_S));

    pVi2Venc2MuxerData->mMuxGrpAttr.mVideoEncodeType = pVi2Venc2MuxerData->mConfigPara.mVideoEncoderFmt;
    pVi2Venc2MuxerData->mMuxGrpAttr.mWidth = pVi2Venc2MuxerData->mConfigPara.dstWidth;
    pVi2Venc2MuxerData->mMuxGrpAttr.mHeight = pVi2Venc2MuxerData->mConfigPara.dstHeight;
    pVi2Venc2MuxerData->mMuxGrpAttr.mVideoFrmRate = pVi2Venc2MuxerData->mConfigPara.mVideoFrameRate*1000;
    //pVi2Venc2MuxerData_main->mMuxGrpAttr.mMaxKeyInterval = 
    pVi2Venc2MuxerData->mMuxGrpAttr.mAudioEncodeType = PT_MAX;

    return SUCCESS;
}

static ERRORTYPE createMuxGrp(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData)
{
    ERRORTYPE ret;
    BOOL nSuccessFlag = FALSE;

    configMuxGrpAttr(pVi2Venc2MuxerData);
    pVi2Venc2MuxerData->mMuxGrp = 0;
    while (pVi2Venc2MuxerData->mMuxGrp < MUX_MAX_GRP_NUM)
    {
        ret = AW_MPI_MUX_CreateGrp(pVi2Venc2MuxerData->mMuxGrp, &pVi2Venc2MuxerData->mMuxGrpAttr);
        if (SUCCESS == ret)
        {
            nSuccessFlag = TRUE;
            alogd("create mux group[%d] success!", pVi2Venc2MuxerData->mMuxGrp);
            break;
        }
        else if (ERR_MUX_EXIST == ret)
        {
            alogd("mux group[%d] is exist, find next!", pVi2Venc2MuxerData->mMuxGrp);
            pVi2Venc2MuxerData->mMuxGrp++;
        }
        else
        {
            alogd("create mux group[%d] ret[0x%x], find next!", pVi2Venc2MuxerData->mMuxGrp, ret);
            pVi2Venc2MuxerData->mMuxGrp++;
        }
    }

    if (FALSE == nSuccessFlag)
    {
        pVi2Venc2MuxerData->mMuxGrp = MM_INVALID_CHN;
        aloge("fatal error! create mux group fail!");
        return FAILURE;
    }
    else
    {
        MPPCallbackInfo cbInfo;
        cbInfo.cookie = (void*)pVi2Venc2MuxerData;
        cbInfo.callback = (MPPCallbackFuncType)&MPPCallbackWrapper;
        AW_MPI_MUX_RegisterCallback(pVi2Venc2MuxerData->mMuxGrp, &cbInfo);
        return SUCCESS;
    }
}

static int addOutputFormatAndOutputSink_1(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData, OUTSINKINFO_S *pSinkInfo)
{
    int retMuxerId = -1;
    MUX_CHN_INFO_S *pEntry, *pTmp;

    alogd("fmt:0x%x, fd:%d, FallocateLen:%d, callback_out_flag:%d", pSinkInfo->mOutputFormat, pSinkInfo->mOutputFd, pSinkInfo->mFallocateLen, pSinkInfo->mCallbackOutFlag);
    if(pSinkInfo->mOutputFd >= 0 && TRUE == pSinkInfo->mCallbackOutFlag)
    {
        aloge("fatal error! one muxer cannot support two sink methods!");
        return -1;
    }

    //find if the same output_format sinkInfo exist or callback out stream is exist.
    pthread_mutex_lock(&pVi2Venc2MuxerData->mMuxChnListLock);
    if (!list_empty(&pVi2Venc2MuxerData->mMuxChnList))
    {
        list_for_each_entry_safe(pEntry, pTmp, &pVi2Venc2MuxerData->mMuxChnList, mList)
        {
            if (pEntry->mSinkInfo.mOutputFormat == pSinkInfo->mOutputFormat)
            {
                alogd("Be careful! same outputForamt[0x%x] exist in array", pSinkInfo->mOutputFormat);
            }
            if (pEntry->mSinkInfo.mCallbackOutFlag == pSinkInfo->mCallbackOutFlag)
            {
                aloge("fatal error! only support one callback out stream");
            }
        }
    }
    pthread_mutex_unlock(&pVi2Venc2MuxerData->mMuxChnListLock);

    MUX_CHN_INFO_S *p_node = (MUX_CHN_INFO_S *)malloc(sizeof(MUX_CHN_INFO_S));
    if (p_node == NULL)
    {
        aloge("alloc mux chn info node fail");
        return -1;
    }

    memset(p_node, 0, sizeof(MUX_CHN_INFO_S));
    p_node->mSinkInfo.mMuxerId = pVi2Venc2MuxerData->mMuxerIdCounter;
    p_node->mSinkInfo.mOutputFormat = pSinkInfo->mOutputFormat;
    if (pSinkInfo->mOutputFd > 0)
    {
        p_node->mSinkInfo.mOutputFd = dup(pSinkInfo->mOutputFd);
    }
    else
    {
        p_node->mSinkInfo.mOutputFd = -1;
    }
    p_node->mSinkInfo.mFallocateLen = pSinkInfo->mFallocateLen;
    p_node->mSinkInfo.mCallbackOutFlag = pSinkInfo->mCallbackOutFlag;

    p_node->mMuxChnAttr.mMuxerId = p_node->mSinkInfo.mMuxerId;
    p_node->mMuxChnAttr.mMediaFileFormat = p_node->mSinkInfo.mOutputFormat;
    p_node->mMuxChnAttr.mMaxFileDuration = pVi2Venc2MuxerData->mConfigPara.mMaxFileDuration *1000;
    p_node->mMuxChnAttr.mFallocateLen = p_node->mSinkInfo.mFallocateLen;
    p_node->mMuxChnAttr.mCallbackOutFlag = p_node->mSinkInfo.mCallbackOutFlag;
    p_node->mMuxChnAttr.mFsWriteMode = FSWRITEMODE_SIMPLECACHE;
    p_node->mMuxChnAttr.mSimpleCacheSize = DEFAULT_SIMPLE_CACHE_SIZE_VFS;

    p_node->mMuxChn = MM_INVALID_CHN;

    if ((pVi2Venc2MuxerData->mCurrentState == REC_PREPARED) || (pVi2Venc2MuxerData->mCurrentState == REC_RECORDING))
    {
        ERRORTYPE ret;
        BOOL nSuccessFlag = FALSE;
        MUX_CHN nMuxChn = 0;
        while (nMuxChn < MUX_MAX_CHN_NUM)
        {
            ret = AW_MPI_MUX_CreateChn(pVi2Venc2MuxerData->mMuxGrp, nMuxChn, &p_node->mMuxChnAttr, p_node->mSinkInfo.mOutputFd);
            if (SUCCESS == ret)
            {
                nSuccessFlag = TRUE;
                alogd("create mux group[%d] channel[%d] success, muxerId[%d]!", pVi2Venc2MuxerData->mMuxGrp, nMuxChn, p_node->mMuxChnAttr.mMuxerId);
                break;
            }
            else if (ERR_MUX_EXIST == ret)
            {
                alogd("mux group[%d] channel[%d] is exist, find next!", pVi2Venc2MuxerData->mMuxGrp, nMuxChn);
                nMuxChn++;
            }
            else
            {
                aloge("fatal error! create mux group[%d] channel[%d] fail ret[0x%x], find next!", pVi2Venc2MuxerData->mMuxGrp, nMuxChn, ret);
                nMuxChn++;
            }
        }

        if (nSuccessFlag)
        {
            retMuxerId = p_node->mSinkInfo.mMuxerId;
            p_node->mMuxChn = nMuxChn;
            pVi2Venc2MuxerData->mMuxerIdCounter++;
        }
        else
        {
            aloge("fatal error! create mux group[%d] channel fail!", pVi2Venc2MuxerData->mMuxGrp);
            if (p_node->mSinkInfo.mOutputFd >= 0)
            {
                close(p_node->mSinkInfo.mOutputFd);
                p_node->mSinkInfo.mOutputFd = -1;
            }

            retMuxerId = -1;
        }

        pthread_mutex_lock(&pVi2Venc2MuxerData->mMuxChnListLock);
        list_add_tail(&p_node->mList, &pVi2Venc2MuxerData->mMuxChnList);
        pthread_mutex_unlock(&pVi2Venc2MuxerData->mMuxChnListLock);
    }
    else
    {
        retMuxerId = p_node->mSinkInfo.mMuxerId;
        pVi2Venc2MuxerData->mMuxerIdCounter++;
        pthread_mutex_lock(&pVi2Venc2MuxerData->mMuxChnListLock);
        list_add_tail(&p_node->mList, &pVi2Venc2MuxerData->mMuxChnList);
        pthread_mutex_unlock(&pVi2Venc2MuxerData->mMuxChnListLock);
    }

    return retMuxerId;
}

static int addOutputFormatAndOutputSink(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData, char* path, MEDIA_FILE_FORMAT_E format)
{
    int muxerId = -1;
    OUTSINKINFO_S sinkInfo = {0};

    if (path != NULL)
    {
        sinkInfo.mFallocateLen = 0;
        sinkInfo.mCallbackOutFlag = FALSE;
        sinkInfo.mOutputFormat = format;
        sinkInfo.mOutputFd = open(path, O_RDWR | O_CREAT, 0666);
        if (sinkInfo.mOutputFd < 0)
        {
            aloge("Failed to open %s", path);
            return -1;
        }

        muxerId = addOutputFormatAndOutputSink_1(pVi2Venc2MuxerData, &sinkInfo);
        pVi2Venc2MuxerData->file_num = 1;
        close(sinkInfo.mOutputFd);
    }

    return muxerId;
}

static int setOutputFileSync_1(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData, int fd, int64_t fallocateLength, int muxerId)
{
    MUX_CHN_INFO_S *pEntry, *pTmp;

    if (pVi2Venc2MuxerData->mCurrentState != REC_RECORDING)
    {
        aloge("must be in recording state");
        return -1;
    }

    alogv("setOutputFileSync fd=%d", fd);
    if (fd < 0)
    {
        aloge("Invalid parameter");
        return -1;
    }

    MUX_CHN muxChn = MM_INVALID_CHN;
    pthread_mutex_lock(&pVi2Venc2MuxerData->mMuxChnListLock);
    if (!list_empty(&pVi2Venc2MuxerData->mMuxChnList))
    {
        list_for_each_entry_safe(pEntry, pTmp, &pVi2Venc2MuxerData->mMuxChnList, mList)
        {
            if (pEntry->mMuxChnAttr.mMuxerId == muxerId)
            {
                muxChn = pEntry->mMuxChn;
                break;
            }
        }
    }
    pthread_mutex_unlock(&pVi2Venc2MuxerData->mMuxChnListLock);

    if (muxChn != MM_INVALID_CHN)
    {
        alogd("switch fd");
        AW_MPI_MUX_SwitchFd(pVi2Venc2MuxerData->mMuxGrp, muxChn, fd, fallocateLength);
        return 0;
    }
    else
    {
        aloge("fatal error! can't find muxChn which muxerId[%d]", muxerId);
        return -1;
    }
}

static int setOutputFileSync(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData, char* path, int64_t fallocateLength, int muxerId)
{
    int ret;

    if (pVi2Venc2MuxerData->mCurrentState != REC_RECORDING)
    {
        aloge("not in recording state");
        return -1;
    }

    if(path != NULL)
    {
        int fd = open(path, O_RDWR | O_CREAT, 0666);
        if (fd < 0)
        {
            aloge("fail to open %s", path);
            return -1;
        }
        ret = setOutputFileSync_1(pVi2Venc2MuxerData, fd, fallocateLength, muxerId);
        close(fd);

        return ret;
    }
    else
    {
        return -1;
    }
}

static ERRORTYPE configVencChnAttr(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData)
{
    memset(&pVi2Venc2MuxerData->mVencChnAttr, 0, sizeof(VENC_CHN_ATTR_S));

    pVi2Venc2MuxerData->mVencChnAttr.VeAttr.Type = pVi2Venc2MuxerData->mConfigPara.mVideoEncoderFmt;
    //pVi2Venc2MuxerData_main->mVencChnAttr.VeAttr.MaxKeyInterval = pVi2Venc2MuxerData_main->mVideoMaxKeyItl;
    pVi2Venc2MuxerData->mVencChnAttr.VeAttr.SrcPicWidth  = pVi2Venc2MuxerData->mConfigPara.srcWidth;
    pVi2Venc2MuxerData->mVencChnAttr.VeAttr.SrcPicHeight = pVi2Venc2MuxerData->mConfigPara.srcHeight;
    pVi2Venc2MuxerData->mVencChnAttr.VeAttr.PixelFormat = pVi2Venc2MuxerData->mConfigPara.dstPixFmt;
    pVi2Venc2MuxerData->mVencChnAttr.VeAttr.Field = pVi2Venc2MuxerData->mConfigPara.mField;

    if (PT_H264 == pVi2Venc2MuxerData->mVencChnAttr.VeAttr.Type)
    {
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH264e.bByFrame = TRUE;
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH264e.Profile = 2;//0:base 1:main 2:high
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH264e.PicWidth  = pVi2Venc2MuxerData->mConfigPara.dstWidth;
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH264e.PicHeight = pVi2Venc2MuxerData->mConfigPara.dstHeight;
        switch (pVi2Venc2MuxerData->mConfigPara.mRcMode)
        {
        case 1:
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264VBR;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264Vbr.mMinQp = 10;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264Vbr.mMaxQp = 52;
            break;
        case 2:
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264FIXQP;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264FixQp.mIQp = 28;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264FixQp.mPQp = 28;
            break;
        case 3:
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264QPMAP;
            break;
        case 0:
        default:
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264CBR;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264Cbr.mSrcFrmRate = pVi2Venc2MuxerData->mConfigPara.mVideoFrameRate;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264Cbr.mBitRate = pVi2Venc2MuxerData->mConfigPara.mVideoBitRate;
            break;
        }
        if (pVi2Venc2MuxerData->mConfigPara.mEnableFastEnc)
        {
            pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH264e.FastEncFlag = TRUE;
        }
    }
    else if (PT_H265 == pVi2Venc2MuxerData->mVencChnAttr.VeAttr.Type)
    {
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH265e.mbByFrame = TRUE;
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH265e.mProfile = 1;//1:main 2:main10 3:sti11
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH265e.mPicWidth = pVi2Venc2MuxerData->mConfigPara.dstWidth;
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH265e.mPicHeight = pVi2Venc2MuxerData->mConfigPara.dstHeight;
        pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265Cbr.mBitRate = pVi2Venc2MuxerData->mConfigPara.mVideoBitRate;
        switch (pVi2Venc2MuxerData->mConfigPara.mRcMode)
        {
        case 1:
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264VBR;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265Vbr.mMinQp = 10;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265Vbr.mMaxQp = 52;
            break;
        case 2:
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264FIXQP;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265FixQp.mIQp = 28;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265FixQp.mPQp = 28;
            break;
        case 3:
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264QPMAP;
            break;
        case 0:
        default:
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264CBR;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265Cbr.mSrcFrmRate = pVi2Venc2MuxerData->mConfigPara.mVideoFrameRate;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265Cbr.mBitRate = pVi2Venc2MuxerData->mConfigPara.mVideoBitRate;
            break;
        }
        if (pVi2Venc2MuxerData->mConfigPara.mEnableFastEnc)
        {
            pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH265e.mFastEncFlag = TRUE;
        }
    }
    else if (PT_MJPEG == pVi2Venc2MuxerData->mVencChnAttr.VeAttr.Type)
    {
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrMjpeg.mbByFrame = TRUE;
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrMjpeg.mPicWidth = pVi2Venc2MuxerData->mConfigPara.dstWidth;
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrMjpeg.mPicHeight = pVi2Venc2MuxerData->mConfigPara.dstHeight;
        switch (pVi2Venc2MuxerData->mConfigPara.mRcMode)
        {
        case 0:
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_MJPEGCBR;
            break;
        case 1:
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_MJPEGFIXQP;
            break;
        case 2:
        case 3:
            aloge("not support! use default cbr mode");
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_MJPEGCBR;
            break;
        default:
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_MJPEGCBR;
            break;
        }
        pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrMjpegeCbr.mBitRate = pVi2Venc2MuxerData->mConfigPara.mVideoBitRate;
    }

    alogd("venc ste Rcmode=%d", pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode);

    return SUCCESS;
}

static ERRORTYPE createVencChn(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData)
{
    ERRORTYPE ret;
    BOOL nSuccessFlag = FALSE;

    configVencChnAttr(pVi2Venc2MuxerData);
    pVi2Venc2MuxerData->mVeChn = pVi2Venc2MuxerData->mViDev;
    while (pVi2Venc2MuxerData->mVeChn < VENC_MAX_CHN_NUM)
    {
        ret = AW_MPI_VENC_CreateChn(pVi2Venc2MuxerData->mVeChn, &pVi2Venc2MuxerData->mVencChnAttr);
        if (SUCCESS == ret)
        {
            nSuccessFlag = TRUE;
            alogd("create venc channel[%d] success!", pVi2Venc2MuxerData->mVeChn);
            break;
        }
        else if (ERR_VENC_EXIST == ret)
        {
            alogd("venc channel[%d] is exist, find next!", pVi2Venc2MuxerData->mVeChn);
            pVi2Venc2MuxerData->mVeChn++;
        }
        else
        {
            alogd("create venc channel[%d] ret[0x%x], find next!", pVi2Venc2MuxerData->mVeChn, ret);
            pVi2Venc2MuxerData->mVeChn++;
        }
    }

    if (nSuccessFlag == FALSE)
    {
        pVi2Venc2MuxerData->mVeChn = MM_INVALID_CHN;
        aloge("fatal error! create venc channel fail!");
        return FAILURE;
    }
    else
    {
        VENC_FRAME_RATE_S stFrameRate;
        stFrameRate.SrcFrmRate = stFrameRate.DstFrmRate = pVi2Venc2MuxerData->mConfigPara.mVideoFrameRate;
        alogd("set venc framerate:%d", stFrameRate.DstFrmRate);
        AW_MPI_VENC_SetFrameRate(pVi2Venc2MuxerData->mVeChn, &stFrameRate);
        AW_MPI_VENC_SetVEFreq(pVi2Venc2MuxerData->mVeChn, 300);
        MPPCallbackInfo cbInfo;
        cbInfo.cookie = (void*)pVi2Venc2MuxerData;
        cbInfo.callback = (MPPCallbackFuncType)&MPPCallbackWrapper;
        AW_MPI_VENC_RegisterCallback(pVi2Venc2MuxerData->mVeChn, &cbInfo);

        if ( ((PT_H264 == pVi2Venc2MuxerData->mVencChnAttr.VeAttr.Type) || (PT_H265 == pVi2Venc2MuxerData->mVencChnAttr.VeAttr.Type))
            && ((VENC_RC_MODE_H264QPMAP == pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode) || (VENC_RC_MODE_H265QPMAP == pVi2Venc2MuxerData_main->mVencChnAttr.RcAttr.mRcMode))
           )
        {
            aloge("fatal error! not support qpmap currently!");
        }

        return SUCCESS;
    }
}

static ERRORTYPE createViChn(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData)
{
    ERRORTYPE ret;

    //create vi channel
    pVi2Venc2MuxerData->mViDev = pVi2Venc2MuxerData->mConfigPara.mDevNo;
    pVi2Venc2MuxerData->mIspDev = 0;
    pVi2Venc2MuxerData->mViChn = 0;

    ret = AW_MPI_VI_CreateVipp(pVi2Venc2MuxerData->mViDev);
    if (ret != SUCCESS)
    {
        aloge("fatal error! AW_MPI_VI CreateVipp failed");
    }

    memset(&pVi2Venc2MuxerData->mViAttr, 0, sizeof(VI_ATTR_S));
    pVi2Venc2MuxerData->mViAttr.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    pVi2Venc2MuxerData->mViAttr.memtype = V4L2_MEMORY_MMAP;
    //pVi2Venc2MuxerData_main->mViAttr.format.pixelformat = map_PIXEL_FORMAT_E_to_V4L2_PIX_FMT();
    pVi2Venc2MuxerData->mViAttr.format.pixelformat = V4L2_PIX_FMT_NV21M;
    pVi2Venc2MuxerData->mViAttr.format.field = V4L2_FIELD_NONE;
    //pVi2Venc2MuxerData_main->mViAttr.format.colorspace = V4L2_COLORSPACE_JPEG;
    pVi2Venc2MuxerData->mViAttr.format.width = pVi2Venc2MuxerData->mConfigPara.srcWidth;
    pVi2Venc2MuxerData->mViAttr.format.height = pVi2Venc2MuxerData->mConfigPara.srcHeight;
    pVi2Venc2MuxerData->mViAttr.nbufs = 5; //5
    aloge("use %d v4l2 buffers!!!", pVi2Venc2MuxerData->mViAttr.nbufs);
    pVi2Venc2MuxerData->mViAttr.nplanes = 2;
    pVi2Venc2MuxerData->mViAttr.fps = pVi2Venc2MuxerData->mConfigPara.mVideoFrameRate;

    ret = AW_MPI_VI_SetVippAttr(pVi2Venc2MuxerData->mViDev, &pVi2Venc2MuxerData->mViAttr);
    if (ret != SUCCESS)
    {
        aloge("fatal error! AW_MPI_VI SetVippAttr failed");
    }

    ret = AW_MPI_VI_EnableVipp(pVi2Venc2MuxerData->mViDev);
    if (ret != SUCCESS)
    {
        aloge("fatal error! enableVipp fail!");
    }
    AW_MPI_ISP_Run(pVi2Venc2MuxerData->mIspDev);

    ret = AW_MPI_VI_CreateVirChn(pVi2Venc2MuxerData->mViDev, pVi2Venc2MuxerData->mViChn, NULL);
    if (ret != SUCCESS)
    {
        aloge("fatal error! createVirChn[%d] fail!", pVi2Venc2MuxerData->mViChn);
    }

    return ret;
}

static ERRORTYPE prepare(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData)
{
    BOOL nSuccessFlag;
    MUX_CHN nMuxChn;
    MUX_CHN_INFO_S *pEntry, *pTmp;
    ERRORTYPE ret;
    ERRORTYPE result = FAILURE;

    if (createViChn(pVi2Venc2MuxerData) != SUCCESS)
    {
        aloge("create vi chn fail");
        return result;
    }

    if (createVencChn(pVi2Venc2MuxerData) != SUCCESS)
    {
        aloge("create venc chn fail");
        return result;
    }

    if (createMuxGrp(pVi2Venc2MuxerData) != SUCCESS)
    {
        aloge("create mux group fail");
        return result;
    }

    //set spspps
    if (pVi2Venc2MuxerData->mConfigPara.mVideoEncoderFmt == PT_H264)
    {
        VencHeaderData H264SpsPpsInfo;
        AW_MPI_VENC_GetH264SpsPpsInfo(pVi2Venc2MuxerData->mVeChn, &H264SpsPpsInfo);
        AW_MPI_MUX_SetH264SpsPpsInfo(pVi2Venc2MuxerData->mMuxGrp, &H264SpsPpsInfo);
    }
    else if(pVi2Venc2MuxerData->mConfigPara.mVideoEncoderFmt == PT_H265)
    {
        VencHeaderData H265SpsPpsInfo;
        AW_MPI_VENC_GetH265SpsPpsInfo(pVi2Venc2MuxerData->mVeChn, &H265SpsPpsInfo);
        AW_MPI_MUX_SetH265SpsPpsInfo(pVi2Venc2MuxerData->mMuxGrp, &H265SpsPpsInfo);
    }

    if (pVi2Venc2MuxerData->mConfigPara.mEnableRoi)
    {
alogd("------------------ROI begin-----------------------");
        VENC_ROI_CFG_S VencRoiCfg;

        VencRoiCfg.bEnable = TRUE;
        VencRoiCfg.Index = 0;
        VencRoiCfg.Qp = 10;
        VencRoiCfg.bAbsQp = 0;
        VencRoiCfg.Rect.X = 20;
        VencRoiCfg.Rect.Y = 0;
        VencRoiCfg.Rect.Width = 1280;
        VencRoiCfg.Rect.Height = 320;
        AW_MPI_VENC_SetRoiCfg(pVi2Venc2MuxerData->mVeChn, &VencRoiCfg);

        VencRoiCfg.Index = 1;
        VencRoiCfg.Rect.X = 200;
        VencRoiCfg.Rect.Y = 600;
        VencRoiCfg.Rect.Width = 1000;
        VencRoiCfg.Rect.Height = 200;
        AW_MPI_VENC_SetRoiCfg(pVi2Venc2MuxerData->mVeChn, &VencRoiCfg);

        VencRoiCfg.Index = 2;
        VencRoiCfg.Rect.X = 640;
        VencRoiCfg.Rect.Y = 384;
        VencRoiCfg.Rect.Width = 640;
        VencRoiCfg.Rect.Height = 360;
        VencRoiCfg.Qp = 40;
        VencRoiCfg.bAbsQp = 0;
        AW_MPI_VENC_SetRoiCfg(pVi2Venc2MuxerData->mVeChn, &VencRoiCfg);

//        VencRoiCfg.Index = 4;
//        VencRoiCfg.Rect.X = 640;
//        VencRoiCfg.Rect.Y = 320;
//        VencRoiCfg.Rect.Width = 160;
//        VencRoiCfg.Rect.Height = 160;
//        AW_MPI_VENC_SetRoiCfg(pVi2Venc2MuxerData_main->mVeChn, &VencRoiCfg);


        alogd("------------------ROI end-----------------------");
    }

    pthread_mutex_lock(&pVi2Venc2MuxerData->mMuxChnListLock);
    if (!list_empty(&pVi2Venc2MuxerData->mMuxChnList))
    {
        list_for_each_entry_safe(pEntry, pTmp, &pVi2Venc2MuxerData->mMuxChnList, mList)
        {
            nMuxChn = 0;
            nSuccessFlag = FALSE;
            while (pEntry->mMuxChn < MUX_MAX_CHN_NUM)
            {
                ret = AW_MPI_MUX_CreateChn(pVi2Venc2MuxerData->mMuxGrp, nMuxChn, &pEntry->mMuxChnAttr, pEntry->mSinkInfo.mOutputFd);
                if (SUCCESS == ret)
                {
                    nSuccessFlag = TRUE;
                    alogd("create mux group[%d] channel[%d] success, muxerId[%d]!", pVi2Venc2MuxerData->mMuxGrp, \
                        nMuxChn, pEntry->mMuxChnAttr.mMuxerId);
                    break;
                }
                else if(ERR_MUX_EXIST == ret)
                {
                    nMuxChn++;
                    //break;
                }
                else
                {
                    nMuxChn++;
                }
            }

            if (FALSE == nSuccessFlag)
            {
                pEntry->mMuxChn = MM_INVALID_CHN;
                aloge("fatal error! create mux group[%d] channel fail!", pVi2Venc2MuxerData->mMuxGrp);
            }
            else
            {
                result = SUCCESS;
                pEntry->mMuxChn = nMuxChn;
            }
        }
    }
    else
    {
        aloge("maybe something wrong,mux chn list is empty");
    }
    pthread_mutex_unlock(&pVi2Venc2MuxerData->mMuxChnListLock);

    if ((pVi2Venc2MuxerData->mViDev >= 0 && pVi2Venc2MuxerData->mViChn >= 0) && pVi2Venc2MuxerData->mVeChn >= 0)
    {
        MPP_CHN_S ViChn = {MOD_ID_VIU, pVi2Venc2MuxerData->mViDev, pVi2Venc2MuxerData->mViChn};
        MPP_CHN_S VeChn = {MOD_ID_VENC, 0, pVi2Venc2MuxerData->mVeChn};

        AW_MPI_SYS_Bind(&ViChn, &VeChn);
    }

    if (pVi2Venc2MuxerData->mVeChn >= 0 && pVi2Venc2MuxerData->mMuxGrp >= 0)
    {
        MPP_CHN_S MuxGrp = {MOD_ID_MUX, 0, pVi2Venc2MuxerData->mMuxGrp};
        MPP_CHN_S VeChn = {MOD_ID_VENC, 0, pVi2Venc2MuxerData->mVeChn};

        AW_MPI_SYS_Bind(&VeChn, &MuxGrp);
        pVi2Venc2MuxerData->mCurrentState = REC_PREPARED;
    }

    return result;
}

static ERRORTYPE start(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData)
{
    ERRORTYPE ret = SUCCESS;

    alogd("start");

    ret = AW_MPI_VI_EnableVirChn(pVi2Venc2MuxerData->mViDev, pVi2Venc2MuxerData->mViChn);
    if (ret != SUCCESS)
    {
        alogd("VI enable error!");
        return FAILURE;
    }

    if (pVi2Venc2MuxerData->mVeChn >= 0)
    {
        AW_MPI_VENC_StartRecvPic(pVi2Venc2MuxerData->mVeChn);
    }

    if (pVi2Venc2MuxerData->mMuxGrp >= 0)
    {
        AW_MPI_MUX_StartGrp(pVi2Venc2MuxerData->mMuxGrp);
    }

    pVi2Venc2MuxerData->mCurrentState = REC_RECORDING;

    return ret;
}

static ERRORTYPE openVO(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData){
    int eRet = 0;
    int ViChn = pVi2Venc2MuxerData->mViChn+1;
    AW_MPI_VI_CreateVirChn(pVi2Venc2MuxerData->mViDev, ViChn, NULL);
    
    pVi2Venc2MuxerData->mVoDev = 0;
    AW_MPI_VO_Enable(pVi2Venc2MuxerData->mVoDev);  
    pVi2Venc2MuxerData->mUILayer = HLAY(2, 0);
    AW_MPI_VO_AddOutsideVideoLayer(pVi2Venc2MuxerData->mUILayer);
    AW_MPI_VO_CloseVideoLayer(pVi2Venc2MuxerData->mUILayer); /* close ui layer. */
    /* enable vo layer */
    int hlay0 = 0;
    while(hlay0 < VO_MAX_LAYER_NUM)
    {
        if(SUCCESS == AW_MPI_VO_EnableVideoLayer(hlay0))
        {
            break;
        }
        hlay0++;
    }
    if(hlay0 >= VO_MAX_LAYER_NUM)
    {
        aloge("fatal error! enable video layer fail!");
    }

    VO_PUB_ATTR_S spPubAttr;
    AW_MPI_VO_GetPubAttr(pVi2Venc2MuxerData->mVoDev, &spPubAttr);
    spPubAttr.enIntfType = VO_INTF_LCD;
    spPubAttr.enIntfSync = VO_OUTPUT_NTSC;
    AW_MPI_VO_SetPubAttr(pVi2Venc2MuxerData->mVoDev, &spPubAttr);

    pVi2Venc2MuxerData->mVoLayer = hlay0;
    VO_VIDEO_LAYER_ATTR_S mLayerAttr;
    AW_MPI_VO_GetVideoLayerAttr(pVi2Venc2MuxerData->mVoLayer, &mLayerAttr);
    mLayerAttr.stDispRect.X = 0;
    mLayerAttr.stDispRect.Y = 0;
    mLayerAttr.stDispRect.Width = 320;
    mLayerAttr.stDispRect.Height = 240;
    AW_MPI_VO_SetVideoLayerAttr(pVi2Venc2MuxerData->mVoLayer, &mLayerAttr);

    /* create vo channel and clock channel.  
    (because frame information has 'pts', there is no need clock channel now) 
    */
    BOOL bSuccessFlag = FALSE;
    pVi2Venc2MuxerData->mVOChn = 0;
    while(pVi2Venc2MuxerData->mVOChn < VO_MAX_CHN_NUM)
    {
        eRet = AW_MPI_VO_EnableChn(pVi2Venc2MuxerData->mVoLayer, pVi2Venc2MuxerData->mVOChn);
        if(SUCCESS == eRet)
        {
            bSuccessFlag = TRUE;
            alogd("create vo channel[%d] success!", pVi2Venc2MuxerData->mVOChn);
            break;
        }
        else if(ERR_VO_CHN_NOT_DISABLE == eRet)
        {
            alogd("vo channel[%d] is exist, find next!", pVi2Venc2MuxerData->mVOChn);
            pVi2Venc2MuxerData->mVOChn++;
        }
        else
        {
            aloge("fatal error! create vo channel[%d] ret[0x%x]!", pVi2Venc2MuxerData->mVOChn, eRet);
            break;
        }
    }
    if(FALSE == bSuccessFlag)
    {
        pVi2Venc2MuxerData->mVOChn = MM_INVALID_CHN;
        aloge("fatal error! create vo channel fail!");
    }
    MPPCallbackInfo cbInfo;
    cbInfo.cookie = (void*)pVi2Venc2MuxerData;
    cbInfo.callback = (MPPCallbackFuncType)&VOCallbackWrapper;
    AW_MPI_VO_RegisterCallback(pVi2Venc2MuxerData->mVoLayer, pVi2Venc2MuxerData->mVOChn, &cbInfo);
    AW_MPI_VO_SetChnDispBufNum(pVi2Venc2MuxerData->mVoLayer, pVi2Venc2MuxerData->mVOChn, 2);

    /* bind clock,vo, viChn
    (because frame information has 'pts', there is no need to bind clock channel now)
    */
    MPP_CHN_S VOChn = {MOD_ID_VOU, pVi2Venc2MuxerData->mVoLayer, pVi2Venc2MuxerData->mVOChn};
    MPP_CHN_S VIChn = {MOD_ID_VIU, pVi2Venc2MuxerData->mViDev, ViChn};
    AW_MPI_SYS_Bind(&VIChn, &VOChn);    
    if(eRet != SUCCESS)
    {
        aloge("fatal error! enableVirChn fail!");
    }
    AW_MPI_VI_EnableVirChn(pVi2Venc2MuxerData->mViDev, ViChn);
    AW_MPI_VO_StartChn(pVi2Venc2MuxerData->mVoLayer, pVi2Venc2MuxerData->mVOChn);
    

}

static ERRORTYPE closeVO(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData){
    AW_MPI_VI_DisableVirChn(pVi2Venc2MuxerData->mViDev, pVi2Venc2MuxerData->mViChn+1);

    AW_MPI_VO_StopChn(pVi2Venc2MuxerData->mVoLayer, pVi2Venc2MuxerData->mVOChn);
    AW_MPI_VO_DisableChn(pVi2Venc2MuxerData->mVoLayer, pVi2Venc2MuxerData->mVOChn);
    pVi2Venc2MuxerData->mVOChn = MM_INVALID_CHN;    
    /* disable vo layer */
    AW_MPI_VO_DisableVideoLayer(pVi2Venc2MuxerData->mVoLayer);
    pVi2Venc2MuxerData->mVoLayer = -1;
    AW_MPI_VO_RemoveOutsideVideoLayer(pVi2Venc2MuxerData->mUILayer);
    /* disalbe vo dev */
    AW_MPI_VO_Disable(pVi2Venc2MuxerData->mVoDev);
    pVi2Venc2MuxerData->mVoDev = -1;
    AW_MPI_VI_DestoryVirChn(pVi2Venc2MuxerData->mViDev, pVi2Venc2MuxerData->mViChn+1);
}

static ERRORTYPE stop(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData)
{
    MUX_CHN_INFO_S *pEntry, *pTmp;
    ERRORTYPE ret = SUCCESS;

    alogd("stop");

    if (pVi2Venc2MuxerData->mViChn >= 0)
    {
        AW_MPI_VI_DisableVirChn(pVi2Venc2MuxerData->mViDev, pVi2Venc2MuxerData->mViChn);
    }

    if (pVi2Venc2MuxerData->mVeChn >= 0)
    {
        alogd("stop venc");
        AW_MPI_VENC_StopRecvPic(pVi2Venc2MuxerData->mVeChn);
    }

    if (pVi2Venc2MuxerData->mMuxGrp >= 0)
    {
        alogd("stop mux grp");
        AW_MPI_MUX_StopGrp(pVi2Venc2MuxerData->mMuxGrp);
    }

    if (pVi2Venc2MuxerData->mViChn >= 0)
    {
        AW_MPI_VI_DestoryVirChn(pVi2Venc2MuxerData->mViDev, pVi2Venc2MuxerData->mViChn);
        AW_MPI_VI_DisableVipp(pVi2Venc2MuxerData->mViDev);
        AW_MPI_VI_DestoryVipp(pVi2Venc2MuxerData->mViDev);
        AW_MPI_ISP_Stop(pVi2Venc2MuxerData->mIspDev);
    }

    if (pVi2Venc2MuxerData->mVeChn >= 0)
    {
        alogd("destory venc");
        AW_MPI_VENC_ResetChn(pVi2Venc2MuxerData->mVeChn);
        AW_MPI_VENC_DestroyChn(pVi2Venc2MuxerData->mVeChn);
        pVi2Venc2MuxerData->mVeChn = MM_INVALID_CHN;
    }

    if (pVi2Venc2MuxerData->mMuxGrp >= 0)
    {
        alogd("destory mux grp");
        AW_MPI_MUX_DestroyGrp(pVi2Venc2MuxerData->mMuxGrp);
        pVi2Venc2MuxerData->mMuxGrp = MM_INVALID_CHN;
    }

    pthread_mutex_lock(&pVi2Venc2MuxerData->mMuxChnListLock);
    if (!list_empty(&pVi2Venc2MuxerData->mMuxChnList))
    {
        alogd("free chn list node");
        list_for_each_entry_safe(pEntry, pTmp, &pVi2Venc2MuxerData->mMuxChnList, mList)
        {
            if (pEntry->mSinkInfo.mOutputFd > 0)
            {
                alogd("close file");
                close(pEntry->mSinkInfo.mOutputFd);
                pEntry->mSinkInfo.mOutputFd = -1;
            }

            list_del(&pEntry->mList);
            free(pEntry);
        }
    }
    pthread_mutex_unlock(&pVi2Venc2MuxerData->mMuxChnListLock);

    return SUCCESS;
}

int main(int argc, char** argv)
{
    int result = -1;
    MUX_CHN_INFO_S *pEntry, *pTmp;

	if(FILE_EXIST(DEFAULT_DST_DIR)){
        system("rm /mnt/extsd/video -rf");
    }
    system("mkdir /mnt/extsd/video");

    pVi2Venc2MuxerData_main = (SAMPLE_VI2VENC2MUXER_S* )malloc(sizeof(SAMPLE_VI2VENC2MUXER_S));    
    pVi2Venc2MuxerData_sub = (SAMPLE_VI2VENC2MUXER_S* )malloc(sizeof(SAMPLE_VI2VENC2MUXER_S));
    if (pVi2Venc2MuxerData_main == NULL || pVi2Venc2MuxerData_sub == NULL)
    {
        aloge("malloc struct fail\n");
        return FAILURE;
    }
    memset(pVi2Venc2MuxerData_main, 0, sizeof(SAMPLE_VI2VENC2MUXER_S));
    memset(pVi2Venc2MuxerData_sub, 0, sizeof(SAMPLE_VI2VENC2MUXER_S));
	printf("sample_v459_FQN running!\n");
    if (InitVi2Venc2MuxerData(pVi2Venc2MuxerData_main) != SUCCESS || InitVi2Venc2MuxerData(pVi2Venc2MuxerData_sub) != SUCCESS)
    {
        return -1;
    }

    cdx_sem_init(&pVi2Venc2MuxerData_main->mSemExit, 0);

    if (parseCmdLine(pVi2Venc2MuxerData_main, argc, argv) != SUCCESS)
    {
        aloge("parse cmdline fail");
        goto err_out_0;
    }
    if (loadConfigPara(pVi2Venc2MuxerData_main, pVi2Venc2MuxerData_main->mCmdLinePara.mConfigFilePath, 0) != SUCCESS)
    {
        aloge("load main config file fail");
        goto err_out_0;
    }
    if (loadConfigPara(pVi2Venc2MuxerData_sub, pVi2Venc2MuxerData_main->mCmdLinePara.mConfigFilePath, 1) != SUCCESS)
    {
        aloge("load sub config file fail");
        goto err_out_0;
    }
    INIT_LIST_HEAD(&pVi2Venc2MuxerData_main->mMuxChnList);
    pthread_mutex_init(&pVi2Venc2MuxerData_main->mMuxChnListLock, NULL);    
    INIT_LIST_HEAD(&pVi2Venc2MuxerData_sub->mMuxChnList);
    pthread_mutex_init(&pVi2Venc2MuxerData_sub->mMuxChnListLock, NULL);
    pVi2Venc2MuxerData_main->mSysConf.nAlignWidth = 32;    
    AW_MPI_SYS_SetConf(&pVi2Venc2MuxerData_main->mSysConf);
    AW_MPI_SYS_Init();

    pVi2Venc2MuxerData_main->mMuxId[0] = addOutputFormatAndOutputSink(pVi2Venc2MuxerData_main, pVi2Venc2MuxerData_main->mConfigPara.dstVideoFile, MEDIA_FILE_FORMAT_MP4);
    if (pVi2Venc2MuxerData_main->mMuxId[0] < 0)
    {
        aloge("add main first out file fail");
        goto err_out_1;
    }

    pVi2Venc2MuxerData_sub->mMuxId[0] = addOutputFormatAndOutputSink(pVi2Venc2MuxerData_sub, pVi2Venc2MuxerData_sub->mConfigPara.dstVideoFile, MEDIA_FILE_FORMAT_MP4);
    if (pVi2Venc2MuxerData_sub->mMuxId[0] < 0)
    {
        aloge("add sub first out file fail");
        goto err_out_1;
    }

    if (prepare(pVi2Venc2MuxerData_main) != SUCCESS)
    {
        aloge("prepare main fail!");
        goto err_out_2;
    }
    if (prepare(pVi2Venc2MuxerData_sub) != SUCCESS)
    {
        aloge("prepare sub fail!");
        goto err_out_2;
    }

    start(pVi2Venc2MuxerData_main);
    start(pVi2Venc2MuxerData_sub);
    openVO(pVi2Venc2MuxerData_sub);
    sleep(10);

    if (pVi2Venc2MuxerData_main->mConfigPara.mTestDuration > 0)
    {
        cdx_sem_down_timedwait(&pVi2Venc2MuxerData_main->mSemExit, pVi2Venc2MuxerData_main->mConfigPara.mTestDuration*1000);
    }
    else
    {
        cdx_sem_down(&pVi2Venc2MuxerData_main->mSemExit);
    }

    stop(pVi2Venc2MuxerData_main);
    closeVO(pVi2Venc2MuxerData_sub);
    stop(pVi2Venc2MuxerData_sub);
    result = 0;

    alogd("start to free res");
err_out_2:
    pthread_mutex_lock(&pVi2Venc2MuxerData_main->mMuxChnListLock);
    if (!list_empty(&pVi2Venc2MuxerData_main->mMuxChnList))
    {
        alogd("chn list not empty");
        list_for_each_entry_safe(pEntry, pTmp, &pVi2Venc2MuxerData_main->mMuxChnList, mList)
        {
            if (pEntry->mSinkInfo.mOutputFd > 0)
            {
                close(pEntry->mSinkInfo.mOutputFd);
                pEntry->mSinkInfo.mOutputFd = -1;
            }
        }

        list_del(&pEntry->mList);
        free(pEntry);
    }
    pthread_mutex_unlock(&pVi2Venc2MuxerData_main->mMuxChnListLock);

    pthread_mutex_lock(&pVi2Venc2MuxerData_sub->mMuxChnListLock);
    if (!list_empty(&pVi2Venc2MuxerData_sub->mMuxChnList))
    {
        alogd("chn list not empty");
        list_for_each_entry_safe(pEntry, pTmp, &pVi2Venc2MuxerData_sub->mMuxChnList, mList)
        {
            if (pEntry->mSinkInfo.mOutputFd > 0)
            {
                close(pEntry->mSinkInfo.mOutputFd);
                pEntry->mSinkInfo.mOutputFd = -1;
            }
        }

        list_del(&pEntry->mList);
        free(pEntry);
    }
    pthread_mutex_unlock(&pVi2Venc2MuxerData_sub->mMuxChnListLock);
err_out_1:
    AW_MPI_SYS_Exit();

    pthread_mutex_destroy(&pVi2Venc2MuxerData_main->mMuxChnListLock);
    pthread_mutex_destroy(&pVi2Venc2MuxerData_sub->mMuxChnListLock);
err_out_0:
    cdx_sem_deinit(&pVi2Venc2MuxerData_main->mSemExit);
    free(pVi2Venc2MuxerData_main);
    pVi2Venc2MuxerData_main = NULL;
    pVi2Venc2MuxerData_sub = NULL;
	if (result == 0) {
		printf("sample_v459_BGA exit!\n");
	}

    return result;
}
