#ifndef __IDLE_SHOW_H__
#define __IDLE_SHOW_H__

#include "ledshow/Show.h"

namespace AW {

class IdleShow : public Show
{
public:
    static std::shared_ptr<IdleShow> create(int setter, std::shared_ptr<EventWatcher> watcher, std::shared_ptr<LedPad> ledpad) {
                return std::shared_ptr<IdleShow>(new IdleShow(setter, watcher, ledpad));
    };
    ~IdleShow(){};

    void action(int index, std::shared_ptr<LedGroup> group) {
        group->setState(m_setter, Led::State::OFF);
    };
    void action_finished(float &next_show_sec_later) {
        next_show_sec_later = -1.0;
    };
    void action_reset(std::shared_ptr<LedPad> ledpad) {
        ledpad->foreach([this](int index, std::shared_ptr<LedGroup> group){
            group->removeProperty(m_setter);
        });
        ledpad->update();
    }
private:
    IdleShow(int setter, std::shared_ptr<EventWatcher> watcher, std::shared_ptr<LedPad> ledpad) :
            m_setter{setter},
            Show{watcher, ledpad}{};


    int target;
    int m_setter;
};

}
#endif
