#ifndef __LEDGROUP_H__
#define __LEDGROUP_H__

#include <memory>
#include <string>
#include <stdio.h>

#include "leds/Led.h"

namespace AW {

class LedGroup
{
public:
    class Property
    {
    public:
        Property(){};
        Property(Led::Property r, Led::Property g, Led::Property b) : red{r}, green{g}, blue{g}{};
        Led::Property red;
        Led::Property green;
        Led::Property blue;
    };
    using Status = Property;

    static std::shared_ptr<LedGroup> create(std::shared_ptr<Led> red, std::shared_ptr<Led> green, std::shared_ptr<Led> blue);

    void setRBrightness(int setter, Led::Brightness brightness);
    void setGBrightness(int setter, Led::Brightness brightness);
    void setBBrightness(int setter, Led::Brightness brightness);

    void setRGBBrightness(int setter, Led::Brightness  r, Led::Brightness  g, Led::Brightness  b);
    void setRGBBreathing(int setter, Led::Breathing r, Led::Breathing g, Led::Breathing b);

    void setState(int setter, Led::State state);
    Led::State getState();

    void setProperty(int setter, Property property);
    void removeProperty(int setter);


    Status getStatus();

    void update();

    void dumpStatus();
private:
    LedGroup(std::shared_ptr<Led> red, std::shared_ptr<Led> green, std::shared_ptr<Led> blue) :
                m_red{red},
                m_green{green},
                m_blue{blue}{};


    std::shared_ptr<Led> m_red;
    std::shared_ptr<Led> m_green;
    std::shared_ptr<Led> m_blue;
};

} /*namespace AW*/

#endif /*__LEDGROUP_H__*/
