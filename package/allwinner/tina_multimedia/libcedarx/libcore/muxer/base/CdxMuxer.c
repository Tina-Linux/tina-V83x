/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxMuxer.c
 * Description : Allwinner Muxer Definition
 * History :
 *
 */

#include <cdx_log.h>
#include <CdxList.h>
#include "CdxMuxer.h"

struct CdxMuxerNodeS
{
    CdxListNodeT node;
    CdxMuxerCreatorT *creator;
    CdxMuxerTypeT type;
};

struct CdxMuxerListS
{
    CdxListT list;
    int size;
};

typedef struct CdxMuxerListS CdxMuxerListT;

CdxMuxerListT muxerList;

#ifdef AAC_MUXER_ENABLE
extern CdxMuxerCreatorT aacMuxerCtor;  // raw audio
#endif

#ifdef MP4_MUXER_ENABLE
extern CdxMuxerCreatorT mp4MuxerCtor;
#endif

#ifdef TS_MUXER_ENABLE
extern CdxMuxerCreatorT tsMuxerCtor;
#endif

int AwMuxerRegister(CdxMuxerCreatorT *creator, CdxMuxerTypeT type)
{
    struct CdxMuxerNodeS *parserNode;

    parserNode = malloc(sizeof(*parserNode));
    parserNode->creator = creator;
    parserNode->type = type;

    CdxListAddTail(&parserNode->node, &muxerList.list);
    muxerList.size++;
    return 0;
}

static void AwMuxerInit(void) __attribute__((constructor));
void AwMuxerInit()
{
    CDX_LOGD("aw muxer init ..");
    CdxListInit(&muxerList.list);
    muxerList.size = 0;
#ifdef AAC_MUXER_ENABLE
    AwMuxerRegister(&aacMuxerCtor, CDX_MUXER_AAC);
#endif

#ifdef MP4_MUXER_ENABLE
    AwMuxerRegister(&mp4MuxerCtor, CDX_MUXER_MOV);
#endif

#ifdef TS_MUXER_ENABLE
    AwMuxerRegister(&tsMuxerCtor, CDX_MUXER_TS);
#endif
    CDX_LOGD("aw muxer size:%d",muxerList.size);
    return;
}

CdxMuxerT *CdxMuxerCreate(CdxMuxerTypeT type, CdxWriterT *stream)
{
    struct CdxMuxerNodeS *muxNode;
    CdxListForEachEntry(muxNode, &muxerList.list, node)
    {
        CDX_CHECK(muxNode->creator);

        if(muxNode->type == type)
        {
            return muxNode->creator->create(stream);
        }
    }

    loge("cannot support this type(%d) muxer", type);
    return NULL;
}
