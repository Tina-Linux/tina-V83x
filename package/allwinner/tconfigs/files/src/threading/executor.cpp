#include "tconfigs/threading/executor.h"

#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace threading {

std::shared_ptr<Executor> Executor::Create(int threads)
{
    auto executor = std::shared_ptr<Executor>(new Executor(threads));
    if (!executor) {
        TCLOG_ERROR("Fail to create Executor");
        return nullptr;
    }
    executor->task_thread_pool_->Start();
    return executor;
}

Executor::Executor(int threads)
    : task_queue_(std::shared_ptr<TaskQueue>(new TaskQueue())),
      task_thread_pool_(std::unique_ptr<TaskThreadPool>(
                  new TaskThreadPool(task_queue_, threads)))
{
}

Executor::~Executor(void)
{
    Shutdown();
}

void Executor::WaitForSubmittedTasks(void)
{
    std::promise<void> flushed_promise;
    auto flushed_future = flushed_promise.get_future();
    auto task = [&flushed_promise] { flushed_promise.set_value(); };
    SubmitBack(task);
    flushed_future.get();
}

void Executor::Shutdown(void)
{
    task_queue_->Shutdown();
    task_thread_pool_.reset();
}

bool Executor::IsShutdown()
{
    return task_queue_->IsShutdown();
}

} // namespace threading
} // namespace tconfigs
