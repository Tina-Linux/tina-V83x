#include "tconfigs/input/key_manager.h"

#include <vector>
#include <map>
#include <mutex>
#include "tconfigs/common/time.h"
#include "tconfigs/input/input_event_watcher.h"
#include "tconfigs/input/key_input_event_handler.h"
#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace input {

const std::map<std::string, KeyManager::Motion> KeyManager::kStringToMotionMap = {
    {"Press",                   Motion::kPress},
    {"Release",                 Motion::kRelease},
    {"LongPressPreRelease",     Motion::kLongPressPreRelease},
    {"LongPressPostRelease",    Motion::kLongPressPostRelease},
};

KeyManager::Motion KeyManager::StringToMotion(const std::string& motion_string)
{
    auto itr = kStringToMotionMap.find(motion_string);
    if (itr == kStringToMotionMap.end()) {
        return Motion::kUnknown;
    }
    return itr->second;
}

const int KeyManager::kDefaultBuiltInExecutorThreads = 2;

std::shared_ptr<KeyManager> KeyManager::Create(const rapidjson::Value& config)
{
    auto key_manager = std::shared_ptr<KeyManager>(new KeyManager());
    if (!key_manager || !key_manager->Init(config)) {
        TCLOG_ERROR("Fail to create KeyManager");
        return nullptr;
    }
    return key_manager;
}

std::shared_ptr<KeyManager> KeyManager::Create(
        std::shared_ptr<event::EventLoop> event_loop,
        std::shared_ptr<threading::Executor> executor,
        const rapidjson::Value& config)
{
    if (!event_loop || !executor) {
        TCLOG_ERROR("Invalid parameters: event_loop or executor");
        return nullptr;
    }
    auto key_manager = std::shared_ptr<KeyManager>(new KeyManager(event_loop, executor));
    if (!key_manager || !key_manager->Init(config)) {
        TCLOG_ERROR("Fail to create KeyManager");
        return nullptr;
    }
    return key_manager;
}

KeyManager::KeyManager(void)
    : loop_and_executor_is_built_in_(true)
{
}

KeyManager::KeyManager(std::shared_ptr<event::EventLoop> event_loop,
        std::shared_ptr<threading::Executor> executor)
    : event_loop_(event_loop),
      executor_(executor),
      loop_and_executor_is_built_in_(false)
{
}

bool KeyManager::Init(const rapidjson::Value& config)
{
    if (loop_and_executor_is_built_in_) {
        int built_in_executor_threads = kDefaultBuiltInExecutorThreads;
        int content_int;
        if (json::pointer::GetInt(config, "/built_in_executor_threads", &content_int)) {
            built_in_executor_threads = content_int;
        }
        event_loop_ = event::EventLoop::Create();
        executor_ = threading::Executor::Create(built_in_executor_threads);
        if (!event_loop_ || !executor_) {
            TCLOG_ERROR("Fail to create built-in event loop or executor");
            return false;
        }
    }

    const rapidjson::Value* behaviors_config = nullptr;
    if (!json::pointer::GetObject(config, "/behaviors", &behaviors_config)) {
        TCLOG_ERROR("Cannot get config \"behaviors\"");
        return false;
    }

    if (!CreateBehaviorsFromConfig(*behaviors_config)) {
        TCLOG_ERROR("Fail to create behaviors from config");
        return false;
    }

    if (!RegisterBehaviors()) {
        TCLOG_ERROR("Fail to register behaviors");
        return false;
    }

    return true;
}

bool KeyManager::CreateBehaviorsFromConfig(const rapidjson::Value& behaviors_config)
{
    for (rapidjson::Value::ConstMemberIterator behavior_config_itr
                = behaviors_config.MemberBegin();
            behavior_config_itr != behaviors_config.MemberEnd();
            ++behavior_config_itr) {

        auto behavior = std::shared_ptr<Behavior>(new Behavior);
        if (!behavior) {
            TCLOG_ERROR("Fail to create Behavior");
            return false;
        }
        auto ret_pair = behaviors_.insert(
                {behavior_config_itr->name.GetString(), behavior});
        if (!ret_pair.second) {
            TCLOG_ERROR("Behavior \"%s\" already exists",
                    behavior_config_itr->name.GetString());
            return false;
        }

        for (rapidjson::Value::ConstMemberIterator key_config_itr
                    = behavior_config_itr->value.MemberBegin();
                key_config_itr != behavior_config_itr->value.MemberEnd();
                ++key_config_itr) {
            std::string key_name(key_config_itr->name.GetString());
            auto key = CreateKeyFromConfig(key_config_itr->value);
            if (!key) {
                TCLOG_ERROR("Fail to create key \"%s\"", key_name.c_str());
                return false;
            }
            auto ret_pair = keys_.insert({key_name, key});
            if (!ret_pair.second) {
                TCLOG_ERROR("Key \"%s\" already exists", key_name.c_str());
                return false;
            }
            if (behavior->keys.size() >= 32) {
                TCLOG_ERROR("The keys in behavior \"%s\" cannot be more than 32",
                        behavior_config_itr->name.GetString());
                return false;
            }
            behavior->keys.push_back(key);
        }
    }
    return true;
}

