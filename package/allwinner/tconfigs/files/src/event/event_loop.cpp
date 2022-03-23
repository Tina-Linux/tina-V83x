#include "tconfigs/event/event_loop.h"

#include <limits>

#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace event {

std::shared_ptr<EventLoop> EventLoop::Create(void)
{
    auto event_loop = std::shared_ptr<EventLoop>(new EventLoop());
    if (!event_loop || !event_loop->Init()) {
        TCLOG_ERROR("Fail to create EventLoop");
        return nullptr;
    }
    return event_loop;
}

EventLoop::~EventLoop(void)
{
    Release();
}

bool EventLoop::Init(void)
{
    main_loop_ = ev_loop_new(EVBACKEND_EPOLL | EVFLAG_NOENV);
    if (!main_loop_) {
        TCLOG_ERROR("No epoll found here");
        return false;
    }
    return true;
}

void EventLoop::Release(void)
{
    if (!is_stopped_) {
        Stop();
    }
    if (main_loop_) {
        ev_loop_destroy(main_loop_);
        main_loop_ = nullptr;
    }
}

bool EventLoop::Start(void)
{
    is_stopped_ = false;

    async_watcher_.set(main_loop_);
    async_watcher_.set<EventLoop, &EventLoop::AsyncWatcherCallback>(this);
    async_watcher_.start();

    main_loop_thread_ = std::shared_ptr<std::thread>(
            new std::thread(std::bind(&EventLoop::MainLoop, this)));
    if (!main_loop_thread_) {
        TCLOG_ERROR("Fail to create EventLoop main loop thread");
        return false;
    }
    TCLOG_DEBUG("EventLoop start");
    return true;
}

bool EventLoop::Stop(void)
{
    if (!main_loop_thread_) {
        TCLOG_ERROR("EventLoop main loop thread has not been created");
        return false;
    }

    if (!io_watchers_.empty()) {
        io_watchers_.clear();
        WakeUpMainLoop();
    }
    if (!timer_watchers_.empty()) {
        timer_watchers_.clear();
        WakeUpMainLoop();
    }

    std::unique_lock<std::mutex> mutex_lock(mutex_);
    is_stopped_ = true;
    mutex_lock.unlock();

    WakeUpMainLoop();

    if (main_loop_thread_->joinable()) {
        main_loop_thread_->join();
    }

    TCLOG_DEBUG("EventLoop stop");
    return true;
}

void EventLoop::MainLoop(void)
{
    TCLOG_DEBUG("EventLoop run");
    ev_run(main_loop_, 0);
    TCLOG_DEBUG("EventLoop exit");
}

void EventLoop::WakeUpMainLoop(void)
{
    if (!async_watcher_.is_pending()) {
        async_watcher_.send();
    }
}

void EventLoop::AsyncWatcherCallback(ev::async& w, int revents)
{
    std::unique_lock<std::mutex> mutex_lock(mutex_);
    if (is_stopped_) {
        w.stop();
        ev_break(main_loop_);
        TCLOG_DEBUG("EventLoop break");
    }
}

bool EventLoop::AddIoWatcher(std::shared_ptr<EventIoWatcherInterface> watcher)
{
    auto io_watcher = IoWatcher::Create(main_loop_, watcher, shared_from_this());
    if (!io_watcher) {
        TCLOG_ERROR("Fail to create IoWatcher");
        return false;
    }
    io_watchers_.push_back(io_watcher);
    WakeUpMainLoop();
    TCLOG_DEBUG("Add io watcher (fd: %d)", watcher->GetFd());
    return true;
}

bool EventLoop::RemoveIoWatcher(int fd)
{
    for (auto itr = io_watchers_.begin(); itr != io_watchers_.end(); ++itr) {
        if ((*itr)->watcher()->GetFd() == fd) {
            TCLOG_DEBUG("Remove io watcher (fd: %d)", fd);
            io_watchers_.erase(itr);
            WakeUpMainLoop();
            return true;
        }
    }
    TCLOG_DEBUG("No io watcher is bound to file descriptor %d", fd);
    return false;
}

bool EventLoop::GetNewTimerWatcherId(int* id)
{
    int current = new_timer_watcher_id_;
    while (timer_watchers_.find(new_timer_watcher_id_) != timer_watchers_.end()) {
        ++new_timer_watcher_id_;
        if (new_timer_watcher_id_ >= std::numeric_limits<int>::max()) {
            new_timer_watcher_id_ = 0;
        }
        if (new_timer_watcher_id_ == current) {
            TCLOG_ERROR("All timer watcher identifiers are being used");
            return false;
        }
    }
    *id = new_timer_watcher_id_;
    return true;
}

bool EventLoop::AddTimerWatcher(std::shared_ptr<EventTimerWatcherInterface> watcher)
{
    int id;
    if (!GetNewTimerWatcherId(&id)) {
        TCLOG_ERROR("Fail to get new timer watcher id");
        return false;
    }
    auto timer_watcher = TimerWatcher::Create(id, main_loop_, watcher, shared_from_this());
    if (!timer_watcher) {
        TCLOG_ERROR("Fail to create TimerWatcher");
        return false;
    }
    timer_watchers_.insert({id, timer_watcher});
    WakeUpMainLoop();
    TCLOG_DEBUG("Add timer watcher (id: %d)", id);
    return true;
}

