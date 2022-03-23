#ifndef __TCONFIGS_INPUT_KEY_MANAGER_H__
#define __TCONFIGS_INPUT_KEY_MANAGER_H__

#include <memory>
#include <map>
#include "tconfigs/json/json_utils.h"
#include "tconfigs/event/event_loop.h"
#include "tconfigs/threading/executor.h"
#include "tconfigs/input/key_handler.h"

namespace tconfigs {
namespace input {

class KeyManager {
public:
    static std::shared_ptr<KeyManager> Create(const rapidjson::Value& config);
    static std::shared_ptr<KeyManager> Create(
            std::shared_ptr<event::EventLoop> event_loop,
            std::shared_ptr<threading::Executor> executor,
            const rapidjson::Value& config);
    ~KeyManager(void) = default;

    bool SetBehaviorCallback(const std::string& behavior_name,
            std::function<void (void)> callback);
    bool StartBuiltInEventLoop(void);

private:
    enum class Motion {
        kUnknown,
        kPress,
        kRelease,
        kLongPressPreRelease,
        kLongPressPostRelease,
    };

    struct Key {
        std::string input_device;
        int code;
        Motion motion;
        double duration_sec;
    };

    struct Behavior {
        std::vector<std::shared_ptr<Key>> keys;
        std::function<void (void)> callback;
        std::mutex mtx;

        // This flag is a bitmap that indicates which key has been responded
        // (Only used in the behavior with multiple keys). The bit index
        // corresponds to the index of vector "keys". For example, if keys[0]
        // is responded, the bit 0 in this flag will be set to 1.
        // This flag is uint32_t, therefore one behavior has 32 keys at most.
        uint32_t responded_keys_flag = 0;
    };

    KeyManager(void);
    KeyManager(std::shared_ptr<event::EventLoop> event_loop,
            std::shared_ptr<threading::Executor> executor);

    bool Init(const rapidjson::Value& config);

    bool CreateBehaviorsFromConfig(const rapidjson::Value& behaviors_config);
    bool RegisterBehaviors(void);

    std::shared_ptr<Key> CreateKeyFromConfig(const rapidjson::Value& key_config);

    bool RegisterSingleKeyCallback(std::shared_ptr<Behavior> behavior,
            const std::vector<std::shared_ptr<Key>>* keys,
            const std::map<std::string,
                std::shared_ptr<std::map<int, std::shared_ptr<KeyHandler>>>>* key_handlers);
    bool RegisterMultiKeysCallback(std::shared_ptr<Behavior> behavior,
            const std::vector<std::shared_ptr<Key>>* keys,
            const std::map<std::string,
                std::shared_ptr<std::map<int, std::shared_ptr<KeyHandler>>>>* key_handlers);
    bool RegisterKeyCallback(std::shared_ptr<Key> key,
            std::shared_ptr<KeyHandler> key_handler,
            std::function<void (void)> callback);

    static Motion StringToMotion(const std::string& motion_string);

    std::shared_ptr<event::EventLoop> event_loop_ = nullptr;
    std::shared_ptr<threading::Executor> executor_ = nullptr;
    bool loop_and_executor_is_built_in_ = false;

    std::map<std::string, std::shared_ptr<Key>> keys_;
    std::map<std::string, std::shared_ptr<Behavior>> behaviors_;

    static const int kDefaultBuiltInExecutorThreads;
    static const std::map<std::string, Motion> kStringToMotionMap;
};

} // namespace input
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_INPUT_KEY_MANAGER_H__ */
