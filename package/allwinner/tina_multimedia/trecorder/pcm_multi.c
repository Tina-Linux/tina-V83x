#include <asoundlib.h>
#include <pthread.h>
#include <stdlib.h>
#include "pcm_multi.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "TRlog.h"

MultiPCM *gMultiPCMHdl = NULL;

static void* pcmInputThread(void *param)
{
	MultiPCM *hdl = (MultiPCM *)param;
	int i;
	int ret;
	int validsize;
	int sizeCopyed;
	char tmpBuffer[SIZE_PER_READ];

	while(hdl->threadStarted)
	{
		ret = pcm_read(hdl->pcm,tmpBuffer,SIZE_PER_READ);

		if(ret < 0)
			break;

#ifdef DUMP_PCM
		if(hdl->fp != NULL)
			fwrite(tmpBuffer,1,SIZE_PER_READ,hdl->fp);
#endif

		for(i = 0; i < MAX_HDL_NUM; i++)
		{
			pthread_mutex_lock(&hdl->buf[i].mutex);
			if(!hdl->buf[i].enable)
			{
				pthread_mutex_unlock(&hdl->buf[i].mutex);
				continue;
			}

			if(hdl->buf[i].write >= hdl->buf[i].read)
				validsize = hdl->buf[i].read + (CACHE_SIZE - hdl->buf[i].write);
			else
				validsize = hdl->buf[i].read - hdl->buf[i].write;

			if(validsize >= SIZE_PER_READ)
				sizeCopyed = SIZE_PER_READ;
			else
			{
				sizeCopyed = validsize;
				TRlog(TR_LOG_AUDIO, "audio data was dropped:%d\n",SIZE_PER_READ - validsize);
			}

			if(hdl->buf[i].write >= hdl->buf[i].read){
				if(CACHE_SIZE - hdl->buf[i].write > sizeCopyed)
					memcpy(hdl->buf[i].buffer + hdl->buf[i].write,tmpBuffer,sizeCopyed);
				else{
					memcpy(hdl->buf[i].buffer + hdl->buf[i].write,tmpBuffer,CACHE_SIZE - hdl->buf[i].write);
					memcpy(hdl->buf[i].buffer,tmpBuffer + CACHE_SIZE - hdl->buf[i].write,sizeCopyed - (CACHE_SIZE - hdl->buf[i].write));
				}
			}else
				memcpy(hdl->buf[i].buffer + hdl->buf[i].write,tmpBuffer,sizeCopyed);

			hdl->buf[i].write = (hdl->buf[i].write + sizeCopyed)%(CACHE_SIZE);

			if(hdl->buf[i].write == hdl->buf[i].read)
				TRwarn("[%s] -------------buffer full,may drop 4K data!\n", __func__);
			pthread_mutex_unlock(&hdl->buf[i].mutex);
                        //printf("%d:post the semphore\n",i);
			sem_post(&hdl->buf[i].mHaveEnoughDataSem);
		}
		//usleep(1*1000);
	}
	return NULL;
}



MultiPCM* MultiPCMInit(struct pcm_config *cfg)
{
	struct pcm *pcm = NULL;
	int i;
	MultiPCM *hdl;

	if(gMultiPCMHdl)
	{
		gMultiPCMHdl->refer++;
		return gMultiPCMHdl;
	}

	hdl = (MultiPCM *)malloc(sizeof(MultiPCM));
	if(hdl == NULL)
		return NULL;

	for(i = 0;i < MAX_HDL_NUM;i++)
	{
		hdl->buf[i].read = hdl->buf[i].write = 0;
		memset(hdl->buf[i].buffer,0,CACHE_SIZE);
		pthread_mutex_init(&hdl->buf[i].mutex,NULL);
		hdl->buf[i].enable = 0;
                sem_init(&hdl->buf[i].mHaveEnoughDataSem, 0, 0);
	}

	pcm = pcm_open(0, 0, PCM_IN, cfg);

	if(pcm == NULL)
	{
		free(hdl);
		return NULL;
	}

	hdl->pcm = pcm;
	hdl->threadStarted = 0;
#ifdef DUMP_PCM
	hdl->fp = fopen("/mnt/SDCARD/dump.pcm","wb+");
	hdl->fp2 = fopen("/mnt/SDCARD/dump2.pcm","wb+");
#endif
	hdl->refer++;
	gMultiPCMHdl = hdl;
	return hdl;
}

