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

using namespace AW;

unsigned char pwm_value = 0xFF;
unsigned char pwm_half_value = 0x66;
unsigned char config_value = 0x01;
unsigned char reset_value = 0x00;

int controller_init(std::shared_ptr<I2CAdapter> i2c_adapter) {
    //SDB low
    int i;
    //SDB high
    i2c_adapter->write_a8_d8(0x4B, config_value);
    for (i = 0x01; i <= 0x24; i++) {
        i2c_adapter->write_a8_d8(i, reset_value);
    }
    for (i = 0x26; i < 0x4A; i++) {
        i2c_adapter->write_a8_d8(i, config_value);
    }
    i2c_adapter->write_a8_d8(0x25, reset_value);
    i2c_adapter->write_a8_d8(0x00, config_value);
    i2c_adapter->write_a8_d8(0x4A, reset_value);
}

int blue_backgroud(std::shared_ptr<I2CAdapter> i2c_adapter, unsigned int msec) {

    int i = 0;
    for (i = 0x24; i > 0; i -= 3) {
        i2c_adapter->write_a8_d8(i, 96);
        i2c_adapter->write_a8_d8(0x25, reset_value);
        usleep(msec*1000); /*change this value to set different patterns*/
    }  //blue
}

int responding(std::shared_ptr<I2CAdapter> i2c_adapter) {
    blue_backgroud(i2c_adapter, 0);
    int received = 0;
    int breath_value[22] = {126, 116, 106, 96, 86, 78, 69, 61, 53, 46, 39, 33, 28, 22, 18, 13, 10, 6, 4, 2, 1, 0};
    int step = 15;
    int start = 21;
    while(!received) {
        for (int i = 0x023; i > 0; i -= 3) {
            i2c_adapter->write_a8_d8(i, breath_value[start]);
            i2c_adapter->write_a8_d8(0x25, reset_value);
        }
        if (start - 1 < 0)
            start = 21;
        else
            start -= 1;
        usleep(100*1000);
    }
}

int thinking(std::shared_ptr<I2CAdapter> i2c_adapter, int sec) {
    blue_backgroud(i2c_adapter, 0);

    while (sec > 0) {
        /*stage 1*/
        for (int i = 0x23; i > 0; i -= 6) {
            i2c_adapter->write_a8_d8(i, pwm_half_value);
            i2c_adapter->write_a8_d8(0x25, reset_value);
        }
        usleep(100*1000);
        for (int i = 0x23; i > 0; i -= 6) {
            i2c_adapter->write_a8_d8(i, reset_value);
        }
        /*stage 2*/
        for (int i = 0x20; i > 0; i -= 6) {
            i2c_adapter->write_a8_d8(i, pwm_half_value);
            i2c_adapter->write_a8_d8(0x25, reset_value);
        }
        usleep(100*1000);
        for (int i = 0x20; i > 0; i -= 6) {
            i2c_adapter->write_a8_d8(i, reset_value);
        }
        sec -= 1;
    }
    return 0;
}

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

    controller_init(i2c_adapter);
    //responding(i2c_adapter);
    thinking(i2c_adapter, 10);
}
