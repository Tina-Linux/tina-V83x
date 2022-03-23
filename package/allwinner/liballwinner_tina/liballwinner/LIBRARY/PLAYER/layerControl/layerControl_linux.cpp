
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "cdx_config.h"
#include "layerControl.h"
#include "log.h"
#include "memoryAdapter.h"

#define SAVE_PIC (0)
//#if 1
//#include "1639/drv_display.h"
//#elif CONFIG_CHIP == OPTION_CHIP_1623
#include "1623/drv_display.h"
//#else
//#error "this module only support chip 1639 and 1623."
//#endif

#if CONFIG_PRODUCT == OPTION_PRODUCT_PAD
static const int DISPLAY_CHANNEL = 0;   //* channel 0 for pad screen.
#else
//* products which use tv(hdmi) for display.
static const int DISPLAY_CHANNEL = 0;   //* channel 1 for hdmi screen.
#endif
static const int DISPLAY_LAYER = 101;     //* use layer 1 to show video, totally 4 layers, gui generally use layer 0.

static VideoPicture* gLastPicture = NULL;

#define NUM_OF_PICTURES_KEEP_IN_LIST    2      //* it may be set to 3 if new version of display system used.

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
    int                  fdDisplay;
    enum EPIXELFORMAT    eReceivePixelFormat;
    int                  nWidth;
    int                  nHeight;
    int                  nLeftOff;
    int                  nTopOff;
    int                  nDisplayWidth;
    int                  nDisplayHeight;
    LayerCtlCallback       callback;
    void*                pUserData;
    int                  bRenderToHardwareLayer;
    enum EPICTURE3DMODE  ePicture3DMode;
    enum EDISPLAY3DMODE  eDisplay3DMode;
    int                  bLayerInitialized;
    int                  bLayerShowed;
    int                  bFullScreenDisplay;

    //* use when render to gpu.
    VideoPicture         bufferWrappers[32];
    int                  bBufferWrapperUsed[32];

    //* use when render derect to hardware layer.
    VPictureNode*        pPictureListHead;
    VPictureNode         picNodes[32];

    int                  nScreenWidth;
    int                  nScreenHeight;

}LayerCtrlContext;

static int SetLayerParam(LayerCtrlContext* lc);
static void setHwcLayerPictureInfo(LayerCtrlContext* lc,
                                   disp_layer_info*  pLayerInfo,
                                   VideoPicture*     pPicture,
                                   VideoPicture*     pSecondPictureOf3D);


LayerCtrl* LayerInit(void* pNativeWindow, int bProtectedFlag)
{
    unsigned long     args[4];
    LayerCtrlContext* lc;
	disp_layer_info layerInfo;

    logv("LayerInit.");

    lc = (LayerCtrlContext*)malloc(sizeof(LayerCtrlContext));
    if(lc == NULL)
    {
        loge("malloc memory fail.");
        return NULL;
    }
    memset(lc, 0, sizeof(LayerCtrlContext));

    lc->fdDisplay = open("/dev/disp", O_RDWR);
    lc->eReceivePixelFormat = PIXEL_FORMAT_YUV_MB32_420;
    lc->bRenderToHardwareLayer = 1;
    lc->bFullScreenDisplay = 1;

    logv("lc->bRenderToHardwareLayer=%d\n", lc->bRenderToHardwareLayer);

    args[0] = DISPLAY_CHANNEL;
    args[1] = DISPLAY_LAYER;
    args[2] = 0;
    args[3] = 0;

	int ret = ioctl(lc->fdDisplay, DISP_CMD_LAYER_REQUEST, args);

	if(ret == DISPLAY_LAYER)
	{
		logv("DISP_CMD_LAYER_REQUEST return %d, success", ret);
	}
	else
	{
		logw("DISP_CMD_LAYER_REQUEST return %d, will reset %d", ret, DISPLAY_LAYER);

		if(ret >= 100)
		{
			args[0] = DISPLAY_CHANNEL;
			args[1] = ret;
			args[2] = 0;
			args[3] = 0;

			ioctl(lc->fdDisplay, DISP_CMD_LAYER_RELEASE, args);
		}

		args[0] = DISPLAY_CHANNEL;
		args[1] = DISPLAY_LAYER;
		args[2] = 0;
		args[3] = 0;

		ioctl(lc->fdDisplay, DISP_CMD_LAYER_CLOSE, args);
		ioctl(lc->fdDisplay, DISP_CMD_VIDEO_STOP, args);
		ioctl(lc->fdDisplay, DISP_CMD_LAYER_RELEASE, args);

		ret = ioctl(lc->fdDisplay, DISP_CMD_LAYER_REQUEST, args);

		if(ret != 101)
			loge("fatal error, DISP_CMD_LAYER_REQUEST return %d", ret);
	}

	memset(&layerInfo, 0, sizeof(disp_layer_info));

	//* get screen size.
	args[0] = DISPLAY_CHANNEL;
	args[1] = DISPLAY_LAYER;
	args[2] = 0;

    lc->nScreenWidth  = ioctl(lc->fdDisplay, DISP_CMD_SCN_GET_WIDTH, args);
    lc->nScreenHeight = ioctl(lc->fdDisplay, DISP_CMD_SCN_GET_HEIGHT, args);

    logd("screen:w %d, screen:h %d", lc->nScreenWidth, lc->nScreenHeight);

    //* open the memory module, we need get physical address of a picture buffer
    //* by MemAdapterGetPhysicAddress().
    if(gLastPicture == NULL)
        MemAdapterOpen();

    return (LayerCtrl*)lc;
}


