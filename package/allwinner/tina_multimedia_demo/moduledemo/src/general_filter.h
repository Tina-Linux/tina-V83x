#ifndef _GENERAL_FILTER_H_
#define _GENERAL_FILTER_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

#include "Trecorder.h"
#include "convert.h"

#define GENERAL_FILTER_BUF_NUM 3

struct Filteraddr{
    unsigned char *pVirBuf;
    unsigned char *pPhyBuf;
    int userFlag;
};

struct generalFilter{
	int enable;
    struct Filteraddr dataArray[GENERAL_FILTER_BUF_NUM];
	pthread_t generalFilterId;
	struct moduleAttach generalFilterPort;
};

int generalFilterTest(void *recorderTestContext, void *RecorderStatus, int number);
void generalFilterQuit(void *recorderTestContext, void *RecorderStatus, int number);

#endif