std::shared_ptr<KeyManager::Key> KeyManager::CreateKeyFromConfig(
        const rapidjson::Value& key_config)
{
    auto key = std::shared_ptr<Key>(new Key);
    if (!key) {
        TCLOG_ERROR("Cannot create Key");
        return nullptr;
    }

    const char* content_string = nullptr;
    int content_int = 0;

    if (!json::pointer::GetString(key_config, "/input_device", &content_string)) {
        TCLOG_ERROR("Cannot get key config \"input_device\"");
        return nullptr;
    }
    key->input_device = content_string;

    if (!json::pointer::GetInt(key_config, "/code", &content_int)) {
        TCLOG_ERROR("Cannot get key config \"code\"");
        return nullptr;
    }
    key->code = content_int;

    if (!json::pointer::GetString(key_config, "/motion", &content_string)) {
        TCLOG_ERROR("Cannot get key config \"motion\"");
        return nullptr;
    }
    key->motion = StringToMotion(content_string);
    if (key->motion == Motion::kUnknown) {
        TCLOG_ERROR("Not supported key motion: %s", content_string);
        return nullptr;
    }

    if (key->motion == Motion::kLongPressPreRelease
            || key->motion == Motion::kLongPressPostRelease) {
        double content_double = 0.0;
        if (!json::pointer::GetDouble(key_config, "/duration_sec", &content_double)) {
            TCLOG_ERROR("Cannot get config \"duration_sec\"");
            return nullptr;
        }
        key->duration_sec = content_double;
    }

    return key;
}

bool KeyManager::RegisterBehaviors(void)
{
    // "string" is the input device name
    std::map<std::string, std::shared_ptr<InputEventWatcher>> watchers;
    // "string" is the input device name
    std::map<std::string, std::shared_ptr<KeyInputEventHandler>> key_input_event_handlers;
    // "string" in the outer map is the input device name,
    // and "int" in the inner map is the key code.
    std::map<std::string,
        std::shared_ptr<std::map<int, std::shared_ptr<KeyHandler>>>> key_handlers;

    for (auto behavior_itr = behaviors_.begin(); behavior_itr != behaviors_.end();
            ++behavior_itr) {
        std::shared_ptr<Behavior> behavior = behavior_itr->second;
        const std::vector<std::shared_ptr<Key>>* keys = &behavior->keys;

        for (auto key_itr = keys->begin(); key_itr != keys->end(); ++key_itr) {
            const std::string& input_device = (*key_itr)->input_device;
            auto watcher_itr = watchers.find(input_device);
            if (watcher_itr == watchers.end()) {
                auto watcher = InputEventWatcher::Create(input_device);
                if (!watcher) {
                    TCLOG_ERROR("Fail to create InputEventWatcher for \"%s\"",
                            input_device.c_str());
                    return false;
                }
                watchers.insert({input_device, watcher});
                watcher_itr = watchers.find(input_device);
            }

            auto key_input_event_handler_itr = key_input_event_handlers.find(input_device);
            if (key_input_event_handler_itr == key_input_event_handlers.end()) {
                auto key_input_event_handler = KeyInputEventHandler::Create();
                if (!key_input_event_handler) {
                    TCLOG_ERROR("Fail to create KeyInputEventHandler for \"%s\"",
                            input_device.c_str());
                    return false;
                }
                key_input_event_handlers.insert({input_device, key_input_event_handler});
                watcher_itr->second->AddHandler(key_input_event_handler);
                key_input_event_handler_itr = key_input_event_handlers.find(input_device);
            }

            auto device_key_handlers_itr = key_handlers.find(input_device);
            if (device_key_handlers_itr == key_handlers.end()) {
                auto device_key_handlers =
                    std::make_shared<std::map<int, std::shared_ptr<KeyHandler>>>();
                if (!device_key_handlers) {
                    TCLOG_ERROR("Fail to create inter_key_handlers");
                    return false;
                }
                key_handlers.insert({input_device, device_key_handlers});
                device_key_handlers_itr = key_handlers.find(input_device);
            }

            int key_code = (*key_itr)->code;
            std::shared_ptr<std::map<int, std::shared_ptr<KeyHandler>>>
                device_key_handlers = device_key_handlers_itr->second;
            auto key_handler_itr = device_key_handlers->find(key_code);
            if (key_handler_itr == device_key_handlers->end()) {
                auto key_handler = KeyHandler::Create(key_code, event_loop_, executor_);
                if (!key_handler) {
                    TCLOG_ERROR("Fail to create KeyHandler for code %d", key_code);
                    return false;
                }
                device_key_handlers->insert({key_code, key_handler});
                key_input_event_handler_itr->second->AddKeyHandler(key_handler);
                key_handler->set_response_style(KeyHandler::ResponseStyle::kOneMotion);
            } else {
                // If a key handler has been created, it means that it is used in
                // more than one behavior. It should be responded in more than
                // one motion.
                key_handler_itr->second->set_response_style(
                        KeyHandler::ResponseStyle::kAllMotions);
            }
        }

        int keys_amount = keys->size();
        if (keys_amount == 1) {
            if (!RegisterSingleKeyCallback(behavior, keys, &key_handlers)) {
                TCLOG_ERROR("Fail to register callback for behavior \"%s\"",
                        behavior_itr->first.c_str());
                return false;
            }
        } else if (keys_amount > 1) {
            if (!RegisterMultiKeysCallback(behavior, keys, &key_handlers)) {
                TCLOG_ERROR("Fail to register callback for behavior \"%s\"",
                        behavior_itr->first.c_str());
                return false;
            }
        } else {
            TCLOG_ERROR("Behavior \"%s\": invalid keys amount: %d",
                    behavior_itr->first.c_str(), keys_amount);
            return false;
        }
    }

    for (auto watcher_itr = watchers.begin(); watcher_itr != watchers.end(); ++watcher_itr) {
        event_loop_->AddIoWatcher(watcher_itr->second);
    }

    return true;
}

