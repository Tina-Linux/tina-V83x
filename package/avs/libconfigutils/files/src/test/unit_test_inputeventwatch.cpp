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

#include "watcher/EventWatcher.h"
#include "threading/Executor.h"
#include "watcher/InputEventWatcher.h"

using namespace AW;

class InputEventWatcherTest : public InputEventWatcher::Listener,
                              public std::enable_shared_from_this<InputEventWatcherTest>{
public:
    InputEventWatcherTest() {
        printf("InputEventWatcherTest\n");
        m_watcher = EventWatcher::create();
        m_executor = Executor::create();

        m_watcher->startWatcher();

    };

    ~InputEventWatcherTest() {
        m_input->release();
        m_input = nullptr;
        m_watcher->stopWatcher();
        printf("~InputEventWatcherTest\n");
    }

    void onInputEvent(const struct input_event &event) {
        if(event.code != 0 && event.value == 0) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_key = event.code;
            printf("onInputEventRecv code: 0x%x, value: %d, this: %p\n", m_key, event.value, this);
            m_processTrigger.notify_all();
        }
    }

    int waitForInputEvent(const std::string name, int target_key, const std::chrono::seconds duration) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_input = InputEventWatcher::create(m_watcher, name);
        m_input->addKeyCodeListener(target_key, shared_from_this());
        m_processTrigger.wait_for(lock, duration, [this, target_key]() {return target_key == m_key; });
        m_input->removeKeyCodeListener(target_key, shared_from_this());
        return m_key;
    }

    int removeInputEvent(const std::string name, int target_key, const std::chrono::seconds duration) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_input = InputEventWatcher::create(m_watcher, name);
        m_input->addKeyCodeListener(target_key, shared_from_this());
        m_processTrigger.wait_for(lock, duration, [this, target_key]() {return target_key == m_key; });
        m_input->removeKeyCodeListener(target_key, shared_from_this());
        m_key = -1;
        m_processTrigger.wait_for(lock, duration, [this, target_key]() {return target_key == m_key; });
        return m_key;
    }

    std::shared_ptr<EventWatcher> m_watcher;
    std::shared_ptr<Executor> m_executor;
    std::shared_ptr<InputEventWatcher> m_input;

    int m_key = 0;

    std::mutex m_mutex;
    std::condition_variable m_processTrigger;
};


class Gtest_ElementTest : public ::testing::Test {

protected:
    virtual void SetUp() override {
        printf("SetUp\n");
        m_test = std::shared_ptr<InputEventWatcherTest>(new InputEventWatcherTest());
    };

    void TearDown() override {
        printf("~TearDown use_count:%lld\n",m_test.use_count());
        m_test = nullptr;
    }

    std::shared_ptr<InputEventWatcherTest> m_test;

};

TEST_F(Gtest_ElementTest, key_power) {
    int target_key = KEY_POWER;
    int key = m_test->waitForInputEvent("axp803-powerkey", target_key, std::chrono::seconds(100));
    ASSERT_EQ(key, target_key);
}

TEST_F(Gtest_ElementTest, key_power_remove) {
    int target_key = KEY_POWER;
    int key = m_test->removeInputEvent("axp803-powerkey", target_key, std::chrono::seconds(10));
    ASSERT_EQ(key, -1);
}

TEST_F(Gtest_ElementTest, key_volumedown) {
    int target_key = KEY_VOLUMEDOWN;
    int key = m_test->waitForInputEvent("sunxi-keyboard", target_key, std::chrono::seconds(100));
    ASSERT_EQ(key, target_key);
}

TEST_F(Gtest_ElementTest, key_volumeup) {
    int target_key = KEY_VOLUMEUP;
    int key = m_test->waitForInputEvent("sunxi-keyboard", target_key, std::chrono::seconds(100));
    ASSERT_EQ(key, target_key);
}

TEST_F(Gtest_ElementTest, key_lights_toggle) {
    int target_key = KEY_LIGHTS_TOGGLE;
    int key = m_test->waitForInputEvent("sunxi-keyboard", target_key, std::chrono::seconds(100));
    ASSERT_EQ(key, target_key);
}

GTEST_API_ int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
