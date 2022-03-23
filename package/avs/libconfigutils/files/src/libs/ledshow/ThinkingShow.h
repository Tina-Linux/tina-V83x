#ifndef __THINKING_SHOW_H__
#define __THINKING_SHOW_H__

#include "ledshow/Show.h"

namespace AW {

class ThinkingShow : public Show
{
public:
    static std::shared_ptr<ThinkingShow> create(int setter, std::shared_ptr<EventWatcher> watcher, std::shared_ptr<LedPad> ledpad) {
                return std::shared_ptr<ThinkingShow>(new ThinkingShow(setter, watcher, ledpad));
    };
    ~ThinkingShow(){};

    void action(int index, std::shared_ptr<LedGroup> group) {
        group->setState(m_setter, Led::State::ON);
        if((index + m_target)%2){
            group->setRGBBreathing(m_setter, 0,0,150);
        }else {
            group->setRGBBreathing(m_setter, 200,200,200);
        }
    };
    void action_finished(float &next_show_sec_later) {
        next_show_sec_later = 0.3;
        m_target = (m_target + 1)%2;
    };
    void action_reset(std::shared_ptr<LedPad> ledpad) {
        ledpad->foreach([this](int index, std::shared_ptr<LedGroup> group){
            group->removeProperty(m_setter);
        });
        ledpad->update();
    }
private:
    ThinkingShow(int setter, std::shared_ptr<EventWatcher> watcher, std::shared_ptr<LedPad> ledpad) :
            m_setter{setter},
            Show{watcher, ledpad}{};



    int m_setter;
};

}
#endif /*__THINKING_SHOW_H__*/
