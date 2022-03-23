/*
 * =====================================================================================
 *
 *      Copyright (c) 2008-2018 Allwinner Technology Co. Ltd.
 *      All rights reserved.
 *
 *       Filename:  omx_venc_adapter.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2018/08
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  SWC-PPD
 *      Company: Allwinner Technology Co. Ltd.
 *
 *       History :
 *        Author : AL3
 *        Date   : 2013/05/05
 *    Comment : complete the design code
 *
 * =====================================================================================
 */
#ifndef __OMX_VENC_ADAPTER__
#define __OMX_VENC_ADAPTER__
#ifdef __cplusplus
extern "C"{
#endif
#include "omx_venc.h"
#include "aw_omx_component.h"
#include "omx_common.h"
#define SAVE_BITSTREAM (0)
#define OPEN_STATISTICS (0)
#define PRINTF_FRAME_SIZE (0)
#define SAVE_INPUTDATA (0)

OMX_ERRORTYPE getDefaultParameter(AwOmxVenc* impl, OMX_IN OMX_INDEXTYPE eParamIndex,
                                           OMX_IN OMX_PTR pParamData);

OMX_ERRORTYPE setDefaultParameter(AwOmxVenc* impl, OMX_IN OMX_INDEXTYPE eParamIndex,
                                          OMX_IN OMX_PTR pParamData);
void getAndAddInputBuffer(AwOmxVenc* impl, OMX_BUFFERHEADERTYPE* pInBufHdr,
                                      VencInputBuffer* sInputBuffer);

void getInputBufferFromBufHdr(AwOmxVenc* impl, OMX_BUFFERHEADERTYPE* pInBufHdr,
                                        VencInputBuffer* sInputBuffer);

void getInputBufferFromBufHdrWithoutCrop (AwOmxVenc* impl, OMX_BUFFERHEADERTYPE* pInBufHdr,
                                        VencInputBuffer* sInputBuffer);

void determineVencColorFormat(AwOmxVenc* impl);

void determineVencBufStrideIfNecessary(AwOmxVenc* impl, VencBaseConfig* pBaseConfig);

int deparseEncInputBuffer(int nIonFd, VideoEncoder *pEncoder, OMXInputBufferInfoT *pInputBufInfo);

void updateOmxDebugFlag(AwOmxVenc* impl);

#ifdef __cplusplus
}
#endif
#endif
