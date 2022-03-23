/******************************************************************************
  Copyright (C), 2001-2016, Allwinner Tech. Co., Ltd.
 ******************************************************************************
  File Name     : sample_demux.c
  Version       : Initial Draft
  Author        : Allwinner BU3-PD2 Team
  Created       : 2016/11/4
  Last Modified :
  Description   : sample_demux module
  Function List :
  History       :
******************************************************************************/

//#define LOG_NDEBUG 0
#define LOG_TAG "SampleDemux"

#include <unistd.h>
#include <fcntl.h>
#include "plat_log.h"
#include <time.h>
#include <mm_common.h>
#include <mpi_sys.h>
#include <mpi_demux.h>

#include "sample_demux.h"
#include "sample_demux_common.h"


#define DEMUX_DEFAULT_SRC_FILE   "/mnt/extsd/sample_demux/test.mp4"
#define DEMUX_DEFAULT_DST_VIDEO_FILE      "/mnt/extsd/sample_demux/video.bin"
#define DEMUX_DEFAULT_DST_AUDIO_FILE      "/mnt/extsd/sample_demux/audio.bin"
#define DEMUX_DEFAULT_DST_SUBTITLE_FILE   "/mnt/extsd/sample_demux/subtitle.bin"


static SAMPLE_DEMUX_S *pDemuxData;


static ERRORTYPE InitDemuxData(void)
{
    pDemuxData = (SAMPLE_DEMUX_S* )malloc(sizeof(SAMPLE_DEMUX_S));
    if (pDemuxData == NULL)
    {
        aloge("malloc struct fail");
        return FAILURE;
    }

    memset(pDemuxData, 0, sizeof(SAMPLE_DEMUX_S));

    strcpy(pDemuxData->mConfigPara.srcFile, DEMUX_DEFAULT_SRC_FILE);
    strcpy(pDemuxData->mConfigPara.dstVideoFile, DEMUX_DEFAULT_DST_VIDEO_FILE);
    strcpy(pDemuxData->mConfigPara.dstAudioFile, DEMUX_DEFAULT_DST_AUDIO_FILE);
    strcpy(pDemuxData->mConfigPara.dstSubtileFile, DEMUX_DEFAULT_DST_SUBTITLE_FILE);

    pDemuxData->mDmxChn = MM_INVALID_CHN;

    return SUCCESS;
}

static int parseCmdLine(SAMPLE_DEMUX_S *pDemuxData, int argc, char** argv)
{
    int ret = -1;

    while (*argv)
    {
       if (!strcmp(*argv, "-path"))
       {
          argv++;
          if (*argv)
          {
              ret = 0;
              if (strlen(*argv) >= MAX_FILE_PATH_LEN)
              {
                 aloge("fatal error! file path[%s] too long:!", *argv);
              }

              strncpy(pDemuxData->mCmdLinePara.mConfigFilePath, *argv, MAX_FILE_PATH_LEN-1);
              pDemuxData->mCmdLinePara.mConfigFilePath[MAX_FILE_PATH_LEN-1] = '\0';
          }
       }
       else if(!strcmp(*argv, "-h"))
       {
            printf("CmdLine param:\n"
                "\t-path /mnt/extsd/sample_demux.conf\n");
            break;
       }
       else if (*argv)
       {
          argv++;
       }
    }

    return ret;
}

static ERRORTYPE loadConfigPara(SAMPLE_DEMUX_S *pDemuxData, const char *conf_path)
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

    pDemuxData->mConfigPara.seekTime = GetConfParaInt(&mConf, DEMUX_CFG_SEEK_POSITION, 0);

    ptr = (char *)GetConfParaString(&mConf, DEMUX_CFG_SRC_FILE_STR, NULL);
    strcpy(pDemuxData->mConfigPara.srcFile, ptr);

    ptr = (char *)GetConfParaString(&mConf, DEMUX_CFG_DST_VIDEO_FILE_STR, NULL);
    strcpy(pDemuxData->mConfigPara.dstVideoFile, ptr);

    ptr = (char *)GetConfParaString(&mConf, DEMUX_CFG_DST_AUDIO_FILE_STR, NULL);
    strcpy(pDemuxData->mConfigPara.dstAudioFile, ptr);

    ptr = (char *)GetConfParaString(&mConf, DEMUX_CFG_DST_SUBTITLE_FILE_STR, NULL);
    strcpy(pDemuxData->mConfigPara.dstSubtileFile, ptr);

    pDemuxData->mConfigPara.mTestDuration = GetConfParaInt(&mConf, DEMUX_CFG_TEST_DURATION, 0);

    destroyConfParser(&mConf);
    return SUCCESS;
}

