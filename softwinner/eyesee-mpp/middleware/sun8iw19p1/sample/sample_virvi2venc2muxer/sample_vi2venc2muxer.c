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
#define LOG_TAG "SampleVi2Venc2Muxer"

#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "plat_log.h"
#include <mm_common.h>

#include "sample_vi2venc2muxer.h"
#include "sample_vi2venc2muxer_conf.h"


#define DEFAULT_SRC_SIZE   1080
#define DEFAULT_DST_VIDEO_FILE      "/mnt/extsd/sample_vi2venc2muxer/1080p.mp4"
#define DEFAULT_SRC_SIZE   1080

#define DEFAULT_MAX_DURATION  60*1000
#define DEFAULT_DST_VIDEO_FRAMERATE 30
#define DEFAULT_DST_VIDEO_BITRATE 12*1000*1000

#define DEFAULT_SRC_PIXFMT   MM_PIXEL_FORMAT_YUV_PLANAR_420
#define DEFAULT_DST_PIXFMT   MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420
#define DEFAULT_ENCODER   PT_H264

#define DEFAULT_SIMPLE_CACHE_SIZE_VFS       (4*1024)


//#define DOUBLE_ENCODER_FILE_OUT


static SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData;

static void handle_exit(int signo)
{
    alogd("user want to exit!");
    if(NULL != pVi2Venc2MuxerData)
    {
        cdx_sem_up(&pVi2Venc2MuxerData->mSemExit);
    }
}

static int setOutputFileSync(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData, char* path, int64_t fallocateLength, int muxerId);


static ERRORTYPE InitVi2Venc2MuxerData(void)
{
    pVi2Venc2MuxerData = (SAMPLE_VI2VENC2MUXER_S* )malloc(sizeof(SAMPLE_VI2VENC2MUXER_S));
    if (pVi2Venc2MuxerData == NULL)
    {
        aloge("malloc struct fail");
        return FAILURE;
    }

    memset(pVi2Venc2MuxerData, 0, sizeof(SAMPLE_VI2VENC2MUXER_S));

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

    pVi2Venc2MuxerData->mConfigPara.mMaxFileDuration = DEFAULT_MAX_DURATION;
    pVi2Venc2MuxerData->mConfigPara.mVideoFrameRate = DEFAULT_DST_VIDEO_FRAMERATE;
    pVi2Venc2MuxerData->mConfigPara.mVideoBitRate = DEFAULT_DST_VIDEO_BITRATE;

    pVi2Venc2MuxerData->mConfigPara.srcPixFmt = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;    //MM_PIXEL_FORMAT_YUV_AW_LBC_2_5X
    //pVi2Venc2MuxerData->mConfigPara.dstPixFmt = DEFAULT_DST_PIXFMT;
    pVi2Venc2MuxerData->mConfigPara.mVideoEncoderFmt = DEFAULT_ENCODER;
    pVi2Venc2MuxerData->mConfigPara.mField = VIDEO_FIELD_FRAME;

    pVi2Venc2MuxerData->mConfigPara.mColor2Grey = FALSE;
    pVi2Venc2MuxerData->mConfigPara.m3DNR = 0;

    pVi2Venc2MuxerData->mMuxGrp = MM_INVALID_CHN;
    pVi2Venc2MuxerData->mVeChn = MM_INVALID_CHN;
    pVi2Venc2MuxerData->mViChn = MM_INVALID_CHN;
    pVi2Venc2MuxerData->mViDev = MM_INVALID_DEV;

    strcpy(pVi2Venc2MuxerData->mConfigPara.dstVideoFile, DEFAULT_DST_VIDEO_FILE);
    int i=0;
    for (i = 0; i < 2; i++)
    {
        INIT_LIST_HEAD(&pVi2Venc2MuxerData->mMuxerFileListArray[i]);
    }

    pVi2Venc2MuxerData->mCurrentState = REC_NOT_PREPARED;

    

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
                "\t-path /home/sample_vi2venc2muxer.conf\n");
            break;
       }
       else if (*argv)
       {
          argv++;
       }
    }

    return ret;
}

