#ifndef _GENERAL_SRC_H_
#define _GENERAL_SRC_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

#include "Trecorder.h"
#include "convert.h"

#define GENERRAL_SRC_BUF 3

struct Srcaddr{
    unsigned char *pVirBuf;
    unsigned char *pPhyBuf;
    int userFlag;
};

struct generalSrc{
	int enable;
    unsigned char *pYUV;
    unsigned int photoWidth;
    unsigned int photoHeight;
    int dataLength;
	pthread_t generalSrcId;
    struct Srcaddr dataArray[GENERRAL_SRC_BUF];
	struct moduleAttach generalSrcPort;
};

int generalSrcTest(void *recorderTestContext, void *RecorderStatus, int number);
void generalSrcQuit(void *recorderTestContext, void *RecorderStatus, int number);


#endif