static ERRORTYPE MPPCallbackWrapper(void *cookie, MPP_CHN_S *pChn, MPP_EVENT_TYPE event, void *pEventData)
{
    SAMPLE_DEMUX_S *pDemuxData = (SAMPLE_DEMUX_S *)cookie;

    switch(event)
    {
    case MPP_EVENT_NOTIFY_EOF:
        alogd("get the endof file");
        cdx_sem_up(&pDemuxData->mSemExit);
        break;

    default:
        break;
    }

    return SUCCESS;
}

static ERRORTYPE configDmxChnAttr(SAMPLE_DEMUX_S *pDemuxData)
{
    pDemuxData->mDmxChnAttr.mStreamType = STREAMTYPE_LOCALFILE;
    pDemuxData->mDmxChnAttr.mSourceType = SOURCETYPE_FD;
    pDemuxData->mDmxChnAttr.mSourceUrl = NULL;
    pDemuxData->mDmxChnAttr.mFd = pDemuxData->mConfigPara.srcFd;
//we not use subtitle in middleware
    pDemuxData->mDmxChnAttr.mDemuxDisableTrack = pDemuxData->mTrackDisFlag = DEMUX_DISABLE_SUBTITLE_TRACK;

    return SUCCESS;
}

static ERRORTYPE createDemuxChn(SAMPLE_DEMUX_S *pDemuxData)
{
    int ret;
    BOOL nSuccessFlag = FALSE;

    configDmxChnAttr(pDemuxData);

    pDemuxData->mDmxChn = 0;
    while (pDemuxData->mDmxChn < DEMUX_MAX_CHN_NUM)
    {
        ret = AW_MPI_DEMUX_CreateChn(pDemuxData->mDmxChn, &pDemuxData->mDmxChnAttr);
        if (SUCCESS == ret)
        {
            nSuccessFlag = TRUE;
            alogd("create demux channel[%d] success!", pDemuxData->mDmxChn);
            break;
        }
        else if (ERR_DEMUX_EXIST == ret)
        {
            alogd("demux channel[%d] is exist, find next!", pDemuxData->mDmxChn);
            pDemuxData->mDmxChn++;
        }
        else
        {
            alogd("create demux channel[%d] ret[0x%x]!", pDemuxData->mDmxChn, ret);
            break;
        }
    }

    if (FALSE == nSuccessFlag)
    {
        pDemuxData->mDmxChn = MM_INVALID_CHN;
        aloge("fatal error! create demux channel fail!");
        return FAILURE;
    }
    else
    {
        MPPCallbackInfo cbInfo;
        cbInfo.cookie = (void*)pDemuxData;
        cbInfo.callback = (MPPCallbackFuncType)&MPPCallbackWrapper;
        AW_MPI_DEMUX_RegisterCallback(pDemuxData->mDmxChn, &cbInfo);
        return SUCCESS;
    }
}