void LayerRelease(LayerCtrl* l, int bKeepPictureOnScreen)
{
    LayerCtrlContext* lc;
    VPictureNode*     nodePtr;
    unsigned long     args[4];

    lc = (LayerCtrlContext*)l;

    logv("Layer release");

    //* disable layer.
    if(gLastPicture == NULL)
        LayerCtrlHideVideo(l);

	args[0] = DISPLAY_CHANNEL;
	args[1] = DISPLAY_LAYER;
	args[2] = 0;
	args[3] = 0;

	if(lc->bLayerInitialized == 1)
	{
	    if(lc->bLayerShowed == 1)
	    {
		lc->bLayerShowed = 0;

	        ioctl(lc->fdDisplay, DISP_CMD_LAYER_CLOSE, args);
	    }

		ioctl(lc->fdDisplay, DISP_CMD_VIDEO_STOP, args);

		lc->bLayerInitialized = 0;
	}

	ioctl(lc->fdDisplay, DISP_CMD_LAYER_RELEASE, args);

    if(lc->fdDisplay >= 0)
        close(lc->fdDisplay);

    //* return pictures.
    while(lc->pPictureListHead != NULL)
    {
        nodePtr = lc->pPictureListHead;
        lc->pPictureListHead = lc->pPictureListHead->pNext;
        lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)nodePtr->pPicture);
    }

    //* free the memory module.
    if(gLastPicture == NULL)
        MemAdapterClose();

    free(lc);
}


int LayerSetExpectPixelFormat(LayerCtrl* l, enum EPIXELFORMAT ePixelFormat)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("Layer set expected pixel format, format = %d", (int)ePixelFormat);

    //* reder directly to hardware layer.
    if(ePixelFormat == PIXEL_FORMAT_YUV_MB32_420   ||
       ePixelFormat == PIXEL_FORMAT_YUV_MB32_422   ||
       ePixelFormat == PIXEL_FORMAT_YUV_MB32_444   ||
       ePixelFormat == PIXEL_FORMAT_YUV_PLANER_420 ||
       ePixelFormat == PIXEL_FORMAT_YV12           ||
       ePixelFormat == PIXEL_FORMAT_NV12           ||
       ePixelFormat == PIXEL_FORMAT_NV21)   //* add new pixel formats supported by hardware layer here.
    {
        lc->eReceivePixelFormat = ePixelFormat;
    }
    else
    {
        loge("receive pixel format is %d, not match.", lc->eReceivePixelFormat);
        return -1;
    }

    return 0;
}


enum EPIXELFORMAT LayerGetPixelFormat(LayerCtrl* l)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("Layer get pixel format, return %d", lc->eReceivePixelFormat);

    return lc->eReceivePixelFormat;
}


int LayerSetPictureSize(LayerCtrl* l, int nWidth, int nHeight)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("Layer set picture size, width = %d, height = %d", nWidth, nHeight);

    lc->nWidth         = nWidth;
    lc->nHeight        = nHeight;
    lc->nDisplayWidth  = nWidth;
    lc->nDisplayHeight = nHeight;
    lc->nLeftOff       = 0;
    lc->nTopOff        = 0;

    return 0;
}


int LayerSetDisplayRegion(LayerCtrl* l, int nLeftOff, int nTopOff, int nDisplayWidth, int nDisplayHeight)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("Layer set display region, leftOffset = %d, topOffset = %d, displayWidth = %d, displayHeight = %d",
        nLeftOff, nTopOff, nDisplayWidth, nDisplayHeight);

    if(nDisplayWidth != 0 && nDisplayHeight != 0)
    {
        lc->nDisplayWidth  = nDisplayWidth;
        lc->nDisplayHeight = nDisplayHeight;
        lc->nLeftOff       = nLeftOff;
        lc->nTopOff        = nTopOff;
        return 0;
    }
    else
        return -1;
}


int LayerSetPicture3DMode(LayerCtrl* l, enum EPICTURE3DMODE ePicture3DMode)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("Layer set picture 3d mode, mode = %d", (int)ePicture3DMode);

    lc->ePicture3DMode = ePicture3DMode;

    return 0;
}


enum EPICTURE3DMODE LayerGetPicture3DMode(LayerCtrl* l)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("Layer get picture 3d mode, mode = %d", (int)lc->ePicture3DMode);

    return lc->ePicture3DMode;
}


