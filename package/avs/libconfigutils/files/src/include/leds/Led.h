#ifndef __LED_H__
#define __LED_H__

#include <memory>
#include <string>
#include <list>
#include <map>

#include <stdio.h>

#include "i2c/I2CAdapter.h"

namespace AW {

class Led
{
public:
    enum class Brightness
    {
        LEVEL1 = 3,
        LEVEL2 = 2,
        LEVEL3 = 1,
        LEVEL4 = 0
    };
    using Breathing = unsigned char;
    enum class State
    {
        ON = 1,
        OFF = 0
    };

    class Property
    {
    public:
        Property(Brightness a = Brightness::LEVEL1, Breathing b = 0, State c = State::OFF): brightness{a},breathing{b},state{c}{};
        Brightness brightness;
        Breathing breathing;
        State state{State::OFF};
        void dump(std::string indenfiy);
    };
    using Status = Property;

    static std::shared_ptr<Led> create(std::shared_ptr<I2CAdapter> i2c_adapter, unsigned char ctr_addr, unsigned char pwm_addr);

    void setBrightness(int setter, Brightness brightness);
    void setBreathing(int setter, Breathing breathing);
    void setState(int setter, State state);
    void setProperty(int setter, Property property);
    void removeProperty(int setter);

    Status getStatus();

    void previous();

    void update();

    void dumpStatus();
private:
    Led(std::shared_ptr<I2CAdapter> i2c_adapter, unsigned char ctr_addr, unsigned char pwm_addr) :
                m_i2c_adapter{i2c_adapter},
                m_ctr_addr{ctr_addr},
                m_pwm_addr{pwm_addr}{
                    m_status_manager.init(5);
                };

    Property sync_property();
    class StatusManager {
    public:
        int init(int num);
        Status previous();
        Status current();
        int update(Status status);
        void dump();

    private:
        std::list<Status> m_list;
    };

    std::shared_ptr<I2CAdapter> m_i2c_adapter;
    unsigned char m_ctr_addr;
    unsigned char m_pwm_addr;

    std::map<int, Property> m_property_map;
    Status m_status;
    Status m_pre_status;

    StatusManager m_status_manager;
};

} /*namespace AW*/

#endif /*__LED_H__*/
