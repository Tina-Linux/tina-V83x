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

#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

#include "watcher/EventWatcher.h"
#include "threading/Executor.h"
#include "Element.h"

using namespace AW;

class element_listener : public Element::Listener
{
public:
    void onElementProcessStart(Element *element){
        printf("%p: onElementStart\n", element);
    }
    void onElementProcessFinished(Element *element){
        printf("%p: onElementFinished\n", element);
    }

};
class unit_test_element1 : public Element
{
public:
    static std::shared_ptr<Element> create(std::shared_ptr<Executor> executor,
                                            std::shared_ptr<Active> active,
                                            std::unordered_set<std::shared_ptr<Listener>> listeners) {
        auto element = std::shared_ptr<Element>(new unit_test_element1(executor, active, listeners));
        active->addListener(element);
        return element;
    }
    ~unit_test_element1() {
        printf("%p: ~unit_test_element1\n", this);
    }
private:
    unit_test_element1(std::shared_ptr<Executor> executor,
                       std::shared_ptr<Active> active,
                       std::unordered_set<std::shared_ptr<Listener>> listeners) :
                Element{executor, active, listeners} {};
    void process(){
        printf("%p: do now\n", this);
    }
};

class unit_test_element2 : public Element
{
public:
    static std::shared_ptr<Element> create(std::shared_ptr<EventWatcher> watcher,
                                            float timeout_s,
                                            std::shared_ptr<Executor> executor,
                                            std::shared_ptr<Active> active,
                                            std::unordered_set<std::shared_ptr<Listener>> listeners) {
        auto element = std::shared_ptr<Element>(new unit_test_element2(watcher, timeout_s, executor, active, listeners));
        active->addListener(element);
        return element;
    }
private:
    unit_test_element2(std::shared_ptr<EventWatcher> watcher,
                       float timeout_s,
                       std::shared_ptr<Executor> executor,
                       std::shared_ptr<Active> active,
                       std::unordered_set<std::shared_ptr<Listener>> listeners) :
                Element{watcher, timeout_s, executor, active, listeners} {};
    void process(){
        printf("%p: do delay\n", this);
    }
};

class ElementTest : public Element::Listener,
                    public std::enable_shared_from_this<ElementTest>{
public:
    ElementTest() {
        printf("ElementTest\n");
        m_watcher = EventWatcher::create();
        m_executor = Executor::create();
        m_active = std::make_shared<Active>();

        m_watcher->startWatcher();

    };

    ~ElementTest() {
        printf("~ElementTest\n");
    }

    void action(float timeout_s) {
        if(timeout_s < 0.001){
            m_element = unit_test_element1::create(m_executor, m_active, {shared_from_this()});
        }else{
            m_element = unit_test_element2::create(m_watcher, timeout_s, m_executor, m_active, {shared_from_this()});
        }
        m_active->enable();
        m_element->action();
    }

    Element::State getState() { return m_element->getState(); }

    Element::State waitForElementFinished(const std::chrono::seconds duration) {
        return m_element->waitForElementFinished(duration);
    }

    bool isProcess() { return m_is_process; };
    void onElementProcessStart(Element *element){
        printf("%p: onElementStart\n", element);
    }

    void disableAsync(int later) {
        m_executor->submit([this, later]() {
            sleep(later);
            m_active->disable();
        });
    }

    void onElementProcessFinished(Element *element){
        m_is_process = true;
        printf("%p: onElementFinished\n", element);
    }

    std::shared_ptr<EventWatcher> m_watcher;
    std::shared_ptr<Executor> m_executor;

    std::shared_ptr<Element> m_element;

    std::shared_ptr<Active> m_active;

    bool m_is_process = false;

    std::shared_ptr<Element::Listener> m_listener;
};

class Gtest_ElementTest : public ::testing::Test {
protected:

    virtual void SetUp() override {
        printf("SetUp\n");
        m_test = std::shared_ptr<ElementTest>(new ElementTest());
    };

    void TearDown() override {
        printf("TearDown\n");
        m_test.reset();
    }

    std::shared_ptr<ElementTest> m_test;

};

#if 1
TEST_F(Gtest_ElementTest, handleElementImmediately) {
    m_test->action(0);
    m_test->waitForElementFinished(std::chrono::seconds(1));
    ASSERT_EQ(m_test->getState(), Element::State::IDLE);
    ASSERT_EQ(m_test->m_is_process, true);
}

TEST_F(Gtest_ElementTest, handleElementDelay) {
    m_test->action(3);
    ASSERT_EQ(m_test->getState(), Element::State::PROCESSING);
    m_test->waitForElementFinished(std::chrono::seconds(10));
    ASSERT_EQ(m_test->getState(), Element::State::IDLE);
}

TEST_F(Gtest_ElementTest, waitForAfterElementFinished) {
    m_test->action(0);
    sleep(1);
    m_test->waitForElementFinished(std::chrono::seconds(1));
    ASSERT_EQ(m_test->getState(), Element::State::IDLE);
}

TEST_F(Gtest_ElementTest, waitForBeforeElementFinished_getState) {
    m_test->action(1.5);
    m_test->waitForElementFinished(std::chrono::seconds(1));
    ASSERT_EQ(m_test->getState(), Element::State::PROCESSING);
    ASSERT_EQ(m_test->waitForElementFinished(std::chrono::seconds(10)), Element::State::IDLE);
}

TEST_F(Gtest_ElementTest, waitForBeforeElementFinished_return) {
    m_test->action(1.5);
    ASSERT_EQ(m_test->waitForElementFinished(std::chrono::seconds(1)), Element::State::PROCESSING);
    ASSERT_EQ(m_test->waitForElementFinished(std::chrono::seconds(10)), Element::State::IDLE);
}
#endif
TEST_F(Gtest_ElementTest, activeDisable) {
    m_test->action(1.5);
    m_test->m_active->disable();
    ASSERT_EQ(m_test->getState(), Element::State::IDLE);
    sleep(2);
    ASSERT_EQ(m_test->m_is_process, false);
}

TEST_F(Gtest_ElementTest, activeDisableAsync) {
    m_test->action(2);
    m_test->disableAsync(1);

    ASSERT_EQ(m_test->getState(), Element::State::PROCESSING);
    ASSERT_EQ(m_test->waitForElementFinished(std::chrono::seconds(10)), Element::State::IDLE);

    sleep(2);
    ASSERT_EQ(m_test->m_is_process, false);
}

GTEST_API_ int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
