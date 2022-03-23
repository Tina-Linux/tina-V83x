
#ifndef FRAME_ESTIMATER_H
#define FRAME_ESTIMATER_H

#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define FRAMERATE_ARRAY_SIZE    4
#define FRAMERATE_START_ESTIMATE_SIZE 2

typedef struct FramerateEstimater
{
    int64_t         nPts[FRAMERATE_ARRAY_SIZE];
	int             nWritePos;
	int             nValidPtsCnt;
	int             nFrameDuration;
	pthread_mutex_t mutex;
}FramerateEstimater;


FramerateEstimater* FramerateEstimaterCreate(void);

void FramerateEstimaterDestroy(FramerateEstimater* fe);

void FramerateEstimaterUpdate(FramerateEstimater* fe, int64_t nPts);

int FramerateEstimaterGetFramerate(FramerateEstimater* fe);

int FramerateEstimaterGetFrameDuration(FramerateEstimater* fe); //* in unit of us.

void FramerateEstimaterReset(FramerateEstimater* fe);


#endif