static ERRORTYPE prepare(SAMPLE_DEMUX_S *pDemuxData)
{
    DEMUX_MEDIA_INFO_S DemuxMediaInfo;

    if (AW_MPI_DEMUX_GetMediaInfo(pDemuxData->mDmxChn, &DemuxMediaInfo) != SUCCESS)
    {
        aloge("fatal error! get media info fail!");
        return FAILURE;
    }

    memcpy(&pDemuxData->mMediaInfo, &DemuxMediaInfo, sizeof(DEMUX_MEDIA_INFO_S));

    if ((DemuxMediaInfo.mVideoNum >0 && DemuxMediaInfo.mVideoIndex>=DemuxMediaInfo.mVideoNum)
       || (DemuxMediaInfo.mAudioNum >0 && DemuxMediaInfo.mAudioIndex>=DemuxMediaInfo.mAudioNum)
       || (DemuxMediaInfo.mSubtitleNum >0 && DemuxMediaInfo.mSubtitleIndex>=DemuxMediaInfo.mSubtitleNum))
    {
        alogd("fatal error, trackIndex wrong! [%d][%d],[%d][%d],[%d][%d]",
           DemuxMediaInfo.mVideoNum, DemuxMediaInfo.mVideoIndex, DemuxMediaInfo.mAudioNum, DemuxMediaInfo.mAudioIndex, DemuxMediaInfo.mSubtitleNum, DemuxMediaInfo.mSubtitleIndex);
        return FAILURE;
    }

    if (DemuxMediaInfo.mSubtitleNum > 0)
    {
        AW_MPI_DEMUX_GetChnAttr(pDemuxData->mDmxChn, &pDemuxData->mDmxChnAttr);
        pDemuxData->mDmxChnAttr.mDemuxDisableTrack |= DEMUX_DISABLE_SUBTITLE_TRACK;
        AW_MPI_DEMUX_SetChnAttr(pDemuxData->mDmxChn, &pDemuxData->mDmxChnAttr);
    }

    return SUCCESS;
}

static void *getDmxBufThread(void *pThreadData)
{
    int ret;
    int cmd;
    unsigned int cmddata;
    EncodedStream demuxOutBuf;
    SAMPLE_DEMUX_S *pDemuxData = (SAMPLE_DEMUX_S *)pThreadData;

    lseek(pDemuxData->mConfigPara.dstVideoFd, 0, SEEK_SET);
    lseek(pDemuxData->mConfigPara.dstAudioFd, 0, SEEK_SET);
    lseek(pDemuxData->mConfigPara.dstSubFd, 0, SEEK_SET);

    while (!pDemuxData->mOverFlag)
    {
        if (SUCCESS == AW_MPI_DEMUX_getDmxOutPutBuf(pDemuxData->mDmxChn, &demuxOutBuf, 100))
        {
            if (demuxOutBuf.media_type == CDX_PacketVideo)
            {
               //alogd("write video");
               write(pDemuxData->mConfigPara.dstVideoFd, demuxOutBuf.pBuffer, demuxOutBuf.nFilledLen);
            }
            else if (demuxOutBuf.media_type == CDX_PacketAudio)
            {
               //alogd("write audio");
               write(pDemuxData->mConfigPara.dstAudioFd, demuxOutBuf.pBuffer, demuxOutBuf.nFilledLen);
            }
            else if (demuxOutBuf.media_type == CDX_PacketSubtitle)
            {
               //alogd("write title");
               write(pDemuxData->mConfigPara.dstSubFd, demuxOutBuf.pBuffer, demuxOutBuf.nFilledLen);
            }

            ret = AW_MPI_DEMUX_releaseDmxBuf(pDemuxData->mDmxChn, &demuxOutBuf);
            if (ret != SUCCESS)
            {
                aloge("dmxoutbuf get, but can not release, maybe something wrong in middleware");
            }
        }
        else
        {
            alogd("-------no buf!-------");
        }
    }

    alogd("thread exit!");
    return NULL;
}

static ERRORTYPE startChn(SAMPLE_DEMUX_S *pDemuxData)
{
    return AW_MPI_DEMUX_Start(pDemuxData->mDmxChn);
}

static ERRORTYPE pauseChn(SAMPLE_DEMUX_S *pDemuxData)
{
  return AW_MPI_DEMUX_Pause(pDemuxData->mDmxChn);
}

static ERRORTYPE stopChn(SAMPLE_DEMUX_S *pDemuxData)
{
    return AW_MPI_DEMUX_Stop(pDemuxData->mDmxChn);
}

static ERRORTYPE seekto(SAMPLE_DEMUX_S *pDemuxData)
{
    return AW_MPI_DEMUX_Seek(pDemuxData->mDmxChn, pDemuxData->mConfigPara.seekTime);
}


