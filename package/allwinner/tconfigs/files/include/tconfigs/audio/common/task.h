#ifndef __TCONFIGS_AUDIO_COMMON_TASK_H__
#define __TCONFIGS_AUDIO_COMMON_TASK_H__

#include <vector>
#include <thread>
#include <functional>
#include "tconfigs/audio/common/buffer.h"
#include "tconfigs/threading/semaphore.h"

namespace tconfigs {
namespace audio {

class Task {
public:
    typedef int (*TaskFunc)(std::shared_ptr<BufferInterface> buffer);
//    typedef std::function<int (std::shared_ptr<BufferInterface>)> TaskFuncNew;

    Task(void) = default;
    virtual ~Task(void) = default;

    void Add(TaskFunc task_func, std::shared_ptr<BufferInterface> task_buffer);
    void Notify(void);
    void Wait(void);

    virtual bool Start(void) = 0;

protected:
    bool Start(int main_sem_count, int task_sems_count);

    int num_tasks(void) { return task_funcs_.size(); }

private:
    std::vector<TaskFunc> task_funcs_;
    std::vector<std::shared_ptr<BufferInterface>> task_buffers_;
    std::vector<std::shared_ptr<std::thread>> task_threads_;
    // Semaphores for tasks
    std::vector<std::shared_ptr<threading::Semaphore>> task_sems_;
    // Semaphore for the main process, which starts the tasks
    std::shared_ptr<threading::Semaphore> main_sem_ = nullptr;
};

class PreTask : public Task {
public:
    PreTask(void) = default;
    virtual ~PreTask(void) = default;

    bool Start(void) override;
};

class PostTask : public Task {
public:
    PostTask(void) = default;
    virtual ~PostTask(void) = default;

    bool Start(void) override;
};

} // namespace audio
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_COMMON_TASK_H__ */