bool EventLoop::RemoveTimerWatcher(std::shared_ptr<EventTimerWatcherInterface> watcher)
{
    for (auto itr = timer_watchers_.begin(); itr != timer_watchers_.end(); ++itr) {
        if (itr->second->watcher() == watcher) {
            TCLOG_DEBUG("Remove timer watcher (id: %d)", itr->second->id());
            timer_watchers_.erase(itr);
            WakeUpMainLoop();
            return true;
        }
    }
    TCLOG_DEBUG("No corresponding timer watcher to remove");
    return false;
}

void EventLoop::RemoveTimerWatcher(int id)
{
    timer_watchers_.erase(id);
}

// IoWatcher ===================================================================
std::shared_ptr<EventLoop::IoWatcher> EventLoop::IoWatcher::Create(
        struct ev_loop* loop, std::shared_ptr<EventIoWatcherInterface> watcher,
        std::shared_ptr<EventLoop> parent)
{
    return std::shared_ptr<IoWatcher>(new IoWatcher(loop, watcher, parent));
}

EventLoop::IoWatcher::IoWatcher(struct ev_loop* loop,
        std::shared_ptr<EventIoWatcherInterface> watcher,
        std::shared_ptr<EventLoop> parent)
    : watcher_(watcher),
      parent_(parent)
{
    int events = ev::UNDEF;
    switch (watcher->GetEventType()) {
    case EventIoWatcherInterface::EventType::kReadable:
        events = ev::READ;
        break;
    case EventIoWatcherInterface::EventType::kWritable:
        events = ev::WRITE;
        break;
    case EventIoWatcherInterface::EventType::kReadableOrWritable:
        events = ev::READ | ev::WRITE;
        break;
    default:
        break;
    }

    io_.set(loop);
    io_.set<IoWatcher, &IoWatcher::Callback>(this);
    io_.start(watcher->GetFd(), events);
}

EventLoop::IoWatcher::~IoWatcher(void)
{
    io_.stop();
}

void EventLoop::IoWatcher::Callback(ev::io& w, int revents)
{
    if (!watcher_) {
        TCLOG_ERROR("Invalid io watcher (fd: %d)", w.fd);
        return;
    }

    EventIoWatcherInterface::EventType event_type;
    switch (revents) {
    case ev::READ:
        event_type = EventIoWatcherInterface::EventType::kReadable;
        break;
    case ev::WRITE:
        event_type = EventIoWatcherInterface::EventType::kWritable;
        break;
    case (ev::READ | ev::WRITE):
        // Will it enter this condition?
        event_type = EventIoWatcherInterface::EventType::kReadableOrWritable;
        break;
    default:
        TCLOG_ERROR("Incorrect events: %d", revents);
        return;
    }
    watcher_->OnEvent(w.fd, event_type);

    if (watcher_->IsOneShot()) {
        auto parent = parent_.lock();
        if (!parent) {
            TCLOG_ERROR("Invalid parent EventLoop");
            return;
        }
        TCLOG_DEBUG("Remove IoWatcher (fd: %d)", w.fd);
        parent->RemoveIoWatcher(w.fd);
        // NOTE:
        //  After RemoveIoWatcher(), this instance has been destructed.
        //  Do not use any of its members at all.
    }
}

// TimerWatcher ================================================================
std::shared_ptr<EventLoop::TimerWatcher> EventLoop::TimerWatcher::Create(
        int id, struct ev_loop* loop,
        std::shared_ptr<EventTimerWatcherInterface> watcher,
        std::shared_ptr<EventLoop> parent)
{
    return std::shared_ptr<TimerWatcher>(
            new TimerWatcher(id, loop, watcher, parent));
}

EventLoop::TimerWatcher::TimerWatcher(int id, struct ev_loop* loop,
        std::shared_ptr<EventTimerWatcherInterface> watcher,
        std::shared_ptr<EventLoop> parent)
    : id_(id),
      watcher_(watcher),
      parent_(parent)
{
    timer_.set(loop);
    timer_.set<TimerWatcher, &TimerWatcher::Callback>(this);
    timer_.start(watcher_->GetTimeoutSeconds(), watcher_->GetRepeatIntervalSeconds());
}

EventLoop::TimerWatcher::~TimerWatcher(void)
{
    timer_.stop();
}

void EventLoop::TimerWatcher::Callback(ev::timer& w, int revents)
{
    if (!watcher_) {
        TCLOG_ERROR("Invalid timer watcher");
        return;
    }
    watcher_->OnEvent(&(w.repeat));

    if (!w.is_active()) {
        auto parent = parent_.lock();
        if (!parent) {
            TCLOG_ERROR("Invalid parent EventLoop");
            return;
        }
        TCLOG_DEBUG("Remove timer watcher (id: %d)", id_);
        parent->RemoveTimerWatcher(id_);
        // NOTE:
        //  After RemoveTimerWatcher(id_), this instance has been destructed.
        //  Do not use any of its members at all.
        TCLOG_DEBUG("After removing timer watcher, id becomes %d", id_);
    }
}

} // namespace event
} // namespace tconfigs
