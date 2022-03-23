#ifndef _PCM_MULTI_H_
#define _PCM_MULTI_H_
#include <asoundlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define CACHE_SIZE 64*1024
#define MAX_HDL_NUM 2
#define SIZE_PER_READ 1024*8

//#define DUMP_PCM

typedef struct
{
	char buffer[CACHE_SIZE];
	int read,write;
	pthread_mutex_t mutex;
	int enable;
        sem_t  mHaveEnoughDataSem;
}MBuffer;

typedef struct
{
	MBuffer buf[MAX_HDL_NUM];
	int threadStarted;
	struct pcm *pcm;
	pthread_t dataThreadId;
#ifdef DUMP_PCM
	FILE *fp;
	FILE *fp2;
#endif
	int refer;
}MultiPCM;

MultiPCM* MultiPCMInit(struct pcm_config *cfg);
int MultiPCMRead(MultiPCM *hdl,char *data,int size,int index);
int MultiPCMClear(MultiPCM *hdl,int index);
int MultiPCMDeInit(MultiPCM *hdl);

#endif