int main(int argc, char** argv)
{
    int result = -1;
    int ret = 0;

    if (InitDemuxData() != SUCCESS)
    {
        return -1;
    }

    cdx_sem_init(&pDemuxData->mSemExit, 0);

    if (parseCmdLine(pDemuxData, argc, argv) != 0)
    {
        goto err_out_0;
    }

    if (loadConfigPara(pDemuxData, pDemuxData->mCmdLinePara.mConfigFilePath) != SUCCESS)
    {
        aloge("no config file or parse conf file fail");
        goto err_out_0;
    }

    pDemuxData->mConfigPara.srcFd = open(pDemuxData->mConfigPara.srcFile, O_RDONLY);
    if (pDemuxData->mConfigPara.srcFd < 0)
    {
        aloge("ERROR: cannot open video src file");
        goto err_out_0;
    }

    pDemuxData->mConfigPara.dstVideoFd = open(pDemuxData->mConfigPara.dstVideoFile, O_CREAT|O_RDWR);
    if (pDemuxData->mConfigPara.dstVideoFd < 0)
    {
        aloge("ERROR: cannot create video dst file");
        goto err_out_1;
    }

    pDemuxData->mConfigPara.dstAudioFd = open(pDemuxData->mConfigPara.dstAudioFile, O_CREAT|O_RDWR);
    if (pDemuxData->mConfigPara.dstAudioFd < 0)
    {
        aloge("ERROR: cannot create audio dst file");
        goto err_out_2;
    }

    pDemuxData->mConfigPara.dstSubFd = open(pDemuxData->mConfigPara.dstSubtileFile, O_CREAT|O_RDWR);
    if (pDemuxData->mConfigPara.dstSubFd < 0)
    {
        aloge("ERROR: cannot create subtitle dst file");
        goto err_out_3;
    }

    pDemuxData->mSysConf.nAlignWidth = 32;
    AW_MPI_SYS_SetConf(&pDemuxData->mSysConf);
    AW_MPI_SYS_Init();

    if (createDemuxChn(pDemuxData) != SUCCESS)
    {
        aloge("create demuxchn fail");
        goto err_out_4;
    }

    if (prepare(pDemuxData) !=  SUCCESS)
    {
        aloge("prepare failed");
        goto err_out_5;
    }

    if (startChn(pDemuxData) != SUCCESS)
    {
        aloge("start play fail");
        goto err_out_7;
    }

    //pauseChn(pDemuxData);

    //ret = seekto(pDemuxData);

    pDemuxData->mOverFlag = FALSE;
    ret = pthread_create(&pDemuxData->mThdId, NULL, getDmxBufThread, pDemuxData);
    if (ret || !pDemuxData->mThdId)
    {
        goto err_out_6;
    }

    if (pDemuxData->mConfigPara.mTestDuration > 0)
    {
        cdx_sem_down_timedwait(&pDemuxData->mSemExit, pDemuxData->mConfigPara.mTestDuration*1000);
    }
    else
    {
        cdx_sem_down(&pDemuxData->mSemExit);
    }

    pDemuxData->mOverFlag = TRUE;
    if (stopChn(pDemuxData) != SUCCESS)
    {
        alogw("stop fail");
    }

    result = 0;

err_out_7:
    pthread_join(pDemuxData->mThdId, NULL);
err_out_6:
err_out_5:
    AW_MPI_DEMUX_DestroyChn(pDemuxData->mDmxChn);
err_out_4:
    //exit mpp system
    AW_MPI_SYS_Exit();
    close(pDemuxData->mConfigPara.dstSubFd);
err_out_3:
    close(pDemuxData->mConfigPara.dstAudioFd);
err_out_2:
    close(pDemuxData->mConfigPara.dstVideoFd);
err_out_1:
    close(pDemuxData->mConfigPara.srcFd);
err_out_0:
    cdx_sem_deinit(&pDemuxData->mSemExit);
    free(pDemuxData);
    pDemuxData = NULL;

    if (result == 0) {
        printf("sample_demux exit!\n");
    }
    return result;
}
