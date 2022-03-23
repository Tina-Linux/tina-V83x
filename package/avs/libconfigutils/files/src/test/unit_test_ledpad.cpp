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
#include "leds/LedPad.h"

using namespace AW;

#define ledabs(x) (x+12)%12

Led::Property property[] = {
    {Led::Brightness::LEVEL1, 10, Led::State::OFF},
    {Led::Brightness::LEVEL2, 10, Led::State::OFF}
};

std::vector<Led::Property> property2 = {
    {Led::Brightness::LEVEL1, 10, Led::State::OFF},
    {Led::Brightness::LEVEL2, 10, Led::State::OFF}
};

void show_rgb_ring(int setter, std::shared_ptr<LedPad> ledpad, int &target) {
    ledpad->foreach([setter, &target](int index, std::shared_ptr<LedGroup> group){
        if(target == index){
            group->setState(setter, Led::State::ON);
            group->setRGBBreathing(setter, 0,0,255);
        }else if(index == ledabs(target-1)){
            group->setState(setter, Led::State::ON);
            group->setRGBBreathing(setter, 0,255,0);
        }else if(index == ledabs(target-2)){
            group->setState(setter, Led::State::ON);
            group->setRGBBreathing(setter, 255,0,0);
        }else
            group->setState(setter, Led::State::OFF);
    });

    target = ledabs(target+1);
}

void show_b_breathing(int setter, std::shared_ptr<LedPad> ledpad, int &target) {
    ledpad->foreach([setter, &target](int index, std::shared_ptr<LedGroup> group){
        if(target == index){
            group->setState(setter, Led::State::ON);
            group->setRGBBreathing(setter, 0,0,255);
        }else if(index == ledabs(target-1)){
            group->setState(setter, Led::State::ON);
            group->setRGBBreathing(setter, 0,0,150);
        }else if(index == ledabs(target-2)){
            group->setState(setter, Led::State::ON);
            group->setRGBBreathing(setter, 0,0,50);
        }else
            group->setState(setter, Led::State::OFF);
    });

    target = ledabs(target+1);
}

void show_r_breathing(int setter, std::shared_ptr<LedPad> ledpad, int &target) {
    ledpad->foreach([setter, &target](int index, std::shared_ptr<LedGroup> group){
        if(target == index){
            group->setState(setter, Led::State::ON);
            group->setRGBBreathing(setter, 255,0,0);
        }else if(index == ledabs(target+1)){
            group->setState(setter, Led::State::ON);
            group->setRGBBreathing(setter, 150,0,0);
        }else if(index == ledabs(target+2)){
            group->setState(setter, Led::State::ON);
            group->setRGBBreathing(setter, 50,0,0);
        }else
            group->setState(setter, Led::State::OFF);
    });

    target = ledabs(target-1);
}

void show_r_breathing_keep(int setter, std::shared_ptr<LedPad> ledpad, int &target) {
    static int count = 0;
    if(count++ > 20) return;

    ledpad->foreach([setter, &target](int index, std::shared_ptr<LedGroup> group){
        if(target == index){
            group->setState(setter, Led::State::ON);
            group->setRGBBreathing(setter, 255,0,0);
        }else if(index == ledabs(target+1)){
            group->setState(setter, Led::State::ON);
            group->setRGBBreathing(setter, 150,0,0);
        }else if(index == ledabs(target+2)){
            group->setState(setter, Led::State::ON);
            group->setRGBBreathing(setter, 50,0,0);
        }else
            group->setState(setter, Led::State::OFF);
    });

    target = ledabs(target-1);
}

void show_r_breathing_reset(int setter, std::shared_ptr<LedPad> ledpad, int &target) {
    static int count = 100;
    if(count == 0) return;
    count--;
    ledpad->foreach([count, setter, &target](int index, std::shared_ptr<LedGroup> group){
        if(count == 0) {
            group->removeProperty(setter);
            return;
        }

        if(target == index){
            group->setState(setter, Led::State::ON);
            group->setRGBBreathing(setter, 255,0,0);
        }else if(index == ledabs(target+1)){
            group->setState(setter, Led::State::ON);
            group->setRGBBreathing(setter, 150,0,0);
        }else if(index == ledabs(target+2)){
            group->setState(setter, Led::State::ON);
            group->setRGBBreathing(setter, 50,0,0);
        }else
            group->setState(setter, Led::State::OFF);
    });

    target = ledabs(target-1);
}

void show_r_breathing_slow(int setter, std::shared_ptr<LedPad> ledpad, int &target) {
    static int count = 0;
    if(count++ %2 == 0) return;

    ledpad->foreach([count, setter, &target](int index, std::shared_ptr<LedGroup> group){

        if(target == index){
            group->setState(setter, Led::State::ON);
            group->setRGBBreathing(setter, 255,0,0);
        }else if(index == ledabs(target-1)){
            group->setState(setter, Led::State::ON);
            group->setRGBBreathing(setter, 150,0,0);
        }else if(index == ledabs(target-2)){
            group->setState(setter, Led::State::ON);
            group->setRGBBreathing(setter, 50,0,0);
        }else
            group->setState(setter, Led::State::OFF);
    });

    target = ledabs(target+1);
}

void show_g_breathing_slow(int setter, std::shared_ptr<LedPad> ledpad, int &target) {
    static int count = 0;
    if(count++ %3 != 0) return;

    ledpad->foreach([count, setter, &target](int index, std::shared_ptr<LedGroup> group){

        if(target == index){
            group->setState(setter, Led::State::ON);
            group->setRGBBreathing(setter, 0,250,0);
        }else if(index == ledabs(target-1)){
            group->setState(setter, Led::State::ON);
            group->setRGBBreathing(setter, 0,150,0);
        }else if(index == ledabs(target-2)){
            group->setState(setter, Led::State::ON);
            group->setRGBBreathing(setter, 0,50,0);
        }else
            group->setState(setter, Led::State::OFF);
    });

    target = ledabs(target+1);
}
int main(int argc, char *argv[]) {

    auto ledpad = LedPad::create();
    ledpad->init();
    int target1 = 0;
    int target2 = 0;
    int target3 = 0;
    while(1){
        show_r_breathing_slow(1, ledpad, target1);
        show_b_breathing(2, ledpad, target2);
        show_g_breathing_slow(3, ledpad, target3);
        ledpad->update();
        usleep(50*1000);
    }
    while(1);
}
