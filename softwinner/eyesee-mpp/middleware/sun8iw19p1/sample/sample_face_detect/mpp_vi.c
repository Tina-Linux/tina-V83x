#include "mpp_vi.h"

VIPP_Config g_VIPP_map[VIPP_NUM];

static void GetVIPPAttr(VIPP_Config* pVIPPConfig, VI_ATTR_S* pViAttr)
{
    assert(pVIPPConfig->eformat == MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420);

    memset(pViAttr, 0, sizeof(VI_ATTR_S));
    pViAttr->type               = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    pViAttr->memtype            = V4L2_MEMORY_MMAP;
    pViAttr->format.field       = V4L2_FIELD_NONE;
    pViAttr->format.width       = pVIPPConfig->width;      // 宽
    pViAttr->format.height      = pVIPPConfig->height;     // 高
    pViAttr->format.pixelformat = V4L2_PIX_FMT_NV21M;      // TODO: 目前写死nv21
    pViAttr->nbufs              = 5;
    pViAttr->nplanes            = 2;
    pViAttr->fps                = pVIPPConfig->frame_rate;
    pViAttr->capturemode        = V4L2_MODE_VIDEO;
    pViAttr->use_current_win    = 0;
    pViAttr->wdr_mode           = 0;
    pViAttr->antishake_enable   = 0;
    memset(&pViAttr->antishake_attr, 0, sizeof(ANTISHAKE_ATTR_S));
}

int create_vi(VirVi_Params* pVirViParams)
{
    for (int VirViIndex = 0; VirViIndex < VirVi_MAX; VirViIndex++)
    {
        VIPP_INDEX VIPPIndex = pVirViParams[VirViIndex].iViDev;  //TODO:type convert

        if (g_VIPP_map[VIPPIndex].bInit == 0)
        {// Create VIPP
            VI_ATTR_S stVIPP_Attr;
            GetVIPPAttr(&g_VIPP_map[VIPPIndex], &stVIPP_Attr);

            AW_MPI_VI_CreateVipp(VIPPIndex); 
            AW_MPI_VI_SetVippAttr(VIPPIndex, &stVIPP_Attr);
            AW_MPI_VI_SetVippFlip(VIPPIndex, g_VIPP_map[VIPPIndex].bFlip);
            AW_MPI_VI_SetVippMirror(VIPPIndex, g_VIPP_map[VIPPIndex].bMirror);
            AW_MPI_VI_EnableVipp(VIPPIndex);
            g_VIPP_map[VIPPIndex].bInit = 1;
        }
        else
        {
            g_VIPP_map[VIPPIndex].bInit++;
        }

        // 启动VirVi通道
        AW_MPI_VI_CreateVirChn(pVirViParams[VirViIndex].iViDev, pVirViParams[VirViIndex].iViChn, NULL);
        AW_MPI_VI_SetVirChnAttr(pVirViParams[VirViIndex].iViDev, pVirViParams[VirViIndex].iViChn, NULL);
        AW_MPI_VI_EnableVirChn(pVirViParams[VirViIndex].iViDev, pVirViParams[VirViIndex].iViChn);
    }

    // Start isp server
    AW_MPI_ISP_Init();
    AW_MPI_ISP_Run(0);

    return 0;
}

int destroy_vi(VirVi_Params* pVirViParams)
{
    // Stop isp server
    AW_MPI_ISP_Stop(0);
    AW_MPI_ISP_Exit();

    for (int VirViIndex = 0; VirViIndex < VirVi_MAX; VirViIndex++)
    {
        VIPP_INDEX VIPPIndex = pVirViParams[VirViIndex].iViDev;  //TODO:type convert

        // 销毁VirVi通道
        AW_MPI_VI_DisableVirChn(pVirViParams[VirViIndex].iViDev, pVirViParams[VirViIndex].iViChn);
        AW_MPI_VI_DestoryVirChn(pVirViParams[VirViIndex].iViDev, pVirViParams[VirViIndex].iViChn);
        
        if (g_VIPP_map[VIPPIndex].bInit == 1)
        {
            AW_MPI_VI_DisableVipp(VIPPIndex);
            AW_MPI_VI_DestoryVipp(VIPPIndex);
            g_VIPP_map[VIPPIndex].bInit = 0;
        }
        else
        {
            g_VIPP_map[VIPPIndex].bInit--;
        }
    }
    return 0;
}


void GetYUVSize(unsigned int width
               ,unsigned int height
               ,unsigned int pixelformat
               ,int* mYsize
               ,int* mUsize
               ,int* mVsize
               )
{
    alogd("GetYUVSize width[%d], height[%d], pixelformat[%d]", width, height, pixelformat);
    
    if ((pixelformat == V4L2_PIX_FMT_YUV420M) || (pixelformat == V4L2_PIX_FMT_YVU420M))
    {// yu12 && yv12
        *mYsize = width * height;
        *mVsize = width * height / 4;
        *mUsize = width * height / 4;
    }
    else if ((pixelformat == V4L2_PIX_FMT_NV12M) || (pixelformat == V4L2_PIX_FMT_NV21M))
    {// nv12 && nv21
        *mYsize = width * height;
        *mUsize = width * height / 2;
        *mVsize = 0;
    }
    else
    {// unsupported
        alogd("GetYUVSize failed! Unsupported Pixel format %d", pixelformat);
    }
}

