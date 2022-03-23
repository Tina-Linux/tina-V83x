
/*
* Copyright (c) 2008-2018 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : sbmFrameAvs2.c
* Description :This is the stream buffer module. The SBM provides
*              methods for managing the stream data before decode.
* History :
*   Author  : xyliu <xyliu@allwinnertech.com>
*   Date    : 2018/09/05
*   Comment :
*
*
*/


#include <stdlib.h>
#include<string.h>
#include <pthread.h>
#include "sbm.h"
#include "log.h"

#define SBM_FRAME_FIFO_SIZE (2048)  //* store 2048 frames of bitstream data at maximum.
#define MAX_INVALID_STREAM_DATA_SIZE (1*1024*1024) //* 1 MB
#define MAX_NALU_NUM_IN_FRAME (1024)
#define min(x, y)   ((x) <= (y) ? (x) : (y));
#define ADD_ONE_FRAME   (0)

extern VideoStreamDataInfo *requestStream(SbmFrame *pSbm);
extern int flushStream(SbmFrame *pSbm, VideoStreamDataInfo *pDataInfo, int bFlush);
extern int returnStream(SbmFrame* pSbm , VideoStreamDataInfo *pDataInfo);
extern int addFramePic(SbmFrame* pSbm, FramePicInfo* pFramePic); //* addFramePic
extern FramePicInfo* requestEmptyFramePic(SbmFrame* pSbm);



static inline char readByteIdx(char *p, char *pStart, char *pEnd, s32 i)
{
    logv("p = %p, start = %p, end = %p, i = %d",p,pStart, pEnd, i);
    char c = 0x0;
    if((p+i) <= pEnd)
        c = p[i];
    else
    {
        s32 d = (s32)(pEnd - p) + 1;
        c = pStart[i - d];
    }
    return c;
}

static inline void ptrPlusOne(char **p, char *pStart, char *pEnd)
{
    if((*p) == pEnd)
        (*p) = pStart;
    else
        (*p) += 1;
}

static s32 checkTmpBufferIsEnough(SbmFrame* pSbm, int nSize)
{
    if(pSbm->nTmpBufferSize < nSize)
    {
        pSbm->nTmpBufferSize += TMP_BUFFER_SIZE;

        logw("** tmp buffer is not enough, remalloc to : %d MB",pSbm->nTmpBufferSize/1024/1024);

        free(pSbm->pTmpBuffer);

        pSbm->pTmpBuffer = (char*)malloc(pSbm->nTmpBufferSize);
        if(pSbm->pTmpBuffer == NULL)
        {
            loge("*** malloc for tmp buffer failed");
            abort();
        }

        pSbm->pDetectBufStart = pSbm->pTmpBuffer;
        pSbm->pDetectBufEnd   = pSbm->pTmpBuffer + pSbm->nTmpBufferSize - 1;
    }
    return 0;
}

static s32 selectCheckBuffer(SbmFrame* pSbm,VideoStreamDataInfo *pStream, char** ppBuf)
{
    if(pSbm->mConfig.bSecureVideoFlag == 1)
    {
        checkTmpBufferIsEnough(pSbm, pStream->nLength);

        int nRingBufSize = pSbm->pStreamBufferEnd - pStream->pData + 1;

        if(pStream->nLength <= nRingBufSize)
        {
            CdcMemRead(pSbm->mConfig.memops, pSbm->pTmpBuffer, pStream->pData, pStream->nLength);
        }
        else
        {
            logw("** buffer ring, %d, %d",pStream->nLength, nRingBufSize);
            CdcMemRead(pSbm->mConfig.memops, pSbm->pTmpBuffer, pStream->pData, nRingBufSize);
            CdcMemRead(pSbm->mConfig.memops, pSbm->pTmpBuffer + nRingBufSize,
                       pSbm->pStreamBuffer, pStream->nLength - nRingBufSize);
        }
        *ppBuf   = pSbm->pTmpBuffer;
    }
    else
    {
        *ppBuf   = pStream->pData;
    }
    return 0;
}




