#include "Element.h"

namespace AW {

int Element::action(){
    if(m_state == State::PROCESSING) return -1;
    std::unique_lock<std::mutex> lock(m_mutex);
    m_state = State::PROCESSING;
    if(m_timeout_s > 0.001) {
        m_watcher->addTimerEventWatcher(m_timeout_s, shared_from_this(), nullptr);
    }else{
        executor_action();
    }

    return 0;
}

void Element::executor_action() {
    m_executor->submit([this]() {
        if(!m_active->isEnable()) return;

        for(auto listener : m_listeners) {
            listener->onElementProcessStart(this);
        }
        process();
        for(auto listener : m_listeners) {
            listener->onElementProcessFinished(this);
        }
        std::unique_lock<std::mutex> lock(m_mutex);
        m_state = State::IDLE;
        m_processTrigger.notify_all();
    });
}

void Element::onEvent(EventWatcher::tEventType event_type, int event_src, void *priv) {
    executor_action();
}

void Element::onActiveEnable() {
}

void Element::onActivedisable() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_state = State::IDLE;
    m_processTrigger.notify_all();
}

void Element::addListener(std::shared_ptr<Listener> listener){
    m_listeners.insert(listener);
}


void Element::deleteListener(std::shared_ptr<Listener> listener) {
    m_listeners.erase(listener);
}

Element::State Element::waitForElementFinished(const std::chrono::seconds duration) {
    if(m_state == State::IDLE) return m_state;

    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_processTrigger.wait_for(lock, duration, [this]() { return m_state == State::IDLE; }));

    return m_state;
}

} /*namespace AW*/
