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
#include "utils/JsonUtils.h"
#include "watcher/EventWatcher.h"
#include "threading/Executor.h"
#include "watcher/InputEventWatcher.h"
#include "i2c/I2CAdapter.h"
#include "leds/LedPad.h"
#include "ledshow/Show.h"

int led_debug = 0;

using namespace AW;

class OneShow : public Show
{
public:
    static std::shared_ptr<OneShow> create(int setter, std::shared_ptr<EventWatcher> watcher, std::shared_ptr<LedPad> ledpad) {
                return std::shared_ptr<OneShow>(new OneShow(setter, watcher, ledpad));
    };
    ~OneShow(){};

    void action(int index, std::shared_ptr<LedGroup> group) {
        if(target == index){
            group->setState(m_setter, Led::State::ON);
            group->setRGBBreathing(m_setter, 0,0,255);
        }else if(index == ledabs(target-1)){
            group->setState(m_setter, Led::State::ON);
            group->setRGBBreathing(m_setter, 0,255,0);
        }else if(index == ledabs(target-2)){
            group->setState(m_setter, Led::State::ON);
            group->setRGBBreathing(m_setter, 255,0,0);
        }else
            group->setState(m_setter, Led::State::OFF);
    };
    void action_finished(float &next_show_sec_later) {
        m_target = ledabs(m_target+1);
        target = m_target;
        next_show_sec_later = 0.1;
    };
    void action_reset(std::shared_ptr<LedPad> ledpad) {
        ledpad->foreach([this](int index, std::shared_ptr<LedGroup> group){
            group->removeProperty(m_setter);
        });
        ledpad->update();
    }
private:
    OneShow(int setter, std::shared_ptr<EventWatcher> watcher, std::shared_ptr<LedPad> ledpad) :
            m_setter{setter},
            Show{watcher, ledpad}{};


    int target;
    int m_setter;
};

class TowShow : public Show
{
public:
    static std::shared_ptr<TowShow> create(int setter, std::shared_ptr<EventWatcher> watcher, std::shared_ptr<LedPad> ledpad) {
                return std::shared_ptr<TowShow>(new TowShow(setter, watcher, ledpad));
    };
    ~TowShow(){};
private:
    void private_action(int target, int index, std::shared_ptr<LedGroup> group) {
        if(target == index){
            group->setState(m_setter, Led::State::ON);
            group->setRGBBreathing(m_setter, 0,0,100);
        }else if(index == ledabs(target-1)){
            group->setState(m_setter, Led::State::ON);
            group->setRGBBreathing(m_setter, 0,0,100);
        }else if(index == ledabs(target-2)){
            group->setState(m_setter, Led::State::ON);
            group->setRGBBreathing(m_setter, 0,0,100);
        }else
            group->setState(m_setter, Led::State::OFF);
    }

public:
    void action(int index, std::shared_ptr<LedGroup> group) {
        private_action(m_target, index, group);
    };
    void action_finished(float &next_show_sec_later) {
        m_target = ledabs(m_target+1);
        target = m_target;
        next_show_sec_later = 0.2;
    };

    void action_reset(std::shared_ptr<LedPad> ledpad) {
        printf("%s\n",__func__);
        ledpad->foreach([this](int index, std::shared_ptr<LedGroup> group){
            group->removeProperty(m_setter);
        });
        ledpad->update();
    }

/********************************************************************************************/
    void sub_action(int target, int index, std::shared_ptr<LedGroup> group) {
        private_action(target, index, group);
    };
    void sub_action_finished() {};
    void sub_remove(std::shared_ptr<LedPad> ledpad){
        ledpad->foreach([this](int index, std::shared_ptr<LedGroup> group){
            group->removeProperty(m_setter);
        });
    };
    void sub_insert(std::shared_ptr<LedPad> ledpad){
    };

    void sub_action_reset(std::shared_ptr<LedPad> ledpad) {
        ledpad->foreach([this](int index, std::shared_ptr<LedGroup> group){
            group->removeProperty(m_setter);
        });
    }
private:
    TowShow(int setter, std::shared_ptr<EventWatcher> watcher, std::shared_ptr<LedPad> ledpad) :
            m_setter{setter},
            Show{watcher, ledpad}{};


