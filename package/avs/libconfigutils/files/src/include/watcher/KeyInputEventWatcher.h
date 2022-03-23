#ifndef __KEY_INPUT_EVENT_WATCHER_H__
#define __KEY_INPUT_EVENT_WATCHER_H__

#include <linux/input.h>
#include <unordered_set>
#include <map>

#include "threading/Executor.h"
#include "watcher/InputEventWatcher.h"

namespace AW{

class KeyInputEventWatcher : public EventWatcher::Listener,
                             public InputEventWatcher::Listener,
                             public std::enable_shared_from_this<KeyInputEventWatcher>
{
public:
    class Listener{
    public:
        virtual ~Listener() = default;
        virtual void onKeyEventDown(int keycode){};
        virtual void onKeyEventUp(int keycode){};
        virtual void onkeyEventLongClick(int keycode, float longclicktime){};
    };

    static std::shared_ptr<KeyInputEventWatcher> create(std::shared_ptr<EventWatcher> watcher,
                                                        std::shared_ptr<Executor> executor,
                                                        std::shared_ptr<InputEventWatcher> input,
                                                        int key,
                                                        float longclicktime,
                                                        std::shared_ptr<Listener> listener);
    void release();
private:
    KeyInputEventWatcher(std::shared_ptr<EventWatcher> watcher,
                         std::shared_ptr<Executor> executor,
                         std::shared_ptr<InputEventWatcher> input,
                         int key,
                         float longclicktime,
                         std::shared_ptr<Listener> listener);
    void onEvent(EventWatcher::tEventType event_type, int event_src, void *priv) override;
    void onInputEvent(const struct input_event &event) override;
    bool initialize();
private:
    std::shared_ptr<EventWatcher> m_watcher;
    std::shared_ptr<Executor> m_executor;
    std::shared_ptr<InputEventWatcher> m_input;
    std::shared_ptr<Listener> m_listener;
    float m_longclicktime;
    int m_key;

    bool m_is_longclick = false;
};

}/*namespace AW*/

#endif/*__KEY_INPUT_EVENT_WATCHER_H__*/
