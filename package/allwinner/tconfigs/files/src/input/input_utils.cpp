#include "tconfigs/input/input_utils.h"

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include "tconfigs/log/logging.h"
#include "tconfigs/file/file_utils.h"

namespace tconfigs {
namespace input {

std::shared_ptr<InputUtils> InputUtils::Create(const std::string& device_name)
{
    auto input_utils = std::shared_ptr<InputUtils>(new InputUtils(device_name));
    if (!input_utils || !input_utils->Init()) {
        TCLOG_ERROR("Fail to create InputUtils \"%s\"", device_name.c_str());
        return nullptr;
    }
    return input_utils;
}

InputUtils::InputUtils(const std::string& device_name)
    : device_name_(device_name)
{
}

InputUtils::~InputUtils(void)
{
    if (event_fd_ >= 0) {
        close(event_fd_);
        event_fd_ = -1;
    }
}

bool InputUtils::ReadEvent(struct input_event* event)
{
    if (event_fd_ < 0) {
        TCLOG_ERROR("Invalid event file descriptor (%d)", event_fd_);
        return false;
    }
    if (sizeof(input_event) != read(event_fd_, event, sizeof(input_event))) {
        TCLOG_ERROR("Error while reading from event descriptor (%d)", event_fd_);
        return false;
    }
    return true;
}

bool InputUtils::Init(void)
{
    event_fd_ = GetEventFdByName(device_name_);
    if (event_fd_ < 0) {
        return false;
    }
    return true;
}

int InputUtils::GetEventFdByName(const std::string& device_name)
{
    static const std::string kEventPathPrefix = "/dev/input/event";

    std::string index = GetDeviceIndexByName(device_name);
    std::string event_path = kEventPathPrefix + index;

    int fd = open(event_path.c_str(), O_RDONLY);
    if (fd < 0) {
        TCLOG_ERROR("Fail to open device \"%s\" (event path: \"%s\")",
                device_name.c_str(), event_path.c_str());
        return -1;
    }
    return fd;
}

std::string InputUtils::GetDeviceIndexByName(const std::string& device_name)
{
    static const std::string kClassDirectory = "/sys/class/input";
    static const int kStringInputBytes = 5;
    std::string index = "";

    DIR* dir = opendir(kClassDirectory.c_str());
    if (!dir) {
        TCLOG_ERROR("Fail to open directory \"%s\"", kClassDirectory.c_str());
        return "";
    }

    struct dirent* dir_entry = NULL;
    while ((dir_entry = readdir(dir))) {
        std::string entry_name(dir_entry->d_name);
        if (0 != entry_name.compare(0, kStringInputBytes, "input")) {
            continue;
        }
        std::string name_path = kClassDirectory + "/" + entry_name + "/name";
        int buf_size = device_name.size() + 1;
        char name_buf[buf_size];
        file::FileUtils file_utils;
        if (0 != file_utils.OpenRead(name_path)) {
            continue;
        }
        int ret = file_utils.Read(name_buf, buf_size - 1);
        if (ret == file::FileUtils::kErrorReturnValue) {
            continue;
        }
        name_buf[buf_size - 1] = '\0';
        if (device_name == std::string(name_buf)) {
            index = entry_name.substr(kStringInputBytes);
            break;
        }
    }
    closedir(dir);
    TCLOG_DEBUG("Input event index: %s", index.c_str());
    return index;
}

} // namespace input
} // namespace tconfigs
