#ifndef __WAKEUP_TEST_SHOW_H__
#define __WAKEUP_TEST_SHOW_H__

#include "ledshow/Show.h"

namespace AW {

class WakeupTestShow : public Show
{
public:
    static std::shared_ptr<WakeupTestShow> create(int setter,
                                                  std::shared_ptr<EventWatcher> watcher,
                                                  std::shared_ptr<LedPad> ledpad,
                                                  std::shared_ptr<DOAInfo> doa) {
                return std::shared_ptr<WakeupTestShow>(new WakeupTestShow(setter, watcher, ledpad, doa));
    };
    ~WakeupTestShow(){};

    void action_start(int &doa_target) {
        if(!m_is_wakup && m_doa != nullptr){
            m_doa_target = m_ledpad->getDirectionLedIndex(m_doa->get());
        }
        doa_target = m_doa_target;
    }

    void action(int index, std::shared_ptr<LedGroup> group) {
        //if(m_doa_target != -1 && index != m_doa_target) return;
        //printf("index: %d, m_doa_target= %d, m_is_wakup = %d\n", index, m_doa_target, m_is_wakup);
        if(!m_is_wakup){
            group->setState(m_setter, Led::State::ON);
            if(index == m_doa_target){
                group->setRGBBreathing(m_setter, 255,255,255);
                group->setRGBBrightness(m_setter, Led::Brightness::LEVEL1, Led::Brightness::LEVEL1, Led::Brightness::LEVEL1);
            }else if(index == ledabs(m_doa_target + 1) || index == ledabs(m_doa_target - 1)){
                group->setRGBBreathing(m_setter, 50,50,50);
                group->setRGBBrightness(m_setter, Led::Brightness::LEVEL4, Led::Brightness::LEVEL4, Led::Brightness::LEVEL4);
            }
            else{
                group->setState(m_setter, Led::State::ON);
                group->setRGBBreathing(m_setter, 0,0,10);
                group->setRGBBrightness(m_setter, Led::Brightness::LEVEL4, Led::Brightness::LEVEL4, Led::Brightness::LEVEL4);
            }
        }else{
            group->setState(m_setter, Led::State::OFF);
        }
    };
    void action_finished(float &next_show_sec_later) {
        if(m_is_wakup){
            next_show_sec_later = -1.0;
            m_is_wakup = false;
        }
        else{
            next_show_sec_later = 2.0;
            m_is_wakup = true;
        }
    };
    void action_reset(std::shared_ptr<LedPad> ledpad) {
        ledpad->foreach([this](int index, std::shared_ptr<LedGroup> group){
            group->removeProperty(m_setter);
        });
        ledpad->update();
    }
private:
    WakeupTestShow(int setter,
                   std::shared_ptr<EventWatcher> watcher,
                   std::shared_ptr<LedPad> ledpad,
                   std::shared_ptr<DOAInfo> doa) :
            m_setter{setter},
            m_doa{doa},
            Show{watcher, ledpad}{};

    int m_setter;
    std::shared_ptr<DOAInfo> m_doa{nullptr};
    int m_doa_target = -1;
    bool m_is_wakup{false};


};

}
#endif /*__WAKEUP_TEST_SHOW_H__*/