bool KeyManager::RegisterSingleKeyCallback(std::shared_ptr<Behavior> behavior,
        const std::vector<std::shared_ptr<Key>>* keys,
        const std::map<std::string,
            std::shared_ptr<std::map<int, std::shared_ptr<KeyHandler>>>>* key_handlers)
{
    std::shared_ptr<Key> key = keys->front();
    auto device_key_handlers_itr = key_handlers->find(key->input_device);
    if (device_key_handlers_itr == key_handlers->end()) {
        TCLOG_ERROR("Cannot find key handlers in input device \"%s\"",
                key->input_device.c_str());
        return false;
    }

    std::shared_ptr<std::map<int, std::shared_ptr<KeyHandler>>>
        device_key_handlers = device_key_handlers_itr->second;
    auto key_handler_itr = device_key_handlers->find(key->code);
    if (key_handler_itr == device_key_handlers->end()) {
        TCLOG_ERROR("Cannot find key handler of code %d", key->code);
        return false;
    }
    std::shared_ptr<KeyHandler> key_handler = key_handler_itr->second;
    bool result = RegisterKeyCallback(key, key_handler, [behavior] {
            TCLOG_DEBUG("Callback of behavior with single key");
            if (behavior->callback) {
                behavior->callback();
            }
        });
    if (!result) {
        TCLOG_ERROR("Fail to register callback for key code %d", key->code);
        return false;
    }
    return true;
}

