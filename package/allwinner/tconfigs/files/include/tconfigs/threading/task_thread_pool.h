#ifndef __TCONFIGS_THREADING_TASK_THREAD_POOL_H__
#define __TCONFIGS_THREADING_TASK_THREAD_POOL_H__

#include "tconfigs/threading/task_thread.h"

namespace tconfigs {
namespace threading {

class TaskThreadPool {
public:
    TaskThreadPool(void) = delete;
    TaskThreadPool(std::shared_ptr<TaskQueue> task_queue, int threads);
    ~TaskThreadPool(void) = default;

    void Start(void);

    bool IsShutdown(void);

private:
    std::vector<std::unique_ptr<TaskThread>> task_threads_;
};

} // namespace threading
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_THREADING_TASK_THREAD_POOL_H__ */
