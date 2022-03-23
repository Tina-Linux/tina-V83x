/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : deinterlace.cpp
 * Description : hardware deinterlacing
 * History :
 *
 */

#include <cdx_log.h>

#include <memoryAdapter.h>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <errno.h>
#include <stdio.h>
#include "tdisp_ctrl.h"
#include <iniparserapi.h>
//-----------------------------------------------------------------------------
// relation with deinterlace

#define SAVE_DI_PICTURE 0
#define DI_MODULE_TIMEOUT    0x1055
#define    DI_IOC_MAGIC        'D'

//DI_IOSTART use int instead of pointer* in double 32/64 platform to avoid bad definition.
#define    DI_IOCSTART             _IOWR(DI_IOC_MAGIC, 0, DiRectSizeT)
//#define    DI_IOCSTART             _IOWR(DI_IOC_MAGIC, 0, DiParaT *)

typedef enum DiPixelformatE {
    DI_FORMAT_NV12      =0x00,
    DI_FORMAT_NV21      =0x01,
    DI_FORMAT_MB32_12   =0x02, //UV mapping like NV12
    DI_FORMAT_MB32_21   =0x03, //UV mapping like NV21
} DiPixelformatE;

typedef struct DiRectSizeT {
    unsigned int nWidth;
    unsigned int nHeight;
} DiRectSizeT;

typedef struct DiFbT {
#if (CONF_KERN_BITWIDE == 64)
    unsigned long long         addr[2];  // the phy address of frame buffer
#else
    uintptr_t         addr[2];  // the phy address of frame buffer
#endif
    DiRectSizeT       mRectSize;
    DiPixelformatE    eFormat;
} DiFbT;

typedef struct DiParaT {
    DiFbT         mInputFb;           //current frame fb
    DiFbT         mPreFb;          //previous frame fb
    DiRectSizeT   mSourceRegion;   //current frame and previous frame process region
    DiFbT         mOutputFb;       //output frame fb
    DiRectSizeT   mOutRegion;       //output frame region
    unsigned int  nField;          //process field <0-top field ; 1-bottom field>
    /* video infomation <0-is not top_field_first; 1-is top_field_first> */
    unsigned int  bTopFieldFirst;
} DiParaT;

struct DiContext
{
    Deinterlace base;

    int fd;
    int picCount;
    int saveCount;
#if (CONF_KERN_BITWIDE == 64)
        unsigned long long         previraddr[2];  // the vir address of prepicture buffer
        unsigned long long         curviraddr[2];  // the vir address of curpicture buffer
        unsigned long long         outviraddr[2];  // the vir address of outpicture buffer
#else
        uintptr_t         previraddr[2]; // the vir address of prepicture buffer
        uintptr_t         curviraddr[2]; // the vir address of curpicture buffer
        uintptr_t         outviraddr[2]; // the vir address of outpicture buffer
#endif
};

