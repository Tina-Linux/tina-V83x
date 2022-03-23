#include "tconfigs/threading/task_thread.h"

namespace tconfigs {
namespace threading {

TaskThread::TaskThread(std::shared_ptr<TaskQueue> task_queue)
    : task_queue_(task_queue)
{
}

TaskThread::~TaskThread(void)
{
    is_shutdown_ = true;

    if (thread_.joinable()) {
        thread_.join();
    }
}

void TaskThread::Start(void)
{
    thread_ = std::thread(std::bind(&TaskThread::ProcessTasksLoop, this));
}

void TaskThread::ProcessTasksLoop(void)
{
    while (!is_shutdown_) {
        auto task_queue = task_queue_.lock();
        if (!task_queue || task_queue->IsShutdown()) {
            is_shutdown_ = true;
            break;
        }
        auto task = task_queue->PopFront();
        if (task) {
            task->operator()();
        }
    }
}

} // namespace threading
} // namespace tconfigs
