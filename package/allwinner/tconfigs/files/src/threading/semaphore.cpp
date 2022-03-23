#include "tconfigs/threading/semaphore.h"

namespace tconfigs {
namespace threading {

Semaphore::Semaphore(int count)
    : count_(count)
{
}

void Semaphore::Signal(void) {
    std::unique_lock<std::mutex> lock(mutex_);
    ++count_;
    cond_var_.notify_one();
}

void Semaphore::Wait(void) {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_var_.wait(lock, [&] { return count_ > 0; });
    --count_;
}

} // namespace threading
} // namespace tconfigs
