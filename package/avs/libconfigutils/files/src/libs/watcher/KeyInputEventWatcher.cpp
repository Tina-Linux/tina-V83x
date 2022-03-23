#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <linux/input.h>

#include "watcher/KeyInputEventWatcher.h"

namespace AW {

std::shared_ptr<KeyInputEventWatcher> KeyInputEventWatcher::create(
                                                        std::shared_ptr<EventWatcher> watcher,
                                                        std::shared_ptr<Executor> executor,
                                                        std::shared_ptr<InputEventWatcher> input,
                                                        int key,
                                                        float longclicktime,
                                                        std::shared_ptr<Listener> listener)
{
    auto keywather = std::shared_ptr<KeyInputEventWatcher>(new KeyInputEventWatcher(watcher, executor, input, key, longclicktime, listener));
    if(keywather->initialize()) {
        return keywather;
    }
    return nullptr;
}

void KeyInputEventWatcher::release()
{
    m_input->removeKeyCodeListener(m_key, shared_from_this());
}

KeyInputEventWatcher::KeyInputEventWatcher(std::shared_ptr<EventWatcher> watcher,
                                           std::shared_ptr<Executor> executor,
                                           std::shared_ptr<InputEventWatcher> input,
                                           int key,
                                           float longclicktime,
                                           std::shared_ptr<Listener> listener) :
                                                m_watcher{watcher},
                                                m_executor{executor},
                                                m_input{input},
                                                m_key{key},
                                                m_longclicktime{longclicktime},
                                                m_listener{listener}

{

}

void KeyInputEventWatcher::onEvent(EventWatcher::tEventType event_type, int event_src, void *priv)
{
    if(event_type == EventWatcher::EVENT_TIMER && m_is_longclick) {
        m_executor->submit([this]() {
            m_listener->onkeyEventLongClick(m_key, m_longclicktime);
            return 0;
        });
    }
}

void KeyInputEventWatcher::onInputEvent(const struct input_event &event)
{
    if(event.value == 1){
        if(m_longclicktime > 0.001){
            m_is_longclick = true;
            m_watcher->addTimerEventWatcher(m_longclicktime, shared_from_this());
        }
        m_executor->submit([this]() {
            m_listener->onKeyEventDown(m_key);
            return 0;
        });
    }else if(event.value == 0){
        m_is_longclick = false;
        m_executor->submit([this]() {
            m_listener->onKeyEventUp(m_key);
            return 0;
        });
    }
}

bool KeyInputEventWatcher::initialize()
{
    m_input->addKeyCodeListener(m_key, shared_from_this());

    return true;
}

}