#if SAVE_DI_PICTURE
static int saveDePicture(Deinterlace* di,DiParaT *para){
    struct DiContext* dc;
    dc = (struct DiContext*)di;
    if(dc->saveCount < 10){
        logd("save the %d picture",dc->saveCount);
        char prePath[128];
        memset(prePath, 0, sizeof(prePath));
        sprintf(prePath,"/mnt/UDISK/pre_%d_%d_%d.yuv",(int)para->mPreFb.mRectSize.nWidth,(int)para->mPreFb.mRectSize.nHeight,dc->saveCount);
        FILE* preFp = fopen(prePath,"wb+");
        if(preFp == NULL){
            logd("open %s fail",prePath);
            return -1;
        }else{
            int preWriteRet1 = fwrite((void*)dc->previraddr[0],para->mPreFb.mRectSize.nWidth*para->mPreFb.mRectSize.nHeight,1,preFp);
            int preWriteRet2 = fwrite((void*)dc->previraddr[1],para->mPreFb.mRectSize.nWidth*para->mPreFb.mRectSize.nHeight/2,1,preFp);
            //logd("preWriteRet1 = %d,preWriteRet2 = %d",preWriteRet1,preWriteRet2);
            fclose(preFp);
        }

        char curPath[128];
        memset(curPath, 0, sizeof(curPath));
        sprintf(curPath,"/mnt/UDISK/cur_%d_%d_%d.yuv",(int)para->mInputFb.mRectSize.nWidth,(int)para->mInputFb.mRectSize.nHeight,dc->saveCount);
        FILE* curFp = fopen(curPath,"wb+");
        if(curFp == NULL){
            logd("open %s fail",curPath);
            return -1;
        }else{
            int curWriteRet1 = fwrite((void*)dc->curviraddr[0],para->mInputFb.mRectSize.nWidth*para->mInputFb.mRectSize.nHeight,1,curFp);
            int curWriteRet2 = fwrite((void*)dc->curviraddr[1],para->mInputFb.mRectSize.nWidth*para->mInputFb.mRectSize.nHeight/2,1,curFp);
            //logd("curWriteRet1 = %d,curWriteRet2 = %d",curWriteRet1,curWriteRet2);
            fclose(curFp);
        }

        char outPath[128];
        memset(outPath, 0, sizeof(outPath));
        sprintf(outPath,"/mnt/UDISK/out_%d_%d_%d.yuv",(int)para->mOutputFb.mRectSize.nWidth,(int)para->mOutputFb.mRectSize.nHeight,dc->saveCount);
        FILE* outFp = fopen(outPath,"wb+");
        if(outFp == NULL){
            logd("open %s fail",outPath);
            return -1;
        }else{
            int outWriteRet1 = fwrite((void*)dc->outviraddr[0],para->mOutputFb.mRectSize.nWidth*para->mOutputFb.mRectSize.nHeight,1,outFp);
            int outWriteRet2 = fwrite((void*)dc->outviraddr[1],para->mOutputFb.mRectSize.nWidth*para->mOutputFb.mRectSize.nHeight/2,1,outFp);
            //logd("outWriteRet1 = %d,outWriteRet2 = %d",outWriteRet1,outWriteRet2);
            fclose(outFp);
        }

        dc->saveCount++;
    }
    return 0;
}
#endif
static int dumpPara(DiParaT *para)
{
    logd("**************************************************************");
    logd("*********************deinterlace info*************************");

    logd(" input_fb: addr=0x%x, 0x%x, format=%d, size=(%d, %d)",
        para->mInputFb.addr[0], para->mInputFb.addr[1], (int)para->mInputFb.eFormat,
        (int)para->mInputFb.mRectSize.nWidth, (int)para->mInputFb.mRectSize.nHeight);
    logd("   pre_fb: addr=0x%x, 0x%x, format=%d, size=(%d, %d)",
        para->mPreFb.addr[0], para->mPreFb.addr[1], (int)para->mPreFb.eFormat,
        (int)para->mPreFb.mRectSize.nWidth, (int)para->mPreFb.mRectSize.nHeight);
    logd("output_fb: addr=0x%x, 0x%x, format=%d, size=(%d, %d)",
        para->mOutputFb.addr[0], para->mOutputFb.addr[1], (int)para->mOutputFb.eFormat,
        (int)para->mOutputFb.mRectSize.nWidth, (int)para->mOutputFb.mRectSize.nHeight);
    logd("top_field_first=%d, field=%d", para->bTopFieldFirst, para->nField);
    logd("source_regn=(%d, %d), out_regn=(%d, %d)",
        para->mSourceRegion.nWidth, para->mSourceRegion.nHeight,
        para->mOutRegion.nWidth, para->mOutRegion.nHeight);

    logd("****************************end*******************************");
    logd("**************************************************************\n\n");
    return 0;
}

