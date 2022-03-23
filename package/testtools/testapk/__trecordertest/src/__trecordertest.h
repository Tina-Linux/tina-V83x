#ifdef __cplusplus
extern "C"{
#endif

#ifndef __TRECORDERTEST_H_
#define __TRECORDERTEST_H_

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <Trecorder.h>
#include <videoOutPort.h>

#define DEBUG(x, arg...) do{ \
				printf("[recotder]"x, ##arg); \
			}while(0)

#define ERROR(x, arg...) do{ \
				printf("\033[1m\033[;31m""[recotder]"x"\033[0m", ##arg); \
			}while(0)

typedef struct RecoderTestContext
{
	TrecorderHandle*	mTrecorder;
	int			mRecorderId;
	int			mRecorderFileCount;
	TvideoEncodeFormat	videoEncodeFormat;
	TaudioEncodeFormat	audioEncodeFormat;
	ToutputFormat		outputFormat;
	char			mRecorderPath[128];
	sem_t			CompSem;
}RecoderTestContext;


int CallbackFromTRecorder(void* pUserData, int msg, void* param);


#endif

#ifdef __cplusplus
}
#endif