int LayerSetDisplay3DMode(LayerCtrl* l, enum EDISPLAY3DMODE eDisplay3DMode)
{
    LayerCtrlContext* lc;
    disp_layer_info   layerInfo;
    unsigned long     args[4];
    int               err;

    lc = (LayerCtrlContext*)l;

    logv("Layer set display 3d mode, mode = %d", (int)eDisplay3DMode);

    lc->eDisplay3DMode = eDisplay3DMode;

    if(lc->bLayerInitialized == 0)
        return 0;

    args[0] = DISPLAY_CHANNEL;
    args[1] = DISPLAY_LAYER;
    args[2] = (unsigned long)&layerInfo;
    args[3] = 0;

	ioctl(lc->fdDisplay, DISP_CMD_LAYER_GET_PARA, args);

    switch(lc->ePicture3DMode)
    {
        case PICTURE_3D_MODE_TWO_SEPERATED_PICTURE:
            layerInfo.fb.b_trd_src = 1;
            layerInfo.fb.trd_mode  = DISP_3D_SRC_MODE_FP;
            break;
        case PICTURE_3D_MODE_SIDE_BY_SIDE:
            layerInfo.fb.b_trd_src = 1;
            layerInfo.fb.trd_mode  = DISP_3D_SRC_MODE_SSH;
            break;
        case PICTURE_3D_MODE_TOP_TO_BOTTOM:
            layerInfo.fb.b_trd_src = 1;
            layerInfo.fb.trd_mode  = DISP_3D_SRC_MODE_TB;
            break;
        case PICTURE_3D_MODE_LINE_INTERLEAVE:
            layerInfo.fb.b_trd_src = 1;
            layerInfo.fb.trd_mode  = DISP_3D_SRC_MODE_LI;
            break;
        default:
            layerInfo.fb.b_trd_src = 0;
            break;
    }

    switch(eDisplay3DMode)
    {
        case DISPLAY_3D_MODE_3D:
            if(lc->ePicture3DMode == PICTURE_3D_MODE_TWO_SEPERATED_PICTURE)
                layerInfo.out_trd_mode = DISP_3D_OUT_MODE_FP;
            else if(lc->ePicture3DMode == PICTURE_3D_MODE_SIDE_BY_SIDE)
                layerInfo.out_trd_mode = DISP_3D_OUT_MODE_SSH;
            else if(lc->ePicture3DMode == PICTURE_3D_MODE_TOP_TO_BOTTOM)
                layerInfo.out_trd_mode = DISP_3D_OUT_MODE_TB;
            else if(lc->ePicture3DMode == PICTURE_3D_MODE_LINE_INTERLEAVE)
                layerInfo.out_trd_mode = DISP_3D_OUT_MODE_LI;
            else
                layerInfo.out_trd_mode = DISP_3D_OUT_MODE_SSH;

            layerInfo.b_trd_out = 1;
            break;

        case DISPLAY_3D_MODE_HALF_PICTURE:
            if(lc->ePicture3DMode == PICTURE_3D_MODE_SIDE_BY_SIDE)
            {
                //* set source window to the top half.
                layerInfo.src_win.x	    = lc->nLeftOff;
                layerInfo.src_win.y	    = lc->nTopOff;
                layerInfo.src_win.width  = lc->nDisplayWidth/2;
                layerInfo.src_win.height = lc->nDisplayHeight;
            }
            else if(lc->ePicture3DMode == PICTURE_3D_MODE_TOP_TO_BOTTOM)
            {
                //* set source window to the left half.
                layerInfo.src_win.x	    = lc->nLeftOff;
                layerInfo.src_win.y	    = lc->nTopOff;
                layerInfo.src_win.width  = lc->nDisplayWidth;
                layerInfo.src_win.height = lc->nDisplayHeight/2;
            }
            else
            {
                //* set source window to the full picture.
                layerInfo.src_win.x	    = lc->nLeftOff;
                layerInfo.src_win.y	    = lc->nTopOff;
                layerInfo.src_win.width  = lc->nDisplayWidth;
                layerInfo.src_win.height = lc->nDisplayHeight;
            }

            layerInfo.b_trd_out = 0;
            break;

        case DISPLAY_3D_MODE_2D:
        default:
            //* set source window to the full picture.
            layerInfo.src_win.x	    = lc->nLeftOff;
            layerInfo.src_win.y	    = lc->nTopOff;
            layerInfo.src_win.width  = lc->nDisplayWidth;
            layerInfo.src_win.height = lc->nDisplayHeight;
            layerInfo.b_trd_out = 0;
            break;
    }

    err = ioctl(lc->fdDisplay, DISP_CMD_LAYER_SET_PARA, args);
    if(err == 0)
    {
        lc->eDisplay3DMode = eDisplay3DMode;
        return 0;
    }
    else
    {
        logw("set 3d mode fail to hardware layer.");
        return -1;
    }
}


enum EDISPLAY3DMODE LayerGetDisplay3DMode(LayerCtrl* l)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logv("Layer get display 3d mode, mode = %d", (int)lc->eDisplay3DMode);

    return lc->eDisplay3DMode;
}


int LayerSetCallback(LayerCtrl* l, LayerCtlCallback callback, void* pUserData)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;
    lc->callback  = callback;
    lc->pUserData = pUserData;
    return 0;
}


int LayerDequeueBuffer(LayerCtrl* l, VideoPicture** ppBuf, int bInitFlag)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    *ppBuf = NULL;

    return LAYER_RESULT_USE_OUTSIDE_BUFFER;
}



