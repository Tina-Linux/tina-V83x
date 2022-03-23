/*
 * =====================================================================================
 *
 *       Filename:  omx_vdec_port.h
 *
 *    Description: the function of ports(in port and out port), called by omx_vdec and omx_vdec_decoder
 *
 *        Version:  1.0
 *        Created:  08/02/2018 04:18:11 PM
 *       Revision:  none
 *
 *         Author:  Gan Qiuye
 *        Company:
 *
 * =====================================================================================
 */

#ifndef __OMX_VDEC_PORT_H__
#define __OMX_VDEC_PORT_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "OMX_Types.h"
#include "OMX_Core.h"
#include "OMX_Component.h"
#include <pthread.h>
#include "omx_mutex.h"
typedef struct _BufferList BufferList;

/*
 * The main structure for buffer management.
 *
 *   pBufHdrList     - An array of pointers to buffer headers.
 *                            The size of the array is set dynamically using the nBufferCountActual value
 *                              send by the client. The order will change.
 *   nListEnd         - Marker to the boundary of the array. This points to the last index of the
 *                              pBufHdr array.
 *   nSizeOfList     - Count of valid data in the list.
 *   nAllocSize      - Size of the allocated list. This is equal to (nListEnd + 1) in most of
 *                               the times. When the list is freed this is decremented and at that
 *                               time the value is not equal to (nListEnd + 1). This is because
 *                               the list is not freed from the end and hence we cannot decrement
 *                               nListEnd each time we free an element in the list. When nAllocSize is zero,
 *                               the list is completely freed and the other paramaters of the list are initialized.
 *                           If the client crashes before freeing up the buffers, this parameter is
 *                               checked (for nonzero value) to see if there are still elements on the list.
 *                           If yes, then the remaining elements are freed.
 *    nWritePos     - The position where the next buffer would be written. The value is incremented
 *                              after the write. It is wrapped around when it is greater than nListEnd.
 *    nReadPos      - The position from where the next buffer would be read. The value is incremented
 *                              after the read. It is wrapped around when it is greater than nListEnd.
 *    eDir               - Type of BufferList.
 *                                  OMX_DirInput  =  Input  Buffer List
 *                                  OMX_DirOutput  =  Output Buffer List
 *
 *   pBufArr          -is not the pBufHdrList, the order of this array is  fixed when allocting, not change
 *
 *   nAllocBySelfFlags -The buffer is allocated by self(api:allocatebuffer)
 *
 *   nBufNeedSize  - The list should allocacte nBufNeedSize buffers
 */

struct _BufferList
{
   OMX_BUFFERHEADERTYPE**   pBufHdrList;
   OMX_U32                  nBufActual;
   OMX_U32                  nSizeOfList;
   OMX_U32                  nAllocSize;
   OMX_U32                  nWritePos;
   OMX_U32                  nReadPos;
   OMX_DIRTYPE              eDir;

   OMX_BUFFERHEADERTYPE*    pBufArr;
   OMX_S32                  nAllocBySelfFlags;
   OMX_U32                  nBufNeedSize;
};

typedef struct _AwOmxVdecPort
{
    OMX_COMPONENTTYPE               mOmxCmp;
    OMX_PTR                         m_pAppData;
    OMX_PARAM_PORTDEFINITIONTYPE    m_sPortDefType;
    OMX_VIDEO_PARAM_PORTFORMATTYPE  m_sPortFormatType;
    OMX_PARAM_BUFFERSUPPLIERTYPE    m_sBufSupplierType;
    BufferList                      m_sBufList;
    OmxMutexHandle                  m_bufMutex;
    OMX_CALLBACKTYPE                m_Callbacks;
    OMX_MARKTYPE*                   pMarkBuf;
    OMX_S32                         mExtraBufferNum;
    OMX_BOOL                        bAllocBySelfFlags;
    OMX_U32                         m_BufferCntActual;
    OMX_BOOL                        bEnabled;
}AwOmxVdecPort;


static inline OMX_BOOL isInputPort(AwOmxVdecPort* mPort)
{
    if(mPort == NULL)
        return OMX_FALSE;
    else
        return (OMX_BOOL)(mPort->m_sPortDefType.eDir == OMX_DirInput);
}

static inline OMX_U32 getPortIndex(AwOmxVdecPort* mPort)
{
    return mPort->m_sPortDefType.nPortIndex;
}