static s32 checkBitStreamTypeWithStartCode(SbmFrame* pSbm,
                                           VideoStreamDataInfo *pStream)
{
    char *pBuf = NULL;
    char tmpBuf[4] = {0};
    const s32 nTsStreamType       = 0x000001;
    char* pStart = pSbm->pDetectBufStart;
    char* pEnd   = pSbm->pDetectBufEnd;
    s32 nHadCheckBytesLen = 0;
    s32 nCheck4BitsValue = -1;
    s32 nBufSize = pStream->nLength;
    //*1. process sbm-cycle-buffer case
    selectCheckBuffer(pSbm, pStream, &pBuf);

    while((nHadCheckBytesLen + 4) < nBufSize)
    {

        tmpBuf[0] = readByteIdx(pBuf, pStart, pEnd, nHadCheckBytesLen + 0);
        tmpBuf[1] = readByteIdx(pBuf, pStart, pEnd, nHadCheckBytesLen + 1);
        tmpBuf[2] = readByteIdx(pBuf, pStart, pEnd, nHadCheckBytesLen + 2);
        tmpBuf[3] = readByteIdx(pBuf, pStart, pEnd, nHadCheckBytesLen + 3);

        nCheck4BitsValue = (tmpBuf[0] << 24) | (tmpBuf[1] << 16) | (tmpBuf[2] << 8) | tmpBuf[3];
        if(nCheck4BitsValue == 0) //*compatible for the case: 00 00 00 00 00 00 00 01
        {
            nHadCheckBytesLen++;
            continue;
        }

        if(nCheck4BitsValue == nTsStreamType)
        {
            pSbm->bStreamWithStartCode = 1;
            return 0;
        }
        else if((nCheck4BitsValue >> 8) == nTsStreamType)
        {
            pSbm->bStreamWithStartCode = 1;
            return 0;
        }
        else
        {
            nHadCheckBytesLen += 4;
            continue;
        }
    }

    return -1;
}

s32 Avs2SbmFrameCheckBitStreamType(SbmFrame* pSbm)
{
    const s32 nUpLimitCount       = 50;
    s32 nReqeustCounter  = 0;
    s32 nRet = VDECODE_RESULT_NO_BITSTREAM;
    s32 bStartCode_with = 0;
    s32 bStartCode_without = 0;

    while(nReqeustCounter < nUpLimitCount)
    {
        VideoStreamDataInfo *pStream = NULL;
        nReqeustCounter++;
        pStream = requestStream(pSbm);
        if(pStream == NULL)
        {
            nRet = VDECODE_RESULT_NO_BITSTREAM;
            break;
        }
        if(pStream->nLength == 0 || pStream->pData == NULL)
        {
            flushStream(pSbm, pStream, 1);
            pStream = NULL;
            continue;
        }

        if(checkBitStreamTypeWithStartCode(pSbm, pStream) == 0)
        {
            bStartCode_with = 1;
        }
        else
        {
            bStartCode_with = 0;
        }

        if(bStartCode_with == 1 && bStartCode_without == 1)
        {
            pSbm->bStreamWithStartCode = 0;
        }
        else if(bStartCode_with == 1 && bStartCode_without == 0)
        {
            pSbm->bStreamWithStartCode = 1;
        }
        else if(bStartCode_with == 0 && bStartCode_without == 1)
        {
            pSbm->bStreamWithStartCode = 0;
        }
        else
        {
           pSbm->bStreamWithStartCode = -1;
        }

        logd("result: bStreamWithStartCode[%d], with[%d], whitout[%d]",
              pSbm->bStreamWithStartCode,
              bStartCode_with, bStartCode_without);

        //*continue reqeust stream from sbm when if judge the stream type
        if(pSbm->bStreamWithStartCode == -1)
        {
            flushStream(pSbm, pStream, 1);
            continue;
        }
        else
        {
            //* judge stream type successfully, return.
            returnStream(pSbm, pStream);
            nRet = 0;
            break;
        }
    }

    return nRet;
}

static void expandNaluList(FramePicInfo* pFramePic)
{
    logd("nalu num for one frame is not enought, expand it: %d, %d",
          pFramePic->nMaxNaluNum, pFramePic->nMaxNaluNum + DEFAULT_NALU_NUM);

    pFramePic->nMaxNaluNum += DEFAULT_NALU_NUM;
    pFramePic->pNaluInfoList = realloc(pFramePic->pNaluInfoList,
                                       pFramePic->nMaxNaluNum*sizeof(NaluInfo));

	logd("********here1:pFramePic->pNaluInfoList=%p\n", pFramePic->pNaluInfoList);

}

