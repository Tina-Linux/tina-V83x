#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stddef.h>

#include "utils/JsonUtils.h"

#include "ledshow/ShowManager.h"
#include "IdleShow.h"
#include "ListeningShow.h"
#include "ThinkingShow.h"
#include "SpeakingShow.h"
#include "MuteShow.h"
#include "ConnectingShow.h"
#include "WakeupTestShow.h"

namespace AW {

Show::Show(std::shared_ptr<EventWatcher> watcher, std::shared_ptr<LedPad> ledpad): m_watcher{watcher},m_ledpad{ledpad}
{

}

void Show::addSubShow(std::shared_ptr<Show> show)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_sub_show_add.insert(show);
}

void Show::removeSubShow(std::shared_ptr<Show> show)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_sub_show.erase(show);
    m_sub_show_remove.insert(show);
}

int Show::start()
{
    m_active.enable();
    do_led_pad();
}

void Show::stop()
{
    m_active.disable();
}

void Show::reset()
{
    m_watcher->addTimerEventWatcher(0.001, shared_from_this(), (void*)1);
}

void Show::do_led_pad()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    /*
    for(auto sub = m_sub_show_add.begin(); sub != m_sub_show_add.end();){
        (*sub)->sub_insert(m_ledpad);
        m_sub_show.insert(*sub);
    }*/
    for(auto sub : m_sub_show_add) {
        sub->sub_insert(m_ledpad);
        m_sub_show.insert(sub);
    }
    m_sub_show_add.clear();

    int doa_target;
    action_start(doa_target);
    for(auto sub : m_sub_show) {
        sub->sub_action_start(doa_target);
    }

    m_ledpad->foreach([this](int index, std::shared_ptr<LedGroup> group){
        action(index, group);
        for(auto sub : m_sub_show) {
            sub->sub_action(m_target, index, group);
        }
    });

    m_ledpad->update();

    float next_show_sec_later;
    action_finished(next_show_sec_later);

    for(auto sub : m_sub_show) {
        sub->sub_action_finished();
    }
/*
    for(auto sub = m_sub_show_remove.begin(); sub != m_sub_show_remove.end();){
        (*sub)->sub_remove(m_ledpad);
    }
    m_sub_show_remove.clear();
*/
    for(auto sub : m_sub_show_remove) {
        sub->sub_remove(m_ledpad);
    }
    m_sub_show_remove.clear();

    if(next_show_sec_later > 0.001)
        m_watcher->addTimerEventWatcher(next_show_sec_later, shared_from_this());
}

void Show::onEvent(EventWatcher::tEventType event_type, int event_src, void *priv) {
    if(priv == nullptr){
        if(m_active.isEnable()) do_led_pad();
    }else {
        for(auto sub : m_sub_show) {
            sub->sub_action_reset(m_ledpad);
        }
        action_reset(m_ledpad);
    }
}

std::shared_ptr<ShowManager> ShowManager::create(std::shared_ptr<EventWatcher> watcher,
                                                 struct json_object *config,
                                                 std::shared_ptr<DOAInfo> doa)
{
    auto showmanager = std::shared_ptr<ShowManager>(new ShowManager(watcher));

    if(showmanager->init(config, doa) < 0) return nullptr;

    return showmanager;
}

int ShowManager::init(struct json_object *config, std::shared_ptr<DOAInfo> doa)
{
    //if(config == nullptr) return -1;

    m_is_enable = true;

    struct json_object *j_direction = nullptr;
    if(config != nullptr)
        JsonUtils::json_object_object_get_ex(config, "direction", &j_direction);
    m_ledpad = LedPad::create(j_direction);

    if(m_ledpad == nullptr) return -1;

    if( socketpair(AF_UNIX, SOCK_STREAM, 0, m_socket_paris ) == -1 ){
        printf("create unnamed socket pair failed:%s\n",strerror(errno) );
        return -1;
    }

    m_watcher->addIOEventWatcher(m_socket_paris[0], shared_from_this());

    auto idle = IdleShow::create(static_cast<int>(Profile::IDLE), m_watcher, m_ledpad);
    m_show_map.insert({Profile::IDLE, idle});

    auto listening = ListeningShow::create(static_cast<int>(Profile::LISTENING), m_watcher, m_ledpad, doa);
    m_show_map.insert({Profile::LISTENING, listening});

    auto thinking = ThinkingShow::create(static_cast<int>(Profile::THINKING), m_watcher, m_ledpad);
    m_show_map.insert({Profile::THINKING, thinking});
    //add more show
    auto speaking = SpeakingShow::create(static_cast<int>(Profile::SPEAKING), m_watcher, m_ledpad);
    m_show_map.insert({Profile::SPEAKING, speaking});

    auto mute = MuteShow::create(static_cast<int>(Profile::MUTE), m_watcher, m_ledpad);
    m_show_map.insert({Profile::MUTE, mute});

    auto connecting = ConnectingShow::create(static_cast<int>(Profile::CONNECTING), m_watcher, m_ledpad);
    m_show_map.insert({Profile::CONNECTING, connecting});

    auto wakeuptest = WakeupTestShow::create(static_cast<int>(Profile::WAKEUPTEST), m_watcher, m_ledpad, doa);
    m_show_map.insert({Profile::WAKEUPTEST, wakeuptest});

    m_cur_profile = Profile::IDLE;
    m_show_map[m_cur_profile]->start();

    return 0;
}

void ShowManager::release()
{
    m_watcher->deleteIOEventWatcher(m_socket_paris[0]);
    m_ledpad->release();
}

int ShowManager::enableShow(Profile profile, ProfileFlag flag)
{
    if(m_is_enable)
       return send_event(profile, flag);

    return 0;
}

void ShowManager::onEvent(EventWatcher::tEventType event_type, int event_src, void *priv)
{
    if(event_src != m_socket_paris[0]) return;

    char profile_serialize;
    recv(event_src, &profile_serialize, 1, 0);

    if(profile_serialize > 0) {
        handle_event(static_cast<Profile>(profile_serialize),ProfileFlag::REPLACE);
    }else
        handle_event(static_cast<Profile>(-profile_serialize),ProfileFlag::APPEND);

}

void ShowManager::add_show(Profile profile, std::shared_ptr<Show> show)
{

}

int ShowManager::send_event(Profile profile, ProfileFlag flag)
{
    char profile_serialize;
    if(flag == ProfileFlag::REPLACE) {
        profile_serialize = static_cast<char>(profile);
    }else{
        profile_serialize = - static_cast<char>(profile);
    }

    return send(m_socket_paris[1], &profile_serialize, 1, 0);
}
void ShowManager::handle_event(Profile profile, ProfileFlag flag)
{
    if(m_cur_profile == Profile::MUTE){
        if(profile != Profile::UNMUTE) return;
        profile = Profile::IDLE;
    }

    if(m_cur_profile != profile) {
        m_show_map[m_cur_profile]->stop();
        m_show_map[m_cur_profile]->reset();

        auto iter = m_show_map.find(profile);
        if(iter == m_show_map.end()) return;

        m_cur_profile = profile;
    }
    m_show_map[m_cur_profile]->start();

}

}
