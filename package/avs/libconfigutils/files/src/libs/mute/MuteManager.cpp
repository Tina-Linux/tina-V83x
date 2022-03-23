#include <string.h>

#include "mute/MuteManager.h"
#include "gpio/GpioHandler.h"
#include "amixer/AmixerHandler.h"
#include "utils/JsonUtils.h"

#define GET_VAL_BY_KEY_CHECKED(json, key, value) \
do { \
    if(!JsonUtils::json_object_object_get_ex(json, key, &value)) { \
        printf("get %s failed\n", key); \
        return -1; }\
}while(0)

namespace AW {

std::shared_ptr<MuteManager> MuteManager::create(struct json_object *config)
{
    auto mute_manager = std::shared_ptr<MuteManager>(new MuteManager());

    if(mute_manager->init(config) < 0) return nullptr;

    return mute_manager;
}

MuteManager::~MuteManager()
{
    release();
}

int MuteManager::init(struct json_object *config)
{
    int ret;
    struct json_object *j_value;

    GET_VAL_BY_KEY_CHECKED(config, "mute-enable", j_value);
    if(strcmp(JsonUtils::json_object_get_string(j_value), "active") == 0) {
        m_mute_enable = [this](void) -> void {
            for(auto iter = m_active_list.begin(); iter != m_active_list.end(); ++iter){
                (*iter)->active();
            }
        };
    }else if(strcmp(JsonUtils::json_object_get_string(j_value), "disactive") == 0) {
        m_mute_enable = [this](void) -> void{
            for(auto iter = m_active_list.rbegin(); iter != m_active_list.rend(); ++iter){
                (*iter)->disactive();
            }
        };
    }else{
        printf("MuteManager init mute-enable: %s is not support!\n",
            JsonUtils::json_object_get_string(j_value));
        return -1;
    }

    GET_VAL_BY_KEY_CHECKED(config, "mute-disable", j_value);
    if(strcmp(JsonUtils::json_object_get_string(j_value), "active") == 0) {
        m_mute_disable = [this](void) -> void {
            for(auto iter = m_active_list.begin(); iter != m_active_list.end(); ++iter){
                (*iter)->active();
            }
        };
    }else if(strcmp(JsonUtils::json_object_get_string(j_value), "disactive") == 0) {
        m_mute_disable = [this](void) -> void{
            for(auto iter = m_active_list.rbegin(); iter != m_active_list.rend(); ++iter){
                (*iter)->disactive();
            }
        };
    }else{
        printf("MuteManager init mute-disable: %s is not support!\n",
            JsonUtils::json_object_get_string(j_value));
        return -1;
    }

    if(init_gpio_ctr(config) < 0) {
        printf("MuteManager init_gpio_ctr failed\n");
        return -1;
    }
    if(init_amixer_ctr(config) < 0) {
        printf("MuteManager init_gpio_ctr failed\n");
        return -1;
    }

    if(m_active_list.empty()) {
        printf("MuteManager no gpio config for mute!\n");
    }

    return 0;
}

void MuteManager::release()
{
    m_active_list.clear();
}

int MuteManager::privacyMute(bool is_mute)
{
    printf("privacyMute: %d, size: %ld\n", is_mute, m_active_list.size());
    if(is_mute){
        m_mute_enable();
    }else {
        m_mute_disable();
    }
    return 0;
}

int MuteManager::init_gpio_ctr(struct json_object *config)
{
    struct json_object *j_value;
    if(!JsonUtils::json_object_object_get_ex(config, "gpio", &j_value)) {
        printf("MuteManager have not gpio control, return!\n");
        return 0;
    }

    if(JsonUtils::json_object_get_type(j_value) != json_type_array) {
        printf("MuteManager init gpio config type is %d, is not support now!\n",
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

int MuteManager::init_amixer_ctr(struct json_object *config)
{
    struct json_object *j_value;
    if(!JsonUtils::json_object_object_get_ex(config, "amixer", &j_value)) {
        printf("MuteManager have not amixer control, return!\n");
        return 0;
    }

    if(JsonUtils::json_object_get_type(j_value) != json_type_array) {
        printf("MuteManager init amixer config type is %d, is not support now!\n",
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

}
