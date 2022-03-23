#ifndef _GENERAL_SINK_H_
#define _GENERAL_SINK_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

#include "Trecorder.h"
#include "convert.h"


struct generalSink{
	int enable;
	pthread_t generalSinkId;
	struct moduleAttach generalSinkPort;
};

int generalSinkTest(void *recorderTestContext, void *RecorderStatus, int number);
void generalSinkQuit(void *recorderTestContext, void *RecorderStatus, int number);


#endif
