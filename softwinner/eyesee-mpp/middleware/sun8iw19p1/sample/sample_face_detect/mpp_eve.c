#include "mpp_eve.h"

pthread_mutex_t          g_stResult_lock = PTHREAD_MUTEX_INITIALIZER;
AW_AI_EVE_EVENT_RESULT_S g_stResult;
int                      g_eve_ready = 0;

// EVE 回调函数
static void DmaCallBackFunc(void* pUsr)
{
    // printf("======= DmaCallBackFunc\n");
}

int create_eve(EVE_Params*     pEVEParams)
{
	int ret = -1;

	// 查询EVE算法版本
    AW_S8 cVersion[128];
    ///-AW_AI_EVE_Event_GetAlgoVersion(cVersion);
    alogd("EVE version is %s!\n", cVersion);

    // 算法基本参数
    AW_AI_EVE_CTRL_S sEVECtrl;
    sEVECtrl.addrInputType  = AW_AI_EVE_ADDR_INPUT_VIR; // for malloc, we use virtual address，算法库可以用AW_AI_EVE_ADDR_INPUT_PHY标志来使用物理地址
    sEVECtrl.scale_factor   = 1;
    sEVECtrl.mScanStageNo   = 10;
    sEVECtrl.yStep          = 4;
    sEVECtrl.xStep0         = 1;
    sEVECtrl.xStep1         = 3;
    sEVECtrl.mRltNum        = AW_AI_EVE_MAX_RESULT_NUM;
    sEVECtrl.mMidRltNum     = 0;
    sEVECtrl.mMidRltStageNo = 10;
    sEVECtrl.rltType        = AW_AI_EVE_RLT_OUTPUT_DETAIL;

    // DMA参数
    sEVECtrl.mDmaOut.s16Width              = pEVEParams->iPicWidth;
    sEVECtrl.mDmaOut.s16Height             = pEVEParams->iPicHeight;
    sEVECtrl.mPyramidLowestLayel.s16Width  = pEVEParams->iPicWidth;
    sEVECtrl.mPyramidLowestLayel.s16Height = pEVEParams->iPicHeight;
    sEVECtrl.dmaSrcSize.s16Width           = pEVEParams->iPicWidth;
    sEVECtrl.dmaSrcSize.s16Height          = pEVEParams->iPicHeight;
    sEVECtrl.dmaDesSize.s16Width           = sEVECtrl.dmaSrcSize.s16Width;
    sEVECtrl.dmaDesSize.s16Height          = sEVECtrl.dmaSrcSize.s16Height; 
    sEVECtrl.dmaRoi.s16X                   = 0;
    sEVECtrl.dmaRoi.s16Y                   = 0;
    sEVECtrl.dmaRoi.s16Width               = sEVECtrl.dmaDesSize.s16Width;
    sEVECtrl.dmaRoi.s16Height              = sEVECtrl.dmaDesSize.s16Height;
    
    // 算法模型
    AW_S8 *awKeyNew = "1111111111111111"; //key
    sEVECtrl.classifierNum = 8;
    sEVECtrl.classifierPath[0].path = (AW_S8*)"./classifier/frontface.ld";
    sEVECtrl.classifierPath[0].key  = (AW_U8*)awKeyNew;
    sEVECtrl.classifierPath[1].path = (AW_S8*)"./classifier/fullprofleftface.ld";
    sEVECtrl.classifierPath[1].key  = (AW_U8*)awKeyNew;
    sEVECtrl.classifierPath[2].path = (AW_S8*)"./classifier/fullprofrightface.ld";
    sEVECtrl.classifierPath[2].key  = (AW_U8*)awKeyNew;
    sEVECtrl.classifierPath[3].path = (AW_S8*)"./classifier/halfdownface.ld";
    sEVECtrl.classifierPath[3].key  = (AW_U8*)awKeyNew;
    sEVECtrl.classifierPath[4].path = (AW_S8*)"./classifier/profileface.ld";
    sEVECtrl.classifierPath[4].key  = (AW_U8*)awKeyNew;
    sEVECtrl.classifierPath[5].path = (AW_S8*)"./classifier/rotleftface.ld";
    sEVECtrl.classifierPath[5].key  = (AW_U8*)awKeyNew;
    sEVECtrl.classifierPath[6].path = (AW_S8*)"./classifier/rotrightface.ld";
    sEVECtrl.classifierPath[6].key  = (AW_U8*)awKeyNew;
    sEVECtrl.classifierPath[7].path = (AW_S8*)"./classifier/smallface.ld";
    sEVECtrl.classifierPath[7].key  = (AW_U8*)awKeyNew;

    // 回调函数
    sEVECtrl.dmaCallBackFunc = &DmaCallBackFunc;
    sEVECtrl.dma_pUsr        = &pEVEParams;
    
    // 创建算法模块并初始化
    ///-pEVEParams->pEVEHd = AW_AI_EVE_Event_Init(&sEVECtrl);
    ///-if (AW_NULL == pEVEParams->pEVEHd)
    ///-{
    ///-    aloge("AW_AI_EVE_Event_Init failure!\n");
    ///-    abort();
    ///-}
	
	// 准备YUV数据内存
    // Notes: 此操作必须在AW_AI_EVE_Event_Init之后调用
    int nFrameSize = pEVEParams->iPicWidth * pEVEParams->iPicHeight * 3 / 2;
    int nPixelSize = pEVEParams->iPicWidth * pEVEParams->iPicHeight;
    
    unsigned int uPhyAddr;
    void *pVirtAddr;
    AW_MPI_SYS_MmzAlloc_Cached(&uPhyAddr, &pVirtAddr, nFrameSize);
	
	pEVEParams->pImage = (AW_IMAGE_S*)malloc(sizeof(AW_IMAGE_S));
    pEVEParams->pImage->mpVirAddr[0] = pVirtAddr;
    pEVEParams->pImage->mpVirAddr[1] = pVirtAddr + nPixelSize;
    pEVEParams->pImage->mpVirAddr[2] = NULL;//pVirtAddr + nPixelSize + nPixelSize / 4;
	
	// 设置EVE内核DMA搬运与EVE算子的执行顺序
    ///-AW_AI_EVE_Event_SetEveDMAExecuteMode(pEVEParams->pEVEHd, AW_AI_EVE_DMA_EXECUTE_SYNC);
	
	// 设置人脸检测参数
    AW_AI_EVE_EVENT_FACEDET_PARAM_S stFaceDetParam;
    stFaceDetParam.sRoiSet.s32RoiNum         = 1;
    stFaceDetParam.sRoiSet.sID[0]            = 1;
    stFaceDetParam.sRoiSet.sRoi[0].s16Top    = 0;
    stFaceDetParam.sRoiSet.sRoi[0].s16Bottom = pEVEParams->iPicHeight;
    stFaceDetParam.sRoiSet.sRoi[0].s16Left   = 0;
    stFaceDetParam.sRoiSet.sRoi[0].s16Right  = pEVEParams->iPicWidth;
    stFaceDetParam.s32ClassifyFlag           = 0; //close
    stFaceDetParam.s32MaxFaceNum             = 128;
    stFaceDetParam.s32MinFaceSize            = 20;
    stFaceDetParam.s32OverLapCoeff           = 20;
    stFaceDetParam.s32MergeThreshold         = 3;
    stFaceDetParam.s8Cfgfile                 = AW_NULL;
    stFaceDetParam.s8Weightfile              = AW_NULL;

    ///-ret = AW_AI_EVE_Event_SetEventParam(pEVEParams->pEVEHd, AW_AI_EVE_EVENT_FACEDETECT, &stFaceDetParam);
    ///-if (AW_STATUS_ERROR == ret)
    ///-{
    ///-    aloge("AW_AI_EVE_Event_SetEventParam failure!, errorcode = %d\n", AW_AI_EVE_Event_GetLastError(pEVEParams->pEVEHd));
    ///-    abort();
    ///-}

	return 0;
}

