#include <string.h>
#include <unistd.h>
#include "utils/JsonUtils.h"
#include "utils/AmixerUtils.h"
#include "gpio/GpioHandler.h"
#include "amixer/AmixerHandler.h"

#include "audiojack/AudioJackManager.h"

#define GET_VAL_BY_KEY_CHECKED(json, key, value) \
do { \
    if(!JsonUtils::json_object_object_get_ex(json, key, &value)) { \
        printf("get %s failed\n", key); \
        return -1; }\
}while(0)

extern "C" void __dump_heap(void);

static int get_audio_jack_init_state() {
    FILE * fp;
    char buffer[80];
    bzero(buffer, 80);
    fp = popen("cat /sys/module/sun50iw1_sndcodec/parameters/switch_state", "r");
    fgets(buffer, sizeof(buffer), fp);
    pclose(fp);
    return atoi(buffer);
}

namespace AW {

std::shared_ptr<AudioJackManager> AudioJackManager::create(struct json_object *config)
{
    auto manager = std::shared_ptr<AudioJackManager>(new AudioJackManager());

    if(manager->init(config) < 0) return nullptr;

    return manager;
}

AudioJackManager::~AudioJackManager()
{
    release();
}

int AudioJackManager::doAudioJackPlugIn()
{
    m_do_audio_jack_plugin();
    return 0;
}

int AudioJackManager::doAudioJackPlugOut()
{
    m_do_audio_jack_plugout();
    return 0;
}

int AudioJackManager::init_gpio_ctr(struct json_object *config)
{
    struct json_object *j_value;
    if(!JsonUtils::json_object_object_get_ex(config, "gpio", &j_value)) {
        printf("AudioJackManager have not gpio control, return!\n");
        return 0;
    }

    if(JsonUtils::json_object_get_type(j_value) != json_type_array) {
        printf("AudioJackManager init gpio config type is %d, is not support now!\n",
            JsonUtils::json_object_get_type(j_value));
        return -1;
    }

    for(int i = 0; i < JsonUtils::json_object_array_length(j_value); i++){
        auto gpio = GpioHandler::create(JsonUtils::json_object_array_get_idx(j_value, i));
        if(gpio == nullptr) return -1;
        m_active_list.push_back(gpio);
    }

    return 0;
}

int AudioJackManager::init_amixer_ctr(struct json_object *config)
{
    struct json_object *j_value;
    if(!JsonUtils::json_object_object_get_ex(config, "amixer", &j_value)) {
        printf("AudioJackManager have not amixer control, return!\n");
        return 0;
    }

    if(JsonUtils::json_object_get_type(j_value) != json_type_array) {
        printf("AudioJackManager init amixer config type is %d, is not support now!\n",
            JsonUtils::json_object_get_type(j_value));
        return -1;
    }

    for(int i = 0; i < JsonUtils::json_object_array_length(j_value); i++){
        auto amixer = AmixerHandler::create(JsonUtils::json_object_array_get_idx(j_value, i));
        if(amixer == nullptr) return -1;
        m_active_list.push_back(amixer);
    }

    return 0;
}

int AudioJackManager::init(struct json_object *config)
{
    struct json_object *j_value;

    GET_VAL_BY_KEY_CHECKED(config, "plugout", j_value);
    if(strcmp(JsonUtils::json_object_get_string(j_value), "active") == 0) {
        m_do_audio_jack_plugout = [this](void) -> void {
            for(auto iter = m_active_list.begin(); iter != m_active_list.end(); ++iter){
                (*iter)->active();
            }
        };
    }else if(strcmp(JsonUtils::json_object_get_string(j_value), "disactive") == 0) {
        m_do_audio_jack_plugout = [this](void) -> void {
            for(auto iter = m_active_list.rbegin(); iter != m_active_list.rend(); ++iter){
                (*iter)->disactive();
            }
        };
    }else {
        printf("AudioJackManager init plugout: %s is not support!\n",
            JsonUtils::json_object_get_string(j_value));
        return -1;
    }

    GET_VAL_BY_KEY_CHECKED(config, "plugin", j_value);
    if(strcmp(JsonUtils::json_object_get_string(j_value), "active") == 0) {
        m_do_audio_jack_plugin = [this](void) -> void {
            for(auto iter = m_active_list.begin(); iter != m_active_list.end(); ++iter){
                (*iter)->active();
            }
        };
    }else if(strcmp(JsonUtils::json_object_get_string(j_value), "disactive") == 0) {
        m_do_audio_jack_plugin = [this](void) -> void {
            for(auto iter = m_active_list.rbegin(); iter != m_active_list.rend(); ++iter){
                (*iter)->disactive();
            }
        };
    }else {
        printf("AudioJackManager init plugin: %s is not support!\n",
            JsonUtils::json_object_get_string(j_value));
        return -1;
    }

    if(init_gpio_ctr(config) < 0) {
        printf("AudioJackManager init_gpio_ctr failed\n");
        return -1;
    }
    if(init_amixer_ctr(config) < 0) {
        printf("AudioJackManager init_gpio_ctr failed\n");
        return -1;
    }

    if(m_active_list.empty()) {
        printf("AudioJackManager no gpio or amixer config for audio-jack plug!\n");
    }

    if(get_audio_jack_init_state() == 0){
        printf("AudioJackManager init status is plugout\n");
        doAudioJackPlugOut();
    }
    else{
        printf("AudioJackManager init status is plugin\n");
        doAudioJackPlugIn();
    }
    return 0;
}

void AudioJackManager::release()
{
    m_active_list.clear();
}

}
