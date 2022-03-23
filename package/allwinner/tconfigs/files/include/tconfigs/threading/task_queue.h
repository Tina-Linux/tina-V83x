#ifndef __TCONFIGS_THREADING_TASK_QUEUE_H__
#define __TCONFIGS_THREADING_TASK_QUEUE_H__

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <future>
#include <memory>
#include <functional>
#include <queue>

namespace tconfigs {
namespace threading {

class TaskQueue {
public:
    TaskQueue(void) = default;

    /**
     * Push a task on the back of the queue. If the queue is shutdown, the task
     * will be dropped, and an invalid future will be returned.
     */
    template <typename Task, typename... Args>
    auto PushBack(Task task, Args&&... args) -> std::future<decltype(task(args...))>;

    /**
     * Similar to @c PushBack, but push a task on the front of the queue.
     */
    template <typename Task, typename... Args>
    auto PushFront(Task task, Args&&... args) -> std::future<decltype(task(args...))>;

    /**
     * Returns and removes the task at the front of the queue. If there are no
     * tasks, this call will block until there is one.
     *
     * @return A task which the caller assumes ownership of, or @c nullptr if
     * the queue expects no more tasks.
     */
    std::unique_ptr<std::function<void (void)>> PopFront(void);

    /**
     * Clears the queue of outstanding tasks and refuses any additional tasks to
     * be pushed onto the queue. Must be called by task enqueuers when no more
     * tasks will be enqueued.
     */
    void Shutdown(void);

    /**
     * Return whether or not the queue is shutdown.
     */
    bool IsShutdown(void) { return is_shutdown_; }

private:
    template <typename Task, typename... Args>
    auto PushTo(bool is_front, Task task, Args&&... args)
        -> std::future<decltype(task(args...))>;

    std::deque<std::unique_ptr<std::function<void (void)>>> queue_;

    std::condition_variable queue_changed_;
    std::mutex queue_mutex_;

    std::atomic_bool is_shutdown_{false};
};

/**
 * Utility function which waits for a @c std::future to be fulfilled and
 * forward the result to a @c std::promise.
 *
 * @param promise The @c std::promise to fulfill when @c future is fulfilled.
 * @param future The @c std::future on which to wait for a result to forward to @c promise.
 */
template <typename T>
inline void ForwardPromise(std::shared_ptr<std::promise<T>> prms, std::future<T>* ftr)
{
    prms->set_value(ftr->get());
}

/**
 * Specialization of @c forwardPromise() for @c void types.
 */
template <>
inline void ForwardPromise(std::shared_ptr<std::promise<void>> prms, std::future<void>* ftr)
{
    ftr->get();
    prms->set_value();
}

template <typename Task, typename... Args>
auto TaskQueue::PushBack(Task task, Args&&... args) -> std::future<decltype(task(args...))>
{
    return PushTo(false, std::forward<Task>(task), std::forward<Args>(args)...);
}

template <typename Task, typename... Args>
auto TaskQueue::PushFront(Task task, Args&&... args) -> std::future<decltype(task(args...))>
{
    return PushTo(true, std::forward<Task>(task), std::forward<Args>(args)...);
}

template <typename Task, typename... Args>
auto TaskQueue::PushTo(bool is_front, Task task, Args&&... args)
    -> std::future<decltype(task(args...))>
{
    auto bound_task = std::bind(std::forward<Task>(task), std::forward<Args>(args)...);

    /*
     * Create a std::packaged_task with the correct return type. The decltype
     * only returns the return value of the bound_task. The following
     * parentheses make it a function call with the boundTask return type. The
     * package task will then return a future of the correct type.
     *
     * Note: A std::packaged_task fulfills its future *during* the call to
     * operator().  If the user of a std::packaged_task hands it off to another
     * thread to execute, and then waits on the future, they will be able to
     * retrieve the return value from the task and know that the task has
     * executed, but they do not know exactly when the task object has been
     * deleted.  This distinction can be significant if the packaged task is
     * holding onto resources that need to be freed (through a std::shared_ptr
     * for example).  If the user needs to wait for those resources to be freed
     * they have no way of knowing how long to wait.  The translated_task lambda
     * below is a workaround for this limitation.  It executes the packaged task,
     * then disposes of it before passing the task's return value back to the
     * future that the user is waiting on.
     */
    using PackagedTaskType = std::packaged_task<decltype(bound_task())()>;
    auto pkg_task = std::shared_ptr<PackagedTaskType>(new PackagedTaskType(bound_task));

    // Create a promise/future that we will fulfill when we have cleaned up the task.
    using PromiseType = std::promise<decltype(task(args...))>;
    auto cleanup_promise = std::shared_ptr<PromiseType>(new PromiseType());
    auto cleanup_future = cleanup_promise->get_future();

    // Remove the return type from the task by wrapping it in a lambda with no
    // return value.
    auto translated_task = [pkg_task, cleanup_promise]() mutable {
        // Execute the task.
        pkg_task->operator()();
        // Note the future for the task's result.
        auto task_future = pkg_task->get_future();
        // Clean up the task.
        pkg_task.reset();
        // Forward the task's result to our cleanup promise/future.
        ForwardPromise(cleanup_promise, &task_future);
    };

    // Release our local reference to packaged task so that the only remaining
    // reference is inside the lambda.
    pkg_task.reset();

    {
        std::lock_guard<std::mutex> queue_lock(queue_mutex_);
        if (is_shutdown_) {
            return std::future<decltype(task(args...))>();
        }
        queue_.emplace(is_front ? queue_.begin() : queue_.end(),
                new std::function<void (void)>(translated_task));
    }

    queue_changed_.notify_all();
    return cleanup_future;
}

} // namespace threading
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_THREADING_TASK_QUEUE_H__ */
