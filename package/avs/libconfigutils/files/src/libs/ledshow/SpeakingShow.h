#ifndef __SPEARKING_SHOW_H__
#define __SPEARKING_SHOW_H__

#include "ledshow/Show.h"

namespace AW {

class SpeakingShow : public Show
{
public:
    static std::shared_ptr<SpeakingShow> create(int setter, std::shared_ptr<EventWatcher> watcher, std::shared_ptr<LedPad> ledpad) {
                return std::shared_ptr<SpeakingShow>(new SpeakingShow(setter, watcher, ledpad));
    };
    ~SpeakingShow(){};

    void action(int index, std::shared_ptr<LedGroup> group) {
        group->setState(m_setter, Led::State::ON);
        int r = 255-2*m_breath_value;
        int g = 255-2*m_breath_value;
        int b = 255-m_breath_value;
        if(r < 0) r = 0;
        if(g < 0) g = 0;
        group->setRGBBreathing(m_setter, r, g, b);
    };
    void action_finished(float &next_show_sec_later) {
        next_show_sec_later = 0.06;
        if(m_breath_value == 250) m_breath_direction = -1;
        if(m_breath_value == 0) m_breath_direction = 1;
        m_breath_value += 10 * m_breath_direction;
    };
    void action_reset(std::shared_ptr<LedPad> ledpad) {
        ledpad->foreach([this](int index, std::shared_ptr<LedGroup> group){
            group->removeProperty(m_setter);
        });
        ledpad->update();
    }
private:
    SpeakingShow(int setter, std::shared_ptr<EventWatcher> watcher, std::shared_ptr<LedPad> ledpad) :
            m_setter{setter},
            Show{watcher, ledpad}{};


    //static const int breath_value[22] = {126, 116, 106, 96, 86, 78, 69, 61, 53, 46, 39, 33, 28, 22, 18, 13, 10, 6, 4, 2, 1, 0};
    int m_setter;
    int m_breath_value = 0;
    int m_breath_direction = 1;
};

}
#endif /*__SPEARKING_SHOW_H__*/
