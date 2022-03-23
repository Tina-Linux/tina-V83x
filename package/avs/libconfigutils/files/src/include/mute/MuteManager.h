#ifndef __MUTE_MAMANGER_H__
#define __MUTE_MAMANGER_H__

#include <memory>
#include <functional>
#include <list>
#include <json-c/json.h>

#include "ActiveHandler.h"
#include "mute/IMuteManager.h"

namespace AW {

class DummyMuteManager : public IMuteManager
{
public:
    static std::shared_ptr<DummyMuteManager> create(struct json_object *config){
        return std::make_shared<DummyMuteManager>();
    };
    int privacyMute(bool is_mute){ return 0; }
};

class MuteManager : public IMuteManager
{
public:
    static std::shared_ptr<MuteManager> create(struct json_object *config);

    ~MuteManager();

    int privacyMute(bool is_mute);

private:
    MuteManager(){};
    int init(struct json_object *config);
    int init_gpio_ctr(struct json_object *config);
    int init_amixer_ctr(struct json_object *config);
    void release();

    std::function<void()> m_mute_enable;
    std::function<void()> m_mute_disable;

    std::list<std::shared_ptr<ActiveHandler>> m_active_list;

};
}
#endif
