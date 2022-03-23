#include "tconfigs/audio/common/task.h"

namespace tconfigs {
namespace audio {

void Task::Add(TaskFunc task_func, std::shared_ptr<BufferInterface> buffer)
{
    task_funcs_.push_back(task_func);
    task_buffers_.push_back(buffer);
}

void Task::Notify(void)
{
    for (auto itr = task_sems_.begin(); itr != task_sems_.end(); ++itr) {
        (*itr)->Signal();
    }
}

void Task::Wait(void)
{
    if (main_sem_) {
        for (int i = 0; i < static_cast<int>(task_funcs_.size()); ++i) {
            main_sem_->Wait();
        }
    }
}

bool Task::Start(int main_sem_count, int task_sems_count)
{
    main_sem_ = std::shared_ptr<threading::Semaphore>(
            new threading::Semaphore(main_sem_count));
    if (!main_sem_) {
        return false;
    }

    int num_tasks = task_funcs_.size();
    if (num_tasks <= 0) {
        return false;
    }
    for (int i = 0; i < num_tasks; ++i) {
        auto task_sem = std::shared_ptr<threading::Semaphore>(
                new threading::Semaphore(task_sems_count));
        if (!task_sem) {
            return false;
        }
        task_sems_.push_back(task_sem);
    }
    for (int i = 0; i < num_tasks; ++i) {
        auto task_thread = std::shared_ptr<std::thread>(
                new std::thread([&](std::shared_ptr<BufferInterface> buffer) {
                                    while (1) {
                                        task_sems_[i]->Wait();
                                        task_funcs_[i](buffer);
                                        main_sem_->Signal();
                                    }
                                },
                                task_buffers_[i]));
        task_threads_.push_back(task_thread);
    }
    return true;
}

bool PreTask::Start(void)
{
    return Task::Start(0, 1);
}

bool PostTask::Start(void)
{
    return Task::Start(num_tasks(), 0);
}

} // namespace audio
} // namespace tconfigs
