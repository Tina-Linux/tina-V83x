#ifndef __TCONFIGS_EVENT_EVENT_WATCHER_INTERFACE_H__
#define __TCONFIGS_EVENT_EVENT_WATCHER_INTERFACE_H__

namespace tconfigs {
namespace event {

class EventIoWatcherInterface {
public:
    enum class EventType {
        kReadable,
        kWritable,
        kReadableOrWritable
    };

    virtual ~EventIoWatcherInterface(void) = default;

    virtual int GetFd(void) = 0;    // which file descriptor to be watched
    virtual EventType GetEventType(void) = 0;
    virtual bool IsOneShot(void) = 0;   // concerns the event only once

    virtual void OnEvent(int fd, EventType event_type) = 0;
};

class EventTimerWatcherInterface {
public:
    virtual ~EventTimerWatcherInterface(void) = default;

    // after how many seconds it will generate an event.
    // If it returns 0, the event will be generated right now.
    virtual double GetTimeoutSeconds(void) = 0;

    // the interval seconds that it will generate events repeatedly.
    // If it returns 0, the event will be generated once.
    virtual double GetRepeatIntervalSeconds(void) = 0;

    // @param[in&out] repeat_interval_sec
    //      The repeat interval seconds can be changed midway by modifying its value
    virtual void OnEvent(double* repeat_interval_sec) = 0;
};

} // namespace event
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_EVENT_EVENT_WATCHER_INTERFACE_H__ */
