#ifndef __TCONFIGS_THREADING_TASK_THREAD_H__
#define __TCONFIGS_THREADING_TASK_THREAD_H__

#include "tconfigs/threading/task_queue.h"

namespace tconfigs {
namespace threading {

class TaskThread {
public:
    TaskThread(void) = delete;
    TaskThread(std::shared_ptr<TaskQueue> task_queue);
    ~TaskThread(void);

    void Start(void);

    bool IsShutdown(void) { return is_shutdown_; }

private:
    void ProcessTasksLoop(void);

    std::weak_ptr<TaskQueue> task_queue_;
    std::atomic_bool is_shutdown_{false};
    std::thread thread_;
};

} // namespace threading
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_THREADING_TASK_THREAD_H__ */
