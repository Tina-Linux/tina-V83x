#ifndef	__AP_RTC_H__
#define	__AP_RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define	RTC_ALL_SET		(0x01)
#define	RTC_DATE_SET	(0x02)
#define	RTC_TIME_SET	(0x03)

typedef struct{
	int year;
	int month;
	int week;
	int day;
	int hour;
	int minute;
	int second;
}rtc_dt_s;

typedef struct{
	rtc_dt_s dt;
	int setting;
	int fd;
}rtc_s;

/*
 * rtc_open() - open the rtc device and malloc rtc_s memory
 * @oflags:open mode:O_RDONLY,O_WRONLY,O_RDWR...
 * @:return value:success-> rtc pointer
 * fail:NULL
 */
rtc_s* rtc_open(int oflags);
/*
 * rtc_close() - close the rtc device and free rtc_s memory
 * @*rtc:rtc pointer from rtc_open() return
 * @:return value:success -> 0
 * fail:-1
 */
int rtc_close(rtc_s *rtc);
/*
 * rtc_get_dt() - get rtc date and time
 * @*rtc:rtc pointer from rtc_open() return,date and time data
 * will be write to buffer
 * @:return value:success -> 0
 * fail:-1
 */
int rtc_get_dt(rtc_s *rtc);
/*
 * rtc_set_dt() - set rtc date and time
 * @*rtc:rtc pointer from rtc_open() return
 * @:return value:success -> 0
 * fail:-1
 */
int rtc_set_dt(const rtc_s *rtc);
/*
 * ap_rtc_time_set() - set rtc date and time
 * @*dt:pointer of struct rtc_dt_s
 * @type:type to want set
 * return: success:0 fail:-1
 */
int ap_rtc_time_set(const rtc_dt_s *dt,int type);
/*
 * ap_rtc_time_get() - get rtc date and time
 * @*dt:pointer of struct rtc_dt_s
 * return: success:0 fail: -1
 */
int ap_rtc_time_get(rtc_dt_s *dt);

#ifdef __cplusplus
}
#endif

#endif /*__AP_RTC_H__*/

