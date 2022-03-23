/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : framerateEstimater.cpp
 * Description : framerate estimater
 * History :
 *
 */

#include <string.h>
#include <stdint.h>
#include "framerateEstimater.h"
#include "cdx_log.h"

FramerateEstimater* FramerateEstimaterCreate(void)
{
    FramerateEstimater* fe;

    fe = (FramerateEstimater*)malloc(sizeof(FramerateEstimater));
    if(fe == NULL)
    {
        loge("malloc memory fail.");
        return NULL;
    }
    memset(fe, 0, sizeof(FramerateEstimater));
    fe->fPlayRate = 1.0;

    pthread_mutex_init(&fe->mutex, NULL);

    return fe;
}

void FramerateEstimaterDestroy(FramerateEstimater* fe)
{
    pthread_mutex_destroy(&fe->mutex);
    free(fe);
    return;
}

void FramerateEstimaterUpdate(FramerateEstimater* fe, int64_t nPts)
{
    int     tmpWritePos;
    int     i;
    int     k, n, tmp;  //* for sorting.
    int     nValidDurationCnt;
    int     nStartPos;
    int64_t nPtsArr[FRAMERATE_ARRAY_SIZE];
    int64_t nDurationArr[FRAMERATE_ARRAY_SIZE-1];
    int64_t nDurationArrAvg[FRAMERATE_ARRAY_SIZE-2];

    pthread_mutex_lock(&fe->mutex);

    fe->nPts[fe->nWritePos] = nPts;
    fe->nWritePos++;
    if(fe->nWritePos >= FRAMERATE_ARRAY_SIZE)
        fe->nWritePos = 0;

    if(fe->nValidPtsCnt < FRAMERATE_ARRAY_SIZE)
        fe->nValidPtsCnt++;

    tmpWritePos = fe->nWritePos;

    if(fe->nValidPtsCnt >= FRAMERATE_START_ESTIMATE_SIZE)
    {
        nStartPos = fe->nWritePos - fe->nValidPtsCnt;
        if(nStartPos < 0)
            nStartPos += FRAMERATE_ARRAY_SIZE;

        for(i=0; i<fe->nValidPtsCnt; i++)
        {
            nPtsArr[i] = fe->nPts[nStartPos];
            nStartPos++;
            if(nStartPos >= FRAMERATE_ARRAY_SIZE)
                nStartPos = 0;
        }

        nValidDurationCnt = fe->nValidPtsCnt-1;
        for(i=0; i<nValidDurationCnt; i++)
        {
            nDurationArr[i] = nPtsArr[i+1] - nPtsArr[i];
        }
        for(i=0; i<nValidDurationCnt-1; i++)
        {
            nDurationArrAvg[i] = (nDurationArr[i] + nDurationArr[i+1])/2;
        }
        //* bubble sort.
        for(k=0; k<nValidDurationCnt-2; k++)
        {
            for(n=0; n<nValidDurationCnt-2-k; n++)
            {
                if(nDurationArrAvg[n]>nDurationArrAvg[n+1])
                {
                    tmp               = nDurationArrAvg[n];
                    nDurationArrAvg[n]   = nDurationArrAvg[n+1];
                    nDurationArrAvg[n+1] = tmp;
                }
            }
        }

        fe->nFrameDuration = (int)nDurationArrAvg[(nValidDurationCnt-1)/2];
    }

    pthread_mutex_unlock(&fe->mutex);
    return;
}

int FramerateEstimaterGetFramerate(FramerateEstimater* fe)
{
    int nFramerate;
    int nFrameDuration;

    nFrameDuration = fe->nFrameDuration;
    if(nFrameDuration <= 0)
        return 0;

    nFramerate = 1000*1000*1000/nFrameDuration;

    if(nFramerate < 5000)
        nFramerate = 24000; //* 24 fps.

    nFramerate = nFramerate / fe->fPlayRate;

    return nFramerate;
}

int FramerateEstimaterGetFrameDuration(FramerateEstimater* fe)
{
    int nFrameDuration;
    nFrameDuration = fe->nFrameDuration;
    //* fps lower than 5 fps or higher than 60 fps is invalid.
    if(nFrameDuration < 16666 || nFrameDuration > 200000)
        nFrameDuration = 41667; //* 24 fps.
    nFrameDuration = nFrameDuration/fe->fPlayRate;
    return nFrameDuration;
}

int FramerateEstimaterSetPlayrate(FramerateEstimater* fe,float rate)
{
    fe->fPlayRate = rate;
    return 0;
}

void FramerateEstimaterReset(FramerateEstimater* fe)
{
    pthread_mutex_lock(&fe->mutex);
    fe->nFrameDuration = 0;
    fe->nWritePos      = 0;
    fe->nValidPtsCnt   = 0;
    pthread_mutex_unlock(&fe->mutex);
    return;
}
