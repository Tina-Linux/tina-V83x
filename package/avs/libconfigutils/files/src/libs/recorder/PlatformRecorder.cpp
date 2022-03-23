#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>

#include "recorder/PlatformRecorder.h"

#include "recorder/provider/FileProvider.h"
#include "recorder/provider/AC108Provider.h"
#include "recorder/provider/AudioCodecProvider.h"
#include "recorder/filter/TutuClearFilter.h"
#include "recorder/convertor/InterleavedConvertor.h"
#include "recorder/convertor/NonInterleavedConvertor.h"
#include "utils/JsonUtils.h"

static std::string getTimeStamp() {
    struct timeval tv;
    struct timezone tz;
    struct tm *p;

    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);

    char buf[200];
    snprintf(buf, 200, "%02d-%02d-%02d-%02d-%02d-%02d-%06ld", p->tm_year + 1900, \
                                                         1+p->tm_mon, \
                                                         p->tm_mday, \
                                                         p->tm_hour, \
                                                         p->tm_min, \
                                                         p->tm_sec, \
                                                         tv.tv_usec);
    return std::string(buf);
}

namespace AW {

std::shared_ptr<RecorderInterface> PlatformRecorder::create(struct json_object *config, struct json_object *platform)
{
    return std::shared_ptr<RecorderInterface>(new PlatformRecorder(config, platform));
}

PlatformRecorder::~PlatformRecorder()
{

}

int PlatformRecorder::init()
{
    struct json_object *j_value, *j_target_filter;
    int ret;
    //init filter
    if(!JsonUtils::json_object_object_get_ex(m_platform_json, "filter", &j_value)) return -1;
    const char* filter_name = JsonUtils::json_object_get_string(j_value);

    if(!JsonUtils::json_object_object_get_ex(m_platform_json, filter_name, &j_target_filter)) {
        printf("platform config json get target filter %s failed!\n", filter_name);
        return -1;
    }

    if(!JsonUtils::json_object_object_get_ex(j_target_filter, "base", &j_value)){
        printf("provide: %s get base failed\n", filter_name);
        return -1;
    }
    filter_name = JsonUtils::json_object_get_string(j_value);

    if(strcmp(filter_name, "tutuclear") == 0) {
        m_filter = TutuClearFilter::create();
    }else {
        printf("filter %s is not supported now\n", filter_name);
        return -1;
    }

    ret = m_filter->init(j_target_filter);
    if(ret < 0) return ret;

    std::string create_timestamp = getTimeStamp();
    //init providers
    if(!JsonUtils::json_object_object_get_ex(m_platform_json, "provider", &j_value)) return -1;
    if(JsonUtils::json_object_get_type(j_value) == json_type_string) {
    ret = init_provider_by_name(JsonUtils::json_object_get_string(j_value), create_timestamp);
    if(ret < 0) return ret;
    }else if(JsonUtils::json_object_get_type(j_value) == json_type_array) {
    for(int i = 0; i < JsonUtils::json_object_array_length(j_value); i++){
        ret = init_provider_by_name(JsonUtils::json_object_get_string(
                                    JsonUtils::json_object_array_get_idx(j_value, i)),
                                    create_timestamp);
        if(ret < 0) return ret;
    }
    }

    m_output_convertor = NonInterleavedConvertor::create(m_filter->getFilterBlock(),
                                                    m_filter->getOutputSampleRate(),
                                                    m_filter->getOutputChannels(),
                                                    m_filter->getOutputSampleBits(),
                                                    m_filter->getOutputSampleRate(),
                                                    16);
    return 0;
}

int PlatformRecorder::init_provider_by_name(const char *name, const std::string &create_timestamp)
{
    std::shared_ptr<ProviderInterface> provider = nullptr;
    struct json_object *j_value, *j_target_provider;
    int ret;

    if(!JsonUtils::json_object_object_get_ex(m_platform_json, name, &j_target_provider)) {
        printf("platform config json get target provider %s failed!\n", name);
        return -1;
    }

    if(!JsonUtils::json_object_object_get_ex(j_target_provider, "base", &j_value)){
        printf("provide: %s get base failed\n", name);
        return -1;
    }

    const char *base_name = JsonUtils::json_object_get_string(j_value);
    if(strcmp(base_name, "ac108") == 0) {
        provider = AC108Provider::create();
    }else if(strcmp(base_name, "audiocodec") == 0) {
        provider = AudioCodecProvider::create();
    }else if(strcmp(base_name, "file") == 0) {
        provider = FileProvider::create();
    }else{
        printf("provider %s is not supported now\n", base_name);
        return -1;
    }

    provider->setCreateTimestamp(create_timestamp);
    ret = provider->init(j_target_provider);
    if(ret < 0) return ret;

    auto convertor = InterleavedConvertor::create(m_filter->getFilterBlock(),
                                                provider->getSampleRate(),
                                                provider->getChannels(),
                                                provider->getSampleBits(),
                                                m_filter->getInputSampleRate(),
                                                m_filter->getInputSampleBits());

    return m_filter->setProvider(provider, convertor);
}

int PlatformRecorder::release()
{
    return m_filter->release();
}

int PlatformRecorder::fetch(std::shared_ptr<ConvertorInterface> &convertor, int samples)
{
    convertor = m_output_convertor;
    return m_filter->fetch(m_output_convertor, 0);
}

std::shared_ptr<ConvertorInterface> PlatformRecorder::fetch(int samples)
{
    int ret = m_filter->fetch(m_output_convertor, 0);
    if(ret < 0) return nullptr;
    return m_output_convertor;
}
/*
int PlatformRecorder::fetch(char **data)
{
    int ret = 0;
    ret = m_filter->fetch(m_output_convertor, 0);
    if(ret < 0) return ret;
    ret = m_output_convertor->getChannelData(0, data);
    return ret;
}
*/
int PlatformRecorder::start()
{
    return m_filter->start();
}

int PlatformRecorder::stop()
{
    return m_filter->stop();
}


PlatformRecorder::PlatformRecorder(struct json_object *config, struct json_object *platform)
{
    m_config_json = config;
    m_platform_json = platform;
}

}
