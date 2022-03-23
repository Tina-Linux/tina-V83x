#ifndef __SHOW_MANGER_H__
#define __SHOW_MANGER_H__

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

#include "ledshow/Show.h"
#include "ledshow/IShowManager.h"

namespace AW {

class DummyShowManager : public IShowManager
{
public:
    static std::shared_ptr<DummyShowManager> create(std::shared_ptr<EventWatcher> watcher, struct json_object *config){
        return std::make_shared<DummyShowManager>();
    };

    void release(){};
    int enableShow(Profile profile, ProfileFlag flag){};
};

class ShowManager : public IShowManager,
                    public EventWatcher::Listener,
                    public std::enable_shared_from_this<ShowManager>
{
public:

    static std::shared_ptr<ShowManager> create(std::shared_ptr<EventWatcher> watcher,
                                               struct json_object *config,
                                               std::shared_ptr<DOAInfo> doa = nullptr);

    void release();
    int enableShow(Profile profile, ProfileFlag flag);

private:
    void onEvent(EventWatcher::tEventType event_type, int event_src, void *priv);

private:
    ShowManager(std::shared_ptr<EventWatcher> watcher) : m_watcher{watcher}{};
    void add_show(Profile profile, std::shared_ptr<Show> show);
    int send_event(Profile profile, ProfileFlag flag);
    void handle_event(Profile profile, ProfileFlag flag);

    int init(struct json_object *config, std::shared_ptr<DOAInfo> doa);
private:
    bool m_is_enable{false};
    std::shared_ptr<EventWatcher> m_watcher;
    std::shared_ptr<LedPad> m_ledpad;
    std::map<Profile, std::shared_ptr<Show>> m_show_map;

    int m_socket_paris[2];

    std::shared_ptr<Show> m_cur_show;

    Profile m_cur_profile{Profile::IDLE};
};

}
#endif
