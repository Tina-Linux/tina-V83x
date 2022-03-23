/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxMuxer.c
 * Description : Allwinner Muxer Definition
 * History :
 *
 */

#include <CdxLog.h>
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

extern CdxMuxerCreatorT aacMuxerCtor;  // raw audio
extern CdxMuxerCreatorT mp4MuxerCtor;
extern CdxMuxerCreatorT tsMuxerCtor;

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

static cdx_void AwMuxerInit(cdx_void) __attribute__((constructor));
void AwMuxerInit()
{
    CDX_LOGD("aw muxer init ..");
    CdxListInit(&muxerList.list);
    muxerList.size = 0;

    AwMuxerRegister(&aacMuxerCtor, CDX_MUXER_AAC);
    AwMuxerRegister(&mp4MuxerCtor, CDX_MUXER_MOV);
    AwMuxerRegister(&tsMuxerCtor, CDX_MUXER_TS);
    AwMuxerRegister(&aacMuxerCtor, CDX_MUXER_MP3);
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