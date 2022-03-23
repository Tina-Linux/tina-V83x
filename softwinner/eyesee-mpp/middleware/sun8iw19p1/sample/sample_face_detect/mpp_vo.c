#include "mpp_vo.h"
#include <stdbool.h>
#include <linux/g2d_driver.h>

// VO Callback Func
static ERRORTYPE VideoOutCallbackWrapper(void *cookie, MPP_CHN_S *pChn, MPP_EVENT_TYPE event, void *pEventData)
{
	VO_Params* pContext = (VO_Params*)cookie;

    ERRORTYPE ret = SUCCESS;
    if (MOD_ID_VOU == pChn->mModId)
    {
        switch(event)
        {
            case MPP_EVENT_RELEASE_VIDEO_BUFFER:
            {
                VIDEO_FRAME_INFO_S *pVideoFrameInfo = (VIDEO_FRAME_INFO_S*)pEventData;
                pContext->mFrameManager.ReleaseFrame(&pContext->mFrameManager, pVideoFrameInfo->mId);
                break;
            }
            case MPP_EVENT_SET_VIDEO_SIZE:
            {
                SIZE_S *pDisplaySize = (SIZE_S*)pEventData;
                alogd("vo report video display size[%dx%d]", pDisplaySize->Width, pDisplaySize->Height);
                break;
            }
            case MPP_EVENT_RENDERING_START:
            {
                alogd("vo report rendering start");
                break;
            }
            default:
            {
                aloge("fatal error! unknown event[0x%x] from channel[0x%x][0x%x][0x%x]!", event, pChn->mModId, pChn->mDevId, pChn->mChnId);
                ret = ERR_VO_ILLEGAL_PARAM;
                break;
            }
        }
    }
    return ret;
}


int create_vo(VO_Params* pVOParams)
{
    // Create VO channel
    AW_MPI_VO_Enable(pVOParams->iVoDev);
    AW_MPI_VO_AddOutsideVideoLayer(pVOParams->iMiniGUILayer);
    AW_MPI_VO_CloseVideoLayer(pVOParams->iMiniGUILayer); /* close ui layer. */
    AW_MPI_VO_EnableVideoLayer(pVOParams->iVoLayer);

    // Config VO channel
    VO_PUB_ATTR_S spPubAttr;
    AW_MPI_VO_GetPubAttr(pVOParams->iVoDev, &spPubAttr);
    alogd("IntfType:%d, IntfSync:%d", spPubAttr.enIntfType, spPubAttr.enIntfSync);
    spPubAttr.enIntfType = pVOParams->iDispType;
    spPubAttr.enIntfSync = pVOParams->iDispSync;

    if(VO_INTF_HDMI == pVOParams->iDispType)
    {
        VO_INTF_SYNC_E  tv_mode = VO_OUTPUT_NTSC;
        if (AW_MPI_VO_GetHdmiHwMode(pVOParams->iVoDev, &tv_mode) != SUCCESS) 
        {
            aloge("hdmi support mode failed, use 1080P60Hz as default");
            tv_mode = VO_OUTPUT_1080P60;  //DISP_TV_MOD_1080P_60HZ;
        }
        alogd("select hdmi mode:[%d]", tv_mode);
        spPubAttr.enIntfType = VO_INTF_HDMI;
        spPubAttr.enIntfSync = tv_mode;
    }
    AW_MPI_VO_SetPubAttr(pVOParams->iVoDev, &spPubAttr);

    // Config display parameter
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    AW_MPI_VO_GetVideoLayerAttr(pVOParams->iVoDev, &stLayerAttr);
    stLayerAttr.stDispRect.X      = 0;
    stLayerAttr.stDispRect.Y      = 0;
    stLayerAttr.stDispRect.Width  = pVOParams->iWidth;
    stLayerAttr.stDispRect.Height = pVOParams->iHeight;
    AW_MPI_VO_SetVideoLayerAttr(pVOParams->iVoLayer, &stLayerAttr);
    AW_MPI_VO_EnableChn(pVOParams->iVoLayer, pVOParams->iVoChn);

    // Config callback func
    MPPCallbackInfo cbInfo;
    cbInfo.cookie   = (void*)pVOParams;
    cbInfo.callback = (MPPCallbackFuncType)&VideoOutCallbackWrapper;
    AW_MPI_VO_RegisterCallback(pVOParams->iVoLayer, pVOParams->iVoChn, &cbInfo);
    AW_MPI_VO_SetChnDispBufNum(pVOParams->iVoLayer, pVOParams->iVoChn, 2);
    AW_MPI_VO_StartChn(pVOParams->iVoLayer, pVOParams->iVoChn);

    return 0;
}

