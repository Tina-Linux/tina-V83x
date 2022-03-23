#ifndef __TCONFIGS_AUDIO_COMMON_BUS_H__
#define __TCONFIGS_AUDIO_COMMON_BUS_H__

#include <string>
#include <memory>
#include <map>
#include <mutex>
#include "tconfigs/threading/executor.h"
#include "tconfigs/audio/common/message.h"

namespace tconfigs {
namespace audio {

class Bus {
public:
    static std::shared_ptr<Bus> Create(int executor_threads);

    ~Bus(void) = default;

    bool RegisterSender(std::shared_ptr<MessageSender> sender);
    bool RegisterCallback(const std::string& message, std::function<void (void)> callback);

private:
    struct Callback {
        std::mutex mtx;
        std::shared_ptr<std::vector<std::function<void (void)>>> instances;
    };

    Bus(void) = delete;
    Bus(int executor_threads);

    bool Init(void);
    void MessageHandler(const Message& message);

    int executor_threads_ = 1;
    std::shared_ptr<threading::Executor> executor_ = nullptr;

    std::shared_ptr<MessageReceiver> receiver_ = nullptr;

    std::map<std::string, std::shared_ptr<Callback>> callbacks_;

    std::mutex mutex_;
};

} // namespace audio
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_COMMON_BUS_H__ */
