#include "gsensor.h"

#include <fcntl.h>
#include <assert.h>
#include <sys/stat.h>
#include <stdio.h>
#include <linux/input.h>

#define DEVICE_PAHT		"/dev/input/event2"
#define G_SENSOR_DEV	"/sys/class/input/input2/threshold"

GsensorDataType *gd = NULL;

int g_sensor_threshold_set(int threshold)
{
	char buf[50];

	if(threshold < 0 || threshold > 30) {
		printf("threshold:0-30\n");
		return -1;
	}

	if(access(G_SENSOR_DEV, F_OK) < 0){
		printf("%s():%d-%s is not exist\n",__func__,__LINE__,G_SENSOR_DEV);
		return -2;
	}

	snprintf(buf, sizeof(buf), "echo %d > /sys/class/input/input2/threshold", threshold);
	system(buf);

	return 0;
}

int g_sensor_threshold_get(int *threshold)
{
	char buf[10];
	int fd = 0;

	if(access(G_SENSOR_DEV, F_OK) < 0){
		printf("%s():%d-%s is not exist\n",__func__,__LINE__,G_SENSOR_DEV);
		return -1;
	}

	fd = open(G_SENSOR_DEV, O_RDONLY);
	if(fd == -1) {
		printf("open id threshold file error\n");
		return -2;
	}

	memset(buf, 0, 10);
	read(fd, buf, 10);
	sscanf(buf, "%d", threshold);

	close(fd);
	return 0;
}

void *GsensorThreadProc(void *arg)
{
	int ret;
	static int gsensorTimer = 0;
	fd_set readfd;
	struct timeval timeout;
	struct input_event key;
	GsensorDataType *gd = NULL;

	gd = (GsensorDataType *)arg;
	gd->bThreadRun = 1;

	while (1)
	{
		if(!gd->bThreadRun)
		{
			printf("thread end\n");
			pthread_exit((void*)1);
			sleep(1);
			printf("thread end\n");
		}

		timeout.tv_sec=0;
		timeout.tv_usec=100*1000;	/* 100ms循环一次 */
		gsensorTimer = (gsensorTimer>0) ? (gsensorTimer-1) : (0);

		FD_ZERO(&readfd);
	    FD_SET(gd->gsensorHdle,&readfd);
	    ret=select(gd->gsensorHdle+1,&readfd,NULL,NULL,&timeout);
		if (ret == -1)
	    {
	        printf("select error");
	    }
		else if (ret == 0)
	    {
	        ;/*printf("time out\n");*/
	    }
	    else if (ret > 0)
	    {
		if(FD_ISSET(gd->gsensorHdle,&readfd))
	        {
			FD_CLR(gd->gsensorHdle, &readfd);
	            read(gd->gsensorHdle, &key, sizeof(key));
				printf("key:(%d, %d, %d)\n", key.code, key.type, key.value);
				if(key.type==0 && key.code==0)	/* 不需要处理xyz,只要能检测到碰撞就行 */
				{
					if(gsensorTimer == 0)
					{
						gsensorTimer = 50;	/* 屏蔽5s内连续的碰撞 */
						gd->gsensorFun();
					}
				}
	        }
	    }
	}

	printf("thread end fail\n");
	return (void*)0;
}

int gsensor_init(gsensor_callback_fun fun)
{
	int ret = 0;

	gd = (GsensorDataType*)malloc(sizeof(GsensorDataType));
	if(NULL == gd)
	{
		printf("malloc fail\n");
		return -1;
	}
	memset((void *)gd, 0, sizeof(GsensorDataType));

	gd->gsensorFun = fun;
	gd->gsensorHdle = open(DEVICE_PAHT, O_RDONLY | O_NONBLOCK);
	if(gd->gsensorHdle == NULL)
	{
		printf("open gensor device fail\n");
		return -1;
	}

	ret = pthread_create(&gd->threadID, NULL, GsensorThreadProc, (void *)gd);
	if (ret == -1)
	{
		printf("create thread fail\n");
		return -1;
	}

	return 0;
}

int gsensor_uninit(gsensor_callback_fun fun)
{
	if(NULL != gd){
		gd->bThreadRun = 0;
		pthread_join(gd->threadID, NULL);
		gd->threadID = 0;
		if(gd->gsensorHdle >=0)
		{
			close(gd->gsensorHdle);
		}
		gd->gsensorFun = fun;

		free((void*)gd);
	}

	return 0;
}