int destroy_vo(VO_Params* pVOParams)
{
    // close VO channel
    AW_MPI_VO_StopChn(pVOParams->iVoLayer, pVOParams->iVoChn);
    AW_MPI_VO_DisableChn(pVOParams->iVoLayer, pVOParams->iVoChn);
    AW_MPI_VO_DisableVideoLayer(pVOParams->iVoLayer);
    AW_MPI_VO_RemoveOutsideVideoLayer(pVOParams->iMiniGUILayer);
    AW_MPI_VO_Disable(pVOParams->iVoDev);

    return 0;
}

static int g_iG2dFd = -1;

void open_g2d_device()
{
    // Initial g2d device
    g_iG2dFd = open("/dev/g2d", O_RDWR);
    assert(g_iG2dFd >= 0);
}

void close_g2d_device()
{
    // close g2d device
    close(g_iG2dFd);
}

void rotate_frame(VIDEO_FRAME_INFO_S* pSrcFrameInfo, VIDEO_FRAME_INFO_S* pDstFrameInfo)
{
    if (g_iG2dFd < 0)
    {
        aloge("fatal error:g2d device is not initialized");
        return;
    }

    //   ת90
    g2d_blt stG2dBlt;
    memset(&stG2dBlt, 0, sizeof(g2d_blt));
    stG2dBlt.flag  = G2D_BLT_ROTATE90;
    stG2dBlt.color = 0xff;
    stG2dBlt.alpha = 0xff;
    stG2dBlt.src_image.addr[0] = (unsigned int)pSrcFrameInfo->VFrame.mPhyAddr[0];
    stG2dBlt.src_image.addr[1] = (unsigned int)pSrcFrameInfo->VFrame.mPhyAddr[1];
    stG2dBlt.src_image.w = pSrcFrameInfo->VFrame.mWidth;
    stG2dBlt.src_image.h = pSrcFrameInfo->VFrame.mHeight;
    stG2dBlt.src_image.format    = G2D_FMT_PYUV420UVC;
    stG2dBlt.src_image.pixel_seq = G2D_SEQ_VUVU;
    stG2dBlt.src_rect.x = 0;
    stG2dBlt.src_rect.y = 0;
    stG2dBlt.src_rect.w = pSrcFrameInfo->VFrame.mWidth;
    stG2dBlt.src_rect.h = pSrcFrameInfo->VFrame.mHeight;

    stG2dBlt.dst_image.addr[0] = (unsigned int)pDstFrameInfo->VFrame.mPhyAddr[0];
    stG2dBlt.dst_image.addr[1] = (unsigned int)pDstFrameInfo->VFrame.mPhyAddr[1];
    stG2dBlt.dst_image.w = pSrcFrameInfo->VFrame.mHeight;
    stG2dBlt.dst_image.h = pSrcFrameInfo->VFrame.mWidth;
    stG2dBlt.dst_image.format    = G2D_FMT_PYUV420UVC;
    stG2dBlt.dst_image.pixel_seq = G2D_SEQ_VUVU;
    stG2dBlt.dst_x = 0;
    stG2dBlt.dst_y = 0;
    int iRet = ioctl(g_iG2dFd, G2D_CMD_BITBLT, &stG2dBlt);
    assert(iRet == 0);

    pDstFrameInfo->VFrame.mWidth  = pSrcFrameInfo->VFrame.mHeight;
    pDstFrameInfo->VFrame.mHeight = pSrcFrameInfo->VFrame.mWidth;
    pDstFrameInfo->VFrame.mOffsetBottom = pDstFrameInfo->VFrame.mHeight;
    pDstFrameInfo->VFrame.mOffsetRight  = pDstFrameInfo->VFrame.mWidth;
}