static inline OMX_PARAM_PORTDEFINITIONTYPE* getPortDef(AwOmxVdecPort* mPort)
{
    return &mPort->m_sPortDefType;
}

static inline OMX_VIDEO_PARAM_PORTFORMATTYPE* getPortFormatType(AwOmxVdecPort* mPort)
{
    return &mPort->m_sPortFormatType;
}

static inline void setPortColorFormat(AwOmxVdecPort* mPort, OMX_U32 color)
{
    mPort->m_sPortDefType.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)color;
    mPort->m_sPortFormatType.eColorFormat = (OMX_COLOR_FORMATTYPE)color;
}

static inline OMX_U32 getPortAllocSize(AwOmxVdecPort* mPort)
{
    return mPort->m_sBufList.nAllocSize;
}

static inline OMX_U32 getPortValidSize(AwOmxVdecPort* mPort)
{
    return mPort->m_sBufList.nSizeOfList;
}

void doEmptyThisBuffer(AwOmxVdecPort* mPort, OMX_BUFFERHEADERTYPE* pBufferHeader);
void doFillThisBuffer(AwOmxVdecPort* mPort, OMX_BUFFERHEADERTYPE* pBufferHeader);
void doFlushPortBuffer(AwOmxVdecPort* mPort);
void doSetPortMarkBuffer(AwOmxVdecPort* mPort, OMX_MARKTYPE* pMarkBuf);
void returnPortBuffer(AwOmxVdecPort* mPort);
OMX_BUFFERHEADERTYPE* doRequestPortBuffer(AwOmxVdecPort* mPort);

OMX_ERRORTYPE AwOmxVdecInPortInit(AwOmxVdecPort* m_InPort, OMX_VIDEO_CODINGTYPE type);
OMX_ERRORTYPE AwOmxVdecOutPortInit(AwOmxVdecPort* mPort);
OMX_ERRORTYPE AwOmxVdecPortDeinit(AwOmxVdecPort* m_InPort);
OMX_ERRORTYPE AwOmxVdecPortGetDefinitioin(AwOmxVdecPort* mPort,
                                                   OMX_PARAM_PORTDEFINITIONTYPE *value);
OMX_ERRORTYPE AwOmxVdecPortSetDefinitioin(AwOmxVdecPort* mPort,
                                                   OMX_PARAM_PORTDEFINITIONTYPE *value);

OMX_ERRORTYPE AwOmxVdecPortSetFormat(AwOmxVdecPort* mPort,
                                               OMX_VIDEO_PARAM_PORTFORMATTYPE *value);

OMX_ERRORTYPE AwOmxVdecPortGetFormat(AwOmxVdecPort* mPort,
                                               OMX_VIDEO_PARAM_PORTFORMATTYPE *value);

OMX_ERRORTYPE AwOmxVdecPortGetBufferSupplier(AwOmxVdecPort* mPort,
                                                        OMX_PARAM_BUFFERSUPPLIERTYPE *value);

OMX_ERRORTYPE AwOmxVdecPortSetBufferSupplier(AwOmxVdecPort* mPort,
                                                        OMX_PARAM_BUFFERSUPPLIERTYPE *value);

OMX_ERRORTYPE AwOmxVdecPortGetProfileLevel(AwOmxVdecPort* mPort,
                                                     OMX_VIDEO_PARAM_PROFILELEVELTYPE *value);

OMX_ERRORTYPE AwOmxVdecPortPopBuffer(AwOmxVdecPort* mPort,
                                        OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                        OMX_IN    OMX_PTR                pAppPrivate,
                                        OMX_IN    OMX_U32                nSizeBytes,
                                        OMX_IN    OMX_U8*                pBuffer,
                                        OMX_IN    OMX_BOOL               bAllocBySelfFlags);


OMX_ERRORTYPE AwOmxVdecPortFreeBuffer(AwOmxVdecPort* mPort,
                                    OMX_IN  OMX_BUFFERHEADERTYPE* pBufferHdr);

OMX_ERRORTYPE AwOmxVdecPortSetCallbacks(AwOmxVdecPort* mPort,
                                                       OMX_IN  OMX_CALLBACKTYPE* pCallbacks,
                                                       OMX_IN  OMX_PTR pAppData);
#ifdef __cplusplus
}
#endif


#endif