int destroy_eve(EVE_Params*     pEVEParams)
{
	AW_MPI_SYS_MmzFree(pEVEParams->pImage->mPhyAddr[0], pEVEParams->pImage->mpVirAddr[0]);
	///-AW_AI_EVE_Event_UnInit(pEVEParams->pEVEHd);
	
	return 0;
}

void ShowFaceDetectResult(int iFrameIndex, AW_AI_EVE_EVENT_RESULT_S* pstEVEResult, int bShowDetail)
{
    char pcTmp[1024];
    if (pstEVEResult->sTarget.s32TargetNum >= 0) 
    {
        printf("\033[32m");
        printf(">>>>>>>find [%d] faces in frame %d<<<<<<<\n", pstEVEResult->sTarget.s32TargetNum, iFrameIndex);
        printf("\033[0m");
        fflush(stdout);
    }

    if (bShowDetail)
    {
        for (int i = 0; i < pstEVEResult->sTarget.s32TargetNum; i++) 
        {
            printf("\033[33m");
            printf("=================================Get one target============================\n");
            printf("the frame index is [%d]\n", iFrameIndex);
            printf("the target ID is %u\n", pstEVEResult->sTarget.astTargets[i].u32ID);
            printf("the target type is %u\n", pstEVEResult->sTarget.astTargets[i].u32Type);
            printf("the target point is [%d, %d]\n", pstEVEResult->sTarget.astTargets[i].stPoint.s16X,
                pstEVEResult->sTarget.astTargets[i].stPoint.s16Y);
            printf("the target rect is [%d, %d, %d, %d]\n", 
                pstEVEResult->sTarget.astTargets[i].stRect.s16Left, 
                pstEVEResult->sTarget.astTargets[i].stRect.s16Top, 
                pstEVEResult->sTarget.astTargets[i].stRect.s16Right, 
                pstEVEResult->sTarget.astTargets[i].stRect.s16Bottom);
            printf("===========================================================================\n");
            printf("\033[0m");
            fflush(stdout); // flush stdout, resume console's output format
        }
    }
}