VIDEO_FRAME_INFO_S* VOFrameManager_PrefetchFirstIdleFrame(void *pThiz)
{
    VO_Frame_Manager *pFrameManager = (VO_Frame_Manager*)pThiz;
    VO_Frame_Node *pFirstNode;
    VIDEO_FRAME_INFO_S *pFrameInfo;
    pthread_mutex_lock(&pFrameManager->mLock);
    if(!list_empty(&pFrameManager->mIdleList))
    {
        pFirstNode = list_first_entry(&pFrameManager->mIdleList, VO_Frame_Node, mList);
        pFrameInfo = &pFirstNode->mFrame;
    }
    else
    {
        pFrameInfo = NULL;
    }
    pthread_mutex_unlock(&pFrameManager->mLock);
    return pFrameInfo;
}

int VOFrameManager_UseFrame(void *pThiz, VIDEO_FRAME_INFO_S *pFrame)
{
    int ret = 0;
    VO_Frame_Manager *pFrameManager = (VO_Frame_Manager*)pThiz;
    if(NULL == pFrame)
    {
        aloge("fatal error! pNode == NULL!");
        return -1;
    }
    pthread_mutex_lock(&pFrameManager->mLock);
    VO_Frame_Node *pFirstNode = list_first_entry_or_null(&pFrameManager->mIdleList, VO_Frame_Node, mList);
    if(pFirstNode)
    {
        if(&pFirstNode->mFrame == pFrame)
        {
            list_move_tail(&pFirstNode->mList, &pFrameManager->mUsingList);
        }
        else
        {
            aloge("fatal error! node is not match [%p]!=[%p]", pFrame, &pFirstNode->mFrame);
            ret = -1;
        }
    }
    else
    {
        aloge("fatal error! idle list is empty");
        ret = -1;
    }
    pthread_mutex_unlock(&pFrameManager->mLock);
    return ret;
}

int VOFrameManager_ReleaseFrame(void *pThiz, unsigned int nFrameId)
{
    int ret = 0;
    VO_Frame_Manager *pFrameManager = (VO_Frame_Manager*)pThiz;
    pthread_mutex_lock(&pFrameManager->mLock);
    int bFindFlag = 0;
    VO_Frame_Node *pEntry, *pTmp;
    list_for_each_entry_safe(pEntry, pTmp, &pFrameManager->mUsingList, mList)
    {
        if(pEntry->mFrame.mId == nFrameId)
        {
            list_move_tail(&pEntry->mList, &pFrameManager->mIdleList);
            bFindFlag = 1;
            break;
        }
    }
    if(0 == bFindFlag)
    {
        aloge("fatal error! frameId[%d] is not find", nFrameId);
        ret = -1;
    }
    pthread_mutex_unlock(&pFrameManager->mLock);
    return ret;
}

