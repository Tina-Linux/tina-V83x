#ifndef __TCONFIGS_AUDIO_ELEMENTS_ALSA_ELEMEHNT_H__
#define __TCONFIGS_AUDIO_ELEMENTS_ALSA_ELEMEHNT_H__

#include "tconfigs/json/json_utils.h"
#include "tconfigs/audio/utils/alsa_utils.h"
#include "tconfigs/audio/common/element.h"

namespace tconfigs {
namespace audio {

class AlsaElement : public Element {
public:
    class AlsaDevice {
    public:
        static std::shared_ptr<AlsaDevice> Create(
                const std::string& name, int index, int loop_frames,
                const char* device, const AlsaUtils::Parameters* params);
        ~AlsaDevice(void) = default;

        void set_xrun_message(const Message& msg) { xrun_message_ = msg; }

        const std::string& name(void) const { return name_; }
        int index(void) const { return index_; }
        int loop_frames(void) const { return loop_frames_; }
        std::shared_ptr<AlsaUtils> utils(void) { return utils_; }
        const Message& xrun_message(void) const { return xrun_message_; }

    private:
        AlsaDevice(void) = delete;
        AlsaDevice(const std::string& name, int index, int loop_frames);
        bool Init(const char* device, const AlsaUtils::Parameters* params);

        std::string name_;
        int index_;
        int loop_frames_;
        std::shared_ptr<AlsaUtils> utils_ = nullptr;
        Message xrun_message_;
    };

    AlsaElement(void) = delete;
    AlsaElement(const std::string& name);
    virtual ~AlsaElement(void) = default;

    /**
     * @brief Create @c AlsaDevice from JSON config
     *
     * @param device_index
     * @param config
     * @param[in&out] params Some parameters not existed in config would use the
     *      default value in the input @c params
     * @return @c shared_ptr to @c AlsaDevice if successful, otherwise @c nullptr
     */
    std::shared_ptr<AlsaDevice> CreateAlsaDeviceFromConfig(
            int device_index,
            rapidjson::Value::ConstMemberIterator config,
            AlsaUtils::Parameters* params);

    bool SetSoftwareParametersFromConfig(std::shared_ptr<AlsaDevice> alsa_device,
            rapidjson::Value::ConstMemberIterator device_config);
    bool SetMessagesFromConfig(std::shared_ptr<AlsaDevice> alsa_device,
            rapidjson::Value::ConstMemberIterator device_config);

    bool GetPropertyFromAlsaDevice(
            std::shared_ptr<AlsaDevice> device, BufferProperty* property);

    void PrintXrunInformation(std::shared_ptr<AlsaUtils> utils);

    static AlsaUtils::AccessType StringToAlsaAccess(const std::string& access_string);
    static AlsaUtils::FormatType StringToAlsaFormat(const std::string& format_string);
    static AlsaUtils::TimestampMode StringToAlsaTimestampMode(const std::string& ts_mode_string);
    static AlsaUtils::TimestampType StringToAlsaTimestampType(const std::string& ts_type_string);
    static BufferProperty::FormatType AlsaFormatToPropertyFormat(AlsaUtils::FormatType format);
    static BufferProperty::StorageType AlsaAccessToPropertyStorage(AlsaUtils::AccessType access);

private:
    bool GetDeviceParameters(rapidjson::Value::ConstMemberIterator device_config,
            AlsaUtils::Parameters* params);

    static const std::unordered_map<std::string, AlsaUtils::AccessType>
        kStringToAlsaAccessMap;
    static const std::unordered_map<std::string, AlsaUtils::FormatType>
        kStringToAlsaFormatMap;
    static const std::unordered_map<std::string, AlsaUtils::TimestampMode>
        kStringToAlsaTimestampModeMap;
    static const std::unordered_map<std::string, AlsaUtils::TimestampType>
        kStringToAlsaTimestampTypeMap;
    static const std::unordered_map<AlsaUtils::FormatType, BufferProperty::FormatType>
        kAlsaFormatToPropertyFormatMap;
    static const std::unordered_map<AlsaUtils::AccessType, BufferProperty::StorageType>
        kAlsaAccessToPropertyStorageMap;
};

} // namespace audio
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_ELEMENTS_ALSA_ELEMEHNT_H__ */
