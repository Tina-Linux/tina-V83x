#ifndef __USR_CONFIG_H__
#define __USR_CONFIG_H__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>

/* ASR MODULE INFO */
#define ASR_MODULE_CHIP_TYPE           (0x06)   //"R6"
#define ASR_MODULE_ASR_VENDOR          (0x04)   //"UVOICE"
#define ASR_MODULE_HW_VERSION_HIGH     (0x01)   //hardware version high
#define ASR_MODULE_HW_VERSION_LOW      (0x00)   //hardware version low
#define ASR_MODULE_SW_MAJOR_VERSION    (0x01)   //software major version
#define ASR_MODULE_SW_MINOR_VERSION_1  (0x07)   //software minor version1
#define ASR_MODULE_SW_MINOR_VERSION_2  (0x00)   //software minor version2

/* 模块使用的协议版本号 */
#define ASR_MODULE_SW_VERSION    (35)

/* 语音版本 */
#define ASR_UVOICE_LIB_VERSION    (0104)

/* 默认音量 */
#define DEFAULT_VOLUME    (50)

/* 设备厂测次数 */
#define FACTORY_TEST_RETRY_TIMES    (2)
#define FACTORY_TEST_PLAYBACK_TIMES    (2)

/* User Conf Info */
#define NUMS_OF_DEV_CONFIGS    (16)
#define FILE_OF_DEVICE_INFO "/etc/std/device.conf"
#define FILE_OF_VOLUME_INFO "/etc/std/volume"
#define FILE_OF_LOCAL_ASR_INFO  "/etc/std/local_asr_enable"
#define FILE_OF_LOCAL_PLAYBACK_INFO  "/etc/std/local_playback_enable"
#define FILE_OF_EII_ASR_INFO  "/etc/std/eii_asr_enable"
#define FILE_OF_EII_PLAYBACK_INFO  "/etc/std/eii_playback_enable"

#define RESULT_OK   (0)
#define RESULT_ERR   (-1)

/**
 * @log level def.
 */
#define    APP_LL_TRACE    1    /* trace + debug + info + warn + error + fatal log will output */
#define    APP_LL_DEBUG    2    /* debug + info + warn + error + fatal log will output */
#define    APP_LL_INFO        3    /* info + warn + error + fatal log will output */
#define    APP_LL_WARNING    4    /* warn + error + fatal log will output(default level) */
#define    APP_LL_ERROR    5    /* error + fatal log will output */
#define    APP_LL_FATAL     6    /* fatal error log will output */
#define    APP_LL_NONE        7    /* disable log */


#define    LOG_TAG_TRACE    " trace: "
#define    LOG_TAG_DEBUG    " debug: "
#define    LOG_TAG_INFO    " info: "
#define    LOG_TAG_WARNING    " warning: "
#define    LOG_TAG_ERROR    " error: "
#define    LOG_TAG_FATAL    " fatal: "

/**
 * @brief log level control
 */
#ifndef SET_APP_LL_LEVEL
#define SET_APP_LL_LEVEL    (APP_LL_INFO)
#endif

//#define DEBUG_FILE    "/tmp/log.dat"

#define LOG_NONE    (void)0

#ifndef _FILE_POINTER_
#define _FILE_POINTER_
    static FILE *fp_log;
#endif

#ifndef _TIME_POINTER_
#define _TIME_POINTER_
    static time_t timep;
    static struct tm *p;
#endif