static int getVeOffset(){
    const char *vd_platform;
    int ve_offset;
    vd_platform = GetConfigParamterString("platform", NULL);
    logv("platform NULL, pls check your config.");
    /*
        note:
        if(ic_version == 0x1639)
            ve_offset = 0x20000000;
        else if(ic_version <= 0x1718)
            ve_offset = 0x40000000;
         else
            ve_offset = 0;
    */
    if (strcmp(vd_platform, "r40") == 0) {//0x1701
        ve_offset = 0x40000000;
    }
    else if (strcmp(vd_platform, "r16") == 0) {//0x1667
        ve_offset = 0x40000000;
    }
    else if (strcmp(vd_platform, "r11") == 0) {//0x1681
        ve_offset = 0x40000000;
    }
    else if (strcmp(vd_platform, "r7") == 0) {//0x1681
        ve_offset = 0x40000000;
    }
    else if (strcmp(vd_platform, "r333") == 0) {//0x1681
        ve_offset = 0x40000000;
    }
    else if (strcmp(vd_platform, "331") == 0) {//0x1681
        ve_offset = 0x40000000;
    }
    else if (strcmp(vd_platform, "r7s") == 0) {//0x1681
        ve_offset = 0x40000000;
    }
    else if (strcmp(vd_platform, "332") == 0) {//0x1681
        ve_offset = 0x40000000;
    }
    else if (strcmp(vd_platform, "v306") == 0) {//0x1681
        ve_offset = 0x40000000;
    }
    else if (strcmp(vd_platform, "g102") == 0) {//0x1699,no ve
        ve_offset = 0x40000000;
    }
    else if (strcmp(vd_platform, "r58") == 0) {//0x1673
        ve_offset = 0x40000000;
    }
    else if (strcmp(vd_platform, "r6") == 0) {//0x1663
        ve_offset = 0x40000000;
    }else if (strcmp(vd_platform, "c200s") == 0) {//0x1663
        ve_offset = 0x40000000;
    }else if (strcmp(vd_platform, "r30") == 0) {//0x1719
        ve_offset = 0;
    }else if (strcmp(vd_platform, "r18") == 0) {//0x1689
        ve_offset = 0x40000000;
    }else if (strcmp(vd_platform, "r311") == 0) {//0x1755
        ve_offset = 0;
    }else if (strcmp(vd_platform, "t7") == 0) {//0x1708
        ve_offset = 0x40000000;
    }else if (strcmp(vd_platform, "mr133") == 0) {//0x1755
        ve_offset = 0;
    } else if (strcmp(vd_platform, "mr813") == 0) {//0x1855
        ve_offset = 0;
    } else if (strcmp(vd_platform, "r328s2") == 0) {//0x1821,no ve
        ve_offset = 0;
    } else if (strcmp(vd_platform, "r328s3") == 0) {//0x1821,no ve
        ve_offset = 0;
    }
    else {
        loge("invalid config '%s', pls check your cedarx.conf.",
                         vd_platform);
        ve_offset = 0;
    }
    return ve_offset;
}
static void __DiDestroy(Deinterlace* di)
{
    struct DiContext* dc;
    dc = (struct DiContext*)di;
    if (dc->fd != -1)
    {
        close(dc->fd);
    }
    free(dc);
}

static int __DiInit(Deinterlace* di)
{
    struct DiContext* dc;
    dc = (struct DiContext*)di;

    logd("%s", __FUNCTION__);
    if (dc->fd != -1)
    {
        logw("already init...");
        return 0;
    }

    dc->fd = open("/dev/deinterlace", O_RDWR);
    if (dc->fd == -1)
    {
        loge("open hw devices failure, errno(%d)", errno);
        return -1;
    }
    dc->picCount = 0;
    dc->saveCount = 0;
    logd("hw deinterlace init success...");

    return 0;
}

static int __DiReset(Deinterlace* di)
{
    struct DiContext* dc = (struct DiContext*)di;

    logd("%s", __FUNCTION__);
    if (dc->fd != -1)
    {
        close(dc->fd);
        dc->fd = -1;
    }
    return __DiInit(di);
}

static enum EPIXELFORMAT __DiExpectPixelFormat(Deinterlace* di)
{
    struct DiContext* dc = (struct DiContext*)di;
    const char *str_outputFmt = GetConfigParamterString("vd_output_fmt", NULL);
    CDX_LOG_CHECK(str_outputFmt, "'vd_output_fmt' not define, please check your cedarx.conf");
    if (strcmp(str_outputFmt, "nv21") == 0)
    {
        return PIXEL_FORMAT_NV21;
    }
    else
    {
        return PIXEL_FORMAT_NV12;
    }
}

static int __DiFlag(Deinterlace* di)
{
    struct DiContext* dc = (struct DiContext*)di;

    return DE_INTERLACE_HW;
}

