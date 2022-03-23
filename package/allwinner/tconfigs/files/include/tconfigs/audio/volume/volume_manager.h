#ifndef __TCONFIGS_AUDIO_VOLUME_VOLUME_MANAGER_H__
#define __TCONFIGS_AUDIO_VOLUME_VOLUME_MANAGER_H__

#include <memory>
#include <string>
#include <unordered_map>

#include "tconfigs/json/json_utils.h"
#include "tconfigs/audio/utils/amixer_utils.h"

namespace tconfigs {
namespace audio {

class VolumeManager {
public:

    class MuteSwitch {
    public:
        MuteSwitch(const std::string& device, int numid);
        MuteSwitch(const std::string& device,
                const std::string& iface, const std::string& element_name);
        ~MuteSwitch(void) = default;

        void InitMuteValue(int mute_value, int unmute_value) {
            mute_value_ = mute_value;
            unmute_value_ = unmute_value;
        }

        bool SetMute(bool mute);
        bool IsMute(void);

    private:
        int mute_value_ = 0;
        int unmute_value_ = 1;

        AmixerControl control_;
    };

    class VolumeControl {
    public:
        VolumeControl(const std::string& device, int numid);
        VolumeControl(const std::string& device,
                const std::string& iface, const std::string& element_name);
        ~VolumeControl(void) = default;

        bool InitValidRange(int min, int max) {
            if (min > max) {
                return false;
            }
            valid_min_ = min;
            valid_max_ = max;
            return true;
        }
        bool InitActualRange(void);

        bool SetVolume(int volume);
        bool GetVolume(int* volume);

    private:
        int valid_min_ = 0;
        int valid_max_ = 100;

        int actual_min_ = 0;
        int actual_max_ = 100;

        // Sometimes we cannot get the ALSA volume control range at the beginning.
        // For example, the softvol control won't appear until the newly defined
        // device is firstly used. Use the following bool flag to judge whether
        // it has got the actual range of volume control value, if not, it will
        // try to get this range every time it sets or gets the control value.
        bool actual_range_has_got_ = false;

        AmixerControl control_;
    };

    static std::shared_ptr<VolumeManager> Create(const rapidjson::Value& config);
    ~VolumeManager(void) = default;

    std::shared_ptr<MuteSwitch> FindMuteSwitch(const std::string& name);
    std::shared_ptr<VolumeControl> FindVolumeControl(const std::string& name);

private:
    VolumeManager(void) = default;

    bool Init(const rapidjson::Value& config);

    std::shared_ptr<MuteSwitch> CreateMuteSwitch(const rapidjson::Value& config);
    std::shared_ptr<VolumeControl> CreateVolumeControl(const rapidjson::Value& config);

    std::unordered_map<std::string, std::shared_ptr<MuteSwitch>> mute_switches_;
    std::unordered_map<std::string, std::shared_ptr<VolumeControl>> volume_controls_;
};

} // namespace audio
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_VOLUME_VOLUME_MANAGER_H__ */
