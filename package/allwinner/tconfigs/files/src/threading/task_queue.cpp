#include "tconfigs/threading/task_queue.h"

namespace tconfigs {
namespace threading {

std::unique_ptr<std::function<void (void)>> TaskQueue::PopFront(void)
{
    std::unique_lock<std::mutex> queue_lock(queue_mutex_);
    if (!is_shutdown_ && queue_.empty()) {
        queue_changed_.wait(queue_lock,
                [&] { return is_shutdown_ || !queue_.empty(); });
    }
    if (!queue_.empty()) {
        auto task = std::move(queue_.front());
        queue_.pop_front();
        return task;
    }
    return nullptr;
}

void TaskQueue::Shutdown(void)
{
    std::lock_guard<std::mutex> queue_lock(queue_mutex_);
    queue_.clear();
    is_shutdown_ = true;
    queue_changed_.notify_all();
}

} // namespace threading
} // namespace tconfigs
