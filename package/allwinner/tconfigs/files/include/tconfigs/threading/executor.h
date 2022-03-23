#ifndef __TCONFIGS_THREADING_EXECUTOR_H__
#define __TCONFIGS_THREADING_EXECUTOR_H__

#include "tconfigs/threading/task_thread_pool.h"

namespace tconfigs {
namespace threading {

/**
 * An Executor is used to run callable types asynchronously.
 */
class Executor {
public:
    static std::shared_ptr<Executor> Create(int threads);

    ~Executor(void);

    /**
     * Submits a callable type (function, lambda expression, bind expression,
     * or another function object) to the back of the internal queue to be
     * executed on an Executor thread. The future must be checked for validity
     * before waiting on it.
     */
    template <typename Task, typename... Args>
    auto SubmitBack(Task task, Args&&... args) -> std::future<decltype(task(args...))>;

    /**
     * Similar to @c SubmitBack, but submits a callback type to the front of the
     * internal queue.
     */
    template <typename Task, typename... Args>
    auto SubmitFront(Task task, Args&&... args) -> std::future<decltype(task(args...))>;

    void WaitForSubmittedTasks(void);

    void Shutdown(void);

    bool IsShutdown(void);

private:
    Executor(void) = delete;
    Executor(int threads);

    std::shared_ptr<TaskQueue> task_queue_ = nullptr;
    std::unique_ptr<TaskThreadPool> task_thread_pool_ = nullptr;
};

template <typename Task, typename... Args>
auto Executor::SubmitBack(Task task, Args&&... args) -> std::future<decltype(task(args...))>
{
    return task_queue_->PushBack(task, std::forward<Args>(args)...);
}

template <typename Task, typename... Args>
auto Executor::SubmitFront(Task task, Args&&... args) -> std::future<decltype(task(args...))>
{
    return task_queue_->PushFront(task, std::forward<Args>(args)...);
}

} // namespace threading
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_THREADING_EXECUTOR_H__ */
