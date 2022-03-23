#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/rtc.h>
#include "ap_rtc.h"

#define	RTC_DEV_NAME	"/dev/rtc0"

rtc_s* rtc_open(int oflags)
{
	rtc_s *rtc;

	rtc = (rtc_s *)malloc(sizeof(rtc_s));
	if(NULL == rtc) {
		printf("%s:%d malloc rtc fail!\n",__func__,__LINE__);
		return NULL;
	}
	memset(rtc, 0 ,sizeof(rtc_s));
	rtc->fd = open(RTC_DEV_NAME, oflags);
	if (rtc->fd < 0) {
		printf("%s:%d open rtc failed!\n",__func__,__LINE__);
		free(rtc);
		return NULL;
	}
	return rtc;
}

int rtc_close(rtc_s *rtc)
{
	if (NULL == rtc)
		return -1;
	if (close(rtc->fd) < 0) {
		printf("%s:%d close rtc fail!\n",__func__,__LINE__);
		return -1;
	}
	free(rtc);

	return 0;
}
int rtc_get_dt(rtc_s *rtc)
{
	struct rtc_time dtime;

	if ((NULL == rtc) || (rtc->fd < 0))
		return -1;
	if (ioctl(rtc->fd,RTC_RD_TIME,&dtime) < 0){
		printf("%s:%d RTC_RD_TIME failed!\n",__func__,__LINE__);
		return -1;
	}
	rtc->dt.year   = dtime.tm_year + 1900;
	rtc->dt.month  = dtime.tm_mon + 1;
	rtc->dt.day    = dtime.tm_mday;
	rtc->dt.week   = dtime.tm_wday;
	rtc->dt.hour   = dtime.tm_hour;
	rtc->dt.minute = dtime.tm_min;
	rtc->dt.second = dtime.tm_sec;

	return 0;
}
int rtc_set_dt(const rtc_s *rtc)
{
	struct rtc_time dtime = {0};

	if ((NULL == rtc) || (rtc->fd < 0))
		return -1;
	if (ioctl(rtc->fd,RTC_RD_TIME,&dtime) < 0) {
		printf("%s:%d RTC_RD_TIME failed!\n",__func__,__LINE__);
		return -1;
	}
	switch(rtc->setting)
	{
		case RTC_ALL_SET:		//all setting
			dtime.tm_year   = rtc->dt.year - 1900;
			dtime.tm_mon    = rtc->dt.month - 1;
			dtime.tm_mday   = rtc->dt.day;
			dtime.tm_wday   = rtc->dt.week;
			dtime.tm_hour   = rtc->dt.hour;
			dtime.tm_min    = rtc->dt.minute;
			dtime.tm_sec    = rtc->dt.second;
			break;
		case RTC_DATE_SET:		//date setting
			dtime.tm_year   = rtc->dt.year - 1900;
			dtime.tm_mon    = rtc->dt.month - 1;
			dtime.tm_mday   = rtc->dt.day;
			dtime.tm_wday   = rtc->dt.week;
			break;
		case RTC_TIME_SET:		//time setting
			dtime.tm_hour   = rtc->dt.hour;
			dtime.tm_min    = rtc->dt.minute;
			dtime.tm_sec    = rtc->dt.second;
			break;
		default:
			break;
	}
	if (ioctl(rtc->fd, RTC_SET_TIME, &dtime) < 0) {
		printf("%s:%d RTC_SET_TIME failed!\n",__func__,__LINE__);
		return -1;
	}

	return 0;
}

int ap_rtc_time_set(const rtc_dt_s *dt, int type)
{
	rtc_s *rtc = NULL;

	if(NULL == dt)
		return -1;
	rtc = rtc_open(O_RDWR);
	if (NULL == rtc) {
		printf("%s()%d-open rtc fail\n",__func__,__LINE__);
		return -1;
	}
	memcpy((void*)&rtc->dt,(const void*)dt,sizeof(rtc_dt_s));
	rtc->setting = type;
	if (rtc_set_dt(rtc)) {
		printf("%s():%d-rtc set time fail\n",__func__,__LINE__);
		return -1;
	}
	rtc_close(rtc);

	return 0;
}

int ap_rtc_time_get(rtc_dt_s *dt)
{
	rtc_s *rtc = NULL;

	if (NULL == dt)
		return -1;
	rtc = rtc_open(O_RDWR);
	if (NULL == rtc){
		printf("%s()%d-open rtc fail\n",__func__,__LINE__);
		return -1;
	}
	if (rtc_get_dt(rtc)){
		printf("%s():%d-rtc get time fail\n",__func__,__LINE__);
		return -1;
	}
	memcpy((void*)dt,(const void*)&rtc->dt,sizeof(rtc_dt_s));
	rtc_close(rtc);

	return 0;
}

