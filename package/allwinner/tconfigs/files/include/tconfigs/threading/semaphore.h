#ifndef __TCONFIGS_THREADING_SEMAPHORE_H__
#define __TCONFIGS_THREADING_SEMAPHORE_H__

#include <mutex>
#include <condition_variable>

namespace tconfigs {
namespace threading {

class Semaphore {
public:
    Semaphore(void) = default;
    Semaphore(int count);
    Semaphore(const Semaphore&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;
    ~Semaphore(void) = default;

    void Signal(void);
    void Wait(void);

private:
    std::mutex mutex_;
    std::condition_variable cond_var_;
    int count_ = 0;
};

} // namespace threading
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_THREADING_SEMAPHORE_H__ */