int initVOFrameManager(VO_Frame_Manager *pFrameManager, int nFrameNum, VO_Config *pConfigPara)
{
    memset(pFrameManager, 0, sizeof(VO_Frame_Manager));
    int err = pthread_mutex_init(&pFrameManager->mLock, NULL);
    if(err!=0)
    {
        aloge("fatal error! pthread mutex init fail!");
    }
    INIT_LIST_HEAD(&pFrameManager->mIdleList);
    INIT_LIST_HEAD(&pFrameManager->mUsingList);
    unsigned int nFrameSize;
    switch(pConfigPara->mPicFormat)
    {
        case MM_PIXEL_FORMAT_YUV_PLANAR_420:
        case MM_PIXEL_FORMAT_YVU_PLANAR_420:
        case MM_PIXEL_FORMAT_YUV_SEMIPLANAR_420:
        case MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420:
            nFrameSize = pConfigPara->mPicWidth*pConfigPara->mPicHeight*3/2;
            break;
        default:
            aloge("fatal error! unknown pixel format[0x%x]", pConfigPara->mPicFormat);
            nFrameSize = pConfigPara->mPicWidth*pConfigPara->mPicHeight;
            break;
    }

    VO_Frame_Node *pNode;
    unsigned int uPhyAddr;
    void *pVirtAddr;
    for(int i = 0; i < nFrameNum; i++)
    {
        pNode = (VO_Frame_Node*)malloc(sizeof(VO_Frame_Node));
        memset(pNode, 0, sizeof(VO_Frame_Node));
        pNode->mFrame.mId = i;
        AW_MPI_SYS_MmzAlloc_Cached(&uPhyAddr, &pVirtAddr, nFrameSize);
        pNode->mFrame.VFrame.mPhyAddr[0]    = uPhyAddr;
        pNode->mFrame.VFrame.mPhyAddr[1]    = uPhyAddr + pConfigPara->mPicWidth*pConfigPara->mPicHeight;
        pNode->mFrame.VFrame.mPhyAddr[2]    = uPhyAddr + pConfigPara->mPicWidth*pConfigPara->mPicHeight + pConfigPara->mPicWidth*pConfigPara->mPicHeight/4;
        pNode->mFrame.VFrame.mpVirAddr[0]   = pVirtAddr;
        pNode->mFrame.VFrame.mpVirAddr[1]   = pVirtAddr + pConfigPara->mPicWidth*pConfigPara->mPicHeight;
        pNode->mFrame.VFrame.mpVirAddr[2]   = pVirtAddr + pConfigPara->mPicWidth*pConfigPara->mPicHeight + pConfigPara->mPicWidth*pConfigPara->mPicHeight/4;
        pNode->mFrame.VFrame.mWidth         = pConfigPara->mPicWidth;
        pNode->mFrame.VFrame.mHeight        = pConfigPara->mPicHeight;
        pNode->mFrame.VFrame.mField         = VIDEO_FIELD_FRAME;
        pNode->mFrame.VFrame.mPixelFormat   = pConfigPara->mPicFormat;
        pNode->mFrame.VFrame.mVideoFormat   = VIDEO_FORMAT_LINEAR;
        pNode->mFrame.VFrame.mCompressMode  = COMPRESS_MODE_NONE;
        pNode->mFrame.VFrame.mOffsetTop     = 0;
        pNode->mFrame.VFrame.mOffsetBottom  = pConfigPara->mPicHeight;
        pNode->mFrame.VFrame.mOffsetLeft    = 0;
        pNode->mFrame.VFrame.mOffsetRight   = pConfigPara->mPicWidth;
        list_add_tail(&pNode->mList, &pFrameManager->mIdleList);
    }
    pFrameManager->mNodeCnt = nFrameNum;

    pFrameManager->PrefetchFirstIdleFrame = VOFrameManager_PrefetchFirstIdleFrame;
    pFrameManager->UseFrame = VOFrameManager_UseFrame;
    pFrameManager->ReleaseFrame = VOFrameManager_ReleaseFrame;
    return 0;
}

int destroyVOFrameManager(VO_Frame_Manager *pFrameManager)
{
    if(!list_empty(&pFrameManager->mUsingList))
    {
        aloge("fatal error! why using list is not empty");
    }
    int cnt = 0;
    struct list_head *pList;
    list_for_each(pList, &pFrameManager->mIdleList)
    {
        cnt++;
    }
    if(cnt != pFrameManager->mNodeCnt)
    {
        aloge("fatal error! frame count is not match [%d]!=[%d]", cnt, pFrameManager->mNodeCnt);
    }
    VO_Frame_Node *pEntry, *pTmp;
    list_for_each_entry_safe(pEntry, pTmp, &pFrameManager->mIdleList, mList)
    {
        AW_MPI_SYS_MmzFree(pEntry->mFrame.VFrame.mPhyAddr[0], pEntry->mFrame.VFrame.mpVirAddr[0]);
        list_del(&pEntry->mList);
        free(pEntry);
    }
    pthread_mutex_destroy(&pFrameManager->mLock);
    return 0;
}