static void chooseFramePts(DetectFramePicInfo* pDetectInfo)
{
    int i;
    pDetectInfo->pCurFramePic->nPts = -1;
    for(i=0; i < MAX_FRAME_PTS_LIST_NUM; i++)
    {
        logv("*** choose pts: %lld, i = %d",pDetectInfo->nCurFramePtsList[i], i);
        if(pDetectInfo->nCurFramePtsList[i] != -1)
        {
            pDetectInfo->pCurFramePic->nPts = pDetectInfo->nCurFramePtsList[i];
            break;
        }
    }
}

static void initFramePicInfo(DetectFramePicInfo* pDetectInfo)
{
    FramePicInfo* pFramePic = pDetectInfo->pCurFramePic;
    pFramePic->bValidFlag = 1;
    pFramePic->nlength = 0;
    pFramePic->pDataStartAddr = NULL;
    pFramePic->nPts = -1;
    pFramePic->nPcr = -1;
    pFramePic->nCurNaluIdx = 0;

    int i;
    for(i = 0; i < MAX_FRAME_PTS_LIST_NUM; i++)
        pDetectInfo->nCurFramePtsList[i] = -1;

    if(pFramePic->nMaxNaluNum > DEFAULT_NALU_NUM)
    {
        pFramePic->nMaxNaluNum   = DEFAULT_NALU_NUM;
        pFramePic->pNaluInfoList = realloc(pFramePic->pNaluInfoList,
                                           pFramePic->nMaxNaluNum*sizeof(NaluInfo));

		logd("********here2:pFramePic->pNaluInfoList=%p\n", pFramePic->pNaluInfoList);
    }

    memset(pFramePic->pNaluInfoList, 0, pFramePic->nMaxNaluNum*sizeof(NaluInfo));

}

static int searchStartCode(SbmFrame* pSbm, int* pAfterStartCodeIdx)
{
    char* pStart = pSbm->pDetectBufStart;
    char* pEnd   = pSbm->pDetectBufEnd;

    DetectFramePicInfo* pDetectInfo = &pSbm->mDetectInfo;

    char *pBuf = pDetectInfo->pCurStreamDataptr;
    s32 nSize = pDetectInfo->nCurStreamDataSize - 3;

    if(pDetectInfo->nCurStreamRebackFlag)
    {
        logv("bHasTwoDataTrunk pSbmBuf: %p, pSbmBufEnd: %p, curr: %p, diff: %d ",
                pStart, pEnd, pBuf, (u32)(pEnd - pBuf));
        while(nSize > 0)
        {
            char tmpBuf[4];
            tmpBuf[0] = readByteIdx(pBuf , pStart, pEnd, 0);
            tmpBuf[1] = readByteIdx(pBuf , pStart, pEnd, 1);
            tmpBuf[2] = readByteIdx(pBuf , pStart, pEnd, 2);
            tmpBuf[3] = readByteIdx(pBuf , pStart, pEnd, 3);
            if(tmpBuf[0] == 0 && tmpBuf[1] == 0 && tmpBuf[2] == 1)
            {
                if((pBuf[3]==0xB0)||(pBuf[3]==0xB1)||(pBuf[3]==0xB2)||(pBuf[3]==0xB3)||
                    (pBuf[3]==0xB5)||(pBuf[3]==0xB6)||(pBuf[3]==0xB7)/*||(pBuf[3]==0x00)*/)
                {

                    (*pAfterStartCodeIdx) += 3; //so that buf[0] is the actual data, not start code
                    return 0;
                }
            }
            ptrPlusOne(&pBuf, pStart, pEnd);
            ++(*pAfterStartCodeIdx);
            --nSize;
        }
    }
    else
    {
        while(nSize > 0)
        {
            if(pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 1)
            {
                if((pBuf[3]==0xB0)||(pBuf[3]==0xB1)||(pBuf[3]==0xB2)||(pBuf[3]==0xB3)||
                    (pBuf[3]==0xB5)||(pBuf[3]==0xB6)||(pBuf[3]==0xB7)/*||(pBuf[3]==0x01)*/)
                {
                    (*pAfterStartCodeIdx) += 3; //so that buf[0] is the actual data, not start code
                    return 0;
                 }
            }
            ++pBuf;
            ++(*pAfterStartCodeIdx);
            --nSize;
        }
    }
    return -1;

}

