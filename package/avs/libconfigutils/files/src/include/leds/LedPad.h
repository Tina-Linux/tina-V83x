#ifndef __LED_PAD_H__
#define __LED_PAD_H__

#include <functional>
#include <memory>
#include <string>
#include <map>
#include <vector>
#include <stdio.h>
#include <ugpio/ugpio.h>
#include <json-c/json.h>

#include "i2c/I2CAdapter.h"
#include "leds/LedGroup.h"

#define ledabs(x) (x+12)%12

namespace AW {

class LedPad
{
public:
    static std::shared_ptr<LedPad> create(struct json_object *config = nullptr);

    void release();
    void update();

    void foreach(std::function<void(int, std::shared_ptr<LedGroup>)> group_each);

    int getDirectionLedIndex(double doa);
private:
    int init(struct json_object *config);
    int init_gpio();
    void release_gpio();

    int init_ledpad();
    void release_ledpad();

private:
    std::shared_ptr<I2CHandler> m_i2c_handler;
    std::shared_ptr<I2CAdapter> m_i2c_adapter;

    std::vector<double> m_ledgroup_direction;
    std::vector<std::shared_ptr<LedGroup>> m_ledgroup;  //std::list? or std::vector

    int m_gpio_handler;
};


} /*namespace AW*/

#endif /*__LED_PAD_H__*/