int MultiPCMClear(MultiPCM *hdl,int index)
{
	if(hdl == NULL)
		return -1;
	pthread_mutex_lock(&hdl->buf[index].mutex);
	hdl->buf[index].read = hdl->buf[index].write = 0;
	memset(hdl->buf[index].buffer,0,CACHE_SIZE);
	hdl->buf[index].enable = 0;
	pthread_mutex_unlock(&hdl->buf[index].mutex);
	return 0;
}

int MultiPCMRead(MultiPCM *hdl,char *data,int size,int index)
{
	int sizeValid;
	int actualRead;

	if(hdl == NULL || data == NULL)
		return -1;

	if(index >= MAX_HDL_NUM)
		return -1;

	if(hdl->threadStarted != 1)
	{
		hdl->threadStarted = 1;
		pthread_create(&hdl->dataThreadId, NULL, pcmInputThread, hdl);
	}

	pthread_mutex_lock(&hdl->buf[index].mutex);
	hdl->buf[index].enable = 1;
        pthread_mutex_unlock(&hdl->buf[index].mutex);
        sem_wait(&hdl->buf[index].mHaveEnoughDataSem);
        //printf("%d:wait the semphore finish\n",index);
	pthread_mutex_lock(&hdl->buf[index].mutex);
	if(hdl->buf[index].write >= hdl->buf[index].read)
		sizeValid = hdl->buf[index].write - hdl->buf[index].read;
	else
		sizeValid = hdl->buf[index].write + (CACHE_SIZE - hdl->buf[index].read);

	if(size <= sizeValid)
		actualRead = size;
	else
		actualRead = sizeValid;

	if(hdl->buf[index].write >= hdl->buf[index].read){
		memcpy(data, hdl->buf[index].buffer + hdl->buf[index].read, actualRead);
	}else{
		if(CACHE_SIZE - hdl->buf[index].read > actualRead)
			memcpy(data, hdl->buf[index].buffer + hdl->buf[index].read, actualRead);
		else{
			memcpy(data, hdl->buf[index].buffer + hdl->buf[index].read, CACHE_SIZE - hdl->buf[index].read);
			memcpy(data + (CACHE_SIZE - hdl->buf[index].read), hdl->buf[index].buffer, actualRead - (CACHE_SIZE - hdl->buf[index].read));
		}
	}

#ifdef DUMP_PCM
	if(hdl->fp2 != NULL)
		fwrite(hdl->buf[index].buffer + hdl->buf[index].read,1,actualRead,hdl->fp2);
#endif

	hdl->buf[index].read = (hdl->buf[index].read + actualRead)%(CACHE_SIZE);
    sizeValid = sizeValid - actualRead;
	pthread_mutex_unlock(&hdl->buf[index].mutex);
	if(sizeValid/size >= 1)
		sem_post(&hdl->buf[index].mHaveEnoughDataSem);

	return actualRead;
}

int MultiPCMDeInit(MultiPCM *hdl)
{
	int i;
	gMultiPCMHdl->refer--;
	if(gMultiPCMHdl->refer != 0)
	{
		TRwarn("[%s] deinit with nothing!\n", __func__);
		return 0;
	}

	if(hdl->threadStarted)
	{
		hdl->threadStarted = 0;
		pthread_join(hdl->dataThreadId,NULL);
	}
	if(hdl->pcm != NULL)
	{
		pcm_close(hdl->pcm);
		hdl->pcm = NULL;
	}

	for(i = 0;i < MAX_HDL_NUM;i++)
	{
		pthread_mutex_destroy(&hdl->buf[i].mutex);
                sem_destroy(&hdl->buf[i].mHaveEnoughDataSem);
	}

#ifdef DUMP_PCM
	if(hdl->fp != NULL)
		fclose(hdl->fp);

	if(hdl->fp2 != NULL)
		fclose(hdl->fp2);
#endif
	free(hdl);

	gMultiPCMHdl = NULL;
	return 0;
}