static inline int supplyStreamData(SbmFrame* pSbm)
{
    DetectFramePicInfo* pDetectInfo = &pSbm->mDetectInfo;

    if(pDetectInfo->pCurStream)
    {
        flushStream(pSbm, pDetectInfo->pCurStream, 0);
        pDetectInfo->pCurStream = NULL;
        if(pDetectInfo->pCurFramePic)
        {
           pDetectInfo->pCurFramePic->nlength += pDetectInfo->nCurStreamDataSize;
        }
        pDetectInfo->nCurStreamDataSize = 0;
        pDetectInfo->pCurStreamDataptr  = NULL;
    }

    VideoStreamDataInfo *pStream = requestStream(pSbm);
    if(pStream == NULL)
    {
        SemTimedWait(&pSbm->streamDataSem, 20);
        return -1;
    }

    if(pStream->nLength <= 0)
    {
        flushStream(pSbm, pStream, 0);
        return -1;
    }
    pDetectInfo->pCurStream = pStream;
    pDetectInfo->nCurStreamDataSize = pDetectInfo->pCurStream->nLength;

    selectCheckBuffer(pSbm, pStream, &pDetectInfo->pCurStreamDataptr);

    pDetectInfo->nCurStreamRebackFlag = 0;
    if(pSbm->mConfig.bSecureVideoFlag == 0)
    {
       int nRingBufSize = pSbm->pStreamBufferEnd - pDetectInfo->pCurStream->pData + 1;
       if(pDetectInfo->pCurStream->nLength > nRingBufSize)
           pDetectInfo->nCurStreamRebackFlag = 1;
    }
    return 0;

}

static void disposeInvalidStreamData(SbmFrame* pSbm)
{
    DetectFramePicInfo* pDetectInfo = &pSbm->mDetectInfo;

    int bNeedAddFramePic = 0;
    char* pStartBuf = NULL;

    if(pSbm->mConfig.bSecureVideoFlag == 1)
    {
        pStartBuf = pSbm->pTmpBuffer;
    }
    else
    {
        pStartBuf = pDetectInfo->pCurStream->pData;
    }

    logd("**1 pCurFramePic->nlength = %d, flag = %d",pDetectInfo->pCurFramePic->nlength,
         (pDetectInfo->pCurStreamDataptr == pStartBuf));

    if(pDetectInfo->pCurStreamDataptr == pStartBuf
       && pDetectInfo->pCurFramePic->nlength == 0)
    {
        pDetectInfo->pCurFramePic->pDataStartAddr = pDetectInfo->pCurStream->pData;
        pDetectInfo->pCurFramePic->nlength = pDetectInfo->pCurStream->nLength;
        pDetectInfo->pCurFramePic->bValidFlag = 0;
        bNeedAddFramePic = 1;
    }
    else
    {
        pDetectInfo->pCurFramePic->nlength += pDetectInfo->nCurStreamDataSize;
        logd("**2, pCurFramePic->nlength = %d, diff = %d",pDetectInfo->pCurFramePic->nlength,
             pDetectInfo->pCurFramePic->nlength - MAX_INVALID_STREAM_DATA_SIZE);

        if(pDetectInfo->pCurFramePic->nlength > MAX_INVALID_STREAM_DATA_SIZE)
        {
            pDetectInfo->pCurFramePic->bValidFlag = 0;
            bNeedAddFramePic = 1;
        }
    }

    logd("disposeInvalidStreamData: bNeedAddFramePic = %d",bNeedAddFramePic);
    flushStream(pSbm, pDetectInfo->pCurStream, 0);
    pDetectInfo->pCurStream = NULL;
    pDetectInfo->pCurStreamDataptr = NULL;
    pDetectInfo->nCurStreamDataSize = 0;

    if(bNeedAddFramePic)
    {
        addFramePic(pSbm, pDetectInfo->pCurFramePic);
        pDetectInfo->pCurFramePic = NULL;
    }

}

