#include "leds/LedPad.h"
#include "utils/JsonUtils.h"

#include <stdio.h>
#include <unistd.h>

#define LED_RESET_REG 0x4f
#define LED_GLOBAL_CONTROL_REG 0x4a
#define LED_UPDATE_REG 0x25
#define LED_SW_SHUTDOWN_REG 0x00

#define LED_CTR_BASE_REG 0x26
#define LED_PWM_BASE_REG 0x01

#define LED_L2C_SLAVE 0x3f
#define I2C_INDEX 1
#define GPIO_PIN 235

#define LED_GROUP_NUM 12

#define GET_VAL_BY_KEY_CHECKED(json, key, value) \
do { \
    if(!JsonUtils::json_object_object_get_ex(json, key, &value)) { \
        printf("get %s failed\n", key); \
        return -1; }\
}while(0)

namespace AW {

std::shared_ptr<LedPad> LedPad::create(struct json_object *config)
{
    auto ledpad = std::shared_ptr<LedPad>(new LedPad());
    if(ledpad->init(config) < 0) return nullptr;

    return ledpad;
}

int LedPad::init(struct json_object *config)
{
    init_gpio();

    m_i2c_handler = I2CHandler::create(I2C_INDEX);
    if(m_i2c_handler == nullptr) return -1;
    if(m_i2c_handler->init() < 0) return -1;

    m_i2c_adapter = I2CAdapter::create(m_i2c_handler, LED_L2C_SLAVE);

    for(int i = 0; i < LED_GROUP_NUM; i++) {
        auto r = Led::create(m_i2c_adapter, LED_CTR_BASE_REG + 3*i, LED_PWM_BASE_REG + 3*i);
        auto g = Led::create(m_i2c_adapter, LED_CTR_BASE_REG + 3*i+1, LED_PWM_BASE_REG + 3*i+1);
        auto b = Led::create(m_i2c_adapter, LED_CTR_BASE_REG + 3*i+2, LED_PWM_BASE_REG + 3*i+2);
        m_ledgroup.push_back(LedGroup::create(r, g, b));
        m_ledgroup_direction.push_back(0.00);
    }

    init_ledpad();

    if(config == nullptr) return 0;

    struct json_object *j_angle_start, *j_angle_step;
    double angle_start, angle_step;
    GET_VAL_BY_KEY_CHECKED(config, "angle-start", j_angle_start);
    GET_VAL_BY_KEY_CHECKED(config, "angle-step", j_angle_step);

    angle_start = JsonUtils::json_object_get_double(j_angle_start);
    angle_step = JsonUtils::json_object_get_double(j_angle_step);

    int len = m_ledgroup_direction.size();

    if(angle_start > 360) return -1;
    if(angle_step * len > 360.1) return -1;

    for(int i = 0; i < len; i++){
        m_ledgroup_direction[i] = (angle_start + i * angle_step);
    }

    return 0;
}

void LedPad::foreach(std::function<void(int, std::shared_ptr<LedGroup>)> group_each)
{
    int size = m_ledgroup.size();
    for(int i = 0; i < size; i++){
        group_each(i, m_ledgroup[i]);
    }
}

void LedPad::update()
{
    int size = m_ledgroup.size();
    for(int i = 0; i < size; i++){
        m_ledgroup[i]->update();
    }

    m_i2c_adapter->write_a8_d8(LED_UPDATE_REG, 0);
}

int LedPad::getDirectionLedIndex(double doa)
{
    if(doa < m_ledgroup_direction[0])
        doa += 360;

    int size = m_ledgroup.size();
    for(int i = 0; i < size; i++){
        if(doa == m_ledgroup_direction[i]) return i;
        if(doa > m_ledgroup_direction[i]) continue;

        int before = (i - 1 + size) % size;
        int after = i;

        double start = m_ledgroup_direction[before];
        double stop = m_ledgroup_direction[after];

        int target = before;
        if((doa - start) > (stop - doa)) target = after;
#if 0
        if(doa > 360) doa -= 360;
        if(start > 360) start = start - 360;
        if(stop > 360) stop = stop - 360;
        printf("doa: %f, [%f(%d)-%f(%d)], return target: %d\n", doa, start, before, stop, after, target);
#endif
        return target;
    }

    if((doa - m_ledgroup_direction[size - 1]) > (m_ledgroup_direction[0] + 360) - doa)
        return 0;
    else
        return size - 1;
}

void LedPad::release()
{
    release_ledpad();
}

int LedPad::init_gpio()
{

    int rq, rv, al;
    rq = gpio_is_requested(GPIO_PIN);
    if(rq < 0) printf("gpio: %d is requested!\n", GPIO_PIN);
    rv = gpio_request(GPIO_PIN, NULL);
    if(rv < 0) printf("gpio: %d requested failed!\n", GPIO_PIN);
    rv = gpio_direction_output(GPIO_PIN, GPIOF_INIT_LOW);
    if(rv < 0) printf("gpio: %d direction output failed!\n", GPIO_PIN);
    al = gpio_get_activelow(GPIO_PIN);
    if(al < 0) printf("gpio: %d set active low failed!\n", GPIO_PIN);
    gpio_set_value(GPIO_PIN, al ? 0 : 1);

    return 0;
}

void LedPad::release_gpio()
{

}

int LedPad::init_ledpad()
{
    //reset
    m_i2c_adapter->write_a8_d8(LED_RESET_REG, 0);

    //enable all channels
    m_i2c_adapter->write_a8_d8(LED_GLOBAL_CONTROL_REG, 0);

    //SW enable
    m_i2c_adapter->write_a8_d8(LED_SW_SHUTDOWN_REG, 1);

    return 0;
}

void LedPad::release_ledpad()
{
    //reset
    m_i2c_adapter->write_a8_d8(LED_RESET_REG, 0);

    //disable all channels
    m_i2c_adapter->write_a8_d8(LED_GLOBAL_CONTROL_REG, 1);

    //SW disable
    m_i2c_adapter->write_a8_d8(LED_SW_SHUTDOWN_REG, 0);
}

}