static ERRORTYPE loadConfigPara(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData, const char *conf_path)
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

    pVi2Venc2MuxerData->mConfigPara.mDevNo = GetConfParaInt(&mConf, CFG_SRC_DEV_NODE, 0);

    pVi2Venc2MuxerData->mConfigPara.srcSize = GetConfParaInt(&mConf, CFG_SRC_SIZE, 0);
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

    pVi2Venc2MuxerData->mConfigPara.dstSize = GetConfParaInt(&mConf, CFG_DST_VIDEO_SIZE, 0);
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
    ptr = (char *)GetConfParaString(&mConf, CFG_DST_VIDEO_FILE_STR, NULL);
    strcpy(pVi2Venc2MuxerData->mConfigPara.dstVideoFile, ptr);
    //parse dst directory form dst file path.
    char *pLastSlash = strrchr(pVi2Venc2MuxerData->mConfigPara.dstVideoFile, '/');
    if(pLastSlash != NULL)
    {
        int dirLen = pLastSlash-pVi2Venc2MuxerData->mConfigPara.dstVideoFile;
        strncpy(pVi2Venc2MuxerData->mDstDir, pVi2Venc2MuxerData->mConfigPara.dstVideoFile, dirLen);
        pVi2Venc2MuxerData->mDstDir[dirLen] = '\0';
        
        char *pFileName = pLastSlash+1;
        strcpy(pVi2Venc2MuxerData->mFirstFileName, pFileName);
    }
    else
    {
        strcpy(pVi2Venc2MuxerData->mDstDir, "");
        strcpy(pVi2Venc2MuxerData->mFirstFileName, pVi2Venc2MuxerData->mConfigPara.dstVideoFile);
    }
    pVi2Venc2MuxerData->mConfigPara.mDstFileMaxCnt = GetConfParaInt(&mConf, CFG_DST_FILE_MAX_CNT, 0);
    pVi2Venc2MuxerData->mConfigPara.mVideoFrameRate = GetConfParaInt(&mConf, CFG_DST_VIDEO_FRAMERATE, 0);
    pVi2Venc2MuxerData->mConfigPara.mVideoBitRate = GetConfParaInt(&mConf, CFG_DST_VIDEO_BITRATE, 0);
    pVi2Venc2MuxerData->mConfigPara.mMaxFileDuration = GetConfParaInt(&mConf, CFG_DST_VIDEO_DURATION, 0);

    pVi2Venc2MuxerData->mConfigPara.mRcMode = GetConfParaInt(&mConf, CFG_RC_MODE, 0);
    pVi2Venc2MuxerData->mConfigPara.mGopMode = GetConfParaInt(&mConf, CFG_GOP_MODE, 0);
    pVi2Venc2MuxerData->mConfigPara.mAdvancedRef_Base = GetConfParaInt(&mConf, CFG_AdvancedRef_Base, 0);
    pVi2Venc2MuxerData->mConfigPara.mAdvancedRef_Enhance = GetConfParaInt(&mConf, CFG_AdvancedRef_Enhance, 0);
    pVi2Venc2MuxerData->mConfigPara.mAdvancedRef_RefBaseEn = GetConfParaInt(&mConf, CFG_AdvancedRef_RefBaseEn, 0);
    pVi2Venc2MuxerData->mConfigPara.mEnableFastEnc = GetConfParaInt(&mConf, CFG_FAST_ENC, 0);
    pVi2Venc2MuxerData->mConfigPara.mbEnableSmart = GetConfParaBoolean(&mConf, CFG_ENABLE_SMART, 0);
    pVi2Venc2MuxerData->mConfigPara.mSVCLayer = GetConfParaInt(&mConf, CFG_SVC_LAYER, 0);
    pVi2Venc2MuxerData->mConfigPara.mEncodeRotate = GetConfParaInt(&mConf, CFG_ENCODE_ROTATE, 0);

    ptr	= (char *)GetConfParaString(&mConf, CFG_DST_VIDEO_ENCODER, NULL);
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

    pVi2Venc2MuxerData->mConfigPara.m3DNR = GetConfParaInt(&mConf, CFG_3DNR, 0);
    pVi2Venc2MuxerData->mConfigPara.mRoiNum = GetConfParaInt(&mConf, CFG_ROI_NUM, 0);
    pVi2Venc2MuxerData->mConfigPara.mbRoiBgFrameRate = GetConfParaBoolean(&mConf, CFG_ROI_BgFrameRate, 0);
    pVi2Venc2MuxerData->mConfigPara.mIntraRefreshBlockNum = GetConfParaInt(&mConf, CFG_IntraRefresh_BlockNum, 0);
    pVi2Venc2MuxerData->mConfigPara.mOrlNum = GetConfParaInt(&mConf, CFG_ORL_NUM, 0);
    pVi2Venc2MuxerData->mConfigPara.mVbvBufferSize = GetConfParaInt(&mConf, CFG_vbvBufferSize, 0);
    pVi2Venc2MuxerData->mConfigPara.mVbvThreshSize = GetConfParaInt(&mConf, CFG_vbvThreshSize, 0);
    destroyConfParser(&mConf);
    return SUCCESS;
}

static unsigned long long GetNowTimeUs(void)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return now.tv_sec * 1000000 + now.tv_usec;
}

static int getFileNameByCurTime(char *pNameBuf)
{
#if 0
    sprintf(pNameBuf, "%s", "/mnt/extsd/sample_mux/");
    sprintf(pNameBuf, "%s%llud.mp4", pNameBuf, GetNowTimeUs());
#else
    static int file_cnt = 0;
    char strStemPath[MAX_FILE_PATH_LEN] = {0};
    int len = strlen(pVi2Venc2MuxerData->mConfigPara.dstVideoFile);
    char *ptr = pVi2Venc2MuxerData->mConfigPara.dstVideoFile;
    while (*(ptr+len-1) != '.')
    {
        len--;
    }

    ++file_cnt;
    strncpy(strStemPath, pVi2Venc2MuxerData->mConfigPara.dstVideoFile, len-1);
    sprintf(pNameBuf, "%s_%d.mp4", strStemPath, file_cnt);
#endif
    return 0;
}

