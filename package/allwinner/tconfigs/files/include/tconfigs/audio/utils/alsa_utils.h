#ifndef __TCONFIGS_AUDIO_UTILS_ALSA_UTILS_H__
#define __TCONFIGS_AUDIO_UTILS_ALSA_UTILS_H__

#include <string>
#include <memory>
#include <unordered_map>
#include <alsa/asoundlib.h>

namespace tconfigs {
namespace audio {

class AlsaUtils {
public:
    enum class StreamType {
        PLAYBACK   = SND_PCM_STREAM_PLAYBACK,
        CAPTURE    = SND_PCM_STREAM_CAPTURE,
        UNKNOWN
    };

    enum class AccessType {
        RW_INTERLEAVED      = SND_PCM_ACCESS_RW_INTERLEAVED,
        MMAP_INTERLEAVED    = SND_PCM_ACCESS_MMAP_INTERLEAVED,
        UNKNOWN
    };

    enum class FormatType {
        S16_LE  = SND_PCM_FORMAT_S16_LE,
        S24_LE  = SND_PCM_FORMAT_S24_LE,
        S32_LE  = SND_PCM_FORMAT_S32_LE,
        UNKNOWN
    };

    struct Parameters {
        StreamType stream = StreamType::CAPTURE;
        AccessType access = AccessType::RW_INTERLEAVED;
        FormatType format = FormatType::S16_LE;
        int rate = 16000;
        int channels = 2;
        int period_frames = 1024;
        int periods = 4;
    };

    enum SoftwareParametersFlag : uint32_t {
        kAvailableFramesMinFlag = 1ul << 0, // snd_pcm_sw_params_set/get_avail_min
        kTimestampModeFlag      = 1ul << 1, // snd_pcm_sw_params_set/get_tstamp_mode
        kTimestampTypeFlag      = 1ul << 2, // snd_pcm_sw_params_set/get_tstamp_type
        kStartThresholdFlag     = 1ul << 3, // snd_pcm_sw_params_set/get_start_threshold
        kStopThresholdFlag      = 1ul << 4, // snd_pcm_sw_params_set/get_stop_threshold
        kSilenceThresholdFlag   = 1ul << 5, // snd_pcm_sw_params_set/get_silence_threshold
    };

    enum class TimestampMode {
        NONE    = SND_PCM_TSTAMP_NONE,
        ENABLE  = SND_PCM_TSTAMP_ENABLE,
        MMAP    = SND_PCM_TSTAMP_MMAP,
        UNKNOWN
    };

    enum class TimestampType {
        GETTIMEOFDAY    = SND_PCM_TSTAMP_TYPE_GETTIMEOFDAY,
        MONOTONIC       = SND_PCM_TSTAMP_TYPE_MONOTONIC,
        MONOTONIC_RAW   = SND_PCM_TSTAMP_TYPE_MONOTONIC_RAW,
        UNKNOWN
    };

    struct SoftwareParameters {
        int available_frames_min;
        TimestampMode timestamp_mode;
        TimestampType timestamp_type;
        int start_threshold;
        int stop_threshold;
        int silence_threshold;
    };

    enum StatusFlag : uint32_t {
        kStateFlag                  = 1ul << 0,     // snd_pcm_status_get_state
        kTriggerTimestampFlag       = 1ul << 1,     // snd_pcm_status_get_trigger_tstamp
        kLastUpdateTimestampFlag    = 1ul << 2,     // snd_pcm_status_get_tstamp
        kDelayFramesFlag            = 1ul << 3,     // snd_pcm_status_get_delay
        kAvailableFramesFlag        = 1ul << 4,     // snd_pcm_status_get_avail
        kAvailableFramesMaxFlag     = 1ul << 5,     // snd_pcm_status_get_avail_max
        kOverrangeFramesFlag        = 1ul << 6,     // snd_pcm_status_get_overrange
    };

    enum class State {
        OPEN            = SND_PCM_STATE_OPEN,
        SETUP           = SND_PCM_STATE_SETUP,
        PREPARED        = SND_PCM_STATE_PREPARED,
        RUNNING         = SND_PCM_STATE_RUNNING,
        XRUN            = SND_PCM_STATE_XRUN,
        DRAINING        = SND_PCM_STATE_DRAINING,
        PAUSED          = SND_PCM_STATE_PAUSED,
        SUSPENDED       = SND_PCM_STATE_SUSPENDED,
        DISCONNECTED    = SND_PCM_STATE_DISCONNECTED,
        UNKNOWN
    };

    struct Status {
        State state;
        struct timeval trigger_timestamp;
        struct timeval last_update_timestamp;
        int delay_frames;
        int available_frames;
        int available_frames_max;
        int overrange_frames;
    };

    static std::shared_ptr<AlsaUtils> Create(void);
    static std::shared_ptr<AlsaUtils> Create(const char* device, const Parameters* params);
    ~AlsaUtils(void);

    int ReadInterleaved(char* data, int frames);
    int WriteInterleaved(char* data, int frames);
    int Prepare(void);
    int Drop(void);
    int Drain(void);
    int Resume(void);

    int Start(void);
    int Pause(void);

    int GetSoftwareParameters(uint32_t flag, SoftwareParameters* sw_params);
    int SetSoftwareParameters(uint32_t flag, const SoftwareParameters* sw_params);

    int GetStatus(uint32_t flag, Status* status);

    static const char* ErrorString(int error_num);

    const std::string& device(void) const { return device_; }
    const Parameters* params(void) const { return &params_; }

private:
    AlsaUtils(void) = delete;
    AlsaUtils(const char* device, const Parameters* params);

    int Init(void);
    void Release(void);

    snd_pcm_t* snd_handle_ = nullptr;
    std::string device_;
    Parameters params_;

    bool support_pause_ = false;

    static const std::unordered_map<snd_pcm_state_t, std::string> kPcmStateToStringMap;
};

} // namespace audio
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_UTILS_ALSA_UTILS_H__ */
