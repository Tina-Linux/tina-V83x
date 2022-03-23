#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <fcntl.h>
#include "ap_rtc.h"
#include "sys_parm.h"

SYS_PARAM *p_sys_param = NULL;

static pthread_mutex_t sys_param_date_time_lock;

int user_get_local_dt(struct tm *buf)
{
	time_t stime;
	struct tm *ptm = NULL;
	if(NULL == buf)
		return -1;
	if(((time_t)-1) == time(&stime))
		return -1;
	if(NULL == (ptm = localtime(&stime)))
		return -1;
	memcpy((void*)buf,(const void*)ptm,sizeof(struct tm));
	return 0;
}

int sys_param_date_time_get(DATE_TIME *dateTime)
{
	int ret = 0;

	if(dateTime == NULL || p_sys_param == NULL){
		printf("%s:param error\n", __func__);
		ret = -1;
		goto errHdl;
	}

	dateTime->m_date.m_year = p_sys_param->m_dateTime.m_date.m_year;
	dateTime->m_date.m_month = p_sys_param->m_dateTime.m_date.m_month;
	dateTime->m_date.m_day = p_sys_param->m_dateTime.m_date.m_day;

	dateTime->m_time.m_hour = p_sys_param->m_dateTime.m_time.m_hour;
	dateTime->m_time.m_minute = p_sys_param->m_dateTime.m_time.m_minute;
	dateTime->m_time.m_sec = p_sys_param->m_dateTime.m_time.m_sec;
errHdl:
	return ret;
}

int sys_param_date_time_set(DATE_TIME *dateTime)
{
	int ret = 0;

	if(dateTime == NULL || p_sys_param == NULL){
		printf("%s:param error\n", __func__);
		ret = -1;
		goto errHdl;
	}

	pthread_mutex_lock(&sys_param_date_time_lock);
	p_sys_param->m_dateTime.m_date.m_year = dateTime->m_date.m_year;
	p_sys_param->m_dateTime.m_date.m_month = dateTime->m_date.m_month;
	p_sys_param->m_dateTime.m_date.m_day = dateTime->m_date.m_day;

	p_sys_param->m_dateTime.m_time.m_hour = dateTime->m_time.m_hour;
	p_sys_param->m_dateTime.m_time.m_minute = dateTime->m_time.m_minute;
	p_sys_param->m_dateTime.m_time.m_sec = dateTime->m_time.m_sec;
	pthread_mutex_unlock(&sys_param_date_time_lock);

errHdl:
	return ret;
}


int sys_param_pwr_on_restore(void)
{
	int ret = 0;
	rtc_dt_s dt;
	char strDT[32];

	p_sys_param = (SYS_PARAM *)malloc(sizeof(SYS_PARAM));
	if(p_sys_param == NULL){
		printf("%s:alloc mem failed!!!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

	pthread_mutex_init(&sys_param_date_time_lock, NULL);

	ret = ap_rtc_time_get(&dt);
	if(ret != 0){
		printf("get rtc time failed!\n");
		dt.year = 2017;
		dt.month = 8;
		dt.day = 15;
		dt.hour = 10;
		dt.minute = 45;
		dt.second = 36;
		p_sys_param->m_dateTime.m_date.m_year = dt.year;
		p_sys_param->m_dateTime.m_date.m_month = dt.month;
		p_sys_param->m_dateTime.m_date.m_day = dt.day;
		p_sys_param->m_dateTime.m_time.m_hour = dt.hour;
		p_sys_param->m_dateTime.m_time.m_minute = dt.minute;
		p_sys_param->m_dateTime.m_time.m_sec = dt.second;
		ap_rtc_time_set(&dt,RTC_ALL_SET);
	}else{
		p_sys_param->m_dateTime.m_date.m_year = dt.year;
		p_sys_param->m_dateTime.m_date.m_month = dt.month;
		p_sys_param->m_dateTime.m_date.m_day = dt.day;
		p_sys_param->m_dateTime.m_time.m_hour = dt.hour;
		p_sys_param->m_dateTime.m_time.m_minute = dt.minute;
		p_sys_param->m_dateTime.m_time.m_sec = dt.second;
	}
	snprintf(strDT, 32, "date -s \"%04d-%02d-%02d %02d:%02d:%02d\"",
		dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
	system(strDT);

errHdl:
	return ret;
}

int sys_param_pwr_off_save(void)
{
	int ret = 0;

	pthread_mutex_destroy(&sys_param_date_time_lock);

	if(p_sys_param){
		free(p_sys_param);
		p_sys_param = NULL;
	}

	return ret;
}
