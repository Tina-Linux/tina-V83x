#ifndef __TCONFIGS_AUDIO_COMMON_ANY_H__
#define __TCONFIGS_AUDIO_COMMON_ANY_H__

#include <memory>
#include <utility>
#include <type_traits>
//#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace common {

class Any {
public:
    Any(void) = delete;

    Any(const Any& that)
        : holder_(that.holder_->Clone())
    {
//        TCLOG_DEBUG("Any: lvalue copy constructor");
    }

    Any(Any&& that)
        : holder_(std::move(that.holder_))
    {
//        TCLOG_DEBUG("Any: rvalue copy constructor");
    }

    Any& operator=(const Any& that)
    {
//        TCLOG_DEBUG("Any: operator =");
        holder_ = that.holder_->Clone();
        return *this;
    }

    template <typename T,
             typename = typename std::enable_if<
                 !std::is_same<typename std::decay<T>::type, Any>::value, T>::type>
    Any(T&& value)
        : holder_(new Holder<typename std::decay<T>::type>(std::forward<T>(value)))
    {
//        TCLOG_DEBUG("Any: constructor T&&");
    }

    ~Any(void) = default;

    template <typename T>
    T Cast(void)
    {
        return std::static_pointer_cast<Holder<T>>(holder_)->value();
    }

private:
    class HolderInterface {
    public:
        virtual ~HolderInterface(void) = default;
        virtual std::shared_ptr<HolderInterface> Clone(void) const = 0;
    };

    template <typename T>
    class Holder : public HolderInterface {
    public:
        Holder(void) = delete;
        Holder(const T& value)
            : value_(value)
        {
//            TCLOG_DEBUG("Holder: constructor const T&");
        }
        Holder(T&& value)
            : value_(std::forward<T>(value))
        {
//            TCLOG_DEBUG("Holder: constructor T&&");
        }
        std::shared_ptr<HolderInterface> Clone(void) const override
        {
            return std::shared_ptr<Holder<T>>(new Holder<T>(value_));
        }

        T& value(void) { return value_; }
    private:
        T value_;
    };

    std::shared_ptr<HolderInterface> holder_ = nullptr;
};

} // namespace common
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_COMMON_ANY_H__ */
