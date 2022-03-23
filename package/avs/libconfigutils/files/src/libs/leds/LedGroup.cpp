#include "leds/LedGroup.h"
#include <stdio.h>
namespace AW {

std::shared_ptr<LedGroup> LedGroup::create(std::shared_ptr<Led> red, std::shared_ptr<Led> green, std::shared_ptr<Led> blue)
{
    return std::shared_ptr<LedGroup>(new LedGroup(red, green, blue));
}

void LedGroup::setRBrightness(int setter, Led::Brightness brightness)
{
    m_red->setBrightness(setter, brightness);
}

void LedGroup::setGBrightness(int setter, Led::Brightness brightness)
{
    m_green->setBrightness(setter, brightness);
}

void LedGroup::setBBrightness(int setter, Led::Brightness brightness)
{
    m_blue->setBrightness(setter, brightness);
}

void LedGroup::setRGBBrightness(int setter, Led::Brightness r, Led::Brightness g, Led::Brightness b)
{
    m_red->setBrightness(setter, r);
    m_green->setBrightness(setter, g);
    m_blue->setBrightness(setter, b);
}

void LedGroup::setRGBBreathing(int setter, Led::Breathing r, Led::Breathing g, Led::Breathing b)
{
    m_blue->setBreathing(setter, b);
    m_green->setBreathing(setter, g);
    m_red->setBreathing(setter, r);
}

void LedGroup::setState(int setter, Led::State state)
{
    m_blue->setState(setter, state);
    m_green->setState(setter, state);
    m_red->setState(setter, state);
}

Led::State LedGroup::getState()
{
    Status status;
    status.red = m_red->getStatus();
    status.green = m_green->getStatus();
    status.blue = m_blue->getStatus();
    if(status.red.state == Led::State::ON |
       status.green.state == Led::State::ON |
       status.blue.state == Led::State::ON) {
        return Led::State::ON;
    }

    return Led::State::OFF;
}

void LedGroup::setProperty(int setter, Property property)
{
    m_blue->setProperty(setter, property.blue);
    m_green->setProperty(setter, property.green);
    m_red->setProperty(setter, property.red);
}

void LedGroup::removeProperty(int setter)
{
    m_blue->removeProperty(setter);
    m_green->removeProperty(setter);
    m_red->removeProperty(setter);
}

LedGroup::Status LedGroup::getStatus()
{
    Status status;
    status.red = m_red->getStatus();
    status.green = m_green->getStatus();
    status.blue = m_blue->getStatus();
    return status;
}

void LedGroup::update()
{
    m_red->update();
    m_green->update();
    m_blue->update();
}

void LedGroup::dumpStatus()
{
    m_red->dumpStatus();
    m_green->dumpStatus();
    m_blue->dumpStatus();
}

}
