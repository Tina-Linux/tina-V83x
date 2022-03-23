#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "recorder/PlatformRecorder.h"
#include "utils/FileUtils.h"
#include "utils/JsonUtils.h"

#include "platformadapter/PlatformAdapter.h"
#include "configs/ConfigOverlay.h"

#include "mute/MuteManager.h"
#include "button/ButtonManager.h"
#include "audiojack/AudioJackManager.h"
#include "ledshow/ShowManager.h"

namespace AW {

std::shared_ptr<PlatformAdapter> PlatformAdapter::create(const char *config)
{
    auto adapter = std::shared_ptr<PlatformAdapter>(new PlatformAdapter(config));
    if(adapter->init() == 0) return adapter;

    return nullptr;
}

PlatformAdapter::~PlatformAdapter()
{
    if(m_recorder) {
        m_recorder->release();
    }

    JsonUtils::json_object_put(m_config_json);
    m_watcher->stopWatcher();
}

PlatformAdapter::PlatformAdapter(const char *config)
{
    m_config_file = config;
}

std::shared_ptr<RecorderInterface> PlatformAdapter::getRecorder()
{
    return m_recorder;
}

int PlatformAdapter::init()
{
    if(m_config_file == nullptr) return -1;
    if(AW::JsonUtils::init("libjson-c.so.2") == -1) return -1;

    m_watcher = EventWatcher::create();
    m_executor = Executor::create(2);
    m_watcher->startWatcher();

    int ret;
    FileUtils fileutils;
    fileutils.create(m_config_file, "rb");

    int file_size = fileutils.size();
    char *file_buf = (char*)malloc(file_size + 1);
    fileutils.read(file_buf, file_size);

    m_config_json = JsonUtils::json_tokener_parse(file_buf);
    if (is_error(m_config_json)) {
        printf("parse json failed\n");
        return -1;
    }

    free(file_buf);
    fileutils.release();

    m_platform_json = ConfigOverlay::getplatform(m_config_json);
    if(m_platform_json == nullptr) return -1;

    ret = ConfigOverlay::update(m_config_json, m_platform_json, "provider");
    if(ret < 0) return ret;

    ret = ConfigOverlay::update(m_config_json, m_platform_json, "filter");
    if(ret < 0) return ret;

    ret = ConfigOverlay::update(m_config_json, m_platform_json, "detector");
    if(ret < 0) return ret;

    printf("platform config:\n%s\n", JsonUtils::json_object_to_json_string_ext(m_platform_json, JSON_C_TO_STRING_PRETTY));


    m_recorder = PlatformRecorder::create(m_config_json, m_platform_json);
    if(m_recorder->init() < 0) return -1; //TODO: what time to release recorder

    struct json_object *j_mute;
    if(!JsonUtils::json_object_object_get_ex(m_platform_json, "mute", &j_mute)) {
        printf("PlatformAdapter get mute failed, ignore, create dummy\n");
        m_mute_manager = DummyMuteManager::create(j_mute);
    }else{
        m_mute_manager = MuteManager::create(j_mute);
    }
    if(m_mute_manager == nullptr) return -1;

    struct json_object *j_button;
    if(!JsonUtils::json_object_object_get_ex(m_platform_json, "button", &j_button)) {
        printf("PlatformAdapter get button failed, ignore, create dummy\n");
        m_button_manager = DummyButtonManager::create(m_watcher, m_executor, j_button);
    }else {
        m_button_manager = ButtonManager::create(m_watcher, m_executor, j_button);
    }
    if(m_button_manager == nullptr) return -1;

    struct json_object *j_audio_jack;
    if(!JsonUtils::json_object_object_get_ex(m_platform_json, "audio-jack", &j_audio_jack)) {
        printf("PlatformAdapter get audio-jack failed, ignore, create dummy\n");
        m_audiojack_manager = DummyAudioJackManager::create(j_audio_jack);
    }else{
        m_audiojack_manager = AudioJackManager::create(j_audio_jack);
    }
    if(m_audiojack_manager == nullptr) return -1;

    struct json_object *j_led_ring;
    if(!JsonUtils::json_object_object_get_ex(m_platform_json, "led-ring", &j_led_ring)) {
        printf("PlatformAdapter get led-ring failed, ignore, create dummy\n");
        m_show_manager = DummyShowManager::create(m_watcher, j_led_ring);
    }else {
        m_show_manager = ShowManager::create(m_watcher,
                                             j_led_ring,
                                             m_recorder->getFilter()->getDOAInfo());
    }
    if(m_show_manager == nullptr) return -1;

    return 0;
}

const char * PlatformAdapter::getSensoryModel()
{
    struct json_object *j_sensory, *j_model;
    if(!JsonUtils::json_object_object_get_ex(m_platform_json, "sensory", &j_sensory) &&
       !JsonUtils::json_object_object_get_ex(m_config_json, "sensory", &j_sensory)) {
        printf("PlatformAdapter get sensory failed\n");
        return nullptr;
    }
    if(!JsonUtils::json_object_object_get_ex(j_sensory, "model", &j_model)) {
        printf("PlatformAdapter get sensory model failed\n");
        return nullptr;
    }

    return JsonUtils::json_object_get_string(j_model);
}

const char * PlatformAdapter::getSensoryOperatingPoint()
{
    struct json_object *j_sensory, *j_point;
    if(!JsonUtils::json_object_object_get_ex(m_platform_json, "sensory", &j_sensory) &&
       !JsonUtils::json_object_object_get_ex(m_config_json, "sensory", &j_sensory)) {
        printf("PlatformAdapter get sensory failed\n");
        return nullptr;
    }
    if(!JsonUtils::json_object_object_get_ex(j_sensory, "operating-point", &j_point)) {
        printf("PlatformAdapter get sensory operating point failed\n");
        return nullptr;
    }

    return JsonUtils::json_object_get_string(j_point);
}

const char * PlatformAdapter::getDetectorType()
{
    struct json_object *j_detector;
    if(!JsonUtils::json_object_object_get_ex(m_platform_json, "detector", &j_detector)) {
        return "sensory";
    }
    return JsonUtils::json_object_get_string(j_detector);
}

bool PlatformAdapter::getVoiceDataLoopback()
{
    struct json_object *j_loopback;
    if(!JsonUtils::json_object_object_get_ex(m_platform_json, "voice-loopback", &j_loopback)) {
        return false;
    }

    if(JsonUtils::json_object_get_int(j_loopback) == 0) return false;
    else return true;
}

const char * PlatformAdapter::getAmazonliteModel()
{
    struct json_object *j_amazon_lite, *j_model;
    if(!JsonUtils::json_object_object_get_ex(m_platform_json, "amazon-lite", &j_amazon_lite) &&
       !JsonUtils::json_object_object_get_ex(m_config_json, "amazon-lite", &j_amazon_lite)) {
        printf("PlatformAdapter get amazon-lite failed\n");
        return nullptr;
    }
    if(!JsonUtils::json_object_object_get_ex(j_amazon_lite, "model", &j_model)) {
        printf("PlatformAdapter get amazon-lite model failed\n");
        return nullptr;
    }

    return JsonUtils::json_object_get_string(j_model);
}

int PlatformAdapter::getAmazonliteDetectThreshold()
{
    struct json_object *j_amazon_lite, *j_threshold;
    if(!JsonUtils::json_object_object_get_ex(m_platform_json, "amazon-lite", &j_amazon_lite) &&
       !JsonUtils::json_object_object_get_ex(m_config_json, "amazon-lite", &j_amazon_lite)) {
        printf("PlatformAdapter get amazon-lite failed\n");
        return -1;
    }
    if(!JsonUtils::json_object_object_get_ex(j_amazon_lite, "detect-threshold", &j_threshold)) {
        printf("PlatformAdapter get amazon-lite detect-threshold failed\n");
        return -1;
    }

    return JsonUtils::json_object_get_int(j_threshold);
}
}
