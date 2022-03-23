#include <gtest/gtest.h>

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
// #include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <endian.h>
#include <iostream>

#include <chrono>
#include <mutex>
#include <condition_variable>

#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

#include <ugpio/ugpio.h>

#include "watcher/EventWatcher.h"
#include "threading/Executor.h"
#include "watcher/InputEventWatcher.h"
#include "i2c/I2CAdapter.h"
#include "leds/LedGroup.h"

using namespace AW;

#define RESET_REG 0x4f
#define GLOBAL_CONTROL_REG 0x4a
#define UPDATE_REG 0x25
#define SW_SHUTDOWN_REG 0x00

int main(int argc, char *argv[]) {

    int gpio_pin = 235;
    int rq, rv, al;
    rq = gpio_is_requested(gpio_pin);
    if(rq < 0) printf("gpio: %d is requested!\n", gpio_pin);
    rv = gpio_request(gpio_pin, NULL);
    if(rv < 0) printf("gpio: %d requested failed!\n", gpio_pin);
    al = gpio_get_activelow(gpio_pin);
    if(al < 0) printf("gpio: %d set active low failed!\n", gpio_pin);
    gpio_set_value(gpio_pin, al ? 0 : 1);

    auto i2c_handler = I2CHandler::create(1);
    int ret = i2c_handler->init();
    if(ret < 0) exit(-1);

    auto i2c_adapter = I2CAdapter::create(i2c_handler, 0x3f);
/*
    auto red = Led::create(i2c_adapter, 0x26, 0x01);
    auto green = Led::create(i2c_adapter, 0x27, 0x02);
    auto blue = Led::create(i2c_adapter, 0x28, 0x03);
*/
    auto red = Led::create(i2c_adapter, 0x29, 0x04);
    auto green = Led::create(i2c_adapter, 0x2a, 0x05);
    auto blue = Led::create(i2c_adapter, 0x2b, 0x06);

    auto led_group1 = LedGroup::create(red, green, blue);
    //reset
    i2c_adapter->write_a8_d8(RESET_REG, 0);

    //enable all channels
    i2c_adapter->write_a8_d8(GLOBAL_CONTROL_REG, 0);

    //SW enable
    i2c_adapter->write_a8_d8(SW_SHUTDOWN_REG, 1);

    led_group1->setState(Led::State::ON);
    led_group1->setRBrightness(Led::Brightness::LEVEL1);
    led_group1->setGBrightness(Led::Brightness::LEVEL1);
    led_group1->setBBrightness(Led::Brightness::LEVEL1);

    led_group1->setRGBBreathing(234,100,10);
    led_group1->update();

    //update all
    i2c_adapter->write_a8_d8(UPDATE_REG, 0);
    led_group1->dumpStatus();

    sleep(10);
    led_group1->setState(Led::State::OFF);
    led_group1->update();
    i2c_adapter->write_a8_d8(UPDATE_REG, 0);
    while(1);
}
