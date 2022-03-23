#ifndef  __SHOW_H__
#define __SHOW_H__

#include <memory>
#include <string>
#include <unordered_set>
#include <map>
#include <mutex>

#include <stdio.h>

#include <json-c/json.h>

#include "doa/DOAInfo.h"

#include "Active.h"
#include "watcher/EventWatcher.h"
#include "leds/LedPad.h"

#include "leds/Led.h"

namespace AW {

/*
class SubShow
{
public:
    virtual ~SubShow() = default;
    virtual void sub_action(int target, int index, std::shared_ptr<LedGroup> group) = 0;
    virtual void sub_action_finished() = 0;
    virtual void sub_action_reset(std::shared_ptr<LedPad> ledpad) = 0;
    virtual void sub_remove(std::shared_ptr<LedPad> ledpad) = 0;
    virtual void sub_insert(std::shared_ptr<LedPad> ledpad) = 0;
};
*/
class Show : public EventWatcher::Listener, public std::enable_shared_from_this<Show>
{
public:
    Show(std::shared_ptr<EventWatcher> watcher, std::shared_ptr<LedPad> ledpad);
    virtual ~Show() = default;

    void addSubShow(std::shared_ptr<Show> show);
    void removeSubShow(std::shared_ptr<Show> show);

    int start();
    void stop();
    void reset();

protected:
    virtual void action_start(int &doa_target){};
    virtual void action(int index, std::shared_ptr<LedGroup> group) = 0;
    virtual void action_finished(float &next_show_sec_later) = 0;
    virtual void action_reset(std::shared_ptr<LedPad> ledpad) = 0;

    virtual void sub_action_start(int doa_target){};
    virtual void sub_action(int target, int index, std::shared_ptr<LedGroup> group){};
    virtual void sub_action_finished(){};
    virtual void sub_action_reset(std::shared_ptr<LedPad> ledpad){};
    virtual void sub_remove(std::shared_ptr<LedPad> ledpad){};
    virtual void sub_insert(std::shared_ptr<LedPad> ledpad){};

    void do_led_pad();
    Active m_active;
    int m_target = 0;
    std::shared_ptr<LedPad> m_ledpad;

private:
    void onEvent(EventWatcher::tEventType event_type, int event_src, void *priv);

private:
    std::shared_ptr<EventWatcher> m_watcher;

    std::mutex m_mutex;
    std::unordered_set<std::shared_ptr<Show>> m_sub_show;
    std::unordered_set<std::shared_ptr<Show>> m_sub_show_add;
    std::unordered_set<std::shared_ptr<Show>> m_sub_show_remove;

};

}
#endif