static inline void skipCurStreamDataBytes(SbmFrame* pSbm, int nSkipSize)
{
    char* pStart = pSbm->pDetectBufStart;
    char* pEnd   = pSbm->pDetectBufEnd;
    DetectFramePicInfo* pDetectInfo = &pSbm->mDetectInfo;

    pDetectInfo->pCurStreamDataptr += nSkipSize;
    pDetectInfo->nCurStreamDataSize -= nSkipSize;
    if(pDetectInfo->pCurStreamDataptr > pEnd)
    {
        pDetectInfo->pCurStreamDataptr = pStart + (pDetectInfo->pCurStreamDataptr - pEnd - 1);
    }
    pDetectInfo->pCurFramePic->nlength += nSkipSize;

}

static inline void storeNaluInfo(SbmFrame* pSbm, int nNaluType,
                                    int nNalRefIdc, int nNaluSize, char* pNaluBuf)
{
    DetectFramePicInfo* pDetectInfo = &pSbm->mDetectInfo;

    int nNaluIdx = pDetectInfo->pCurFramePic->nCurNaluIdx;
    NaluInfo* pNaluInfo = &pDetectInfo->pCurFramePic->pNaluInfoList[nNaluIdx];
    logv("*** nNaluIdx = %d, pts = %lld, naluSize = %d, nalutype = %d",
          nNaluIdx, pDetectInfo->pCurStream->nPts, nNaluSize, nNaluType);
    if(nNaluIdx < MAX_FRAME_PTS_LIST_NUM)
        pDetectInfo->nCurFramePtsList[nNaluIdx] = pDetectInfo->pCurStream->nPts;

    pNaluInfo->nNalRefIdc = nNalRefIdc;
    pNaluInfo->nType = nNaluType;
    pNaluInfo->pDataBuf = pNaluBuf;
    pNaluInfo->nDataSize = nNaluSize;
    pDetectInfo->pCurFramePic->nCurNaluIdx++;
    if(pDetectInfo->pCurFramePic->nCurNaluIdx >= pDetectInfo->pCurFramePic->nMaxNaluNum)
    {
        expandNaluList(pDetectInfo->pCurFramePic);
    }
    if(pSbm->mDetectInfo.pCurStream->bVideoInfoFlag == 1)
    {
        pDetectInfo->pCurFramePic->bVideoInfoFlag = 1;
        pSbm->mDetectInfo.pCurStream->bVideoInfoFlag = 0;
    }
}

static char* checkCurStreamDataAddr(SbmFrame* pSbm)
{
    DetectFramePicInfo* pDetectInfo = &pSbm->mDetectInfo;
    char* pCurStreamDataAddr = NULL;

    int nSize = pDetectInfo->pCurStreamDataptr - pSbm->pDetectBufStart;
    int nRingBufSize = pSbm->pStreamBufferEnd - pDetectInfo->pCurStream->pData + 1;

    if(nSize >= nRingBufSize)
    {
        logd("*** buffer ring ");
        pCurStreamDataAddr = pSbm->pStreamBuffer + (nSize - nRingBufSize);
    }
    else
        pCurStreamDataAddr = pDetectInfo->pCurStream->pData + nSize;

    return pCurStreamDataAddr;
}