int LayerQueueBuffer(LayerCtrl* l, VideoPicture* pBuf, int bValid)
{
    LayerCtrlContext* lc;
    int               i;
    VPictureNode*     newNode;
    VPictureNode*     nodePtr;
    disp_layer_info   layerInfo;
    unsigned long     args[4];
	disp_video_fb	  videoInfo;

    lc = (LayerCtrlContext*)l;
    if(pBuf == NULL)
    {
        loge("pBuf == NULL");
        return -1;
    }

#if (SAVE_PIC)
static int dump = 0;
    if (dump < 2)
    {
        FILE* fp1 = NULL;
        char path[512] = {0};

        loge("layer[%d] dump image hw(%d) size(%d * %d) , addr(%p/%p)",
                    dump, lc->bRenderToHardwareLayer,  pBuf->nWidth, pBuf->nHeight,
                    pBuf->pData0, pBuf->pData1);

        sprintf(path, "/mnt/out/layer.%d.dat", dump);
        fp1 = fopen(path, "wb");
//        MemAdapterFlushCache(pBuf->pData0, (pBuf->nWidth*pBuf->nHeight*3)/2);
        fwrite(pBuf->pData0, 1, (pBuf->nWidth*pBuf->nHeight*3)/2, fp1);
        fclose(fp1);

        dump++;
    }
#endif
	memset(pBuf->pData0, 0xff, (pBuf->nWidth*pBuf->nHeight));
	MemAdapterFlushCache(pBuf->pData0, (pBuf->nWidth*pBuf->nHeight));

    if(bValid == 0)
    {
        lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)pBuf);
        return 0;
    }

    if(lc->bLayerInitialized == 0)
    {
        if(SetLayerParam(lc) != 0)
        {
            loge("can not initialize layer.");
            return -1;
        }

		//* set picture to display hardware.
		setHwcLayerPictureInfo(lc, &layerInfo, pBuf, NULL);

		args[0] = DISPLAY_CHANNEL;
		args[1] = DISPLAY_LAYER;
		args[2] = 0;
		args[3] = 0;

		ioctl(lc->fdDisplay, DISP_CMD_VIDEO_START, args);

		if(lc->bLayerShowed == 0)
		{
			lc->bLayerShowed = 1;

			ioctl(lc->fdDisplay, DISP_CMD_LAYER_OPEN, args);
		}

        lc->bLayerInitialized = 1;

        if(lc->eDisplay3DMode != DISPLAY_3D_MODE_2D || lc->ePicture3DMode != PICTURE_3D_MODE_NONE)
           LayerSetDisplay3DMode(l, lc->eDisplay3DMode);
    }

    if(pBuf->nWidth != lc->nWidth ||
       pBuf->nHeight != lc->nHeight ||
       pBuf->ePixelFormat != lc->eReceivePixelFormat)
    {
        //* config the display hardware again.
        //* TODO.
    }

	memset(&videoInfo, 0, sizeof(disp_video_fb));

	videoInfo.interlace         = (pBuf->bIsProgressive?0:1);
	videoInfo.top_field_first   = pBuf->bTopFieldFirst;
	videoInfo.addr[0]           = (unsigned long)MemAdapterGetPhysicAddress(pBuf->pData0);
	videoInfo.addr[1]           = (unsigned long)MemAdapterGetPhysicAddress(pBuf->pData1);
	videoInfo.addr[2]			= (unsigned long)MemAdapterGetPhysicAddress(pBuf->pData2);
	videoInfo.addr_right[0]     = (unsigned long)MemAdapterGetPhysicAddress(pBuf->pData2);
	videoInfo.addr_right[1]     = (unsigned long)MemAdapterGetPhysicAddress(pBuf->pData3);
	videoInfo.addr_right[2]		= 0;
	videoInfo.id                = pBuf->nID;
	videoInfo.maf_valid         = pBuf->bMafValid;
	videoInfo.pre_frame_valid   = pBuf->bPreFrmValid;
	videoInfo.flag_addr         = (unsigned long)MemAdapterGetPhysicAddress(pBuf->pMafData);
	videoInfo.flag_stride       = pBuf->nMafFlagStride;

    args[0] = DISPLAY_CHANNEL;
    args[1] = DISPLAY_LAYER;
    args[2] = (unsigned long)(&videoInfo);
    args[3] = 0;

    int ret = ioctl(lc->fdDisplay, DISP_CMD_VIDEO_SET_FB, args);
    if(ret)
    {
	loge("++++ DISP_CMD_VIDEO_SET_FB failed", ret);
    }

    if(lc->bLayerShowed == 1)
    {
        int nCurFrameId;
        int nWaitTime;

        args[0] = DISPLAY_CHANNEL;
        args[1] = DISPLAY_LAYER;
        args[2] = 0;
        args[3] = 0;

        nWaitTime = 50000;  //* max frame interval is 1000/24fps = 41.67ms, here we wait 50ms for max.

        do
        {
			nCurFrameId = ioctl(lc->fdDisplay, DISP_CMD_VIDEO_GET_FRAME_ID, args);
            if(nCurFrameId == pBuf->nID)
                break;
            else
            {
                if(nWaitTime <= 0)
                {
                    loge("check frame id fail, maybe something error with the HWC layer. nCurFrameId(%d),nID(%d)", nCurFrameId, pBuf->nID);
                    break;
                }
                else
                {
                    usleep(5000);
                    nWaitTime -= 5000;
                }
            }
        }while(1);
    }

    //* free last picture of last video stream in case gConfigHoldLastPicture is set.
    if(gLastPicture != NULL)
    {
        logv("xxxx free gLastPicture, pData0 = %p", gLastPicture->pData0);
        FreePictureBuffer(gLastPicture);
        gLastPicture = NULL;
    }

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

    return 0;
}


int LayerDequeue3DBuffer(LayerCtrl* l, VideoPicture** ppBuf0, VideoPicture** ppBuf1)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;
    *ppBuf0 = NULL;
    *ppBuf1 = NULL;

    return LAYER_RESULT_USE_OUTSIDE_BUFFER;
}


