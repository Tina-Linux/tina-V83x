#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include "common.h"

void *MainThreadProc(void *arg)
{
	sm_debug("main thread begin\n");
	while (1)
	{
		sleep(1);
	}

	sm_error("main thread end fail\n");
	return (void*)0;
}

pthread_t createMainThread()
{
	int ret = 0;
	pthread_t threadID;
	ret = pthread_create(&threadID, NULL, MainThreadProc, NULL);
	if (ret == -1)
	{
		sm_debug("create thread fail\n");
		return NULL;
	}
	sm_debug("mwd->threadID=%d\n", (int)threadID);

	return threadID;
}

int destroryMainThread(pthread_t pid)
{
	pthread_join(pid, NULL);

	return 0;
}

