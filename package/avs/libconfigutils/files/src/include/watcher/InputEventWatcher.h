#ifndef __INPUT_EVENT_WATCHER_H__
#define __INPUT_EVENT_WATCHER_H__

#include <linux/input.h>
#include <unordered_set>
#include <map>

#include "EventWatcher.h"

namespace AW{

class InputEventWatcher : public EventWatcher::Listener,
                          public std::enable_shared_from_this<InputEventWatcher>
{
public:
    class Listener
    {
    public:
    virtual ~Listener() = default;
    virtual void onInputEvent(const struct input_event &event) = 0;
    };

    static std::shared_ptr<InputEventWatcher> create(std::shared_ptr<EventWatcher> watcher,
                                                 const std::string names);

    void release();

    std::string getInputHandlerName() { return m_name; };
    int getInputHandlerFd() { return m_fd; };

    void addKeyCodeListener(int code, std::shared_ptr<Listener> listener);
    void removeKeyCodeListener(int code, std::shared_ptr<Listener> listener);
private:
    InputEventWatcher(std::shared_ptr<EventWatcher> watcher,
                  const std::string name) :
                                 m_watcher{watcher},
                                     m_name{name}{};
    void onEvent(EventWatcher::tEventType event_type, int event_src, void *priv) override;
    bool initialize();

private:
    std::shared_ptr<EventWatcher> m_watcher; //??
    std::map<int, std::unordered_set<std::shared_ptr<Listener>>> m_listeners;
    //std::unordered_set<std::shared_ptr<Listener>> m_listeners;
    int m_fd;

    std::string m_name;
};

}/*namespace AW*/

#endif/*__INPUT_EVENT_WATCHER_H__*/