int LayerQueue3DBuffer(LayerCtrl* l, VideoPicture* pBuf0, VideoPicture* pBuf1, int bValid)
{
    LayerCtrlContext* lc;
    int               i;
    VPictureNode*     newNode;
    VPictureNode*     nodePtr;
    disp_layer_info   layerInfo;
    unsigned long     args[4];
    disp_video_fb	  videoInfo;

    lc = (LayerCtrlContext*)l;

    if(bValid == 0)
    {
        if(pBuf0 != NULL)
            lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)pBuf0);
        if(pBuf1 != NULL)
            lc->callback(lc->pUserData, MESSAGE_ID_LAYER_RETURN_BUFFER, (void*)pBuf1);
        return 0;
    }

    if(lc->bLayerInitialized == 0)
    {
        if(SetLayerParam(lc) != 0)
        {
            loge("can not initialize layer.");
            return -1;
        }

		//* set picture to display hardware.
		setHwcLayerPictureInfo(lc, &layerInfo, pBuf0, pBuf1);

		args[0] = DISPLAY_CHANNEL;
		args[1] = DISPLAY_LAYER;
		args[2] = 0;
		args[3] = 0;

		ioctl(lc->fdDisplay, DISP_CMD_VIDEO_START, args);

		if(lc->bLayerShowed == 0)
		{
			lc->bLayerShowed = 1;

			ioctl(lc->fdDisplay, DISP_CMD_LAYER_OPEN, args);
		}

        lc->bLayerInitialized = 1;

        if(lc->eDisplay3DMode != DISPLAY_3D_MODE_2D || lc->ePicture3DMode != PICTURE_3D_MODE_NONE)
           LayerSetDisplay3DMode(l, lc->eDisplay3DMode);
    }

    if(pBuf0->nWidth != lc->nWidth ||
       pBuf0->nHeight != lc->nHeight ||
       pBuf0->ePixelFormat != lc->eReceivePixelFormat)
    {
        //* config the display hardware again.
        //* TODO.
    }

	//* 3D maybe fail
	memset(&videoInfo, 0, sizeof(disp_video_fb));

	videoInfo.interlace         = (pBuf0->bIsProgressive?0:1);
	videoInfo.top_field_first   = pBuf0->bTopFieldFirst;
	videoInfo.addr[0]           = (unsigned long)MemAdapterGetPhysicAddress(pBuf0->pData0);
	videoInfo.addr[1]           = (unsigned long)MemAdapterGetPhysicAddress(pBuf0->pData1);
	videoInfo.addr[2]			= (unsigned long)MemAdapterGetPhysicAddressCpu(pBuf1->pData0);
	videoInfo.addr_right[0]     = (unsigned long)MemAdapterGetPhysicAddressCpu(pBuf1->pData0);
	videoInfo.addr_right[1]     = (unsigned long)MemAdapterGetPhysicAddressCpu(pBuf1->pData1);
	videoInfo.addr_right[2]		= 0;
	videoInfo.id                = pBuf0->nID;
	videoInfo.maf_valid         = pBuf0->bMafValid;
	videoInfo.pre_frame_valid   = pBuf0->bPreFrmValid;
	videoInfo.flag_addr         = (unsigned long)MemAdapterGetPhysicAddress(pBuf0->pMafData);
	videoInfo.flag_stride       = pBuf0->nMafFlagStride;

    args[0] = DISPLAY_CHANNEL;
    args[1] = DISPLAY_LAYER;
    args[2] = (unsigned long)(&videoInfo);
    args[3] = 0;

    ioctl(lc->fdDisplay, DISP_CMD_VIDEO_SET_FB, args);

    //* wait for new frame showed.
    if(lc->bLayerShowed == 1)
    {
        int nCurFrameId;
        int nWaitTime;

        args[0] = DISPLAY_CHANNEL;
        args[1] = DISPLAY_LAYER;
        args[2] = 0;
        args[3] = 0;

        nWaitTime = 50000;  //* max frame interval is 1000/24fps = 41.67ms, here we wait 50ms for max.
        do
        {
			nCurFrameId = ioctl(lc->fdDisplay, DISP_CMD_VIDEO_GET_FRAME_ID, args);
            if(nCurFrameId == pBuf0->nID)
                break;
            else
            {
                if(nWaitTime <= 0)
                {
                    loge("check frame id fail, maybe something error with the HWC layer.");
                    break;
                }
                else
                {
                    usleep(5000);
                    nWaitTime -= 5000;
                }
            }
        }while(1);
    }

    //* attach the new picture to list and return the old picture.
    newNode = NULL;
    for(i=0; i<32; i++)
    {
        if(lc->picNodes[i].bUsed == 0)
        {
            newNode = &lc->picNodes[i];
            newNode->pNext              = NULL;
            newNode->bUsed              = 1;
            newNode->pPicture           = pBuf0;
            newNode->pSecondPictureOf3D = pBuf1;
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

    return 0;
}


static int SetLayerParam(LayerCtrlContext* lc)
{
    disp_layer_info   layerInfo;
    unsigned long     args[4];

    //* close the layer first, otherwise, in case when last frame is kept showing,
    //* the picture showed will not valid because parameters changed.
    if(lc->bLayerShowed == 1)
    {
	lc->bLayerShowed = 0;

        args[0] = DISPLAY_CHANNEL;
        args[1] = DISPLAY_LAYER;
        args[2] = 0;
        args[3] = 0;

        ioctl(lc->fdDisplay, DISP_CMD_LAYER_CLOSE, args);
    }

	args[0] = DISPLAY_CHANNEL;
	args[1] = DISPLAY_LAYER;
    args[2] = (unsigned long)(&layerInfo);
    args[3] = 0;

    ioctl(lc->fdDisplay, DISP_CMD_LAYER_GET_PARA, args);

    //* transform pixel format.
    switch(lc->eReceivePixelFormat)
    {
        case PIXEL_FORMAT_YUV_PLANER_420:
            layerInfo.fb.format = DISP_FORMAT_YUV420;
			layerInfo.fb.mode = DISP_MOD_NON_MB_PLANAR;
			layerInfo.fb.seq = DISP_SEQ_P3210;
            break;
        case PIXEL_FORMAT_YV12:
            layerInfo.fb.format = DISP_FORMAT_YUV420;
			layerInfo.fb.mode = DISP_MOD_NON_MB_PLANAR;
			layerInfo.fb.seq = DISP_SEQ_P3210;
            break;
        case PIXEL_FORMAT_YUV_MB32_420:
            layerInfo.fb.format = DISP_FORMAT_YUV420;
			layerInfo.fb.mode = DISP_MOD_MB_UV_COMBINED;
			layerInfo.fb.seq = DISP_SEQ_UVUV;
            break;
        case PIXEL_FORMAT_YUV_MB32_422:
            layerInfo.fb.format = DISP_FORMAT_YUV422;
			layerInfo.fb.mode = DISP_MOD_MB_UV_COMBINED;
			layerInfo.fb.seq = DISP_SEQ_UVUV;
            break;
        case PIXEL_FORMAT_YUV_PLANER_422:
            layerInfo.fb.format = DISP_FORMAT_YUV422;
			layerInfo.fb.mode = DISP_MOD_MB_UV_COMBINED;
			layerInfo.fb.seq = DISP_SEQ_UVUV;
            break;
        default:
        {
            loge("unsupported pixel format.");
            return -1;
            break;
        }
    }

    //* initialize the layerInfo.
    layerInfo.fb.cs_mode	 = (lc->nHeight < 720) ? DISP_BT601 : DISP_BT709;
    layerInfo.mode			 = DISP_LAYER_WORK_MODE_SCALER;
    layerInfo.pipe		 = 0;
    layerInfo.ck_enable      = 0;

	if(lc->bFullScreenDisplay)
	{
		layerInfo.scn_win.x      = 0;
		layerInfo.scn_win.y	 = 0;
		layerInfo.scn_win.width  = lc->nScreenWidth;
		layerInfo.scn_win.height = lc->nScreenHeight;
	}
	else
	{
		if(lc->nScreenWidth < lc->nDisplayWidth)
		{
		    layerInfo.scn_win.x      = 0;
		    layerInfo.scn_win.width  = lc->nScreenWidth;
		}
		else
		{
		    layerInfo.scn_win.x      = (lc->nScreenWidth - lc->nDisplayWidth)/2;
		    layerInfo.scn_win.width  = lc->nDisplayWidth;
		}
		if(lc->nScreenHeight < lc->nDisplayHeight)
		{
			layerInfo.scn_win.y	 = 0;
			layerInfo.scn_win.height = lc->nScreenHeight;
		}
		else
		{
			layerInfo.scn_win.y	 = (lc->nScreenHeight - lc->nDisplayHeight)/2;
			layerInfo.scn_win.height = lc->nDisplayHeight;
		}
	}

    //* image size.
    layerInfo.fb.size.width     = lc->nDisplayWidth;
	layerInfo.fb.size.height    = lc->nDisplayHeight;

    //* source window.
    layerInfo.src_win.x	    = lc->nLeftOff;
    layerInfo.src_win.y	    = lc->nTopOff;
    layerInfo.src_win.width  = lc->nDisplayWidth;
    layerInfo.src_win.height = lc->nDisplayHeight;

    //* set layerInfo to the driver.
    args[0] = DISPLAY_CHANNEL;
    args[1] = DISPLAY_LAYER;
    args[2] = (unsigned long)(&layerInfo);
    args[3] = 0;

	ioctl(lc->fdDisplay, DISP_CMD_LAYER_SET_PARA, args);

    //* set the video layer to the top.
    args[0] = DISPLAY_CHANNEL;
    args[1] = DISPLAY_LAYER;
    args[2] = 0;
    args[3] = 0;

	ioctl(lc->fdDisplay, DISP_CMD_LAYER_TOP, args);

    return 0;
}


static void setHwcLayerPictureInfo(LayerCtrlContext* lc,
                                   disp_layer_info*  pLayerInfo,
                                   VideoPicture*     pPicture,
                                   VideoPicture*     pSecondPictureOf3D)
{
    unsigned long   args[4];

    args[0] = DISPLAY_CHANNEL;
    args[1] = DISPLAY_LAYER;
    args[2] = (unsigned long)pLayerInfo;
    args[3] = 0;

    ioctl(lc->fdDisplay, DISP_CMD_LAYER_GET_PARA, args);

    if(pSecondPictureOf3D == NULL)
    {
        pLayerInfo->fb.addr[0]			 = (unsigned long)MemAdapterGetPhysicAddressCpu(pPicture->pData0);
        pLayerInfo->fb.addr[1]			 = (unsigned long)MemAdapterGetPhysicAddressCpu(pPicture->pData1);
        pLayerInfo->fb.addr[2]           = (unsigned long)MemAdapterGetPhysicAddressCpu(pPicture->pData2);
        pLayerInfo->fb.trd_right_addr[0] = (unsigned long)MemAdapterGetPhysicAddressCpu(pPicture->pData2);
        pLayerInfo->fb.trd_right_addr[1] = (unsigned long)MemAdapterGetPhysicAddressCpu(pPicture->pData3);
        pLayerInfo->fb.trd_right_addr[2] = 0;
        //* should we set the address fb.addr[2] according to the pixel format?
    }
    else
    {
        pLayerInfo->fb.addr[0]           = (unsigned long)MemAdapterGetPhysicAddressCpu(pPicture->pData0);
        pLayerInfo->fb.addr[1]           = (unsigned long)MemAdapterGetPhysicAddressCpu(pPicture->pData1);
        pLayerInfo->fb.addr[2]           = (unsigned long)MemAdapterGetPhysicAddressCpu(pSecondPictureOf3D->pData0);
        pLayerInfo->fb.trd_right_addr[0] = (unsigned long)MemAdapterGetPhysicAddressCpu(pSecondPictureOf3D->pData0);
        pLayerInfo->fb.trd_right_addr[1] = (unsigned long)MemAdapterGetPhysicAddressCpu(pSecondPictureOf3D->pData1);
        pLayerInfo->fb.trd_right_addr[2] = 0;
        //* should we set the address fb.addr[2] according to the pixel format?
    }

    //* set layer info
	ioctl(lc->fdDisplay, DISP_CMD_LAYER_SET_PARA, args);

    return;
}


int LayerCtrlShowVideo(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    int               i;
    unsigned long     args[4];

    lc = (LayerCtrlContext*)l;
    logv("xxxx show video, current show flag = %d", lc->bLayerShowed);
    if(lc->bLayerShowed == 0)
    {
	lc->bLayerShowed = 1;

        args[0] = DISPLAY_CHANNEL;
        args[1] = DISPLAY_LAYER;
        args[2] = 0;
        args[3] = 0;

        ioctl(lc->fdDisplay, DISP_CMD_LAYER_OPEN, args);
    }
    return 0;
}


int LayerCtrlHideVideo(LayerCtrl* l)
{
    LayerCtrlContext* lc;
    int               i;
    unsigned long     args[4];

    lc = (LayerCtrlContext*)l;
    logv("xxxx hide video, current show flag = %d", lc->bLayerShowed);
    if(lc->bLayerShowed == 1)
    {
	lc->bLayerShowed = 0;

        args[0] = DISPLAY_CHANNEL;
        args[1] = DISPLAY_LAYER;
        args[2] = 0;
        args[3] = 0;

        ioctl(lc->fdDisplay, DISP_CMD_LAYER_CLOSE, args);
    }
    return 0;
}


int LayerCtrlIsVideoShow(LayerCtrl* l)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;
    return lc->bLayerShowed;
}


