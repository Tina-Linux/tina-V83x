#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "cdx_config.h"
#include "layerControl.h"
#include "log.h"
#include "player_i.h"


#define SAVE_PIC (0)

#include "1623/drv_display.h"


static VideoPicture* gLastPicture = NULL;

#define BUF_MANAGE (0)


int LayerCtrlHideVideo(LayerCtrl* l);

typedef struct VPictureNode_t VPictureNode;
struct VPictureNode_t
{
    VideoPicture* pPicture;
    VideoPicture* pSecondPictureOf3D;
    VPictureNode* pNext;
    int           bUsed;
};


typedef struct LayerCtrlContext
{
    int                  nScreenWidth;
    int                  nScreenHeight;
    LayerCtlCallback       callback;
    void*                pUserData;

    //* use when render derect to hardware layer.
    VPictureNode*        pPictureListHead;
    VPictureNode         picNodes[32];

}LayerCtrlContext;


LayerCtrl* __LayerInit(void* pNativeWindow)
{
    unsigned long     args[4];
    LayerCtrlContext* lc;

    logd("LayerInit.");

    lc = (LayerCtrlContext*)malloc(sizeof(LayerCtrlContext));
    if(lc == NULL)
    {
        loge("malloc memory fail.");
        return NULL;
    }
    memset(lc, 0, sizeof(LayerCtrlContext));


    return (LayerCtrl*)lc;
}


void __LayerRelease(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    VPictureNode*     nodePtr;
    unsigned long     args[4];

    lc = (LayerCtrlContext*)l;

    logv("Layer release");

    free(lc);
}

int __LayerSetDisplayBufferSize(LayerCtrl* l, int nWidth, int nHeight)
{
    LayerCtrlContext* lc;

    return 0;
}

//* Description: set initial param -- display region
int __LayerSetDisplayRegion(LayerCtrl* l, int nLeftOff, int nTopOff, int nDisplayWidth, int nDisplayHeight)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("Layer set display region, leftOffset = %d, topOffset = %d, displayWidth = %d, displayHeight = %d",
        nLeftOff, nTopOff, nDisplayWidth, nDisplayHeight);
    return 0;
}

//* Description: set initial param -- display pixelFormat
int __LayerSetDisplayPixelFormat(LayerCtrl* l, enum EPIXELFORMAT ePixelFormat)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    return 0;
}

//* Description: set initial param -- deinterlace flag
int __LayerSetDeinterlaceFlag(LayerCtrl* l,int bFlag)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    return 0;
}

//* Description: set buffer timestamp -- set this param every frame
int __LayerSetBufferTimeStamp(LayerCtrl* l, int64_t nPtsAbs)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

	return 0;
}

int __LayerGetRotationAngle(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    int nRotationAngle = 0;

    lc = (LayerCtrlContext*)l;

    return 0;
}

int __LayerSetCallback(LayerCtrl* l, LayerCtlCallback callback, void* pUserData)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;
    lc->callback  = callback;
    lc->pUserData = pUserData;
    return 0;
}

int __LayerCtrlShowVideo(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    int               i;

    lc = (LayerCtrlContext*)l;

    return 0;
}


int __LayerCtrlHideVideo(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    int               i;

    lc = (LayerCtrlContext*)l;

    return 0;
}


int __LayerCtrlIsVideoShow(LayerCtrl* l)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;
    return 0;
}


int  __LayerCtrlHoldLastPicture(LayerCtrl* l, int bHold)
{
    logd("LayerCtrlHoldLastPicture, bHold = %d", bHold);

    LayerCtrlContext* lc;
    lc = (LayerCtrlContext*)l;

    return 0;
}

int __LayerDequeueBuffer(LayerCtrl* l, VideoPicture** ppBuf)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;
    *ppBuf = NULL;

    return LAYER_RESULT_USE_OUTSIDE_BUFFER;
}

