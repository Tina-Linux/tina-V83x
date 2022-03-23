#ifndef __GPIO_HANDLER_H__
#define __GPIO_HANDLER_H__

#include <memory>
#include <list>
#include <json-c/json.h>

#include "ActiveHandler.h"

namespace AW {

class GpioHandler : public ActiveHandler
{
public:
    static std::shared_ptr<GpioHandler> create(struct json_object *config);

    int active();
    int disactive();

private:
    GpioHandler(){};
    int init(struct json_object *config);

    int m_pin{-1};
    int m_active_value{-1};
    int m_disactive_value{-1};
};

}
#endif