    int target;
    int m_setter;
};

void test_show1()
{
    printf("%s start!!\n",__func__);
    auto ledpad = LedPad::create();

    auto watcher = EventWatcher::create();
    watcher->startWatcher();

    auto show1 = OneShow::create(1, watcher, ledpad);

    show1->start();
    sleep(5);
    show1->stop();
    show1->reset();

    ledpad->release();
    watcher->stopWatcher();
}

void test_show2()
{
    printf("%s start!!\n",__func__);
    auto ledpad = LedPad::create();

    auto watcher = EventWatcher::create();
    watcher->startWatcher();

    auto show2 = TowShow::create(2, watcher, ledpad);
    show2->start();
    sleep(5);
    show2->stop();
    show2->reset();

    ledpad->release();
    watcher->stopWatcher();
}

void test_show1_show2_independent()
{
    printf("%s start!!\n",__func__);
    auto ledpad = LedPad::create();

    auto watcher = EventWatcher::create();
    watcher->startWatcher();

    auto show1 = OneShow::create(1, watcher, ledpad);
    auto show2 = TowShow::create(2, watcher, ledpad);

    show1->start();
    sleep(1);
    show2->start();
    sleep(5);
    show2->stop();
    show2->reset();

    sleep(2);
    show1->stop();
    show1->reset();

    ledpad->release();
    watcher->stopWatcher();
}

void test_show1_subshow_show2_start_bind()
{
    printf("%s start!!\n",__func__);
    auto ledpad = LedPad::create();

    auto watcher = EventWatcher::create();
    watcher->startWatcher();

    auto show1 = OneShow::create(1, watcher, ledpad);
    auto show2 = TowShow::create(2, watcher, ledpad);

    show1->addSubShow(show2);
    show1->start();

    sleep(5);
    show1->stop();
    show1->reset();

    ledpad->release();
    watcher->stopWatcher();
}

void test_show1_subshow_show2_start_bind_and_remove()
{
    printf("%s start!!\n",__func__);
    auto ledpad = LedPad::create();

    auto watcher = EventWatcher::create();
    watcher->startWatcher();

    auto show1 = OneShow::create(1, watcher, ledpad);
    auto show2 = TowShow::create(2, watcher, ledpad);

    show1->addSubShow(show2);
    show1->start();

    sleep(5);
    show1->removeSubShow(show2);
    sleep(2);
    show1->stop();
    show1->reset();

    ledpad->release();
    watcher->stopWatcher();
}

void test_show1_subshow_show2_async()
{
    printf("%s start!!\n",__func__);
    auto ledpad = LedPad::create();

    auto watcher = EventWatcher::create();
    watcher->startWatcher();

    auto show1 = OneShow::create(1, watcher, ledpad);
    auto show2 = TowShow::create(2, watcher, ledpad);

    show1->start();
    sleep(2);
    show1->addSubShow(show2);
    sleep(5);
    show1->stop();
    show1->reset();

    ledpad->release();
    watcher->stopWatcher();
}

void test_show1_subshow_show2_async_remove()
{
    printf("%s start!!\n",__func__);
    auto ledpad = LedPad::create();

    auto watcher = EventWatcher::create();
    watcher->startWatcher();

    auto show1 = OneShow::create(1, watcher, ledpad);
    auto show2 = TowShow::create(2, watcher, ledpad);

    show1->start();
    sleep(2);
    show1->addSubShow(show2);
    sleep(5);

    show1->removeSubShow(show2);
    sleep(2);

    show1->stop();
    show1->reset();

    ledpad->release();
    watcher->stopWatcher();
}

int main(int argc, char *argv[]) {

    test_show1();
    sleep(2);
    test_show2();
    sleep(2);
    test_show1_show2_independent();
    sleep(2);
    test_show1_subshow_show2_start_bind();
    sleep(2);
    test_show1_subshow_show2_async();
    sleep(2);
    test_show1_subshow_show2_start_bind_and_remove();
    sleep(2);
    test_show1_subshow_show2_async_remove();
    while(1);
}
