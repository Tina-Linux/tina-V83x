#ifndef __CONNECTING_SHOW_H__
#define __CONNECTING_SHOW_H__

#include "ledshow/Show.h"

namespace AW {

class ConnectingShow : public Show
{
public:
    static std::shared_ptr<ConnectingShow> create(int setter, std::shared_ptr<EventWatcher> watcher, std::shared_ptr<LedPad> ledpad) {
                return std::shared_ptr<ConnectingShow>(new ConnectingShow(setter, watcher, ledpad));
    };
    ~ConnectingShow(){};

    void action(int index, std::shared_ptr<LedGroup> group) {
        if(m_target == index){
            group->setState(m_setter, Led::State::ON);
            group->setRGBBreathing(m_setter, 0,0,255);
        }else if(index == ledabs(m_target-1)){
            group->setState(m_setter, Led::State::ON);
            group->setRGBBreathing(m_setter, 0,255,0);
        }else if(index == ledabs(m_target-2)){
            group->setState(m_setter, Led::State::ON);
            group->setRGBBreathing(m_setter, 255,0,0);
        }else
            group->setState(m_setter, Led::State::OFF);
    };
    void action_finished(float &next_show_sec_later) {
        m_target = ledabs(m_target+1);
        next_show_sec_later = 0.1;
    };
    void action_reset(std::shared_ptr<LedPad> ledpad) {
        ledpad->foreach([this](int index, std::shared_ptr<LedGroup> group){
            group->removeProperty(m_setter);
        });
        ledpad->update();
    }
private:
    ConnectingShow(int setter, std::shared_ptr<EventWatcher> watcher, std::shared_ptr<LedPad> ledpad) :
            m_setter{setter},
            Show{watcher, ledpad}{};



    int m_setter;
};

}
#endif /*__CONNECTING_SHOW_H__*/