/*
detect step:
    1. request bit stream
    2. find startCode
    3.  read the naluType and bFirstSliceSegment
    4. skip nAfterStartCodeIdx
    5. find the next startCode to determine size of cur nalu
    6. store  nalu info
    7. skip naluSize bytes
*/
static void detectWithStartCode(SbmFrame* pSbm)
{
    char tmpBuf[6] = {0};
    char* pStart = pSbm->pDetectBufStart;
    char* pEnd   = pSbm->pDetectBufEnd;
#if ADD_ONE_FRAME
    int   bFirstSliceSegment = 0;
#endif
    char* pAfterStartCodeBuf = NULL;
    DetectFramePicInfo* pDetectInfo = &pSbm->mDetectInfo;

    if(pDetectInfo->pCurFramePic == NULL)
    {
       pDetectInfo->pCurFramePic = requestEmptyFramePic(pSbm);
       if(pDetectInfo->pCurFramePic == NULL)
       {
            SemTimedWait(&pSbm->emptyFramePicSem, 20);
            return ;
       }
       initFramePicInfo(pDetectInfo);
    }

    while(1)
    {
        int   nRet = -1;

        //*1. request bit stream
        if(pDetectInfo->nCurStreamDataSize < 5 || pDetectInfo->pCurStreamDataptr == NULL)
        {
            if(supplyStreamData(pSbm) != 0)
            {
                if(pDetectInfo->bCurFrameStartCodeFound == 1 && pSbm->nEosFlag == 1)
                {
                    pDetectInfo->bCurFrameStartCodeFound = 0;
                    chooseFramePts(pDetectInfo);
                    addFramePic(pSbm, pDetectInfo->pCurFramePic);
                    logd("find eos, flush last stream frame, pts = %lld",
                          (long long int)pDetectInfo->pCurFramePic->nPts);
                    pDetectInfo->pCurFramePic = NULL;
                }
                return ;
            }
        }

        logv("*** new new pSbm->mConfig.bSecureVideoFlag = %d",pSbm->mConfig.bSecureVideoFlag);
        if(pDetectInfo->pCurFramePic->pDataStartAddr == NULL)
        {
           if(pSbm->mConfig.bSecureVideoFlag == 1)
           {
                pDetectInfo->pCurFramePic->pDataStartAddr = checkCurStreamDataAddr(pSbm);
           }
           else
           {
                pDetectInfo->pCurFramePic->pDataStartAddr = pDetectInfo->pCurStreamDataptr;
           }
           pDetectInfo->pCurFramePic->pVideoInfo = pDetectInfo->pCurStream->pVideoInfo;
        }

        //*2. find startCode
        int nAfterStartCodeIdx = 0;
        nRet = searchStartCode(pSbm,&nAfterStartCodeIdx);
        if(nRet != 0 //*  can not find startCode
           || pDetectInfo->pCurFramePic->nCurNaluIdx > MAX_NALU_NUM_IN_FRAME)
        {
            logw("can not find startCode, curNaluIdx = %d, max = %d",
                  pDetectInfo->pCurFramePic->nCurNaluIdx, MAX_NALU_NUM_IN_FRAME);
            disposeInvalidStreamData(pSbm);
            return ;
        }

        //* now had find the startCode
        //*3.  read the naluType and bFirstSliceSegment

        pAfterStartCodeBuf = pDetectInfo->pCurStreamDataptr + nAfterStartCodeIdx;
        tmpBuf[0] = readByteIdx(pAfterStartCodeBuf ,pStart, pEnd, 0);

        int nNaluType = tmpBuf[0];
        char* pSliceStartPlusOne = pAfterStartCodeBuf + 1;

        if(pSliceStartPlusOne > pEnd)
            pSliceStartPlusOne -= (pEnd - pStart + 1);

        if(pDetectInfo->bCurFrameStartCodeFound == 0)
        {
            pDetectInfo->bCurFrameStartCodeFound = 1;
            pDetectInfo->pCurFramePic->nFrameNaluType = nNaluType;
        }
        else
        {
            logv("**** have found one frame pic ****");
            pDetectInfo->bCurFrameStartCodeFound = 0;
            chooseFramePts(pDetectInfo);
            addFramePic(pSbm, pDetectInfo->pCurFramePic);
            pDetectInfo->pCurFramePic = NULL;
            return ;
        }

        //*if code run here, it means that this is the normal nalu of new frame, we should store it
        //*4. skip nAfterStartCodeIdx
        skipCurStreamDataBytes(pSbm, nAfterStartCodeIdx);

        //*5. find the next startCode to determine size of cur nalu
        int nNaluSize = 0;
        nAfterStartCodeIdx = 0;
        nRet = searchStartCode(pSbm,&nAfterStartCodeIdx);
        if(nRet != 0)//* can not find next startCode
        {
            nNaluSize = pDetectInfo->nCurStreamDataSize;
        }
        else
        {
            nNaluSize = nAfterStartCodeIdx - 3; //* 3 is the length of startCode

        }

        logv("gqy***nNaluType =%d, store nalu size = %d\n",
               nNaluType, nNaluSize);
        //*6. store  nalu info
        char* pNaluBuf = NULL;
        if(pSbm->mConfig.bSecureVideoFlag == 1)
        {
            pNaluBuf = checkCurStreamDataAddr(pSbm);
        }
        else
            pNaluBuf = pDetectInfo->pCurStreamDataptr;

        storeNaluInfo(pSbm, nNaluType, 0, nNaluSize, pNaluBuf);

        //*7. skip naluSize bytes
        skipCurStreamDataBytes(pSbm, nNaluSize);
    }

    return ;
}


void Avs2SbmFrameDetectOneFramePic(SbmFrame* pSbm)
{
    detectWithStartCode(pSbm);
}