static void setOutPictureInfo(VideoPicture *pOutPicture,VideoPicture *pCurPicture){
    if(pOutPicture && pCurPicture){
        pOutPicture->nStreamIndex         = pCurPicture->nStreamIndex;
        pOutPicture->ePixelFormat           = pCurPicture->ePixelFormat;
        pOutPicture->nWidth                    = pCurPicture->nWidth;
        pOutPicture->nHeight                   = pCurPicture->nHeight;
        pOutPicture->nLineStride              = pCurPicture->nLineStride;
        pOutPicture->nTopOffset              = pCurPicture->nTopOffset;
        pOutPicture->nLeftOffset              = pCurPicture->nLeftOffset;
        pOutPicture->nBottomOffset        = pCurPicture->nBottomOffset;
        pOutPicture->nRightOffset            = pCurPicture->nRightOffset;
        pOutPicture->nFrameRate            = pCurPicture->nFrameRate;
        pOutPicture->nAspectRatio           = pCurPicture->nAspectRatio;
        pOutPicture->bIsProgressive         = pCurPicture->bIsProgressive;
        pOutPicture->bTopFieldFirst          = pCurPicture->bTopFieldFirst;
        pOutPicture->bRepeatTopField      = pCurPicture->bRepeatTopField;
        pOutPicture->nPts                        = pCurPicture->nPts;
        pOutPicture->nPcr                        = pCurPicture->nPcr;
        pOutPicture->nColorPrimary          = pCurPicture->nColorPrimary;
        pOutPicture->nLower2BitBufSize    = pCurPicture->nLower2BitBufSize;
        pOutPicture->nLower2BitBufOffset  = pCurPicture->nLower2BitBufOffset;
        pOutPicture->nLower2BitBufStride  = pCurPicture->nLower2BitBufStride;
        pOutPicture->b10BitPicFlag            = pCurPicture->b10BitPicFlag;
        pOutPicture->bEnableAfbcFlag        = pCurPicture->bEnableAfbcFlag;
        pOutPicture->nBufSize                   = pCurPicture->nBufSize;
        pOutPicture->nAfbcSize                  = pCurPicture->nAfbcSize;
    }
}

