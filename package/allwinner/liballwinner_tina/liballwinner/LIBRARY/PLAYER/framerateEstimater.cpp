
#include <string.h>
#include <stdint.h>
#include "framerateEstimater.h"
#include "log.h"

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

        //* bubble sort.
        for(k=0; k<nValidDurationCnt-1; k++)
        {
            for(n=0; n<nValidDurationCnt-1-k; n++)
            {
                if(nDurationArr[n]>nDurationArr[n+1])
                {
                    tmp               = nDurationArr[n];
                    nDurationArr[n]   = nDurationArr[n+1];
                    nDurationArr[n+1] = tmp;
                }
            }
        }

        fe->nFrameDuration = (int)nDurationArr[nValidDurationCnt/2];
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

    if(nFramerate < 10000 || nFramerate > 60000)
        nFramerate = 24000; //* 24 fps.

    return nFramerate;
}


int FramerateEstimaterGetFrameDuration(FramerateEstimater* fe)
{
    int nFrameDuration;
    nFrameDuration = fe->nFrameDuration;
    //* fps lower than 10 fps or higher than 60 fps is invalid.
    if(nFrameDuration < 16666 || nFrameDuration > 100000)
        nFrameDuration = 41667; //* 24 fps.
    return nFrameDuration;
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
