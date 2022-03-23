#ifndef _AW_AI_EVE_TYPE_H
#define _AW_AI_EVE_TYPE_H

#include "aw_ai_common_type.h"

#ifdef __cplusplus
extern "C"{
#endif

#define  AW_AI_EVE_MAX_CLASSIFIER_NUM		16
#define  AW_AI_EVE_MAX_RESULT_NUM			2048
#define  AW_AI_EVE_MAX_MIDRESULT_NUM		2048

enum AW_AI_EVE_ADDR_INPUT_TYPE{
	AW_AI_EVE_ADDR_INPUT_PHY,
	AW_AI_EVE_ADDR_INPUT_VIR
};
enum AW_AI_EVE_RLT_OUTPUT_TYPE{
	AW_AI_EVE_RLT_OUTPUT_DEFAULT,
	AW_AI_EVE_RLT_OUTPUT_DETAIL
};
enum AW_AI_EVE_DMA_EXECUTE_TYPE
{
	AW_AI_EVE_DMA_EXECUTE_ASYNC = 0, //�첽
	AW_AI_EVE_DMA_EXECUTE_SYNC  = 1  //ͬ��
};

typedef struct _AW_AI_EVE_DMA_CTRL_S
{
	AW_SIZE_S mDmaOut;
	AW_SIZE_S mPyramidLowestLayel;
	AW_SIZE_S dmaSrcSize;
	AW_BLOB_S dmaRoi;
	AW_SIZE_S dmaDesSize;
}AW_AI_EVE_DMA_CTRL_S;


//eve parameter
typedef struct _AW_AI_EVE_CTRL_S
{
	AW_U8 addrInputType;
	AW_U8 classifierNum;
	AW_U8 scale_factor;
	AW_U8 yStep;
	AW_U8 xStep0;
	AW_U8 xStep1;
	AW_U8 mScanStageNo;
	AW_U8 rltType;
	AW_U8 mMidRltStageNo;
	AW_U32 mMidRltNum;
	AW_U32 mRltNum;

	// Size
	AW_SIZE_S mDmaOut;
	AW_SIZE_S mPyramidLowestLayel;
	AW_SIZE_S dmaSrcSize;
	AW_BLOB_S dmaRoi;
	AW_SIZE_S dmaDesSize;

	AW_CLSPATH_S classifierPath[AW_AI_EVE_MAX_CLASSIFIER_NUM];
	void(*dmaCallBackFunc)(void* pUsr);
	void* dma_pUsr;
}AW_AI_EVE_CTRL_S;

typedef struct _AW_AI_EVE_RESULTS
{
	AW_BLOB_S* Results;
	AW_U32* rltNumArray;
	AW_U32 total_rltNum;
}AW_AI_EVE_RESULTS;

typedef struct _AW_AI_EVE_MIDRESULTS
{
	AW_BLOB_CONF_S* midResults;
	AW_U32* midRltNumArray;
	AW_U32 total_midRltNum;
}AW_AI_EVE_MIDRESULTS;


#ifdef __cplusplus
}
#endif

#endif // _AW_AI_EVE_TYPE_H