static int __DiProcess(Deinterlace* di,
                    VideoPicture *pPrePicture,
                    VideoPicture *pCurPicture,
                    VideoPicture *pOutPicture,
                    int nField)
{
    struct DiContext* dc;
    dc = (struct DiContext*)di;

    logv("call DeinterlaceProcess");

    if(pPrePicture == NULL || pCurPicture == NULL || pOutPicture == NULL)
    {
        loge("the input param is null : %p, %p, %p", pPrePicture, pCurPicture, pOutPicture);
        return -1;
    }

    if(dc->fd == -1)
    {
        loge("not init...");
        return -1;
    }

    DiParaT        mDiParaT;
    DiRectSizeT    mSrcSize;
    DiRectSizeT    mDstSize;
    DiPixelformatE eInFormat;
    DiPixelformatE eOutFormat;
    /*copy some useful info from pCurPicture to pOutPicture*/
    //logd("pCurPicture->ePixelFormat = %d",pCurPicture->ePixelFormat);
    setOutPictureInfo(pOutPicture,pCurPicture);
    //* compute pts again
    if (dc->picCount < 2)
    {
        int nFrameRate = 30000; //* we set the frameRate to 30
        pOutPicture->nPts = pCurPicture->nPts + nField * (1000 * 1000 * 1000 / nFrameRate) / 2;
    }
    else
    {
        pOutPicture->nPts = pCurPicture->nPts + nField * (pCurPicture->nPts - pPrePicture->nPts)/2;
    }

    logv("pCurPicture->nPts = %lld  ms, pOutPicture->nPts = %lld ms, diff = %lld ms ",
          pCurPicture->nPts/1000,
          pOutPicture->nPts/1000,
          (pOutPicture->nPts -  pCurPicture->nPts)/1000
          );

    if (pOutPicture->ePixelFormat == PIXEL_FORMAT_NV12)
    {
        eInFormat = DI_FORMAT_NV12;
    }
    else if (pOutPicture->ePixelFormat == PIXEL_FORMAT_NV21)
    {
        eInFormat = DI_FORMAT_NV21;
    }
    else
    {
        loge("the inputPixelFormat is not support : %d",pOutPicture->ePixelFormat);
        return -1;
    }

    const char *str_difmt = GetConfigParamterString("deinterlace_fmt", NULL);
    CDX_LOG_CHECK(str_difmt, "'deinterlace_fmt' not define, pls check your cedarx.conf");
    if (strcmp(str_difmt, "mb32") == 0)
    {
        eOutFormat = DI_FORMAT_MB32_12;
    }
    else if (strcmp(str_difmt, "nv") == 0)
    {
        if (pCurPicture->ePixelFormat == PIXEL_FORMAT_NV12)
        {
            eOutFormat = DI_FORMAT_NV12;
        }
        else if (pCurPicture->ePixelFormat == PIXEL_FORMAT_NV21)
        {
            eOutFormat = DI_FORMAT_NV21;
        }
        else
        {
            loge("the outputPixelFormat is not support : %d",pCurPicture->ePixelFormat);
            return -1;
        }
    }
    else if (strcmp(str_difmt, "nv21") == 0)
    {
        eOutFormat = DI_FORMAT_NV21;
    }
    else
    {
        eOutFormat = DI_FORMAT_NV12;
    }
    //logd("eInFormat = %d,eOutFormat = %d,pCurPicture->ePixelFormat = %d",eInFormat,eOutFormat,pCurPicture->ePixelFormat);
    mSrcSize.nWidth  = pCurPicture->nWidth;
    mSrcSize.nHeight = pCurPicture->nHeight;
    mDstSize.nWidth  = pOutPicture->nLineStride;
    mDstSize.nHeight = pOutPicture->nHeight;
    mDiParaT.mInputFb.mRectSize  = mSrcSize;
    mDiParaT.mInputFb.eFormat    = eInFormat;
    mDiParaT.mPreFb.mRectSize    = mSrcSize;
    mDiParaT.mPreFb.eFormat      = eInFormat;
    mDiParaT.mOutputFb.mRectSize = mDstSize;
    mDiParaT.mOutputFb.eFormat   = eOutFormat;
    mDiParaT.mSourceRegion       = mSrcSize;
    mDiParaT.mOutRegion          = mSrcSize;
    mDiParaT.nField              = nField;
    mDiParaT.bTopFieldFirst      = pCurPicture->bTopFieldFirst;

    //* we can use the phy address
    mDiParaT.mInputFb.addr[0]    = pCurPicture->phyYBufAddr + getVeOffset();
    mDiParaT.mInputFb.addr[1]    = pCurPicture->phyCBufAddr +  getVeOffset();
    mDiParaT.mPreFb.addr[0]      = pPrePicture->phyYBufAddr + getVeOffset();
    mDiParaT.mPreFb.addr[1]      = pPrePicture->phyCBufAddr + getVeOffset();
    mDiParaT.mOutputFb.addr[0]   = pOutPicture->phyYBufAddr + getVeOffset();
    mDiParaT.mOutputFb.addr[1]   = pOutPicture->phyCBufAddr + getVeOffset();

    //save the vir address
#if SAVE_DI_PICTURE
    dc->previraddr[0]    =   pPrePicture->pData0;
    dc->previraddr[1]    =   pPrePicture->pData1;
    dc->curviraddr[0]    =   pCurPicture->pData0;
    dc->curviraddr[1]    =   pCurPicture->pData1 ;
    dc->outviraddr[0]    =   pOutPicture->pData0;
    dc->outviraddr[1]    =   pOutPicture->pData1;
#endif

    logv("VideoRender_CopyFrameToGPUBuffer aw_di_setpara start");

    //dumpPara(&mDiParaT);

    if (ioctl(dc->fd, DI_IOCSTART, &mDiParaT) != 0)
    {
        loge("aw_di_setpara failed!");
        return -1;
    }
    dc->picCount++;
    if(eOutFormat == DI_FORMAT_NV21){
        pOutPicture->ePixelFormat = PIXEL_FORMAT_NV21;
    }else if(eOutFormat == DI_FORMAT_NV12){
        pOutPicture->ePixelFormat = PIXEL_FORMAT_NV12;
    }else if(eOutFormat == DI_FORMAT_MB32_12){
        pOutPicture->ePixelFormat = PIXEL_FORMAT_YUV_MB32_420;
    }else{
        loge("the deinterlace output format can not support");
    }

#if SAVE_DI_PICTURE
    saveDePicture((Deinterlace*)dc,&mDiParaT);
#endif
    return 0;
}


static struct DeinterlaceOps mDi =
{
    .destroy           = __DiDestroy,
    .init              = __DiInit,
    .reset             = __DiReset,
    .expectPixelFormat = __DiExpectPixelFormat,
    .flag              = __DiFlag,
    .process           = __DiProcess,
};

Deinterlace* DeinterlaceCreate()
{
    struct DiContext* dc = (struct DiContext*)malloc(sizeof(struct DiContext));
    if(dc == NULL)
    {
        logw("deinterlace create failed");
        return NULL;
    }
    memset(dc, 0, sizeof(struct DiContext));

    // we must set the fd to -1; or it will close the file which fd is 0
    // when destroy
    dc->fd = -1;

    dc->base.ops = &mDi;

    return &dc->base;
}
