/*
 * =====================================================================================
 *
 *      Copyright (c) 2008-2018 Allwinner Technology Co. Ltd.
 *      All rights reserved.
 *
 *       Filename:  omx_venc.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2018/08
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  SWC-PPD
 *      Company: Allwinner Technology Co. Ltd.
 *
 *       History :
 *        Author : AL3
 *        Date   : 2013/05/05
 *    Comment : complete the design code
 *
 * =====================================================================================
 */
#ifndef __OMX_VENC_H__
#define __OMX_VENC_H__

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include "CdcUtil.h"
#include "OMX_Core.h"
#include "AWOMX_VideoIndexExtension.h"
#include "aw_omx_component.h"
#include "vencoder.h"
#include "omx_tsem.h"
#include "OMX_IndexExt.h"
#include "OMX_VideoExt.h"
#include "CdcMessageQueue.h"

typedef enum VIDENC_CUSTOM_INDEX
{
    VideoEncodeCustomParamStoreMetaDataInBuffers = OMX_IndexVendorStartUnused,
    VideoEncodeCustomParamPrependSPSPPSToIDRFrames,
    VideoEncodeCustomParamEnableAndroidNativeBuffers,
    VideoEncodeCustomParamextendedVideo,
    VideoEncodeCustomParamextendedVideoSuperframe,
    VideoEncodeCustomParamextendedVideoSVCSkip,
    VideoEncodeCustomParamextendedVideoVBR,
    VideoEncodeCustomParamStoreANWBufferInMetadata,
    VideoEncodeCustomParamextendedVideoPSkip,
    VideoEncodeCustomParamextendedAVCNaluFormat,
} VIDENC_CUSTOM_INDEX;

//////////////////////////////////////////////////////////////////////////////
//                       Module specific globals
//////////////////////////////////////////////////////////////////////////////
#define OMX_SPEC_VERSION  0x00000101

/*
 *  D E C L A R A T I O N S
 */
#define OMX_NOPORT                      0xFFFFFFFE
#define NUM_IN_BUFFERS                  2            // Input Buffers
#define NUM_MAX_IN_BUFFERS              16            // Input Buffers
#define NUM_OUT_BUFFERS                 4           // Output Buffers
#define OMX_TIMEOUT                     10          // Timeout value in milisecond
// Count of Maximum number of times the component can time out
#define OMX_MAX_TIMEOUTS                160
#define OMX_VIDEO_ENC_INPUT_BUFFER_SIZE (128*1024)
#define DEFAULT_OMX_COLOR_FORMAT        OMX_COLOR_FormatYVU420Planar

typedef int (*OmxVencCallback)(void* pUserData, int eMessageId, void* param);

/*
*     D E F I N I T I O N S
*/

typedef struct _BufferList BufferList;

/*
* The main structure for buffer management.
*
*   pBufHdr     - An array of pointers to buffer headers.
*                 The size of the array is set dynamically using the nBufferCountActual value
*                   send by the client.
*   nListEnd    - Marker to the boundary of the array. This points to the last index of the
*                   pBufHdr array.
*   nSizeOfList - Count of valid data in the list.
*   nAllocSize  - Size of the allocated list. This is equal to (nListEnd + 1) in most of
*                   the times. When the list is freed this is decremented and at that
*                   time the value is not equal to (nListEnd + 1). This is because
*                   the list is not freed from the end and hence we cannot decrement
*                   nListEnd each time we free an element in the list. When nAllocSize is zero,
*                   the list is completely freed and the other paramaters of the list are
*                   initialized.
*                 If the client crashes before freeing up the buffers, this parameter is
*                   checked (for nonzero value) to see if there are still elements on the list.
*                   If yes, then the remaining elements are freed.
*    nWritePos  - The position where the next buffer would be written. The value is incremented
*                   after the write. It is wrapped around when it is greater than nListEnd.
*    nReadPos   - The position from where the next buffer would be read. The value is incremented
*                   after the read. It is wrapped around when it is greater than nListEnd.
*    eDir       - Type of BufferList.
*                            OMX_DirInput  =  Input  Buffer List
*                           OMX_DirOutput  =  Output Buffer List
*/
struct _BufferList
{
    OMX_BUFFERHEADERTYPE**   pBufHdrList;
    OMX_U32                  nSizeOfList;
    OMX_S32                  nWritePos;
    OMX_S32                  nReadPos;

    OMX_BUFFERHEADERTYPE*    pBufArr;
    OMX_S32                  nAllocBySelfFlags;
    OMX_S32                  nBufArrSize;
    OMX_U32                  nAllocSize;
    OMX_DIRTYPE              eDir;
};

typedef struct OMXInputBufferInfoS
{
    aw_ion_user_handle_t  handle_ion;
    unsigned long         nBufPhyAddr;
    char*                 pBufVirAddr;
    int                   nBufFd;
    int                   nShareFd;
    long long             nAwBufId;
}OMXInputBufferInfoT;

typedef struct OMXDebugInfoS
{
    OMX_U32 nOkRetCount;
    OMX_U32 nErrCount;
    OMX_U32 nNoFrameRetCount;
    OMX_U32 nNoBitstreamCount;
    OMX_U32 nEmptyBitCount;
    OMX_U32 nOtherRetCount;
    OMX_U32 nEmptyBufferCount;
}OMXDebugInfoT;

 // OMX video decoder class
