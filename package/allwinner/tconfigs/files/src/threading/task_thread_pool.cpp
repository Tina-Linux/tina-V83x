#include "tconfigs/threading/task_thread_pool.h"

namespace tconfigs {
namespace threading {

TaskThreadPool::TaskThreadPool(std::shared_ptr<TaskQueue> task_queue, int threads)
{
    for (int i = 0; i < threads; ++i) {
        task_threads_.emplace_back(new TaskThread(task_queue));
    }
}

void TaskThreadPool::Start(void)
{
    for (auto itr = task_threads_.begin(); itr != task_threads_.end(); ++itr) {
        (*itr)->Start();
    }
}

bool TaskThreadPool::IsShutdown(void)
{
    for (auto itr = task_threads_.begin(); itr != task_threads_.end(); ++itr) {
        if (!(*itr)->IsShutdown()) {
            return false;
        }
    }
    return true;
}

} // namespace threading
} // namespace tconfigs