int __LayerDequeue3DBuffer(LayerCtrl* l, VideoPicture** ppBuf0, VideoPicture** ppBuf1)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;
    *ppBuf0 = NULL;
    *ppBuf1 = NULL;

    return LAYER_RESULT_USE_OUTSIDE_BUFFER;
}

int __LayerQueueBuffer(LayerCtrl* l, VideoPicture* pBuf, int bValid)
{
    LayerCtrlContext* lc  = NULL;

    int               i;
    VPictureNode*     newNode;
    VPictureNode*     nodePtr;

    lc = (LayerCtrlContext*)l;

    if(bValid == 0)
    {
        if(pBuf != NULL)
            lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)pBuf);
        return 0;
    }

	if(pBuf != NULL)
	{
		lc->callback(lc->pUserData, MESSAGE_ID_LAYER_NOTIFY_BUFFER, pBuf);
		lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)pBuf);
    }

#if BUF_MANAGE
    //* attach the new picture to list and return the old picture.
    newNode = NULL;
    for(i=0; i<32; i++)
    {
        if(lc->picNodes[i].bUsed == 0)
        {
            newNode = &lc->picNodes[i];
            newNode->pNext              = NULL;
            newNode->bUsed              = 1;
            newNode->pPicture           = pBuf;
            newNode->pSecondPictureOf3D = NULL;
            break;
        }
    }
    if(i == 32)
    {
        loge("not enough picture nodes, shouldn't run here.");
        abort();
    }

    if(lc->pPictureListHead != NULL)
    {
        nodePtr = lc->pPictureListHead;
        i = 1;
        while(nodePtr->pNext != NULL)
        {
            i++;
            nodePtr = nodePtr->pNext;
        }

        nodePtr->pNext = newNode;
        i++;

        //* return one picture.
        while(i > NUM_OF_PICTURES_KEEP_IN_LIST)
        {
            nodePtr = lc->pPictureListHead;
            lc->pPictureListHead = lc->pPictureListHead->pNext;
            lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)nodePtr->pPicture);
            nodePtr->pPicture = NULL;

            if(nodePtr->pSecondPictureOf3D != NULL)
            {
                lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)nodePtr->pSecondPictureOf3D);
                nodePtr->pSecondPictureOf3D = NULL;
            }
            nodePtr->bUsed = 0;
            i--;
        }
    }
    else
    {
        lc->pPictureListHead = newNode;
    }
#endif

    return 0;

	return 0;
}

int __LayerQueue3DBuffer(LayerCtrl* l, VideoPicture* pBuf0, VideoPicture* pBuf1, int bValid)
{
    LayerCtrlContext* lc  = NULL;
    int               i   = 0;
    int               ret = -1;

    lc = (LayerCtrlContext*)l;

    return 0;
}


LayerControlOpsT mLayerControlOps =
{
    init:                       __LayerInit                      ,
    release:                    __LayerRelease                   ,

    setCallback:                __LayerSetCallback               ,
    setDisplayBufferSize:       __LayerSetDisplayBufferSize      ,
    setDisplayRegion:           __LayerSetDisplayRegion          ,
    setDisplayPixelFormat:      __LayerSetDisplayPixelFormat     ,
    setDeinterlaceFlag:         __LayerSetDeinterlaceFlag        ,
    setBufferTimeStamp:         __LayerSetBufferTimeStamp        ,

    getRotationAngle:           __LayerGetRotationAngle          ,

    ctrlShowVideo:              __LayerCtrlShowVideo             ,
    ctrlHideVideo:              __LayerCtrlHideVideo             ,
    ctrlIsVideoShow:		        __LayerCtrlIsVideoShow           ,
    ctrlHoldLastPicture:        __LayerCtrlHoldLastPicture       ,

    dequeueBuffer:              __LayerDequeueBuffer             ,
    dequeue3DBuffer:            __LayerDequeue3DBuffer           ,
    queueBuffer:                __LayerQueueBuffer               ,
    queue3DBuffer:              __LayerQueue3DBuffer
};


LayerControlOpsT* __GetLayerControlOps()
{
    return &mLayerControlOps;
}