int LayerCtrlHoldLastPicture(LayerCtrl* l, int bHold)
{
    LayerCtrlContext* lc;
    VPictureNode*     nodePtr;
    unsigned long     args[4];

    lc = (LayerCtrlContext*)l;

    logv("LayerCtrlHoldLastPicture, bHold = %d", bHold);

    if(bHold == 0)
    {
        if(gLastPicture != NULL)
        {
            FreePictureBuffer(gLastPicture);
            gLastPicture = NULL;
        }
        return 0;
    }

    VideoPicture* lastPicture;

    lastPicture = NULL;
    nodePtr = lc->pPictureListHead;
    if(nodePtr != NULL)
    {
        while(nodePtr->pNext != NULL)
            nodePtr = nodePtr->pNext;
        lastPicture = nodePtr->pPicture;
    }

    if(lastPicture != NULL)
    {
        if(gLastPicture != NULL)
            FreePictureBuffer(gLastPicture);

        gLastPicture = AllocatePictureBuffer(lastPicture->nWidth,
                                             lastPicture->nHeight,
                                             lastPicture->nLineStride,
                                             lastPicture->ePixelFormat);
        logw("width = %d, height = %d, pdata0 = %p",
            gLastPicture->nWidth, gLastPicture->nHeight, gLastPicture->pData0);
        if(gLastPicture != NULL)
        {
            disp_video_fb videoInfo;

            gLastPicture->nID = 0xa5a5a5a5;
            int nGpuYAlign = 16;
            int nGpuCAlign = 8;
            RotatePicture(lastPicture, gLastPicture, 0,nGpuYAlign,nGpuCAlign);

            //* set picture to display hardware.

			memset(&videoInfo, 0, sizeof(disp_video_fb));

			videoInfo.interlace		= (gLastPicture->bIsProgressive?0:1);
			videoInfo.top_field_first	= gLastPicture->bTopFieldFirst;
			videoInfo.addr[0]			= (unsigned long)MemAdapterGetPhysicAddress(gLastPicture->pData0);
			videoInfo.addr[1]			= (unsigned long)MemAdapterGetPhysicAddress(gLastPicture->pData1);
			videoInfo.addr[2]			= (unsigned long)MemAdapterGetPhysicAddress(gLastPicture->pData2);
			videoInfo.addr_right[0]		= (unsigned long)MemAdapterGetPhysicAddress(gLastPicture->pData2);
			videoInfo.addr_right[1]		= (unsigned long)MemAdapterGetPhysicAddress(gLastPicture->pData3);
			videoInfo.addr_right[2]		= 0;
			videoInfo.id				= gLastPicture->nID;
			videoInfo.maf_valid		= gLastPicture->bMafValid;
			videoInfo.pre_frame_valid	= gLastPicture->bPreFrmValid;
			videoInfo.flag_addr		= (unsigned long)MemAdapterGetPhysicAddress(gLastPicture->pMafData);
			videoInfo.flag_stride		= gLastPicture->nMafFlagStride;

			args[0] = DISPLAY_CHANNEL;
			args[1] = DISPLAY_LAYER;
			args[2] = (unsigned long)(&videoInfo);
			args[3] = 0;

			ioctl(lc->fdDisplay, DISP_CMD_VIDEO_SET_FB, args);

            //* wait for new frame showed.
            if(lc->bLayerShowed == 1)
            {
                int nCurFrameId;
                int nWaitTime;

                args[0] = DISPLAY_CHANNEL;
                args[1] = DISPLAY_LAYER;
                args[2] = 0;
                args[3] = 0;

                nWaitTime = 50000;  //* max frame interval is 1000/24fps = 41.67ms, here we wait 50ms for max.
                do
                {
					nCurFrameId = ioctl(lc->fdDisplay, DISP_CMD_VIDEO_GET_FRAME_ID, args);
                    if(nCurFrameId == gLastPicture->nID)
                        break;
                    else
                    {
                        if(nWaitTime <= 0)
                        {
                            loge("check frame id fail, maybe something error with the HWC layer. nCurFrameId(%d)", nCurFrameId);
                            break;
                        }
                        else
                        {
                            usleep(5000);
                            nWaitTime -= 5000;
                        }
                    }
                }while(1);
            }
        }
    }

    return 0;
}