static ERRORTYPE MPPCallbackWrapper(void *cookie, MPP_CHN_S *pChn, MPP_EVENT_TYPE event, void *pEventData)
{
    SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData = (SAMPLE_VI2VENC2MUXER_S *)cookie;

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
                int ret;
                int muxerId = *(int*)pEventData;
                alogd("file done, mux_id=%d", muxerId);
                int idx=-1;
                if (muxerId == pVi2Venc2MuxerData->mMuxId[0])
                {
                    idx = 0;
                }
            #ifdef DOUBLE_ENCODER_FILE_OUT
                else if(muxerId == pVi2Venc2MuxerData->mMuxId[1])
                {
                    idx = 1;
                }
            #endif
                if (idx >= 0)
                {
                    int cnt = 0;
                    struct list_head *pList;
                    list_for_each(pList, &pVi2Venc2MuxerData->mMuxerFileListArray[idx]){cnt++;} 
                    FilePathNode *pNode = NULL;
                    while(cnt > pVi2Venc2MuxerData->mConfigPara.mDstFileMaxCnt)
                    {
                        pNode = list_first_entry(&pVi2Venc2MuxerData->mMuxerFileListArray[idx], FilePathNode, mList);
                        if ((ret = remove(pNode->strFilePath)) != 0)
                        {
                            aloge("fatal error! delete file[%s] failed:%s", pNode->strFilePath, strerror(errno));
                        }
                        else
                        {
                            alogd("delete file[%s] success", pNode->strFilePath);
                        }
                        cnt--;
                        list_del(&pNode->mList);
                        free(pNode);
                    }
                }
                break;
            }
            case MPP_EVENT_NEED_NEXT_FD:
            {
                int muxerId = *(int*)pEventData;
                SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData = (SAMPLE_VI2VENC2MUXER_S *)cookie;
                char fileName[MAX_FILE_PATH_LEN] = {0};

                if (muxerId == pVi2Venc2MuxerData->mMuxId[0])
                {
                    getFileNameByCurTime(fileName);
                    FilePathNode *pFilePathNode = (FilePathNode*)malloc(sizeof(FilePathNode));
                    memset(pFilePathNode, 0, sizeof(FilePathNode));
                    strncpy(pFilePathNode->strFilePath, fileName, MAX_FILE_PATH_LEN-1);
                    list_add_tail(&pFilePathNode->mList, &pVi2Venc2MuxerData->mMuxerFileListArray[0]);
                }
            #ifdef DOUBLE_ENCODER_FILE_OUT
                else if(muxerId == pVi2Venc2MuxerData->mMuxId[1])
                {
                    //strcpy(fileName, "/mnt/extsd/sample_vi2venc2muxer/");
                    static int cnt = 0;
                    cnt++;
                    sprintf(fileName, "/mnt/extsd/sample_vi2venc2muxer/%d.ts", cnt);
                    FilePathNode *pFilePathNode = (FilePathNode*)malloc(sizeof(FilePathNode));
                    memset(pFilePathNode, 0, sizeof(FilePathNode));
                    strncpy(pFilePathNode->strFilePath, fileName, MAX_FILE_PATH_LEN-1);
                    list_add_tail(&pFilePathNode->mList, &pVi2Venc2MuxerData->mMuxerFileListArray[1]);
                }
            #endif
                alogd("muxId[%d] set next fd, filepath=%s", muxerId, fileName);
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

static ERRORTYPE configMuxGrpAttr(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData)
{
    memset(&pVi2Venc2MuxerData->mMuxGrpAttr, 0, sizeof(MUX_GRP_ATTR_S));

    pVi2Venc2MuxerData->mMuxGrpAttr.mVideoEncodeType = pVi2Venc2MuxerData->mConfigPara.mVideoEncoderFmt;
    pVi2Venc2MuxerData->mMuxGrpAttr.mWidth = pVi2Venc2MuxerData->mConfigPara.dstWidth;
    pVi2Venc2MuxerData->mMuxGrpAttr.mHeight = pVi2Venc2MuxerData->mConfigPara.dstHeight;
    pVi2Venc2MuxerData->mMuxGrpAttr.mVideoFrmRate = pVi2Venc2MuxerData->mConfigPara.mVideoFrameRate*1000;
    //pVi2Venc2MuxerData->mMuxGrpAttr.mMaxKeyInterval = 
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
    //pVi2Venc2MuxerData->mVencChnAttr.VeAttr.MaxKeyInterval = pVi2Venc2MuxerData->mVideoMaxKeyItl;
    pVi2Venc2MuxerData->mVencChnAttr.VeAttr.SrcPicWidth  = pVi2Venc2MuxerData->mConfigPara.srcWidth;
    pVi2Venc2MuxerData->mVencChnAttr.VeAttr.SrcPicHeight = pVi2Venc2MuxerData->mConfigPara.srcHeight;
    pVi2Venc2MuxerData->mVencChnAttr.VeAttr.Field = pVi2Venc2MuxerData->mConfigPara.mField;
    pVi2Venc2MuxerData->mVencChnAttr.VeAttr.PixelFormat = pVi2Venc2MuxerData->mConfigPara.srcPixFmt;
    switch(pVi2Venc2MuxerData->mConfigPara.mEncodeRotate)
    {
        case 90:
            pVi2Venc2MuxerData->mVencChnAttr.VeAttr.Rotate = ROTATE_90;
            break;
        case 180:
            pVi2Venc2MuxerData->mVencChnAttr.VeAttr.Rotate = ROTATE_180;
            break;
        case 270:
            pVi2Venc2MuxerData->mVencChnAttr.VeAttr.Rotate = ROTATE_270;
            break;
        default:
            pVi2Venc2MuxerData->mVencChnAttr.VeAttr.Rotate = ROTATE_NONE;
            break;
    }

    if (PT_H264 == pVi2Venc2MuxerData->mVencChnAttr.VeAttr.Type)
    {
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH264e.BufSize = pVi2Venc2MuxerData->mConfigPara.mVbvBufferSize;
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH264e.mThreshSize = pVi2Venc2MuxerData->mConfigPara.mVbvThreshSize;
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH264e.bByFrame = TRUE;
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH264e.Profile = 2;//0:base 1:main 2:high
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH264e.mLevel = H264_LEVEL_51;
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH264e.PicWidth  = pVi2Venc2MuxerData->mConfigPara.dstWidth;
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH264e.PicHeight = pVi2Venc2MuxerData->mConfigPara.dstHeight;
        switch (pVi2Venc2MuxerData->mConfigPara.mRcMode)
        {
        case 1:
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264VBR;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264Vbr.mMinQp = 10;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264Vbr.mMaxQp = 51;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264Vbr.mMaxBitRate = pVi2Venc2MuxerData->mConfigPara.mVideoBitRate;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264Vbr.mMovingTh = 20;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264Vbr.mQuality = 5;
            break;
        case 2:
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264FIXQP;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264FixQp.mIQp = 28;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264FixQp.mPQp = 28;
            break;
        case 3:
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264ABR;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264Abr.mMaxBitRate = pVi2Venc2MuxerData->mConfigPara.mVideoBitRate;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264Abr.mRatioChangeQp = 85;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264Abr.mQuality = 8;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264Abr.mMinIQp = 20;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264Abr.mMaxQp = 51;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264Abr.mMinQp = 10;
            break;
        case 0:
        default:
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264CBR;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264Cbr.mBitRate = pVi2Venc2MuxerData->mConfigPara.mVideoBitRate;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264Cbr.mMaxQp = 51;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH264Cbr.mMinQp = 1;
            break;
        }
        if (pVi2Venc2MuxerData->mConfigPara.mEnableFastEnc)
        {
            pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH264e.FastEncFlag = TRUE;
        }
    }
    else if (PT_H265 == pVi2Venc2MuxerData->mVencChnAttr.VeAttr.Type)
    {
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH265e.mBufSize = pVi2Venc2MuxerData->mConfigPara.mVbvBufferSize;
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH265e.mThreshSize = pVi2Venc2MuxerData->mConfigPara.mVbvThreshSize;
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH265e.mbByFrame = TRUE;
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH265e.mProfile = 0;//0:main 1:main10 2:sti11
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH265e.mLevel = H265_LEVEL_62;
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH265e.mPicWidth = pVi2Venc2MuxerData->mConfigPara.dstWidth;
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH265e.mPicHeight = pVi2Venc2MuxerData->mConfigPara.dstHeight;
        pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265Cbr.mBitRate = pVi2Venc2MuxerData->mConfigPara.mVideoBitRate;
        switch (pVi2Venc2MuxerData->mConfigPara.mRcMode)
        {
        case 1:
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H265VBR;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265Vbr.mMinQp = 10;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265Vbr.mMaxQp = 51;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265Vbr.mMaxBitRate = pVi2Venc2MuxerData->mConfigPara.mVideoBitRate;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265Vbr.mMovingTh = 20;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265Vbr.mQuality = 5;
            break;
        case 2:
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H265FIXQP;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265FixQp.mIQp = 28;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265FixQp.mPQp = 28;
            break;
        case 3:
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H265ABR;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265Abr.mMaxBitRate = pVi2Venc2MuxerData->mConfigPara.mVideoBitRate;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265Abr.mRatioChangeQp = 85;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265Abr.mQuality = 8;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265Abr.mMinIQp = 20;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265Abr.mMaxQp = 51;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265Abr.mMinQp = 10;
            break;
        case 0:
        default:
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H265CBR;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265Cbr.mSrcFrmRate = pVi2Venc2MuxerData->mConfigPara.mVideoFrameRate;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265Cbr.mBitRate = pVi2Venc2MuxerData->mConfigPara.mVideoBitRate;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265Cbr.mMaxQp = 51;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrH265Cbr.mMinQp = 1;
            break;
        }
        if (pVi2Venc2MuxerData->mConfigPara.mEnableFastEnc)
        {
            pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH265e.mFastEncFlag = TRUE;
        }
    }
    else if (PT_MJPEG == pVi2Venc2MuxerData->mVencChnAttr.VeAttr.Type)
    {
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrMjpeg.mBufSize = pVi2Venc2MuxerData->mConfigPara.mVbvBufferSize;
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrMjpeg.mbByFrame = TRUE;
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrMjpeg.mPicWidth = pVi2Venc2MuxerData->mConfigPara.dstWidth;
        pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrMjpeg.mPicHeight = pVi2Venc2MuxerData->mConfigPara.dstHeight;
        switch (pVi2Venc2MuxerData->mConfigPara.mRcMode)
        {
        case 0:
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_MJPEGCBR;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrMjpegeCbr.mBitRate = pVi2Venc2MuxerData->mConfigPara.mVideoBitRate;
            break;
        case 1:
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_MJPEGFIXQP;
            pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mAttrMjpegeFixQp.mQfactor = 40;
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

    if(0 == pVi2Venc2MuxerData->mConfigPara.mGopMode)
    {
        pVi2Venc2MuxerData->mVencChnAttr.GopAttr.enGopMode = VENC_GOPMODE_NORMALP;
    }
    else if(1 == pVi2Venc2MuxerData->mConfigPara.mGopMode)
    {
        pVi2Venc2MuxerData->mVencChnAttr.GopAttr.enGopMode = VENC_GOPMODE_DUALP;
    }
    else if(2 == pVi2Venc2MuxerData->mConfigPara.mGopMode)
    {
        pVi2Venc2MuxerData->mVencChnAttr.GopAttr.enGopMode = VENC_GOPMODE_SMARTP;
        pVi2Venc2MuxerData->mVencChnAttr.GopAttr.stSmartP.mVirtualIFrameInterval = 15;
    }
    pVi2Venc2MuxerData->mVencChnAttr.GopAttr.mGopSize = pVi2Venc2MuxerData->mConfigPara.mVideoFrameRate;

    return SUCCESS;
}

static ERRORTYPE createVencChn(SAMPLE_VI2VENC2MUXER_S *pVi2Venc2MuxerData)
{
    ERRORTYPE ret;
    BOOL nSuccessFlag = FALSE;

    configVencChnAttr(pVi2Venc2MuxerData);
    pVi2Venc2MuxerData->mVeChn = 0;
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

        VENC_PARAM_REF_S stRefParam;
        memset(&stRefParam, 0, sizeof(VENC_PARAM_REF_S));
        stRefParam.Base = pVi2Venc2MuxerData->mConfigPara.mAdvancedRef_Base;
        stRefParam.Enhance = pVi2Venc2MuxerData->mConfigPara.mAdvancedRef_Enhance;
        stRefParam.bEnablePred = pVi2Venc2MuxerData->mConfigPara.mAdvancedRef_RefBaseEn;
        AW_MPI_VENC_SetRefParam(pVi2Venc2MuxerData->mVeChn, &stRefParam);

        AW_MPI_VENC_Set3DNR(pVi2Venc2MuxerData->mVeChn, pVi2Venc2MuxerData->mConfigPara.m3DNR);

        //test PIntraRefresh
        if(pVi2Venc2MuxerData->mConfigPara.mIntraRefreshBlockNum > 0)
        {
            VENC_PARAM_INTRA_REFRESH_S stIntraRefresh;
            memset(&stIntraRefresh, 0, sizeof(VENC_PARAM_INTRA_REFRESH_S));
            stIntraRefresh.bRefreshEnable = TRUE;
            stIntraRefresh.RefreshLineNum = pVi2Venc2MuxerData->mConfigPara.mIntraRefreshBlockNum;
            ret = AW_MPI_VENC_SetIntraRefresh(pVi2Venc2MuxerData->mVeChn, &stIntraRefresh);
            if(ret != SUCCESS)
            {
                aloge("fatal error! set roiBgFrameRate fail[0x%x]!", ret);
            }
            else
            {
                alogd("set intra refresh:%d", stIntraRefresh.RefreshLineNum);
            }
        }

        if(pVi2Venc2MuxerData->mConfigPara.mbEnableSmart)
        {
            VencSmartFun smartParam;
            memset(&smartParam, 0, sizeof(VencSmartFun));
            smartParam.smart_fun_en = 1;
            smartParam.img_bin_en = 1;
            smartParam.img_bin_th = 0;
            smartParam.shift_bits = 2;
            AW_MPI_VENC_SetSmartP(pVi2Venc2MuxerData->mVeChn, &smartParam);
        }

        if(pVi2Venc2MuxerData->mConfigPara.mSVCLayer > 0)
        {
            VencH264SVCSkip stSVCSkip;
            memset(&stSVCSkip, 0, sizeof(VencH264SVCSkip));
            stSVCSkip.nTemporalSVC = pVi2Venc2MuxerData->mConfigPara.mSVCLayer;
            AW_MPI_VENC_SetH264SVCSkip(pVi2Venc2MuxerData->mVeChn, &stSVCSkip);
        }
        
        MPPCallbackInfo cbInfo;
        cbInfo.cookie = (void*)pVi2Venc2MuxerData;
        cbInfo.callback = (MPPCallbackFuncType)&MPPCallbackWrapper;
        AW_MPI_VENC_RegisterCallback(pVi2Venc2MuxerData->mVeChn, &cbInfo);

        if ( ((PT_H264 == pVi2Venc2MuxerData->mVencChnAttr.VeAttr.Type) || (PT_H265 == pVi2Venc2MuxerData->mVencChnAttr.VeAttr.Type))
            && ((VENC_RC_MODE_H264QPMAP == pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode) || (VENC_RC_MODE_H265QPMAP == pVi2Venc2MuxerData->mVencChnAttr.RcAttr.mRcMode))
           )
        {
            aloge("fatal error! not support qpmap currently!");
            /*
            unsigned int width, heigth;
            int num;
            VENC_PARAM_H264_QPMAP_S QpMap;

            width = pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH264e.PicWidth;
            heigth = pVi2Venc2MuxerData->mVencChnAttr.VeAttr.AttrH264e.PicHeight;
            num = (ALIGN(width, 16) >> 4) * (ALIGN(heigth, 16) >> 4);

            QpMap.num = num;
            QpMap.p_info = (VENC_QPMAP_BLOCK_QP_INFO *)malloc(sizeof(VENC_QPMAP_BLOCK_QP_INFO) * num);
            if (QpMap.p_info == NULL)
            {
                aloge("QPmap buffer malloc fail!!");
            }
            else
            {
                int i;
                for (i = 0; i < num / 2; i++)
                {
                    QpMap.p_info[i].mb_en = 1;
                    QpMap.p_info[i].mb_skip_flag = 0;
                    QpMap.p_info[i].mb_qp = 10;
                }
                for (; i < num; i++)
                {
                    QpMap.p_info[i].mb_en = 1;
                    QpMap.p_info[i].mb_skip_flag = 0;
                    QpMap.p_info[i].mb_qp = 42;
                }

                alogd("set QPMAP");
                AW_MPI_VENC_SetH264QPMAP(pVi2Venc2MuxerData->mVeChn, (const VENC_PARAM_H264_QPMAP_S *)&QpMap);
                free(QpMap.p_info);
            }
            */
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
    pVi2Venc2MuxerData->mViAttr.format.pixelformat = map_PIXEL_FORMAT_E_to_V4L2_PIX_FMT(pVi2Venc2MuxerData->mConfigPara.srcPixFmt);
    pVi2Venc2MuxerData->mViAttr.format.field = V4L2_FIELD_NONE;
    //pVi2Venc2MuxerData->mViAttr.format.colorspace = V4L2_COLORSPACE_JPEG;
    pVi2Venc2MuxerData->mViAttr.format.width = pVi2Venc2MuxerData->mConfigPara.srcWidth;
    pVi2Venc2MuxerData->mViAttr.format.height = pVi2Venc2MuxerData->mConfigPara.srcHeight;
    pVi2Venc2MuxerData->mViAttr.nbufs = 5;
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

ERRORTYPE SampleVI2Venc2Muxer_CreateFolder(const char* pStrFolderPath)
{
    if(NULL == pStrFolderPath || 0 == strlen(pStrFolderPath))
    {
        aloge("folder path is wrong!");
        return FAILURE;
    }
    //check folder existence
    struct stat sb;
    if (stat(pStrFolderPath, &sb) == 0)
    {
        if(S_ISDIR(sb.st_mode))
        {
            return SUCCESS;
        }
        else
        {
            aloge("fatal error! [%s] is exist, but mode[0x%x] is not directory!", pStrFolderPath, sb.st_mode);
            return FAILURE;
        }
    }
    //create folder if necessary
    int ret = mkdir(pStrFolderPath, S_IRWXU | S_IRWXG | S_IRWXO);
    if(!ret)
    {
        alogd("create folder[%s] success", pStrFolderPath);
        return SUCCESS;
    }
    else
    {
        aloge("fatal error! create folder[%s] failed!", pStrFolderPath);
        return FAILURE;
    }
}

int main(int argc, char** argv)
{
    int result = -1;
    MUX_CHN_INFO_S *pEntry, *pTmp;
    GLogConfig stGLogConfig = 
    {
        .FLAGS_logtostderr = 1,
        .FLAGS_colorlogtostderr = 1,
        .FLAGS_stderrthreshold = _GLOG_INFO,
        .FLAGS_minloglevel = _GLOG_INFO,
        .FLAGS_logbuflevel = -1,
        .FLAGS_logbufsecs = 0,
        .FLAGS_max_log_size = 1,
        .FLAGS_stop_logging_if_full_disk = 1,
    };
    strcpy(stGLogConfig.LogDir, "/tmp/log");
    strcpy(stGLogConfig.InfoLogFileNameBase, "LOG-");
    strcpy(stGLogConfig.LogFileNameExtension, "IPC-");
    log_init(argv[0], &stGLogConfig);
    
	printf("sample_virvi2venc2muxer running!\n");
    
    if (InitVi2Venc2MuxerData() != SUCCESS)
    {
        return -1;
    }

    cdx_sem_init(&pVi2Venc2MuxerData->mSemExit, 0);

    /* register process function for SIGINT, to exit program. */
    if (signal(SIGINT, handle_exit) == SIG_ERR)
    {
        aloge("can't catch SIGSEGV");
    }

    if (parseCmdLine(pVi2Venc2MuxerData, argc, argv) != SUCCESS)
    {
        aloge("parse cmdline fail");
        goto err_out_0;
    }

    if (loadConfigPara(pVi2Venc2MuxerData, pVi2Venc2MuxerData->mCmdLinePara.mConfigFilePath) != SUCCESS)
    {
        aloge("load config file fail");
        goto err_out_0;
    }
    SampleVI2Venc2Muxer_CreateFolder(pVi2Venc2MuxerData->mDstDir);

    INIT_LIST_HEAD(&pVi2Venc2MuxerData->mMuxChnList);
    pthread_mutex_init(&pVi2Venc2MuxerData->mMuxChnListLock, NULL);

    pVi2Venc2MuxerData->mSysConf.nAlignWidth = 32;
    AW_MPI_SYS_SetConf(&pVi2Venc2MuxerData->mSysConf);
    AW_MPI_SYS_Init();

    pVi2Venc2MuxerData->mMuxId[0] = addOutputFormatAndOutputSink(pVi2Venc2MuxerData, pVi2Venc2MuxerData->mConfigPara.dstVideoFile, MEDIA_FILE_FORMAT_MP4);
    if (pVi2Venc2MuxerData->mMuxId[0] < 0)
    {
        aloge("add first out file fail");
        goto err_out_1;
    }
    FilePathNode *pFilePathNode = (FilePathNode*)malloc(sizeof(FilePathNode));
    memset(pFilePathNode, 0, sizeof(FilePathNode));
    strncpy(pFilePathNode->strFilePath, pVi2Venc2MuxerData->mConfigPara.dstVideoFile, MAX_FILE_PATH_LEN-1);
    list_add_tail(&pFilePathNode->mList, &pVi2Venc2MuxerData->mMuxerFileListArray[0]);

#ifdef DOUBLE_ENCODER_FILE_OUT
    char mov_path[MAX_FILE_PATH_LEN];
    strcpy(mov_path, "/mnt/extsd/sample_vi2venc2muxer/0.ts");
    pVi2Venc2MuxerData->mMuxId[1] = addOutputFormatAndOutputSink(pVi2Venc2MuxerData, mov_path, MEDIA_FILE_FORMAT_TS);
    if (pVi2Venc2MuxerData->mMuxId[1] < 0)
    {
        alogd("add mMuxId[1] ts file sink fail");
    }
    else
    {
        FilePathNode *pFilePathNode = (FilePathNode*)malloc(sizeof(FilePathNode));
        memset(pFilePathNode, 0, sizeof(FilePathNode));
        strncpy(pFilePathNode->strFilePath, mov_path, MAX_FILE_PATH_LEN-1);
        list_add_tail(&pFilePathNode->mList, &pVi2Venc2MuxerData->mMuxerFileListArray[1]);
    }
#endif

    if (prepare(pVi2Venc2MuxerData) != SUCCESS)
    {
        aloge("prepare fail!");
        goto err_out_2;
    }

    start(pVi2Venc2MuxerData);

    //test roi.
    int i = 0;
    ERRORTYPE ret;
    VENC_ROI_CFG_S stMppRoiBlockInfo;
    memset(&stMppRoiBlockInfo, 0, sizeof(VENC_ROI_CFG_S));
    for(i=0; i<pVi2Venc2MuxerData->mConfigPara.mRoiNum; i++)
    {
        stMppRoiBlockInfo.Index = i;
        stMppRoiBlockInfo.bEnable = TRUE;
        stMppRoiBlockInfo.bAbsQp = TRUE;
        stMppRoiBlockInfo.Qp = 20;
        stMppRoiBlockInfo.Rect.X = 128*i;
        stMppRoiBlockInfo.Rect.Y = 128*i;
        stMppRoiBlockInfo.Rect.Width = 128;
        stMppRoiBlockInfo.Rect.Height = 128;
        ret = AW_MPI_VENC_SetRoiCfg(pVi2Venc2MuxerData->mVeChn, &stMppRoiBlockInfo);
        if(ret != SUCCESS)
        {
            aloge("fatal error! set roi[%d] fail[0x%x]!", i, ret);
        }
        else
        {
            alogd("set roiIndex:%d, Qp:%d-%d, Rect[%d,%d,%dx%d]", i, stMppRoiBlockInfo.bAbsQp, stMppRoiBlockInfo.Qp, 
                stMppRoiBlockInfo.Rect.X, stMppRoiBlockInfo.Rect.Y, stMppRoiBlockInfo.Rect.Width, stMppRoiBlockInfo.Rect.Height);
        }
    }
    
    if(pVi2Venc2MuxerData->mConfigPara.mRoiNum>0 && pVi2Venc2MuxerData->mConfigPara.mbRoiBgFrameRate)
    {
        VENC_ROIBG_FRAME_RATE_S stRoiBgFrmRate;
        ret = AW_MPI_VENC_GetRoiBgFrameRate(pVi2Venc2MuxerData->mVeChn, &stRoiBgFrmRate);
        if(ret != SUCCESS)
        {
            aloge("fatal error! get roiBgFrameRate fail[0x%x]!", ret);
        }
        alogd("get roi bg frame rate:%d-%d", stRoiBgFrmRate.mSrcFrmRate, stRoiBgFrmRate.mDstFrmRate);
        stRoiBgFrmRate.mDstFrmRate = stRoiBgFrmRate.mSrcFrmRate/3;
        if(stRoiBgFrmRate.mDstFrmRate <= 0)
        {
            stRoiBgFrmRate.mDstFrmRate = 1;
        }
        ret = AW_MPI_VENC_SetRoiBgFrameRate(pVi2Venc2MuxerData->mVeChn, &stRoiBgFrmRate);
        if(ret != SUCCESS)
        {
            aloge("fatal error! set roiBgFrameRate fail[0x%x]!", ret);
        }
        alogd("set roi bg frame rate param:%d-%d", stRoiBgFrmRate.mSrcFrmRate, stRoiBgFrmRate.mDstFrmRate);
    }

    //test orl
    RGN_ATTR_S stRgnAttr;
    RGN_CHN_ATTR_S stRgnChnAttr;
    memset(&stRgnAttr, 0, sizeof(RGN_ATTR_S));
    memset(&stRgnChnAttr, 0, sizeof(RGN_CHN_ATTR_S));
    MPP_CHN_S viChn = {MOD_ID_VIU, pVi2Venc2MuxerData->mViDev, pVi2Venc2MuxerData->mViChn};
    for(i=0; i<pVi2Venc2MuxerData->mConfigPara.mOrlNum; i++)
    {
        stRgnAttr.enType = ORL_RGN;
        ret = AW_MPI_RGN_Create(i, &stRgnAttr);
        if(ret != SUCCESS)
        {
            aloge("fatal error! why create ORL region fail?[0x%x]", ret);
            break;
        }
        stRgnChnAttr.bShow = TRUE;
        stRgnChnAttr.enType = ORL_RGN;
        stRgnChnAttr.unChnAttr.stOrlChn.enAreaType = AREA_RECT;
        stRgnChnAttr.unChnAttr.stOrlChn.stRect.X = i*120;
        stRgnChnAttr.unChnAttr.stOrlChn.stRect.Y = i*60;
        stRgnChnAttr.unChnAttr.stOrlChn.stRect.Width = 100;
        stRgnChnAttr.unChnAttr.stOrlChn.stRect.Height = 50;
        stRgnChnAttr.unChnAttr.stOrlChn.mColor = 0xFF0000 >> ((i % 3)*8);
        stRgnChnAttr.unChnAttr.stOrlChn.mThick = 6;
        stRgnChnAttr.unChnAttr.stOrlChn.mLayer = i;
        ret = AW_MPI_RGN_AttachToChn(i, &viChn, &stRgnChnAttr);
        if(ret != SUCCESS)
        {
            aloge("fatal error! why attach to vi channel[%d,%d] fail?", pVi2Venc2MuxerData->mViDev, pVi2Venc2MuxerData->mViChn);
        }
        
    }

    sleep(10);
    for(i=0; i<pVi2Venc2MuxerData->mConfigPara.mOrlNum; i++)
    {
        ret = AW_MPI_RGN_Destroy(i);
        if(ret != SUCCESS)
        {
            aloge("fatal error! why destory region:%d fail?", i);
        }
    }
    
    int b3DNR;
    VENC_COLOR2GREY_S bColor2Grey;

    AW_MPI_VENC_Get3DNR(pVi2Venc2MuxerData->mVeChn, &b3DNR);
    AW_MPI_VENC_GetColor2Grey(pVi2Venc2MuxerData->mVeChn, &bColor2Grey);

    if(!b3DNR)
    {
        if(pVi2Venc2MuxerData->mConfigPara.m3DNR)
        {
            alogd("now,let us open 3dnr ");
            AW_MPI_VENC_Set3DNR(pVi2Venc2MuxerData->mVeChn, pVi2Venc2MuxerData->mConfigPara.m3DNR);
        }
    }
    if(!bColor2Grey.bColor2Grey)
    {
        if(pVi2Venc2MuxerData->mConfigPara.mColor2Grey)
        {
            alogd("now, let us open color2grey!");
            bColor2Grey.bColor2Grey = pVi2Venc2MuxerData->mConfigPara.mColor2Grey;
            AW_MPI_VENC_SetColor2Grey(pVi2Venc2MuxerData->mVeChn, &bColor2Grey);
        }
    }

    sleep(10);

    AW_MPI_VENC_Set3DNR(pVi2Venc2MuxerData->mVeChn, 0);

    bColor2Grey.bColor2Grey = FALSE;
    AW_MPI_VENC_SetColor2Grey(pVi2Venc2MuxerData->mVeChn, &bColor2Grey);


    if (pVi2Venc2MuxerData->mConfigPara.mTestDuration > 0)
    {
        cdx_sem_down_timedwait(&pVi2Venc2MuxerData->mSemExit, pVi2Venc2MuxerData->mConfigPara.mTestDuration*1000);
    }
    else
    {
        cdx_sem_down(&pVi2Venc2MuxerData->mSemExit);
    }

    stop(pVi2Venc2MuxerData);
    result = 0;

    alogd("start to free res");
err_out_2:
    pthread_mutex_lock(&pVi2Venc2MuxerData->mMuxChnListLock);
    if (!list_empty(&pVi2Venc2MuxerData->mMuxChnList))
    {
        alogd("chn list not empty");
        list_for_each_entry_safe(pEntry, pTmp, &pVi2Venc2MuxerData->mMuxChnList, mList)
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
    pthread_mutex_unlock(&pVi2Venc2MuxerData->mMuxChnListLock);
err_out_1:
    AW_MPI_SYS_Exit();

    pthread_mutex_destroy(&pVi2Venc2MuxerData->mMuxChnListLock);
err_out_0:
    cdx_sem_deinit(&pVi2Venc2MuxerData->mSemExit);
    free(pVi2Venc2MuxerData);
    pVi2Venc2MuxerData = NULL;
	if (result == 0) {
		printf("sample_virvi2venc2muxer exit!\n");
	}
    log_quit();
    return result;
}
