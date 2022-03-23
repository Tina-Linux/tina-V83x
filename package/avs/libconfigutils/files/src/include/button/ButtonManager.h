#ifndef __BUTTION_MANAGER_H__
#define __BUTTION_MANAGER_H__

#include <memory>
#include <unordered_set>

#include <json-c/json.h>

#include <watcher/KeyInputEventWatcher.h>

#include "button/IButtonManager.h"

namespace AW {

class Button
{
public:
    Button(const char *name, int code) : m_name{name}, m_code{code} {};
    const char *getName() { return m_name; };
    int getCode() { return m_code; }

private:
    const char *m_name{nullptr};
    int m_code{0};
};

class DummyButtonManager : public IButtonManager
{
public:
    static std::shared_ptr<DummyButtonManager> create(std::shared_ptr<AW::EventWatcher> watcher,
                                                      std::shared_ptr<AW::Executor> executor,
                                                      struct json_object *config){
        return std::make_shared<DummyButtonManager>();
    };

    void release(){};
    void addButtonObserver(std::shared_ptr<Observer> observer){};
    void removeButtonObserver(std::shared_ptr<Observer> observer){};
};

class ButtonManager : public IButtonManager,
                      public KeyInputEventWatcher::Listener,
                      public std::enable_shared_from_this<ButtonManager>
{
public:
    static std::shared_ptr<ButtonManager> create(std::shared_ptr<AW::EventWatcher> watcher,
                                                 std::shared_ptr<AW::Executor> executor,
                                                 struct json_object *config);

    void release();
    void addButtonObserver(std::shared_ptr<Observer> observer);
    void removeButtonObserver(std::shared_ptr<Observer> observer);
private:
    ButtonManager(std::shared_ptr<AW::EventWatcher> watcher,
                  std::shared_ptr<AW::Executor> executor) : m_watcher{watcher}, m_executor{executor} {};

    int init(struct json_object *config);
    int init_button(struct json_object *config);

    void onKeyEventDown(int keycode);
    void onKeyEventUp(int keycode);
    void onkeyEventLongClick(int keycode, float longclicktime);

private:
    std::shared_ptr<EventWatcher> m_watcher;
    std::shared_ptr<Executor> m_executor;
    std::unordered_set<std::shared_ptr<Observer>> m_observers{};

    std::unordered_set<std::shared_ptr<InputEventWatcher>> m_input_set{};
    std::unordered_set<std::shared_ptr<Button>> m_button_set{};
};
}
#endif