int LayerSetRenderToHardwareFlag(LayerCtrl* l,int bFlag)
{
    return 0;
}

int LayerSetDeinterlaceFlag(LayerCtrl* l,int bFlag)
{
    return 0;
}

int LayerSetFullScreenDisplay(LayerCtrl* l,int bFullScreenDisplay)
{
    LayerCtrlContext* lc;

    lc = (LayerCtrlContext*)l;

    logw("LayerSetFullScreenDisplay, bFullScreenDisplay = %d", bFullScreenDisplay);

	lc->bFullScreenDisplay = bFullScreenDisplay;

	return 0;
}

int LayerSetBufferCount(LayerCtrl* l, int nBufferCount)
{
    CDX_PLAYER_UNUSE(l);
    CDX_PLAYER_UNUSE(nBufferCount);
    logw("not implement the function LayerSetBufferCount");
    return -1;
}

int LayerSetVideoWithTwoStreamFlag(LayerCtrl* l, int bVideoWithTwoStreamFlag)
{
    CDX_PLAYER_UNUSE(l);
    CDX_PLAYER_UNUSE(bVideoWithTwoStreamFlag);
    logw("not implement the function LayerSetVideoWithTwoStreamFlag");
    return -1;
}

int LayerSetIsSoftDecoderFlag(LayerCtrl* l, int bIsSoftDecoderFlag)
{
    CDX_PLAYER_UNUSE(l);
    CDX_PLAYER_UNUSE(bIsSoftDecoderFlag);
    logw("not implement the function LayerSetIsSoftDecoderFlag");
    return -1;
}

