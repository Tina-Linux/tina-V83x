/*
 * =====================================================================================
 *
 *       Filename:  omx_vdec_decoder.h
 *
 *    Description: the interface of decoder, now include aw_decoder and ffmepg decoder
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


#ifndef __OMX_VDEC_DECODER_H__
#define __OMX_VDEC_DECODER_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "omx_vdec_port.h"
#include "OMX_Core.h"
#include "OMX_IVCommon.h"
#include "omx_pub_def.h"
#include <semaphore.h>

typedef int (*DecoderCallback)(void* pUserData, int eMessageId, void* param);

typedef struct OmxDecoder OmxDecoder;
typedef struct OmxDecoderOpsS
{
    OMX_ERRORTYPE         (*getExtPara)(OmxDecoder* , AW_VIDEO_EXTENSIONS_INDEXTYPE, OMX_PTR );
    OMX_ERRORTYPE         (*setExtPara)(OmxDecoder* , AW_VIDEO_EXTENSIONS_INDEXTYPE, OMX_PTR );
    OMX_ERRORTYPE         (*getExtConfig)(OmxDecoder* , AW_VIDEO_EXTENSIONS_INDEXTYPE, OMX_PTR );
    OMX_ERRORTYPE         (*setExtConfig)(OmxDecoder* , AW_VIDEO_EXTENSIONS_INDEXTYPE, OMX_PTR );
    int                   (*prepare)(OmxDecoder*);
    void                  (*close)(OmxDecoder*);
    void                  (*flush)(OmxDecoder*);
    void                  (*decode)(OmxDecoder*);
    int                   (*submit)(OmxDecoder*, OMX_BUFFERHEADERTYPE*);
    OMX_BUFFERHEADERTYPE* (*drain)(OmxDecoder*);
    int                   (*callback)(OmxDecoder*, DecoderCallback , void* );
    void                  (*setInputEos)(OmxDecoder* , OMX_BOOL );
    int                   (*setOutputEos)(OmxDecoder*);
    void                  (*standbyBuf)(OmxDecoder*);
    int                   (*returnBuf)(OmxDecoder*);
    void                  (*getFormat)(OmxDecoder*);
    void                  (*setExtBufNum)(OmxDecoder*, OMX_S32);
    OMX_U8*               (*allocPortBuf)(OmxDecoder*, AwOmxVdecPort*, OMX_U32);
    void                  (*freePortBuf)(OmxDecoder*, AwOmxVdecPort*, OMX_U8*);
}OmxDecoderOpsT;

struct OmxDecoder {
   struct OmxDecoderOpsS* ops;
};

static inline int OmxVdecoderSetCallback(OmxDecoder* d, DecoderCallback c, void* p)
{
    return d->ops->callback(d, c, p);
}

static inline OMX_ERRORTYPE OmxVdecoderGetExtPara(OmxDecoder* d, AW_VIDEO_EXTENSIONS_INDEXTYPE i, OMX_PTR p)
{
    return d->ops->getExtPara(d, i, p);
}

static inline OMX_ERRORTYPE OmxVdecoderSetExtPara(OmxDecoder* d, AW_VIDEO_EXTENSIONS_INDEXTYPE i, OMX_PTR p)
{
    return d->ops->setExtPara(d, i, p);
}

static inline OMX_ERRORTYPE OmxVdecoderGetExtConfig(OmxDecoder* d, AW_VIDEO_EXTENSIONS_INDEXTYPE i, OMX_PTR p)
{
    return d->ops->getExtConfig(d, i, p);
}

static inline OMX_ERRORTYPE OmxVdecoderSetExtConfig(OmxDecoder* d, AW_VIDEO_EXTENSIONS_INDEXTYPE i, OMX_PTR p)
{
    return d->ops->setExtConfig(d, i, p);
}

static inline int OmxVdecoderPrepare(OmxDecoder* d)
{
    return d->ops->prepare(d);
}

static inline void OmxVdecoderClose(OmxDecoder* d)
{
    return d->ops->close(d);
}

static inline void OmxVdecoderFlush(OmxDecoder* d)
{
    return d->ops->flush(d);
}

static inline void OmxVdecoderDecode(OmxDecoder* d)
{
    return d->ops->decode(d);
}

static inline int OmxVdecoderSubmit(OmxDecoder* d, OMX_BUFFERHEADERTYPE* p)
{
    return d->ops->submit(d, p);
}

static inline OMX_BUFFERHEADERTYPE* OmxVdecoderDrain(OmxDecoder* d)
{
    return d->ops->drain(d);
}

static inline void OmxVdecoderSetInputEos(OmxDecoder* d, OMX_BOOL b)
{
    return d->ops->setInputEos(d, b);
}

static inline int OmxVdecoderSetOutputEos(OmxDecoder* d)
{
    return d->ops->setOutputEos(d);
}

static inline void OmxVdecoderStandbyBuffer(OmxDecoder* d)
{
    return d->ops->standbyBuf(d);
}

static inline int OmxVdecoderReturnBuffer(OmxDecoder* d)
{
    return d->ops->returnBuf(d);
}

static inline void OmxVdecoderGetFormat(OmxDecoder* d)
{
    return d->ops->getFormat(d);
}

static inline void OmxVdecoderSetExtBufNum(OmxDecoder* d, OMX_S32 n)
{
    return d->ops->setExtBufNum(d, n);
}

static inline OMX_U8* OmxVdecoderAllocatePortBuffer(OmxDecoder* d, AwOmxVdecPort* m, OMX_U32 n)
{
    return d->ops->allocPortBuf(d, m, n);
}

static inline void OmxVdecoderFreePortBuffer(OmxDecoder*d, AwOmxVdecPort* m, OMX_U8* p)
{
    return d->ops->freePortBuf(d, m, p);
}

OmxDecoder* OmxDecoderCreate(AwOmxVdecPort* in, AwOmxVdecPort* out);
void OmxDestroyDecoder(OmxDecoder* pDec);
#ifdef __cplusplus
}
#endif

#endif
