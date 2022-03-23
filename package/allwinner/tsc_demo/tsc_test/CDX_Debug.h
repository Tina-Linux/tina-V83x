#ifndef OMX_Debug_h
#define OMX_Debug_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CEDARX_VERSION 0x00010800

//#include<CDX_MemWatch.h>

#define  ALOGV LOGV
#define  ALOGD LOGD
#define  ALOGI LOGI
#define  ALOGW LOGW
#define  ALOGE LOGE


#if 0

#define LOGV(...)   ((void)0)
#define LOGD(...)   ((void)0)
#define LOGI(...)   ((void)0)
#define LOGW(...)   ((void)0)
#define LOGE(...)   ((void)0)
#define LOGH
#define LOGHD
#define LOGS

#else
	#ifndef __OS_ANDROID
	#include <stdio.h>

	#ifndef LOG_NDEBUG
	#define LOG_NDEBUG 1
	#endif

	#if LOG_NDEBUG
	#define LOGV(...)   ((void)0)
	#else
	#define LOGV(...) ((void)printf("V/" LOG_TAG ": "));         \
			((void)printf("(%d) ",__LINE__));      \
			((void)printf(__VA_ARGS__));          \
			((void)printf("\n"))
	#endif

	#define LOGD(...) ((void)printf("D/" LOG_TAG ": "));         \
			((void)printf("(%d) ",__LINE__));      \
			((void)printf(__VA_ARGS__));          \
			((void)printf("\n"))


	#define LOGI(...) ((void)printf("I/" LOG_TAG ": "));         \
			((void)printf("(%d) ",__LINE__));      \
			((void)printf(__VA_ARGS__));          \
			((void)printf("\n"))


	#define LOGW(...) ((void)printf("W/" LOG_TAG ": "));         \
			((void)printf("(%d) ",__LINE__));      \
			((void)printf(__VA_ARGS__));          \
			((void)printf("\n"))


	#define LOGE(...) ((void)printf("E/" LOG_TAG ": "));         \
			((void)printf("(%d) ",__LINE__));      \
			((void)printf(__VA_ARGS__));          \
			((void)printf("\n"))


	#if LOG_NDEBUG
	#define LOGH
	#define LOGHD
	#else
	//#define LOGH printf("%s %s() line:%d\n",__FILE__,__FUNCTION__,__LINE__)
	#define LOGH printf("H/%s line:%d\n",__FILE__,__LINE__)
	#define LOGHD printf("H/%s line:%d\n",__FILE__,__LINE__)
	#endif

	#if LOG_NDEBUG
	#define LOGS
	#else
	#define LOGS printf("\n\n\n\n !!!!!!!!!!!!!!!!!!!! %s %s() line:%d !!!!!!!!!!!!!!!!!!!!\n\n\n\n",__FILE__,__FUNCTION__,__LINE__)
	#endif

	#else

	#include <utils/Log.h>

	#if LOG_NDEBUG
	#define LOGH
	#define LOGHD
	#else
	#define LOGH LOGV("H/%s line:%d\n",__FILE__,__LINE__)
	#define LOGHD LOGD("H/%s line:%d\n",__FILE__,__LINE__)
	#endif

	#if LOG_NDEBUG
	#define LOGS
	#else
	#define LOGS LOGV("\n\n\n\n !!!!!!!!!!!!!!!!!!!! %s %s() line:%d !!!!!!!!!!!!!!!!!!!!\n\n\n\n",__FILE__,__FUNCTION__,__LINE__)
	#endif

	#endif

#endif


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
