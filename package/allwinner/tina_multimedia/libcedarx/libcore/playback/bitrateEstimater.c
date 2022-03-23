/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : bitrateEstimater.cpp
 * Description : bitrate estimater
 * History :
 *
 */

#include <string.h>
#include <stdint.h>
#include "bitrateEstimater.h"
#include "cdx_log.h"

BitrateEstimater* BitrateEstimaterCreate(void)
{
    BitrateEstimater* be;

    be = (BitrateEstimater*)malloc(sizeof(BitrateEstimater));
    if(be == NULL)
    {
        loge("malloc memory fail.");
        return NULL;
    }
    memset(be, 0, sizeof(BitrateEstimater));

    pthread_mutex_init(&be->mutex, NULL);

    return be;
}

void BitrateEstimaterDestroy(BitrateEstimater* be)
{
    pthread_mutex_destroy(&be->mutex);
    free(be);
    return;
}

void BitrateEstimaterUpdate(BitrateEstimater* be, int64_t nPts, int nFrameLen)
{
    int tmpWritePos;

    pthread_mutex_lock(&be->mutex);

    if(nPts == -1)
    {
        if(be->nValidNodeCnt == 0)
        {
            pthread_mutex_unlock(&be->mutex);
            return;
        }
        tmpWritePos = be->nWritePos;
        if(tmpWritePos == 0)
            tmpWritePos = BITRATE_ARRAY_SIZE-1;
        else
            tmpWritePos--;

        be->nodes[tmpWritePos].nFrameLen += nFrameLen;
        pthread_mutex_unlock(&be->mutex);
        return;
    }

    be->nodes[be->nWritePos].nFramePts = nPts;
    be->nodes[be->nWritePos].nFrameLen = nFrameLen;
    be->nWritePos++;
    if(be->nWritePos >= BITRATE_ARRAY_SIZE)
        be->nWritePos = 0;

    if(be->nValidNodeCnt < BITRATE_ARRAY_SIZE)
        be->nValidNodeCnt++;

    tmpWritePos = be->nWritePos;
    if(tmpWritePos < be->nWritePosLastEstimate)
        tmpWritePos += BITRATE_ARRAY_SIZE;

    if(tmpWritePos >= (be->nWritePosLastEstimate+BITRATE_ESTIMATE_INTERVAL))
    {
        int     i;
        int     j;
        int     nStartPos;
        int     nRunLen;
        int64_t nByteRate;
        int64_t nTotalSize;
        int64_t nFirstPts;
        int64_t nLastPts;
        int64_t nDuration;
        int     frameLenArr[BITRATE_ARRAY_SIZE];
        int64_t ptsArr[BITRATE_ARRAY_SIZE+1];

        nStartPos = be->nWritePos - be->nValidNodeCnt;
        if(nStartPos < 0)
            nStartPos += BITRATE_ARRAY_SIZE;

        for(i=0; i<be->nValidNodeCnt; i++)
        {
            frameLenArr[i] = be->nodes[nStartPos].nFrameLen;
            ptsArr[i]      = be->nodes[nStartPos].nFramePts;
            nStartPos++;
            if(nStartPos >= BITRATE_ARRAY_SIZE)
                nStartPos = 0;
        }

        //* find the longest pts continue frame run length.
        for(i=0; i<be->nValidNodeCnt-1; i++)
        {
            if(ptsArr[i] > (ptsArr[i+1]+PTS_DISCONTINUE_INTERVAL) ||
               (ptsArr[i]+PTS_DISCONTINUE_INTERVAL) < ptsArr[i+1])
            {
                logv("pts jump at %d, pts[%d] = %lld, pts[%d] = %lld",
                        i, i, ptsArr[i], i+1, ptsArr[i+1]);
                break;
            }
        }

        if(i == be->nValidNodeCnt-1)
        {
            //* pts normal mode.
            nFirstPts = ptsArr[0];
            nLastPts  = ptsArr[be->nValidNodeCnt-1];

            nDuration = nLastPts - nFirstPts;
            if(nDuration <= 0)
            {
//                logw("bitrate estimater get an invalid duration %lld", nDuration);
                be->nWritePosLastEstimate = be->nWritePos;
                pthread_mutex_unlock(&be->mutex);
                return;
            }

            nTotalSize = 0;
            for(j=0; j<be->nValidNodeCnt; j++)
                nTotalSize += frameLenArr[j];

            nByteRate = nTotalSize*1000000/nDuration;
            be->nBitrate = (int)(nByteRate*8);
        }
        else
        {
            //* pts loop back mode.
            j                         = 0;
            nStartPos                 = 0;
            nRunLen                   = 0;
            ptsArr[be->nValidNodeCnt] = -1;

            for(i=0; i<be->nValidNodeCnt; i++)
            {
                if(ptsArr[i] > (ptsArr[i+1] + PTS_DISCONTINUE_INTERVAL) ||
                   (ptsArr[i] + PTS_DISCONTINUE_INTERVAL) < ptsArr[i+1])
                {
                    if(i - nStartPos > nRunLen)
                    {
                        j       = nStartPos;
                        nRunLen = i - nStartPos;
                    }

                    nStartPos = i+1;
                }
            }

            if(nRunLen > 0)
            {
                nFirstPts   = ptsArr[j];
                nLastPts    = ptsArr[j+nRunLen];

                nDuration = nLastPts - nFirstPts;
                if(nDuration <= 0)
                {
                    logw("bitrate estimater get an invalid duration %lld", nDuration);
                    be->nWritePosLastEstimate = be->nWritePos;
                    pthread_mutex_unlock(&be->mutex);
                    return;
                }

                nTotalSize = 0;
                for(i=0; i<=nRunLen; i++)
                    nTotalSize += frameLenArr[j+i];

                nByteRate = nTotalSize*1000000/nDuration;
                be->nBitrate = (int)(nByteRate*8);
            }
        }

        be->nWritePosLastEstimate = be->nWritePos;
    }

    pthread_mutex_unlock(&be->mutex);
    return;
}

int BitrateEstimaterGetBitrate(BitrateEstimater* be)
{
    return be->nBitrate;
}

void BitrateEstimaterReset(BitrateEstimater* be)
{
    pthread_mutex_lock(&be->mutex);
    be->nBitrate              = 0;
    be->nWritePos             = 0;
    be->nValidNodeCnt         = 0;
    be->nWritePosLastEstimate = 0;
    pthread_mutex_unlock(&be->mutex);
    return;
}
