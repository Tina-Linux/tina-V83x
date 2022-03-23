#include "leds/Led.h"

namespace AW {

void Led::Property::dump(std::string indenfiy)
{
    //printf("%s, brightness=%d, breathing=%d, state=%d\n", indenfiy.c_str(), (int)(brightness), (int)(breathing), (int)(state));
}

std::shared_ptr<Led> Led::create(std::shared_ptr<I2CAdapter> i2c_adapter, unsigned char ctr_addr, unsigned char pwm_addr)
{
    return std::shared_ptr<Led>(new Led(i2c_adapter, ctr_addr, pwm_addr));
}

void Led::setBrightness(int setter, Brightness brightness)
{
    m_property_map[setter].brightness = brightness;
}

void Led::setBreathing(int setter, Breathing breathing)
{
    m_property_map[setter].breathing = breathing;
}

void Led::setState(int setter, State state)
{
    m_property_map[setter].state = state;
}

void Led::setProperty(int setter, Property property)
{
    m_property_map[setter] = property;
}

Led::Status Led::getStatus()
{
    return m_status;
}

void Led::update()
{
    //printf("%d update", m_pwm_addr);
    m_status = sync_property();
    if(m_pre_status.breathing != m_status.breathing ||
       m_pre_status.state != m_status.state) {
        unsigned char ctr_value = 0;
        switch(m_status.brightness) {
            case Brightness::LEVEL1: ctr_value += (3 << 1); break;
            case Brightness::LEVEL2: ctr_value += (2 << 1); break;
            case Brightness::LEVEL3: ctr_value += (1 << 1); break;
            case Brightness::LEVEL4: ctr_value += (0 << 1); break;
        }

        switch(m_status.state) {
            case State::ON: ctr_value += 1; break;
            case State::OFF: ctr_value += 0; break;
        }

        m_i2c_adapter->write_a8_d8(m_ctr_addr, ctr_value);
        //printf(" ctr");
    }

    if(m_status.state == State::ON && m_pre_status.breathing != m_status.breathing){
        m_i2c_adapter->write_a8_d8(m_pwm_addr, m_status.breathing);
        //printf(" pwm ");
    }
    //printf("\n");
    m_pre_status = m_status;
}

void Led::dumpStatus()
{
    m_status.dump("aa");
}

void Led::removeProperty(int setter)
{
    m_property_map.erase(setter);
}

Led::Property Led::sync_property()
{
    Property prop;
    for(auto property = m_property_map.begin(); property != m_property_map.end(); ++property){
        property->second.dump(std::to_string(m_pwm_addr)+ ":" + std::to_string(property->first));
        if(property->second.state == Led::State::OFF) continue;

        if(property->second.brightness > prop.brightness)
            prop.brightness = property->second.brightness;
        if(property->second.breathing > prop.breathing)
            prop.breathing = property->second.breathing;

        if(prop.breathing > 0)
            prop.state = Led::State::ON;
    }

    prop.dump(std::to_string(m_pwm_addr)+ ": final ");
    return prop;
}

int Led::StatusManager::init(int num)
{
    Status s;
    for(int i = 0; i < num; i++)
        m_list.push_back(s);
}

Led::Status Led::StatusManager::previous()
{
    return m_list.back();
}

int Led::StatusManager::update(Status s)
{
    m_list.push_front(s);
    m_list.pop_back();
    dump();
    return 0;
}

void Led::StatusManager::dump()
{
    auto iter = m_list.begin();
    //for(auto iter = m_list.begin(); iter != m_list.end(); iter++)
    //  printf("brightness=%d, breathing=%d, state=%d\n", iter->brightness, iter->breathing, iter->state);
}

}