bool KeyManager::RegisterMultiKeysCallback(std::shared_ptr<Behavior> behavior,
        const std::vector<std::shared_ptr<Key>>* keys,
        const std::map<std::string,
            std::shared_ptr<std::map<int, std::shared_ptr<KeyHandler>>>>* key_handlers)
{
    for (auto key_itr = keys->begin(); key_itr != keys->end(); ++key_itr) {
        std::shared_ptr<Key> key = *key_itr;
        auto device_key_handlers_itr = key_handlers->find(key->input_device);
        if (device_key_handlers_itr == key_handlers->end()) {
            TCLOG_ERROR("Cannot find key handlers in input device \"%s\"",
                    key->input_device.c_str());
            return false;
        }
        std::shared_ptr<std::map<int, std::shared_ptr<KeyHandler>>>
            device_key_handlers = device_key_handlers_itr->second;
        auto key_handler_itr = device_key_handlers->find(key->code);
        if (key_handler_itr == device_key_handlers->end()) {
            TCLOG_ERROR("Cannot find key handler of code %d", key->code);
            return false;
        }

        std::shared_ptr<KeyHandler> key_handler = key_handler_itr->second;
        unsigned int key_index = key_itr - keys->begin();
        bool result = RegisterKeyCallback(key, key_handler, [key_index, behavior] {
                std::unique_lock<std::mutex> lock(behavior->mtx);
                TCLOG_DEBUG("Callback of behavior with multiple keys");
                behavior->responded_keys_flag |= (0x01 << key_index);
                uint32_t target_flag = (0x01 << behavior->keys.size()) - 1;
                if (behavior->responded_keys_flag == target_flag) {
                    behavior->responded_keys_flag = 0;
                    lock.unlock();
                    if (behavior->callback) {
                        behavior->callback();
                    }
                }
            });
        if (!result) {
            TCLOG_ERROR("Fail to register callback for key code %d", key->code);
            return false;
        }
    }

    // At last, traverse all the keys and theirs handlers once again to add
    // release motions and make them responded all motions.
    // Ensure that the @c responded_keys_flag will be cleaned to 0 when
    // at least one of the keys is released.
    for (auto key_itr = keys->begin(); key_itr != keys->end(); ++key_itr) {
        std::shared_ptr<Key> key = *key_itr;
        auto device_key_handlers_itr = key_handlers->find(key->input_device);
        if (device_key_handlers_itr == key_handlers->end()) {
            TCLOG_ERROR("Cannot find key handlers in input device \"%s\"",
                    key->input_device.c_str());
            return false;
        }
        auto device_key_handlers = device_key_handlers_itr->second;
        auto key_handler_itr = device_key_handlers->find(key->code);
        if (key_handler_itr == device_key_handlers->end()) {
            TCLOG_ERROR("Cannot find key handler of code %d", key->code);
            return false;
        }
        std::shared_ptr<KeyHandler> key_handler = key_handler_itr->second;
        key_handler->AddReleaseMotion([behavior] {
                std::lock_guard<std::mutex> lock(behavior->mtx);
                behavior->responded_keys_flag = 0;
            });
        key_handler->set_response_style(KeyHandler::ResponseStyle::kAllMotions);
    }

    return true;
}

bool KeyManager::RegisterKeyCallback(std::shared_ptr<Key> key,
        std::shared_ptr<KeyHandler> key_handler,
        std::function<void (void)> callback)
{
    timeval timeout = {0, 0};

    switch (key->motion) {
    case Motion::kPress:
        if (!key_handler->AddPressMotion(callback)) {
            TCLOG_ERROR("Fail to add press motion for key code %d", key->code);
            return false;
        }
        break;
    case Motion::kRelease:
        if (!key_handler->AddReleaseMotion(callback)) {
            TCLOG_ERROR("Fail to add release motion for key code %d", key->code);
            return false;
        }
        break;
    case Motion::kLongPressPreRelease:
        if (!key_handler->AddLongPressPreReleaseMotion(key->duration_sec, callback)) {
            TCLOG_ERROR("Fail to add long press motion (pre release) for key code %d",
                    key->code);
            return false;
        }
        break;
    case Motion::kLongPressPostRelease:
        if (!common::FloatingPointToTimeval(key->duration_sec, &timeout)) {
            TCLOG_ERROR("Invalid duration sec: %f", key->duration_sec);
            return false;
        }
        if (!key_handler->AddLongPressPostReleaseMotion(&timeout, callback)) {
            TCLOG_ERROR("Fail to add long press motion (post release) for key code %d",
                    key->code);
            return false;
        }
        break;
    default:
        TCLOG_ERROR("Unknown motion");
        return false;
    }
    return true;
}

bool KeyManager::SetBehaviorCallback(const std::string& behavior_name,
        std::function<void (void)> callback)
{
    auto itr = behaviors_.find(behavior_name);
    if (itr == behaviors_.end()) {
        TCLOG_ERROR("Cannot find bahavior \"%s\"", behavior_name.c_str());
        return false;
    }
    itr->second->callback = callback;
    return true;
}

bool KeyManager::StartBuiltInEventLoop(void)
{
    if (!loop_and_executor_is_built_in_) {
        TCLOG_ERROR("Event loop is not built-in");
        return false;
    }
    if (!event_loop_->Start()) {
        TCLOG_ERROR("Fail to start built-in event loop");
        return false;
    }
    return true;
}

} // namespace input
} // namespace tconfigs
