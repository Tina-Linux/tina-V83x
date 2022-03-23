#ifndef __TCONFIGS_COMMON_SIGNAL_SLOT_H__
#define __TCONFIGS_COMMON_SIGNAL_SLOT_H__

#include <string>
#include <memory>
#include <list>
#include <functional>

namespace tconfigs {
namespace common {

template <typename... Args> class Slot;

template <typename... Args>
class Signal {
public:
    Signal(void) = delete;
    Signal(const std::string& name)
        : name_(name)
    {
    }
    ~Signal(void) = default;

    void Connect(std::shared_ptr<Slot<Args...>> slot)
    {
        for (auto itr = slots_.begin(); itr != slots_.end(); ++itr) {
            if ((*itr)->name() == slot->name()) {
                return;
            }
        }
        slots_.push_back(slot);
    }

    void Disconnect(std::shared_ptr<Slot<Args...>> slot)
    {
        for (auto itr = slots_.begin(); itr != slots_.end(); ++itr) {
            if (*itr == slot) {
                slots_.erase(itr);
                break;
            }
        }
    }

    void Disconnect(const std::string& slot_name)
    {
        for (auto itr = slots_.begin(); itr != slots_.end(); ++itr) {
            if ((*itr)->name() == slot_name) {
                slots_.erase(itr);
                break;
            }
        }
    }

    void Emit(Args... args)
    {
        for (auto itr = slots_.begin(); itr != slots_.end(); ++itr) {
            (*itr)->Callback(std::forward<Args>(args)...);
        }
    }

    const std::string& name(void) const { return name_; }

private:
    std::string name_;
    std::list<std::shared_ptr<Slot<Args...>>> slots_;
};

template <typename... Args>
class Slot {
public:
    typedef std::function<void (Args...)> CallbackType;

    Slot(void) = delete;
    Slot(const std::string& name, CallbackType callback)
        : name_(name),
          callback_(callback)
    {
    }
    ~Slot(void) = default;

    void Callback(Args... args)
    {
        callback_(std::forward<Args>(args)...);
    }

    const std::string& name(void) const { return name_; }

private:
    std::string name_;
    CallbackType callback_;
};

template <typename... Args>
std::shared_ptr<Signal<Args...>> SignalCreate(const std::string& name)
{
    return std::shared_ptr<Signal<Args...>>(new Signal<Args...>(name));
}

template <typename... Args>
std::shared_ptr<Slot<Args...>> SlotCreate(
        const std::string& name, typename Slot<Args...>::CallbackType callback)
{
    return std::shared_ptr<Slot<Args...>>(new Slot<Args...>(name, callback));
}

} // namespace common
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_COMMON_SIGNAL_SLOT_H__ */
