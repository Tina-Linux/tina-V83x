#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include <atomic>
#include <unordered_set>
#include <chrono>
#include <mutex>
#include <condition_variable>

#include "Active.h"
#include "watcher/EventWatcher.h"
#include "threading/Executor.h"

namespace AW {

class Element : public EventWatcher::Listener,
                public Active::Listener,
                public std::enable_shared_from_this<Element>
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void onElementProcessStart(Element *element){};
        virtual void onElementProcessFinished(Element *element){};
    };

    enum class State
    {
        PROCESSING,
        IDLE
    };

    Element(std::shared_ptr<Executor> executor,
            std::shared_ptr<Active> active,
            std::unordered_set<std::shared_ptr<Listener>> listeners) :
                m_executor{executor},
                m_active{active},
                m_listeners{listeners} {};

    Element(std::shared_ptr<EventWatcher> watcher,
            float timeout_s,
            std::shared_ptr<Executor> executor,
            std::shared_ptr<Active> active,
            std::unordered_set<std::shared_ptr<Listener>> listeners) :
                m_watcher{watcher},
                m_timeout_s{timeout_s},
                m_executor{executor},
                m_active{active},
                m_listeners{listeners} {};

    virtual ~Element() = default;

    virtual void process() = 0;

    int action();

    void addListener(std::shared_ptr<Listener> listener);
    void deleteListener(std::shared_ptr<Listener> listener);

    State waitForElementFinished(const std::chrono::seconds duration);

    State getState() { return m_state; };

private:
    void onActiveEnable();
    void onActivedisable();
private:
    void executor_action();
    void onEvent(EventWatcher::tEventType event_type, int event_src, void *priv);

private:
    std::shared_ptr<EventWatcher> m_watcher;
    std::shared_ptr<Executor> m_executor;

    std::shared_ptr<Active> m_active;  //should add a ActiveManager? Ctr the all elemets avtivity?

    std::unordered_set<std::shared_ptr<Listener>> m_listeners;

    float m_timeout_s = 0.0;

    std::mutex m_mutex;
    std::condition_variable m_processTrigger;
    std::atomic<State> m_state{State::IDLE};
};

} /*namespace AW*/
#endif /*__ELEMENT_H__*/
