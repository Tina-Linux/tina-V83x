#ifndef __LISTENING_SHOW_H__
#define __LISTENING_SHOW_H__

#include "ledshow/Show.h"

namespace AW {

class ListeningShow : public Show
{
public:
    static std::shared_ptr<ListeningShow> create(int setter,
                                                 std::shared_ptr<EventWatcher> watcher,
                                                 std::shared_ptr<LedPad> ledpad,
                                                 std::shared_ptr<DOAInfo> doa) {
                return std::shared_ptr<ListeningShow>(new ListeningShow(setter, watcher, ledpad, doa));
    };
    ~ListeningShow(){};

    void action_start(int &doa_target) {
        if(m_doa != nullptr){
            m_doa_target = m_ledpad->getDirectionLedIndex(m_doa->get());
        }
        doa_target = m_doa_target;
    }

    void action(int index, std::shared_ptr<LedGroup> group) {
        group->setState(m_setter, Led::State::ON);
        if(index == ledabs(m_doa_target)){
            group->setRGBBreathing(m_setter, 255,255,255);
            group->setRGBBrightness(m_setter, Led::Brightness::LEVEL1, Led::Brightness::LEVEL1, Led::Brightness::LEVEL1);
        }else if(index == ledabs(m_doa_target + 1) || index == ledabs(m_doa_target - 1)){
            group->setRGBBreathing(m_setter, 50,50,50);
            group->setRGBBrightness(m_setter, Led::Brightness::LEVEL4, Led::Brightness::LEVEL4, Led::Brightness::LEVEL4);
        }
        else{
            group->setState(m_setter, Led::State::OFF);
            group->setRGBBreathing(m_setter, 0,0,10);
            group->setRGBBrightness(m_setter, Led::Brightness::LEVEL4, Led::Brightness::LEVEL4, Led::Brightness::LEVEL4);
        }
    };
    void action_finished(float &next_show_sec_later) {
        next_show_sec_later = 0.1;
    };
    void action_reset(std::shared_ptr<LedPad> ledpad) {
        ledpad->foreach([this](int index, std::shared_ptr<LedGroup> group){
            group->removeProperty(m_setter);
        });
        ledpad->update();
    }
private:
    ListeningShow(int setter,
                  std::shared_ptr<EventWatcher> watcher,
                  std::shared_ptr<LedPad> ledpad,
                  std::shared_ptr<DOAInfo> doa) :
            m_setter{setter},
            m_doa{doa},
            Show{watcher, ledpad}{};

    std::shared_ptr<DOAInfo> m_doa{nullptr};
    int m_doa_target = -1;
    int m_setter;
};

}
#endif /*__LISTENING_SHOW_H__*/
