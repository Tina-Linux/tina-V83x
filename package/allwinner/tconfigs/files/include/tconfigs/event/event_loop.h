#ifndef __TCONFIGS_EVENT_EVENT_LOOP_H__
#define __TCONFIGS_EVENT_EVENT_LOOP_H__

#include <memory>
#include <mutex>
#include <thread>
#include <list>
#include <unordered_map>

#include <ev++.h>

#include "tconfigs/event/event_watcher_interface.h"

namespace tconfigs {
namespace event {

class EventLoop : public std::enable_shared_from_this<EventLoop> {
public:
    static std::shared_ptr<EventLoop> Create(void);
    ~EventLoop(void);

    bool Start(void);
    bool Stop(void);

    bool AddIoWatcher(std::shared_ptr<EventIoWatcherInterface> watcher);
    bool RemoveIoWatcher(int fd);

    bool AddTimerWatcher(std::shared_ptr<EventTimerWatcherInterface> watcher);
    bool RemoveTimerWatcher(std::shared_ptr<EventTimerWatcherInterface> watcher);

private:
    class IoWatcher {
    public:
        static std::shared_ptr<IoWatcher> Create(struct ev_loop* loop,
                std::shared_ptr<EventIoWatcherInterface> watcher,
                std::shared_ptr<EventLoop> parent);
        ~IoWatcher(void);
        std::shared_ptr<EventIoWatcherInterface> watcher(void) { return watcher_; }
    private:
        IoWatcher(struct ev_loop* loop,
                std::shared_ptr<EventIoWatcherInterface> watcher,
                std::shared_ptr<EventLoop> parent);
        void Callback(ev::io& w, int revents);
        ev::io io_;
        std::shared_ptr<EventIoWatcherInterface> watcher_ = nullptr;
        std::weak_ptr<EventLoop> parent_;
    };

    class TimerWatcher {
    public:
        static std::shared_ptr<TimerWatcher> Create(int id, struct ev_loop* loop,
                std::shared_ptr<EventTimerWatcherInterface> watcher,
                std::shared_ptr<EventLoop> parent);
        ~TimerWatcher(void);
        std::shared_ptr<EventTimerWatcherInterface> watcher(void) { return watcher_; }
        int id(void) const { return id_; }
    private:
        TimerWatcher(int id, struct ev_loop* loop,
                std::shared_ptr<EventTimerWatcherInterface> watcher,
                std::shared_ptr<EventLoop> parent);
        void Callback(ev::timer& w, int revents);
        int id_;
        ev::timer timer_;
        std::shared_ptr<EventTimerWatcherInterface> watcher_ = nullptr;
        std::weak_ptr<EventLoop> parent_;
    };

    EventLoop(void) = default;

    bool Init(void);
    void Release(void);

    void MainLoop(void);
    void WakeUpMainLoop(void);
    void AsyncWatcherCallback(ev::async& w, int revents);

    bool GetNewTimerWatcherId(int* id);
    void RemoveTimerWatcher(int id);

    struct ev_loop* main_loop_ = nullptr;
    ev::async async_watcher_;

    std::shared_ptr<std::thread> main_loop_thread_ = nullptr;
    std::mutex mutex_;
    bool is_stopped_ = true;

    std::list<std::shared_ptr<IoWatcher>> io_watchers_;

    int new_timer_watcher_id_ = 0;
    std::unordered_map<int, std::shared_ptr<TimerWatcher>> timer_watchers_;
};

} // namespace event
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_EVENT_EVENT_LOOP_H__ */
