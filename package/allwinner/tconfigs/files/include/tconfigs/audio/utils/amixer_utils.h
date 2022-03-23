#ifndef __TCONFIGS_AUDIO_UTILS_AMIXER_UTILS_H__
#define __TCONFIGS_AUDIO_UTILS_AMIXER_UTILS_H__

#include <string>
#include <vector>

#include <alsa/asoundlib.h>

#include "tconfigs/common/any.h"

namespace tconfigs {
namespace audio {

class AmixerUtils {
public:
    AmixerUtils(void) = default;
    ~AmixerUtils(void) = default;

    bool SetControl(const std::string& device, int numid, const std::string& value);
    bool SetControl(const std::string& device, const std::string& iface,
            const std::string& name, const std::string& value);

    bool GetControl(const std::string& device, int numid, std::vector<int>* values);
    bool GetControl(const std::string& device, const std::string& iface,
           const std::string& name, std::vector<int>* values);

    bool GetControlWithRange(const std::string& device, int numid,
            std::vector<int>* values, int* minimum, int* maximum);
    bool GetControlWithRange(const std::string& device, const std::string& iface,
           const std::string& name, std::vector<int>* values,
           int* minimum, int* maximum);

private:
    class OperationUtils {
    public:
        OperationUtils(void);
        ~OperationUtils(void);

        bool Init(const std::string& device, const std::string& id_string);

        snd_ctl_t* handle(void) { return handle_; }
        snd_ctl_elem_info_t* info(void) { return info_; }
        snd_ctl_elem_id_t* id(void) { return id_; }
        snd_ctl_elem_value_t* control(void) { return control_; }

    private:
        snd_ctl_t* handle_ = nullptr;
        snd_ctl_elem_info_t* info_ = nullptr;
        snd_ctl_elem_id_t* id_ = nullptr;
        snd_ctl_elem_value_t* control_ = nullptr;
        bool handle_is_opened_ = false;
    };

    // The comment after each enumeration is its type
    enum class Type {
        kUnknown,
        kInteger,       // int
        kBoolean,       // int
        kEnumerated,    // int
    };

    struct Value {
        Type type = Type::kUnknown;
        std::vector<common::Any> contents;
    };

    std::string GetControlElementIdString(int numid);
    std::string GetControlElementIdString(
            const std::string& iface, const std::string& name);

    bool WriteControl(const std::string& device, const std::string& id_string,
            const std::string& value);
    bool ReadControl(const std::string& device, const std::string& id_string,
            Value* value);
    bool ReadControlWithRange(const std::string& device,
            const std::string& id_string, Value* value,
            int* minimum, int* maximum);
};

class AmixerControl {
public:
    AmixerControl(const std::string& device, int numid);
    AmixerControl(const std::string& device,
            const std::string& iface, const std::string& element_name);
    ~AmixerControl(void) = default;

    bool SetValue(const std::string& value);

    // TODO: Currently it only supports getting the first item if the control
    // has more than one value. Add extended function such as
    // "GetValue(int* value, int index)" to support selecting item;
    bool GetValue(int* value);
    bool GetValueWithRange(int* value, int* minimum, int* maximum);

private:
    enum class IdType {
        kNumid,
        kIfaceAndName
    };

    std::string device_;

    IdType id_type_;
    int numid_;
    std::string iface_;
    std::string element_name_;

    AmixerUtils utils_;
};

} // namespace audio
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_UTILS_AMIXER_UTILS_H__ */
