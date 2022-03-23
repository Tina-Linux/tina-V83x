#ifndef __SYS_PARAM_H__
#define __SYS_PARAM_H__


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	LANGUAGE_ENGLISH,
	LANGUAGE_SIMPLE_CHINESE,
	LANGUAGE_TRADITIONAL_CHINESE,
	LANGUAGE_MAX,
}LANGUAGE;

typedef struct {
	unsigned int  m_year;
	unsigned int  m_month;
	unsigned int  m_day;
}DATE;

typedef struct {
	unsigned int  m_hour;
	unsigned int  m_minute;
	unsigned int  m_sec;
}TIME;

typedef struct {
	DATE m_date;
	TIME m_time;
}DATE_TIME;

typedef struct {

	DATE_TIME m_dateTime;
	LANGUAGE m_lang;
}SYS_PARAM;

int sys_param_date_time_get(DATE_TIME *dateTime);
int sys_param_date_time_set(DATE_TIME *dateTime);
int sys_param_pwr_on_restore(void);
int sys_param_pwr_off_save(void);

#ifdef __cplusplus
}
#endif

#endif
