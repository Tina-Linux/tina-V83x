#include "tconfigs/audio/utils/amixer_utils.h"

#include "tconfigs/log/logging.h"
#include "tconfigs/common/string.h"

namespace tconfigs {
namespace audio {

bool AmixerUtils::SetControl(const std::string& device, int numid, const std::string& value)
{
    std::string id_string = GetControlElementIdString(numid);
    return WriteControl(device, id_string, value);
}

bool AmixerUtils::SetControl(const std::string& device, const std::string& iface,
        const std::string& name, const std::string& value)
{
    std::string id_string = GetControlElementIdString(iface, name);
    return WriteControl(device, id_string, value);
}

bool AmixerUtils::GetControl(const std::string& device, int numid,
        std::vector<int>* values)
{
    std::string id_string = GetControlElementIdString(numid);
    Value value_got;
    if (!ReadControl(device, id_string, &value_got)) {
        return false;
    }
    for (auto itr = value_got.contents.begin(); itr != value_got.contents.end(); ++itr) {
        switch (value_got.type) {
        case Type::kBoolean:
        case Type::kInteger:
        case Type::kEnumerated:
            values->push_back(itr->Cast<int>());
            break;
        default:
            TCLOG_ERROR("Control \"%s\" numid=%d: the value type is not int",
                    device.c_str(), numid);
            return false;
        }
    }
    return true;
}

bool AmixerUtils::GetControl(const std::string& device, const std::string& iface,
        const std::string& name, std::vector<int>* values)
{
    std::string id_string = GetControlElementIdString(iface, name);
    Value value_got;
    if (!ReadControl(device, id_string, &value_got)) {
        return false;
    }
    for (auto itr = value_got.contents.begin(); itr != value_got.contents.end(); ++itr) {
        switch (value_got.type) {
        case Type::kBoolean:
        case Type::kInteger:
        case Type::kEnumerated:
            values->push_back(itr->Cast<int>());
            break;
        default:
            TCLOG_ERROR("Control \"%s\" iface=%s,name='%s': the value type is not int",
                    device.c_str(), iface.c_str(), name.c_str());
            return false;
        }
    }
    return true;
}

bool AmixerUtils::GetControlWithRange(const std::string& device, int numid,
        std::vector<int>* values, int* minimum, int* maximum)
{
    std::string id_string = GetControlElementIdString(numid);
    Value value_got;
    if (!ReadControlWithRange(device, id_string, &value_got, minimum, maximum)) {
        return false;
    }
    for (auto itr = value_got.contents.begin(); itr != value_got.contents.end(); ++itr) {
        switch (value_got.type) {
        case Type::kInteger:
            values->push_back(itr->Cast<int>());
            break;
        default:
            TCLOG_ERROR("Control \"%s\" numid=%d: the value type is not int",
                    device.c_str(), numid);
            return false;
        }
    }
    return true;
}

bool AmixerUtils::GetControlWithRange(const std::string& device,
        const std::string& iface, const std::string& name,
        std::vector<int>* values, int* minimum, int* maximum)
{
    std::string id_string = GetControlElementIdString(iface, name);
    Value value_got;
    if (!ReadControlWithRange(device, id_string, &value_got, minimum, maximum)) {
        return false;
    }
    for (auto itr = value_got.contents.begin(); itr != value_got.contents.end(); ++itr) {
        switch (value_got.type) {
        case Type::kInteger:
            values->push_back(itr->Cast<int>());
            break;
        default:
            TCLOG_ERROR("Control \"%s\" iface=%s,name='%s': the value type is not int",
                    device.c_str(), iface.c_str(), name.c_str());
            return false;
        }
    }
    return true;
}

bool AmixerUtils::WriteControl(const std::string& device,
        const std::string& id_string, const std::string& value)
{
    OperationUtils utils;
    if (!utils.Init(device, id_string)) {
        TCLOG_ERROR("Fail to init operation utils");
        return false;
    }

    if (snd_ctl_ascii_value_parse(utils.handle(), utils.control(), utils.info(),
                value.c_str()) < 0) {
        TCLOG_ERROR("Control \"%s\" element \"%s\": value \"%s\" parse error",
                device.c_str(), id_string.c_str(), value.c_str());
        return false;
    }

    if (snd_ctl_elem_write(utils.handle(), utils.control()) < 0) {
        TCLOG_ERROR("Control \"%s\" element \"%s\": value \"%s\" write error",
                device.c_str(), id_string.c_str(), value.c_str());
        return false;
    }

    return true;
}

bool AmixerUtils::ReadControl(const std::string& device,
        const std::string& id_string, Value* value)
{
    OperationUtils utils;
    if (!utils.Init(device, id_string)) {
        TCLOG_ERROR("Fail to init operation utils");
        return false;
    }

    unsigned int cnt = snd_ctl_elem_info_get_count(utils.info());
    snd_ctl_elem_type_t type = snd_ctl_elem_info_get_type(utils.info());

    for (unsigned int i = 0; i < cnt; ++i) {
        switch (type) {
        case SND_CTL_ELEM_TYPE_INTEGER: {
            int content = snd_ctl_elem_value_get_integer(utils.control(), i);
            value->type = Type::kInteger;
            value->contents.push_back(content);
            break;
        }
        case SND_CTL_ELEM_TYPE_BOOLEAN: {
            int content = snd_ctl_elem_value_get_boolean(utils.control(), i);
            value->type = Type::kBoolean;
            value->contents.push_back(content);
            break;
        }
        case SND_CTL_ELEM_TYPE_ENUMERATED: {
            int content = snd_ctl_elem_value_get_enumerated(utils.control(), i);
            value->type = Type::kEnumerated;
            value->contents.push_back(content);
            break;
        }
        default:
            TCLOG_ERROR("Control \"%s\" element \"%s\": not supported element type",
                    device.c_str(), id_string.c_str());
            return false;
        }
    }

    return true;
}

bool AmixerUtils::ReadControlWithRange(const std::string& device,
        const std::string& id_string, Value* value, int* minimum, int* maximum)
{
    OperationUtils utils;
    if (!utils.Init(device, id_string)) {
        TCLOG_ERROR("Fail to init operation utils");
        return false;
    }

    snd_ctl_elem_type_t type = snd_ctl_elem_info_get_type(utils.info());
    switch (type) {
    case SND_CTL_ELEM_TYPE_INTEGER:
        *minimum = snd_ctl_elem_info_get_min(utils.info());
        *maximum = snd_ctl_elem_info_get_max(utils.info());
        break;
    default:
        TCLOG_ERROR("Control \"%s\" element \"%s\": not supported element type",
                device.c_str(), id_string.c_str());
        return false;
    }

    unsigned int cnt = snd_ctl_elem_info_get_count(utils.info());
    for (unsigned int i = 0; i < cnt; ++i) {
        int content = snd_ctl_elem_value_get_integer(utils.control(), i);
        value->type = Type::kInteger;
        value->contents.push_back(content);
    }
    return true;
}

std::string AmixerUtils::GetControlElementIdString(int numid)
{
    std::string id_string = "numid=" + common::IntToString(numid);
    return id_string;
}

std::string AmixerUtils::GetControlElementIdString(
        const std::string& iface, const std::string& name)
{
    std::string id_string = "iface=" + iface + ",name='" + name + "'";
    return id_string;
}

AmixerUtils::OperationUtils::OperationUtils(void)
{
    snd_ctl_elem_info_malloc(&info_);
    snd_ctl_elem_id_malloc(&id_);
    snd_ctl_elem_value_malloc(&control_);
}

AmixerUtils::OperationUtils::~OperationUtils(void)
{
    if (handle_is_opened_) {
        snd_ctl_close(handle_);
    }
    snd_ctl_elem_value_free(control_);
    snd_ctl_elem_id_free(id_);
    snd_ctl_elem_info_free(info_);
}

bool AmixerUtils::OperationUtils::Init(
        const std::string& device, const std::string& id_string)
{
    if (0 != snd_ctl_ascii_elem_id_parse(id_, id_string.c_str())) {
        TCLOG_ERROR("Wrong control identifier: %s", id_string.c_str());
        return false;
    }

    if (snd_ctl_open(&handle_, device.c_str(), 0) < 0) {
        TCLOG_ERROR("Control \"%s\" open error", device.c_str());
        return false;
    }
    handle_is_opened_ = true;

    snd_ctl_elem_info_set_id(info_, id_);
    if (snd_ctl_elem_info(handle_, info_) < 0) {
        TCLOG_ERROR("Cannot find the given element \"%s\" from control \"%s\"",
                id_string.c_str(), device.c_str());
        return false;
    }

    snd_ctl_elem_info_get_id(info_, id_);

    snd_ctl_elem_value_set_id(control_, id_);
    if (snd_ctl_elem_read(handle_, control_) < 0) {
        TCLOG_ERROR("Cannot read the given element \"%s\" from control \"%s\"",
                id_string.c_str(), device.c_str());
        return false;
    }

    return true;
}

// AmixerControl ===============================================================
AmixerControl::AmixerControl(const std::string& device, int numid)
    : device_(device),
      id_type_(IdType::kNumid),
      numid_(numid)
{
}

AmixerControl::AmixerControl(const std::string& device,
        const std::string& iface, const std::string& element_name)
    : device_(device),
      id_type_(IdType::kIfaceAndName),
      iface_(iface),
      element_name_(element_name)
{
}

bool AmixerControl::SetValue(const std::string& value)
{
    switch (id_type_) {
    case IdType::kNumid:
        if (!utils_.SetControl(device_, numid_, value)) {
            TCLOG_ERROR("Control \"%s\" numid=%d: fail to set value \"%s\"",
                    device_.c_str(), numid_, value.c_str());
            return false;
        }
        break;
    case IdType::kIfaceAndName:
        if (!utils_.SetControl(device_, iface_, element_name_, value)) {
            TCLOG_ERROR("Control \"%s\" iface=%s,name='%s': fail to set value \"%s\"",
                    device_.c_str(), iface_.c_str(), element_name_.c_str(),
                    value.c_str());
            return false;
        }
        break;
    default:
        return false;
    }
    return true;
}

bool AmixerControl::GetValue(int* value)
{
    std::vector<int> values_got;

    switch (id_type_) {
    case IdType::kNumid:
        if (!utils_.GetControl(device_, numid_, &values_got) || values_got.empty()) {
            TCLOG_ERROR("Control \"%s\" numid=%d: fail to get value",
                    device_.c_str(), numid_);
            return false;
        }
        break;
    case IdType::kIfaceAndName:
        if (!utils_.GetControl(device_, iface_, element_name_, &values_got)
                || values_got.empty()) {
            TCLOG_ERROR("Control \"%s\" iface=%s,name='%s': fail to set value",
                    device_.c_str(), iface_.c_str(), element_name_.c_str());
            return false;
        }
        break;
    default:
        return false;
    }

    *value = values_got.front();
    return true;
}

bool AmixerControl::GetValueWithRange(int* value, int* minimum, int* maximum)
{
    std::vector<int> values_got;

    switch (id_type_) {
    case IdType::kNumid:
        if (!utils_.GetControlWithRange(
                    device_, numid_, &values_got, minimum, maximum)
                || values_got.empty()) {
            TCLOG_ERROR("Control \"%s\" numid=%d: fail to get value",
                    device_.c_str(), numid_);
            return false;
        }
        break;
    case IdType::kIfaceAndName:
        if (!utils_.GetControlWithRange(
                    device_, iface_, element_name_, &values_got, minimum, maximum)
                || values_got.empty()) {
            TCLOG_ERROR("Control \"%s\" iface=%s,name='%s': fail to set value",
                    device_.c_str(), iface_.c_str(), element_name_.c_str());
            return false;
        }
        break;
    default:
        return false;
    }

    *value = values_got.front();
    return true;
}

} // namespace audio
} // namespace tconfigs
