#ifndef __TCONFIGS_INPUT_INPUT_UTILS_H__
#define __TCONFIGS_INPUT_INPUT_UTILS_H__

#include <linux/input.h>
#include <memory>
#include <string>

namespace tconfigs {
namespace input {

class InputUtils {
public:
    static std::shared_ptr<InputUtils> Create(const std::string& device_name);
    ~InputUtils(void);

    bool ReadEvent(struct input_event* event);

    const std::string& device_name(void) const { return device_name_; }
    int event_fd(void) const { return event_fd_; }

private:
    InputUtils(void) = delete;
    InputUtils(const std::string& device_name);

    bool Init(void);

    std::string GetDeviceIndexByName(const std::string& device_name);
    int GetEventFdByName(const std::string& device_name);

    std::string device_name_;
    int event_fd_ = -1;
};

} // namespace input
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_INPUT_INPUT_UTILS_H__ */
