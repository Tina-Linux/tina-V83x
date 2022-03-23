#ifndef __MUTE_SHOW_H__
#define __MUTE_SHOW_H__

#include "ledshow/Show.h"

namespace AW {

class MuteShow : public Show
{
public:
    static std::shared_ptr<MuteShow> create(int setter, std::shared_ptr<EventWatcher> watcher, std::shared_ptr<LedPad> ledpad) {
                return std::shared_ptr<MuteShow>(new MuteShow(setter, watcher, ledpad));
    };
    ~MuteShow(){};

    void action(int index, std::shared_ptr<LedGroup> group) {
        group->setState(m_setter, Led::State::ON);
        group->setRGBBreathing(m_setter, 150,0,0);
    };
    void action_finished(float &next_show_sec_later) {
        next_show_sec_later = -0.3;
    };
    void action_reset(std::shared_ptr<LedPad> ledpad) {
        ledpad->foreach([this](int index, std::shared_ptr<LedGroup> group){
            group->removeProperty(m_setter);
        });
        ledpad->update();
    }
private:
    MuteShow(int setter, std::shared_ptr<EventWatcher> watcher, std::shared_ptr<LedPad> ledpad) :
            m_setter{setter},
            Show{watcher, ledpad}{};

    int m_setter;
};

}
#endif /*__MUTE_SHOW_H__*/