#ifdef DEBUG_FILE
    #define LOG_PRINT(level, fmt, ...)        \
        fp_log = fopen(DEBUG_FILE, "a");    \
        time(&timep);                        \
        p = localtime(&timep);                \
        fprintf(fp_log,                        \
        "[%02d/%02d %02d:%02d:%02d] In %s (%s, %u)" level fmt "\n",    \
        1+p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec,    \
        __func__, __FILE__, __LINE__, ##__VA_ARGS__);    \
        fclose(fp_log);    \
        fp_log = NULL;
#else
    #define LOG_PRINT(level, fmt, ...)        \
        time(&timep);                        \
        p = localtime(&timep);                \
        fprintf(stderr,                        \
        "[%02d/%02d %02d:%02d:%02d] In %s (%s, %u)" level fmt "\n",    \
        1+p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec,    \
        __func__, __FILE__, __LINE__, ##__VA_ARGS__);
#endif

/* TRACE */
#if (APP_LL_TRACE >= SET_APP_LL_LEVEL)
    #define    LOG_TRACE(fmt, ...)    LOG_PRINT(LOG_TAG_TRACE, fmt, ##__VA_ARGS__)
#else
    #define LOG_TRACE(fmt, ...) LOG_NONE
#endif

#define LOGT(format, ...)    LOG_TRACE(format, ##__VA_ARGS__)

/* DEBUG */
#if (APP_LL_DEBUG >= SET_APP_LL_LEVEL)
    #define    LOG_DEBUG(fmt, ...)    LOG_PRINT(LOG_TAG_DEBUG, fmt, ##__VA_ARGS__)
#else
    #define LOG_DEBUG(fmt, ...) LOG_NONE
#endif

#define LOGD(format, ...)    LOG_DEBUG(format, ##__VA_ARGS__)

/* INFO */
#if (APP_LL_INFO >= SET_APP_LL_LEVEL)
    #define    LOG_INFO(fmt, ...)    LOG_PRINT(LOG_TAG_INFO, fmt, ##__VA_ARGS__)
#else
    #define LOG_INFO(fmt, ...) LOG_NONE
#endif

#define LOGI(format, ...)    LOG_INFO(format, ##__VA_ARGS__)

/* WARNING */
#if (APP_LL_WARNING >= SET_APP_LL_LEVEL)
    #define    LOG_WARNING(fmt, ...)    LOG_PRINT(LOG_TAG_WARNING, fmt, ##__VA_ARGS__)
#else
    #define LOG_WARNING(fmt, ...) LOG_NONE
#endif

#define LOGW(format, ...)    LOG_WARNING(format, ##__VA_ARGS__)

/* ERROR */
#if (APP_LL_ERROR >= SET_APP_LL_LEVEL)
    #define    LOG_ERROR(fmt, ...)    LOG_PRINT(LOG_TAG_ERROR, fmt, ##__VA_ARGS__)
#else
    #define LOG_ERROR(fmt, ...) LOG_NONE
#endif

#define LOGE(format, ...)    LOG_ERROR(format, ##__VA_ARGS__)

/* FATAL */
#if (APP_LL_FATAL >= SET_APP_LL_LEVEL)
    #define    LOG_FATAL(fmt, ...)    LOG_PRINT(LOG_TAG_FATAL, fmt, ##__VA_ARGS__)
#else
    #define LOG_FATAL(fmt, ...) LOG_NONE
#endif

#define LOGF(format, ...)    LOG_FATAL(format, ##__VA_ARGS__)

/*
#define APP_LOG(flags, fmt, arg...)    \
    do {                            \
        if (flags)                    \
            printf(fmt, ##arg);        \
    } while(0)                        \

#define APP_LOGT(format, args...)    APP_LOG(SET_APP_LL_LEVEL >= APP_LL_TRACE, "[LOG TRACE] "format, ##args)
#define APP_LOGD(format, args...)    APP_LOG(SET_APP_LL_LEVEL >= APP_LL_DEBUG, "[LOG DEBUG] "format, ##args)
#define APP_LOGI(format, args...)    APP_LOG(SET_APP_LL_LEVEL >= APP_LL_INFO, "[LOG INFO] "format, ##args)
#define APP_LOGW(format, args...)    APP_LOG(SET_APP_LL_LEVEL >= APP_LL_WARNING, "[LOG WARNING] "format, ##args)
#define APP_LOGE(format, args...)    APP_LOG(SET_APP_LL_LEVEL >= APP_LL_ERROR, "[LOG ERROR] "format, ##args)
#define APP_LOGF(format, args...)    APP_LOG(SET_APP_LL_LEVEL >= APP_LL_FATAL, "[LOG FATAL] "format, ##args)
*/

sem_t    factory_sem;
pthread_t    factoryThread;
struct timeval timer_start;
struct timeval timer_end;
//struct timeval timer_msg_start;
//struct timeval timer_msg_end;
uint8_t eii_ctl_timeout_flag; //联动控制设备失败状态标志位(0-normal, 1-开始判断)

void *factory_test_thread(void *arg);
int save_conf_info(const char *file_path, uint8_t* buffer);
int read_conf_info(const char *file_path);
int is_file_exist(const char *file_path, uint32_t delay_time_us);
uint8_t random_value_in_range(uint8_t min_value, uint8_t max_value);
#endif
