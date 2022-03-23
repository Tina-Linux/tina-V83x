#include <ugpio/ugpio.h>

#include "utils/JsonUtils.h"
#include "gpio/GpioHandler.h"

#define GET_VAL_BY_KEY_CHECKED(json, key, value) \
do { \
    if(!JsonUtils::json_object_object_get_ex(json, key, &value)) { \
        printf("get %s failed\n", key); \
        return -1; }\
}while(0)

namespace AW {

std::shared_ptr<GpioHandler> GpioHandler::create(struct json_object *config)
{
    auto gpio = std::shared_ptr<GpioHandler>(new GpioHandler());
    if(gpio->init(config) < 0) return nullptr;

    return gpio;
}

int GpioHandler::init(struct json_object *config)
{
    struct json_object *j_value;

    GET_VAL_BY_KEY_CHECKED(config, "pin", j_value);
    m_pin = JsonUtils::json_object_get_int(j_value);

    GET_VAL_BY_KEY_CHECKED(config, "active", j_value);
    m_active_value = JsonUtils::json_object_get_int(j_value);

    GET_VAL_BY_KEY_CHECKED(config, "disactive", j_value);
    m_disactive_value = JsonUtils::json_object_get_int(j_value);

    int rq, rv;
    rq = gpio_is_requested(m_pin);
    if(rq < 0) {
        printf("GpioHandler gpio: %d is requested!\n", m_pin);
        return -1;
    }

    rv = gpio_request(m_pin, NULL);
    if(rv < 0) {
        printf("GpioHandler gpio: %d request failed!\n", m_pin);
        //return -1;
    }

    rv = gpio_direction_output(m_pin, GPIOF_INIT_LOW);
    if(rv < 0) {
        printf("GpioHandler gpio: %d direction output failed!\n", m_pin);
        return -1;
    }

    return disactive();
}

int GpioHandler::active()
{
    printf("GpioHandler %d active(%d)\n", m_pin, m_active_value);
    int rv = gpio_direction_output(m_pin, GPIOF_INIT_LOW);
    if(rv < 0) {
        printf("gpio: %d direction output failed!\n", m_pin);
        return -1;
    }

    return gpio_set_value(m_pin, m_active_value);
}

int GpioHandler::disactive()
{
    printf("GpioHandler %d disactive(%d)\n", m_pin, m_disactive_value);
    int rv = gpio_direction_output(m_pin, GPIOF_INIT_LOW);
    if(rv < 0) {
        printf("GpioHandler gpio: %d direction output failed!\n", m_pin);
        return -1;
    }

    return gpio_set_value(m_pin, m_disactive_value);
}

}
