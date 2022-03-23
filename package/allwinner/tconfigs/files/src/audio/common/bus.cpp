#include "tconfigs/audio/common/bus.h"

#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace audio {

std::shared_ptr<Bus> Bus::Create(int executor_threads)
{
    auto bus = std::shared_ptr<Bus>(new Bus(executor_threads));
    if (!bus || !bus->Init()) {
        TCLOG_ERROR("Fail to create Bus");
        return nullptr;
    }
    return bus;
}

Bus::Bus(int executor_threads)
    : executor_threads_(executor_threads)
{
}

bool Bus::Init(void)
{
    if (executor_threads_ > 0) {
        executor_ = threading::Executor::Create(executor_threads_);
        if (!executor_) {
            TCLOG_ERROR("Fail to create internal Executor in Bus");
            return false;
        }
    }

    receiver_ = MessageReceiver::Create([&](const Message& message) {
                MessageHandler(message);
            });
    if (!receiver_) {
        TCLOG_ERROR("Fail to create message_receiver");
        return false;
    }

    return true;
}

bool Bus::RegisterSender(std::shared_ptr<MessageSender> sender)
{
    if (!sender) {
        TCLOG_ERROR("Message sender cannot be nullptr");
        return false;
    }
    sender->Connect(receiver_);
    return true;
}

bool Bus::RegisterCallback(const std::string& message, std::function<void (void)> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto itr = callbacks_.find(message);
    if (itr == callbacks_.end()) {
        auto cb_struct = std::shared_ptr<Callback>(new Callback);
        auto cb_instances =
                std::shared_ptr<std::vector<std::function<void (void)>>>
                (new std::vector<std::function<void (void)>>);
        if (!cb_struct || !cb_instances) {
            TCLOG_ERROR("Fail to allocate memory");
            return false;
        }
        std::lock_guard<std::mutex> cb_struct_lock(cb_struct->mtx);
        cb_instances->push_back(callback);
        cb_struct->instances = cb_instances;
        callbacks_.insert({message, cb_struct});
    } else {
        std::lock_guard<std::mutex> callback_struct_lock(itr->second->mtx);
        itr->second->instances->push_back(callback);
    }
    return true;
}

void Bus::MessageHandler(const Message& message)
{
    auto callback = [&] {
        std::unique_lock<std::mutex> find_lock(mutex_);
        auto itr = callbacks_.find(message.content());
        if (itr != callbacks_.end()) {
            std::shared_ptr<Callback> cb_struct = itr->second;
            std::lock_guard<std::mutex> execute_lock(cb_struct->mtx);
            find_lock.unlock();
            for (auto cb_itr = cb_struct->instances->begin();
                    cb_itr != cb_struct->instances->end(); ++cb_itr) {
                (*cb_itr)();
            }
        }
    };

    if (executor_ && message.priority() != Message::Priority::kSync) {
        if (message.priority() == Message::Priority::kHigh) {
            executor_->SubmitFront(callback);
        } else {
            executor_->SubmitBack(callback);
        }
    } else {
        callback();
    }
}

} // namespace audio
} // namespace tconfigs
