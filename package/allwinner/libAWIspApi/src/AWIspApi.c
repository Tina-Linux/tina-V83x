#include "AWIspApi.h"

#define LOG_TAG    "AWIspApi"

#ifdef __cplusplus
extern "C" {
#endif

#include "device/isp_dev.h"
#include "isp_dev/tools.h"

#include "isp_events/events.h"
#include "isp_tuning/isp_tuning_priv.h"
#include "isp_manage.h"

#include "iniparser/src/iniparser.h"

#include "isp.h"

#ifdef __cplusplus
}
#endif

#define ALOG(level, ...) \
        ((void)printf("cutils:" level "/" LOG_TAG ": " __VA_ARGS__))
#define ALOGV(...)   ALOG("V", __VA_ARGS__)
#define ALOGD(...)   ALOG("D", __VA_ARGS__)
#define ALOGI(...)   ALOG("I", __VA_ARGS__)
#define ALOGW(...)   ALOG("W", __VA_ARGS__)
#define ALOGE(...)   ALOG("E", __VA_ARGS__)
#define LOG_ALWAYS_FATAL(...)   do { ALOGE(__VA_ARGS__); exit(1); } while (0)

#define MAX_ISP_NUM 2

static int awIspApiInit()
{
    media_dev_init();

    return 0;
}

static int awIspGetIspId(int video_id)
{
    int id = -1;

    id = isp_get_isp_id(video_id);

    ALOGD("F:%s, L:%d, video%d --> isp%d",__FUNCTION__, __LINE__, video_id, id);
    if (id > MAX_ISP_NUM - 1) {
        id = -1;
        ALOGE("F:%s, L:%d, get isp id error!",__FUNCTION__, __LINE__);
    }
    return id;
}

static int awIspStart(int isp_id)
{
    int ret = -1;

    ret = isp_init(isp_id);
    ret = isp_run(isp_id);

    if (ret < 0) {
        ALOGE("F:%s, L:%d, ret:%d",__FUNCTION__, __LINE__, ret);
        return ret;
    }

    return ret;
}

static int awIspStop(int isp_id)
{
    int ret = -1;

    ret = isp_stop(isp_id);
    ret = isp_pthread_join(isp_id);
    ret = isp_exit(isp_id);

    if (ret < 0) {
        ALOGE("F:%s, L:%d, ret:%d",__FUNCTION__, __LINE__, ret);
        return ret;
    }

    return ret;
}

static int awIspWaitToExit(int isp_id)
{
    int ret = -1;

    ret = isp_pthread_join(isp_id);
    ret = isp_exit(isp_id);

    if (ret < 0) {
        ALOGE("F:%s, L:%d, ret:%d",__FUNCTION__, __LINE__, ret);
        return ret;
    }

    return ret;
}

static int awIspApiUnInit()
{
    int ret = 0;
    media_dev_exit();
    return 0;
}

AWIspApi *CreateAWIspApi(void)
{
	AWIspApi *ispport = (AWIspApi *)malloc(sizeof(AWIspApi));
	if(!ispport)
		return NULL;

    memset(ispport, 0, sizeof(AWIspApi));

    ispport->ispApiInit = awIspApiInit;
    ispport->ispGetIspId = awIspGetIspId;
    ispport->ispStart = awIspStart;
    ispport->ispStop = awIspStop;
    ispport->ispWaitToExit = awIspWaitToExit;
    ispport->ispApiUnInit = awIspApiUnInit;

	return ispport;
}

void DestroyAWIspApi(AWIspApi *hdl)
{
    if(hdl)
        free(hdl);
}