typedef struct _AwOmxVenc
{
    AwOmxComponentType  base;
    OMX_STATETYPE                   m_state;
    OMX_U8                          m_cRole[OMX_MAX_STRINGNAME_SIZE];
    OMX_U8                          m_cName[OMX_MAX_STRINGNAME_SIZE];
    OMX_VIDEO_CODINGTYPE            m_eCompressionFormat;
    OMX_CALLBACKTYPE                m_Callbacks;
    OMX_PTR                         m_pAppData;
    OMX_PORT_PARAM_TYPE             m_sPortParam;
    OMX_PARAM_PORTDEFINITIONTYPE    m_sInPortDefType;
    OMX_PARAM_PORTDEFINITIONTYPE    m_sOutPortDefType;
    OMX_VIDEO_PARAM_PORTFORMATTYPE  m_sInPortFormatType;
    OMX_VIDEO_PARAM_PORTFORMATTYPE  m_sOutPortFormatType;
    OMX_PRIORITYMGMTTYPE            m_sPriorityMgmtType;
    OMX_PARAM_BUFFERSUPPLIERTYPE    m_sInBufSupplierType;
    OMX_PARAM_BUFFERSUPPLIERTYPE    m_sOutBufSupplierType;
    OMX_VIDEO_PARAM_BITRATETYPE     m_sOutPutBitRateType;
    OMX_VIDEO_PARAM_AVCTYPE         m_sH264Type;
    OMX_VIDEO_PARAM_HEVCTYPE        m_sH265Type;
    pthread_t                       m_thread_id;
    pthread_t                       m_venc_thread_id;
    BufferList                      m_sInBufList;
    BufferList                      m_sOutBufList;
    pthread_mutex_t                 m_inBufMutex;
    pthread_mutex_t                 m_outBufMutex;
    VideoEncoder *                  m_encoder;
    omx_sem_t                       m_msg_sem;
    omx_sem_t                       m_input_sem;
    OMX_BOOL                        m_useAndroidBuffer;
    OMX_U32                         m_frameCountForCSD;
    OMX_BOOL                        m_useAllocInputBuffer;
    OMX_U32                         m_framerate;
    OMX_BOOL                        m_useMetaDataInBuffers;
    OMX_BOOL                        m_prependSPSPPSToIDRFrames;
    OMX_COLOR_FORMATTYPE            m_inputcolorFormats[5];
    VencH264Param                   m_vencH264Param;
    VencH264FixQP                   m_fixQp;
    VencH265Param                   m_vencH265Param;
    VencAllocateBufferParam         m_vencAllocBufferParam;
    VencCyclicIntraRefresh          m_vencCyclicIntraRefresh;
    VencHeaderData                  m_headdata;
    VENC_PIXEL_FMT                  m_vencColorFormat;
    VENC_CODEC_TYPE                 m_vencCodecType;
    OMX_BOOL                        mIsFromCts;
    OMX_BOOL                        mIsFromVideoeditor;
    OMX_BOOL                        mFirstInputFrame;
    OMX_VIDEO_PARAMS_EXTENDED       mVideoExtParams;
    OMX_VIDEO_PARAMS_SUPER_FRAME    mVideoSuperFrame;
    OMX_VIDEO_PARAMS_SVC            mVideoSVCSkip;
    OMX_VIDEO_PARAMS_VBR            mVideoVBR;
    struct ScMemOpsS *memops;
    OMX_BOOL                        m_usePSkip;
    OMXInputBufferInfoT             mInputBufInfo[NUM_MAX_IN_BUFFERS];
    OMX_S32                         mIonFd;
    int                             mFrameCnt;
    int                             mAllFrameSize;
    uint64_t                        mTimeStart;
    uint64_t                        mTimeOut;
    int                             mEmptyBufCnt;
    int                             mFillBufCnt;
    OMX_VIDEO_CONFIG_AVCINTRAPERIOD m_avcIntraPeriod;
    OMX_NALSTREAMFORMATTYPE         m_avcNaluFormat;
    OMX_U32                         m_enableAvcIntraPeriod;

    CdcMessageQueue*                mqMainThread;
    CdcMessageQueue*                mqVencThread;
    OMX_U32                         mSizeY;
    OMX_U32                         mSizeC;
    OMX_U32                         mInputBufferStep;
    OMX_BUFFERHEADERTYPE*           mInBufHdr;
    OMX_BUFFERHEADERTYPE*           mOutBufHdr;
    OMX_BOOL                        m_port_setting_match;
    OMX_BOOL                        m_inBufEos;
    OMX_PTR                         m_pMarkData;
    OMX_HANDLETYPE                  m_hMarkTargetComponent;
    VencInputBuffer                 m_sInputBuffer;
    VencInputBuffer                 m_sInputBuffer_return;
    VencOutputBuffer                m_sOutputBuffer;
    omx_sem_t                       m_need_sleep_sem;

    OMX_U32                         m_outBufferCountOwnedByEncoder;
    OMX_U32                         m_outBufferCountOwnedByRender;
    FILE*                           m_outFile;
    OMX_BOOL                        bSaveStreamFlag;
    OMX_BOOL                        bOpenStatisticFlag;
    OMX_BOOL                        bShowFrameSizeFlag;
    OMX_BOOL                        bSaveInputDataFlag;
    FILE*                           mInFile;
    int64_t                         mTimeUs1;
    int64_t                         mTimeUs2;
    OMXDebugInfoT                   mDebugInfo;
    OMX_BOOL                        mInportEnable;
    OMX_BOOL                        mOutportEnable;
}AwOmxVenc;

void callbackEmptyBufferDone(AwOmxVenc* impl,
                                        OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);//*kay: have not been tested!!!!
//int callbackFillBufferDone();
//int callbackEventHandle();
void post_message_to_venc_and_wait(AwOmxVenc *impl, OMX_S32 id);

#endif // __OMX_VENC_H__
