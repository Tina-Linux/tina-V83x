#ifndef __OMX_VDEC_H__
#define __OMX_VDEC_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include "aw_omx_component.h"
#include "omx_vdec_port.h"
#include <pthread.h>
#include "vdecoder.h"
#include "CdcMessageQueue.h"
#include "omx_vdec_decoder.h"
#include "omx_vdec_config.h"
#include "omx_thread.h"
#include "omx_sem.h"


typedef struct _AwOmxVdec AwOmxVdec;

typedef int (*doStateTransition)(AwOmxVdec* impl);

struct _AwOmxVdec
{
    AwOmxComponentType              base;
    OMX_U8                          m_cName[OMX_MAX_STRINGNAME_SIZE];
    OMX_U8                          m_cRole[OMX_MAX_STRINGNAME_SIZE];
    OMX_VIDEO_CODINGTYPE            m_eCompressionFormat;

    OMX_CALLBACKTYPE                m_Callbacks;
    OMX_PTR                         m_pAppData;
    OMX_PORT_PARAM_TYPE             m_sPortParam;
    AwOmxVdecPort*                  m_InPort;
    AwOmxVdecPort*                  m_OutPort;
    OMX_PRIORITYMGMTTYPE            m_sPriorityMgmtType;
    OMX_STATETYPE                   m_state;
    doStateTransition               m_StateTransitTable[5][5]; /*[current state][target state]*/
    OMX_PTR                         pMarkData;
    OMX_HANDLETYPE                  hMarkTargetComponent;
    OMX_CONFIG_RECTTYPE             m_VideoRect;


    CdcMessageQueue*                mqMsgHandle;
    pthread_t                       m_msgId;
    OmxThreadHandle                 mSubmitThreadHdl;
    OmxThreadHandle                 mDecodeThreadHdl;
    OmxThreadHandle                 mDrainThreadHdl;
    OmxSemHandle                    m_semFlushAllFrame;
    OmxSemHandle                    m_semKeepSubmit;
    OmxSemHandle                    m_semKeepDecode;
    OmxSemHandle                    m_semKeepDrain;
    OmxSemHandle                    m_semSubmitData;

    OmxDecoder*                     pDec;

    /*m_OutputNum and m_InputNum will be reseted when seek*/
    OMX_S32                         m_OutputNum;
    OMX_S32                         m_InputNum;

    OMX_BOOL                        bHadInitDecoderFlag;
    OMX_BOOL                        bVdrvNotifyEosFlag;
    OMX_BOOL                        bUseAndroidBuffer;
    OMX_BOOL                        bFlushingPicFlag;
    OMX_BOOL                        bResolutionChangeFlag;
    OMX_BOOL                        bIsSecureVideoFlag;
    OMX_BOOL                        bContinueToSubmitFlag;
    OMX_U32                         mWaitCount; //for sts test
    OMX_BOOL                        bDecodeForceStopFlag;
    OMX_S32                         nNeedToFlushPicNum;
};
#ifdef __cplusplus
}
#endif

#endif
