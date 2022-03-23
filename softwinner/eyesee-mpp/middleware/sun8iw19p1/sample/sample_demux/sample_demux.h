#ifndef __SAMPLE_DEMUX_H__
#define __SAMPLE_DEMUX_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>

#include "mm_comm_sys.h"
#include "mpi_sys.h"

#include "DemuxCompStream.h"
#include "mm_comm_demux.h"

#include "tsemaphore.h"

#include <confparser.h>


#define MAX_FILE_PATH_LEN  (128)

typedef enum {
   MEDIA_VIDEO = 0,
   MEDIA_AUDIO = 1,
   MEDIA_SUBTITLE = 2,
}MEDIA_TYPE_E;


typedef enum
{
    STATE_PREPARED = 0,
    STATE_PAUSE,
    STATE_PLAY,
    STATE_STOP,
}STATE_E;

typedef struct DemuxerCmdLineParam
{
    char mConfigFilePath[MAX_FILE_PATH_LEN];
}DEMUXERCMDLINEPARAM_S;

typedef struct DemuxerConfig
{
    char srcFile[MAX_FILE_PATH_LEN];
    char dstVideoFile[MAX_FILE_PATH_LEN];
    char dstAudioFile[MAX_FILE_PATH_LEN];
    char dstSubtileFile[MAX_FILE_PATH_LEN];

    int srcFd;
    int dstVideoFd;
    int dstAudioFd;
    int dstSubFd;

    int seekTime;
    int mTestDuration;

}DEMUXERCONFIG_S;


typedef struct sample_demux_s
{
    DEMUXERCMDLINEPARAM_S mCmdLinePara;
    DEMUXERCONFIG_S mConfigPara;

    cdx_sem_t mSemExit;

    MPP_SYS_CONF_S mSysConf;

    DEMUX_CHN mDmxChn;
    DEMUX_CHN_ATTR_S mDmxChnAttr;
    DEMUX_MEDIA_INFO_S mMediaInfo;

    pthread_t mThdId;

    BOOL mOverFlag;

    BOOL mWaitBufFlag;
    STATE_E mState;
    int mTrackDisFlag;

}SAMPLE_DEMUX_S;






#endif //#define __SAMPLE_DEMUX_H__

