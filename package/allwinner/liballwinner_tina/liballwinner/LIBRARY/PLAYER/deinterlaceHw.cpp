#include <log.h>

#include <deinterlaceHw.h>
#include <memoryAdapter.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <errno.h>
//-----------------------------------------------------------------------------
// relation with deinterlace

#define DI_MODULE_TIMEOUT    0x1055
#define	DI_IOC_MAGIC		'D'
#if (DEINTERLACE_IOWR == 1)
#define	DI_IOCSTART		     _IOWR(DI_IOC_MAGIC, 0, DiRectSizeT)
#else
#define	DI_IOCSTART		     _IOWR(DI_IOC_MAGIC, 0, DiParaT *)
#endif

//-----------------------------------------------------------------------------

DeinterlaceHw::DeinterlaceHw()
{
    fd = -1;
}

DeinterlaceHw::~DeinterlaceHw()
{
	if (-1 == fd)
	{
		return ;
    }
    else
    {
	    close(fd);
	}
}

int DeinterlaceHw::init()
{
    logd("%s", __FUNCTION__);
    if (fd != -1)
    {
        logw("already init...");
        return 0;
    }

    fd = open("/dev/deinterlace", O_RDWR);
    if (fd == -1)
    {
        loge("open hw devices failure, errno(%d)", errno);
        return -1;
    }
    picCount = 0;
    logd("hw deinterlace init success...");
    return 0;
}

int DeinterlaceHw::reset()
{
    logd("%s", __FUNCTION__);
    if (fd != -1)
    {
        close(fd);
        fd = -1;
    }
    return init();
}

EPIXELFORMAT DeinterlaceHw::expectPixelFormat()
{
    return PIXEL_FORMAT_NV21;
}

int DeinterlaceHw::flag()
{
    return DE_INTERLACE_HW;
}

// need to reset deinterlace when failed
int DeinterlaceHw::para(DiParaT *para)
{
	int ret = 0;

    if (-1 == fd)
    {
        loge("not init...");
	    return -1;
	}
    ret = ioctl(fd, DI_IOCSTART, para);

    if (ret != 0) // DI_MODULE_TIMEOUT
    {
	    loge("DE-Interlace work fialed!!!\n");
	    return -1;
    }
    return 0;
}

int DeinterlaceHw::dumpPara(DiParaT *para)
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

int DeinterlaceHw::process(VideoPicture *pPrePicture,
            VideoPicture *pCurPicture,
            VideoPicture *pOutPicture,
            int nField)
{
    logv("call DeinterlaceProcess");

    if(pPrePicture == NULL || pCurPicture == NULL || pOutPicture == NULL)
    {
        loge("the input param is null : %p, %p, %p", pPrePicture, pCurPicture, pOutPicture);
        return -1;
    }

    if(fd == -1)
    {
        loge("not init...");
        return -1;
    }

    DiParaT        mDiParaT;
	DiRectSizeT    mSrcSize;
	DiRectSizeT    mDstSize;
	DiPixelformatE eInFormat;
	DiPixelformatE eOutFormat;

    //* compute pts again
    if (picCount < 2)
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
		eOutFormat = DI_FORMAT_NV12;
    }
	else if (pOutPicture->ePixelFormat == PIXEL_FORMAT_NV21)
    {
		eOutFormat = DI_FORMAT_NV21;
    }
    else
    {
        loge("the outputPixelFormat is not support : %d",pOutPicture->ePixelFormat);
        return -1;
    }

#if (DEINTERLACE_FORMAT == DEINTERLACE_FORMAT_MB32_12)
	eInFormat = DI_FORMAT_MB32_12;
#elif (DEINTERLACE_FORMAT == DEINTERLACE_FORMAT_NV)
	if (pCurPicture->ePixelFormat == PIXEL_FORMAT_NV12)
	{
		eInFormat = DI_FORMAT_NV12;
	}
	else if (pCurPicture->ePixelFormat == PIXEL_FORMAT_NV21)
	{
		eInFormat = DI_FORMAT_NV21;
	}
	else
	{
		loge("the inputPixelFormat is not support : %d",pCurPicture->ePixelFormat);
		return -1;
	}
#elif (DEINTERLACE_FORMAT == DEINTERLACE_FORMAT_NV21)
	eInFormat = DI_FORMAT_NV21;
#else
	eInFormat = DI_FORMAT_NV12;
#endif

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
    mDiParaT.mInputFb.addr[0]    = pCurPicture->phyYBufAddr + VE_PHY_OFFSET;
	mDiParaT.mInputFb.addr[1]    = pCurPicture->phyCBufAddr + VE_PHY_OFFSET;
	mDiParaT.mPreFb.addr[0]      = pPrePicture->phyYBufAddr + VE_PHY_OFFSET;
	mDiParaT.mPreFb.addr[1]      = pPrePicture->phyCBufAddr + VE_PHY_OFFSET;
	mDiParaT.mOutputFb.addr[0]   = pOutPicture->phyYBufAddr + VE_PHY_OFFSET;
	mDiParaT.mOutputFb.addr[1]   = pOutPicture->phyCBufAddr + VE_PHY_OFFSET;

	logv("VideoRender_CopyFrameToGPUBuffer aw_di_setpara start");

	//dumpPara(&mDiParaT);

	if (para(&mDiParaT) < 0)
	{
		loge("aw_di_setcardpara failed!");
        return -1;
	}
    picCount++;

    return 0;
}
