#include <iostream>

#include "utils/JsonUtils.h"
#include "button/ButtonManager.h"

#define GET_VAL_BY_KEY_CHECKED(json, key, value) \
do { \
    if(!JsonUtils::json_object_object_get_ex(json, key, &value)) { \
        printf("get %s failed\n", key); \
        return -1; }\
}while(0)

namespace AW {

std::shared_ptr<ButtonManager> ButtonManager::create(std::shared_ptr<AW::EventWatcher> watcher,
                                                     std::shared_ptr<AW::Executor> executor,
                                                     struct json_object *config)
{
    auto button_manager = std::shared_ptr<ButtonManager>(new ButtonManager(watcher, executor));

    if(button_manager->init(config) < 0) return nullptr;

    return button_manager;
}

void ButtonManager::addButtonObserver(std::shared_ptr<Observer> observer)
{
    m_observers.insert(observer);
}

void ButtonManager::removeButtonObserver(std::shared_ptr<Observer> observer)
{
    m_observers.erase(observer);
}

void ButtonManager::onKeyEventDown(int keycode)
{
    for(auto button : m_button_set) {
        if(button->getCode() != keycode) continue;
        if(strcmp(button->getName(), "volume-down") == 0) {
            for(auto observer: m_observers) {
                observer->onVolumeDown();
            }
        }else if(strcmp(button->getName(), "volume-up") == 0) {
            for(auto observer: m_observers) {
                observer->onVolumeUp();
            }
        }else if(strcmp(button->getName(), "mute") == 0) {
            for(auto observer: m_observers) {
                observer->onMute();
            }
        }else if(strcmp(button->getName(), "audio-jack") == 0) {
            for(auto observer: m_observers) {
                observer->onAudioJackPlugIn();
            }
        }
    }
}

void ButtonManager::onKeyEventUp(int keycode)
{
    for(auto button : m_button_set) {
        if(button->getCode() != keycode) continue;
        if(strcmp(button->getName(), "audio-jack") == 0) {
            for(auto observer: m_observers) {
                observer->onAudioJackPlugOut();
            }
        }
    }
}

void ButtonManager::onkeyEventLongClick(int keycode, float longclicktime)
{

}


int ButtonManager::init(struct json_object *config)
{
    if(JsonUtils::json_object_get_type(config) != json_type_array) {
        printf("ButtonManager init config type is %d, is not support now!\n",
            JsonUtils::json_object_get_type(config));
        return -1;
    }

    for(int i = 0; i < JsonUtils::json_object_array_length(config); i++){
        int ret = init_button(JsonUtils::json_object_array_get_idx(config, i));
        if(ret < 0) return -1;
    }

    return 0;
}

void ButtonManager::release()
{
    for(auto input : m_input_set) {
        input->release();
    }
}

int ButtonManager::init_button(struct json_object *config)
{
    const char *input_name, *button_name;
    int code;
    std::shared_ptr<InputEventWatcher> exist_input = nullptr;
    struct json_object *j_value;

    GET_VAL_BY_KEY_CHECKED(config, "name", j_value);
    button_name = JsonUtils::json_object_get_string(j_value);

    GET_VAL_BY_KEY_CHECKED(config, "input", j_value);
    input_name = JsonUtils::json_object_get_string(j_value);

    GET_VAL_BY_KEY_CHECKED(config, "code", j_value);
    code = JsonUtils::json_object_get_int(j_value);
    if(code <= 0) {
        printf("ButtonManager %s code is %d, failed\n", input_name, code);
        return -1;
    }

    auto button = std::make_shared<Button>(button_name, code);
    m_button_set.insert(button);

    for(auto input : m_input_set) {
        if(strcmp(input->getInputHandlerName().data(), input_name) == 0) {
            exist_input = input;
            printf("ButtonManager %s input exist, add!\n", input_name);
            break;
        }
    }

    if(exist_input == nullptr) {
        exist_input = InputEventWatcher::create(m_watcher, input_name);
        if(exist_input == nullptr) return -1;
        m_input_set.insert(exist_input);
    }

    auto keywatcher = KeyInputEventWatcher::create(m_watcher, m_executor, exist_input, code, -1, shared_from_this());
    if(keywatcher == nullptr) return -1;

    return 0;
}

}
