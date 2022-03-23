#ifndef __ACTIVE_H__
#define __ACTIVE_H__

#include <atomic>
#include <unordered_set>

namespace AW {

class Active
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void onActiveEnable(){};
        virtual void onActivedisable(){};
    };

    void enable() {
        m_active.store(true);
        for(auto listener : m_listeners) {
            listener->onActiveEnable();
        }
    };

    void disable() {
        m_active.store(false);
        for(auto listener : m_listeners) {
            listener->onActivedisable();
        }
    };

    bool isEnable() { return m_active; };

    void addListener(std::shared_ptr<Listener> listener) {
        m_listeners.insert(listener);
    };
    void deleteListener(std::shared_ptr<Listener> listener) {
        m_listeners.erase(listener);
    };

private:
    std::atomic<bool> m_active{false};  //should add a ActiveManager? Ctr the all elemets avtivity?

    std::unordered_set<std::shared_ptr<Listener>> m_listeners{};

};

} /*namespace AW*/
#endif /*__ACTIVE_H__*/
