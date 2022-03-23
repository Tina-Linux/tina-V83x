#ifndef __GSENSOR_H__
#define __GSENSOR_H__

#include <sys/types.h>

typedef void (*gsensor_callback_fun)(void);

typedef struct
{
	int bThreadRun;
	pthread_t threadID;
	gsensor_callback_fun gsensorFun;
	int gsensorHdle;
}GsensorDataType;

int gsensor_init(gsensor_callback_fun fun);
int gsensor_uninit(gsensor_callback_fun fun);
int g_sensor_threshold_set(int threshold);
int g_sensor_threshold_get(int *threshold);

#endif
