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
#include "watcher/KeyInputEventWatcher.h"

using namespace AW;

class KeyInputEventWatcherTest : public KeyInputEventWatcher::Listener,
                                 public std::enable_shared_from_this<KeyInputEventWatcherTest>{
public:
    KeyInputEventWatcherTest() {
        printf("KeyInputEventWatcherTest\n");
        m_watcher = EventWatcher::create();
        m_executor = Executor::create();

        m_watcher->startWatcher();

    };

    ~KeyInputEventWatcherTest() {
        m_input->release();
        m_input = nullptr;
        m_watcher->stopWatcher();
        printf("~KeyInputEventWatcherTest\n");
    }

    void onKeyEventDown(int keycode){
        m_is_down = true;
        printf("onKeyEventDown, this: %p\n", this);
        m_processTrigger.notify_all();
    }

    void onKeyEventUp(int keycode){
        m_is_up = true;
        printf("onKeyEventUp, this: %p\n",this);
        m_mutil_result.insert(keycode);
        m_processTrigger.notify_all();
    }

    void onkeyEventLongClick(int keycode, float longclicktime){
        m_is_longclick = true;
        printf("onkeyEventLongClick, this: %p\n", this);
        m_processTrigger.notify_all();
    }


    bool waitForKeyInputEvent(const std::string name, int target_key, float longclicktime, const std::chrono::seconds duration) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_input = InputEventWatcher::create(m_watcher, name);
        m_key = KeyInputEventWatcher::create(m_watcher, m_executor, m_input, target_key, longclicktime, shared_from_this());

        m_processTrigger.wait_for(lock, duration, [this, target_key]() {return m_is_down; });
        if(longclicktime > 0.0001)
            m_processTrigger.wait_for(lock, duration, [this, target_key]() {return m_is_longclick; });
        m_processTrigger.wait_for(lock, duration, [this, target_key]() {return m_is_up; });
        m_key->release();
        m_key = nullptr;
        m_input->release();
        if(longclicktime > 0.0001)
            return m_is_down&&m_is_up&&m_is_longclick;
        else
            return m_is_down&&m_is_up;
    }

    bool waitForKeyInputEvent(const std::string name, std::unordered_set<int> target_key, float longclicktime, const std::chrono::seconds duration) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_input = InputEventWatcher::create(m_watcher, name);
        std::unordered_set<std::shared_ptr<KeyInputEventWatcher>> key_set;
        for(auto keycode : target_key){
            auto key = KeyInputEventWatcher::create(m_watcher, m_executor, m_input, keycode, longclicktime, shared_from_this());
            key_set.insert(key);
        }
        m_processTrigger.wait_for(lock, duration, [this, target_key]() {return m_mutil_result.size() == target_key.size(); });

        for(auto key : key_set) {
            key->release();
        }
        key_set.clear();
        m_input->release();

        return m_mutil_result.size() == target_key.size();
    }

    std::shared_ptr<EventWatcher> m_watcher;
    std::shared_ptr<Executor> m_executor;
    std::shared_ptr<InputEventWatcher> m_input;
    std::shared_ptr<KeyInputEventWatcher> m_key;

    std::unordered_set<int> m_mutil_result;

    bool m_is_down = false;
    bool m_is_up = false;
    bool m_is_longclick = false;

    std::mutex m_mutex;
    std::condition_variable m_processTrigger;
};


class Gtest_ElementTest : public ::testing::Test {

protected:
    virtual void SetUp() override {
        printf("SetUp\n");
        m_test = std::shared_ptr<KeyInputEventWatcherTest>(new KeyInputEventWatcherTest());
    };

    void TearDown() override {
        printf("~TearDown use_count:%lld\n",m_test.use_count());
        m_test = nullptr;
    }

    std::shared_ptr<KeyInputEventWatcherTest> m_test;

};

TEST_F(Gtest_ElementTest, key_power) {
    int target_key = KEY_POWER;
    bool key = m_test->waitForKeyInputEvent("axp803-powerkey", target_key, -1, std::chrono::seconds(10));
    ASSERT_EQ(key, true);
}

TEST_F(Gtest_ElementTest, key_power_longclick) {
    int target_key = KEY_POWER;
    bool key = m_test->waitForKeyInputEvent("axp803-powerkey", target_key, 1.5, std::chrono::seconds(10));
    ASSERT_EQ(key, true);
}

TEST_F(Gtest_ElementTest, key_mutil) {

    bool key = m_test->waitForKeyInputEvent("sunxi-keyboard", {KEY_VOLUMEDOWN, KEY_VOLUMEUP, KEY_LIGHTS_TOGGLE}, 1.5, std::chrono::seconds(100));
    ASSERT_EQ(key, true);

    sleep(4);
}

GTEST_API_ int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