void LayerResetNativeWindow(LayerCtrl* l,void* pNativeWindow)
{
    CDX_PLAYER_UNUSE(l);
    CDX_PLAYER_UNUSE(pNativeWindow);
    logw("not implement the function LayerResetNativeWindow");
    return ;
}

int LayerReleaseBuffer(LayerCtrl* l,VideoPicture* pPicture)
{
    CDX_PLAYER_UNUSE(l);
    CDX_PLAYER_UNUSE(pPicture);
    logw("not implement the function LayerReleaseBuffer");
    return -1;
}

VideoPicture* LayerGetPicNode(LayerCtrl* l)
{
    CDX_PLAYER_UNUSE(l);
    logw("not implement the function LayerGetPicNode");
    return NULL;
}

int LayerGetAddedPicturesCount(LayerCtrl* l)
{
    CDX_PLAYER_UNUSE(l);
    logw("not implement the function LayerGetAddedPicturesCount");
    return -1;
}

int LayerGetDisplayFPS(LayerCtrl* l)
{
    CDX_PLAYER_UNUSE(l);
    logw("not implement the function LayerGetDisplayFPS");
    return -1;
}

int LayerSetBufferTimeStamp(LayerCtrl* l, int64_t nPtsAbs)
{

	return 0;
}

int LayerGetRotationAngle(LayerCtrl* l)
{
	return 0;
}



LayerControlOpsT mLayerControlOps =
{
    LayerInit:                       LayerInit                      ,
    LayerRelease:                    LayerRelease                   ,
    LayerSetExpectPixelFormat:       LayerSetExpectPixelFormat      ,
    LayerGetPixelFormat:             LayerGetPixelFormat            ,
    LayerSetPictureSize:             LayerSetPictureSize            ,
    LayerSetDisplayRegion:           LayerSetDisplayRegion          ,
    LayerSetBufferTimeStamp:         LayerSetBufferTimeStamp        ,
    LayerDequeueBuffer:              LayerDequeueBuffer             ,
    LayerQueueBuffer:                LayerQueueBuffer               ,
    LayerCtrlHideVideo:              LayerCtrlHideVideo             ,
    LayerCtrlShowVideo:              LayerCtrlShowVideo             ,
    LayerCtrlIsVideoShow:		     LayerCtrlIsVideoShow           ,
    LayerCtrlHoldLastPicture:        LayerCtrlHoldLastPicture       ,
    LayerSetRenderToHardwareFlag:    LayerSetRenderToHardwareFlag   ,
    LayerSetDeinterlaceFlag:         LayerSetDeinterlaceFlag        ,
    LayerSetPicture3DMode:           LayerSetPicture3DMode          ,
    LayerGetPicture3DMode:           LayerGetPicture3DMode          ,
    LayerSetDisplay3DMode:           LayerSetDisplay3DMode          ,
    LayerGetDisplay3DMode:           LayerGetDisplay3DMode          ,
    LayerDequeue3DBuffer:            LayerDequeue3DBuffer           ,
    LayerQueue3DBuffer:              LayerQueue3DBuffer             ,
    LayerGetRotationAngle:           LayerGetRotationAngle          ,
    LayerSetCallback:                LayerSetCallback               ,
    LayerSetBufferCount:             LayerSetBufferCount            ,
    LayerSetVideoWithTwoStreamFlag:  LayerSetVideoWithTwoStreamFlag ,
    LayerSetIsSoftDecoderFlag:       LayerSetIsSoftDecoderFlag      ,
    LayerResetNativeWindow:          LayerResetNativeWindow         ,
    LayerReleaseBuffer:              LayerReleaseBuffer             ,
    LayerGetPicNode:                 LayerGetPicNode                ,
    LayerGetAddedPicturesCount:      LayerGetAddedPicturesCount     ,
    LayerGetDisplayFPS:              LayerGetDisplayFPS           ,
};
