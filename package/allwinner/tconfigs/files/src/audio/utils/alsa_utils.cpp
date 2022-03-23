#include "tconfigs/audio/utils/alsa_utils.h"
#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace audio {

const std::unordered_map<snd_pcm_state_t, std::string>
AlsaUtils::kPcmStateToStringMap = {
    {SND_PCM_STATE_OPEN, "OPEN"},
    {SND_PCM_STATE_SETUP, "SETUP"},
    {SND_PCM_STATE_PREPARED, "PREPARED"},
    {SND_PCM_STATE_RUNNING, "RUNNING"},
    {SND_PCM_STATE_XRUN, "XRUN"},
    {SND_PCM_STATE_DRAINING, "DRAINING"},
    {SND_PCM_STATE_PAUSED, "PAUSED"},
    {SND_PCM_STATE_SUSPENDED, "SUSPENDED"},
    {SND_PCM_STATE_DISCONNECTED, "DISCONNECTED"},
    {SND_PCM_STATE_LAST, "LAST(DISCONNECTED)"}
};

std::shared_ptr<AlsaUtils> AlsaUtils::Create(void)
{
    Parameters params;
    return Create("default", &params);
}

std::shared_ptr<AlsaUtils> AlsaUtils::Create(
        const char* device, const Parameters* params)
{
    auto utils = std::shared_ptr<AlsaUtils>(new AlsaUtils(device, params));
    if (!utils || 0 != utils->Init()) {
        return nullptr;
    }
    return utils;
}

AlsaUtils::AlsaUtils(const char* device, const Parameters* params)
    : device_(device),
      params_(*params)
{
}

AlsaUtils::~AlsaUtils(void)
{
    Release();
}

int AlsaUtils::Init(void)
{
    int ret = 0;
    snd_pcm_hw_params_t *hw_params;
    unsigned int rate;
    snd_pcm_uframes_t period_frames;
    unsigned int periods;

    ret = snd_pcm_open(&snd_handle_, device_.c_str(),
            static_cast<snd_pcm_stream_t>(params_.stream), 0);
    if (ret < 0) {
        TCLOG_ERROR("%s: snd_pcm_open failed (%s)", device_.c_str(), snd_strerror(ret));
        goto out;
    }

    snd_pcm_hw_params_alloca(&hw_params);

    ret = snd_pcm_hw_params_any(snd_handle_, hw_params);
    if (ret < 0) {
        TCLOG_ERROR("%s: snd_pcm_hw_params_any failed", device_.c_str());
        goto out;
    }

    ret = snd_pcm_hw_params_set_access(snd_handle_, hw_params,
            static_cast<snd_pcm_access_t>(params_.access));
    if (ret < 0) {
        TCLOG_ERROR("%s: snd_pcm_hw_params_set_access failed", device_.c_str());
        goto out;
    }

    ret = snd_pcm_hw_params_set_format(snd_handle_, hw_params,
            static_cast<snd_pcm_format_t>(params_.format));
    if (ret < 0) {
        TCLOG_ERROR("%s: snd_pcm_hw_params_set_format failed", device_.c_str());
        goto out;
    }

    ret = snd_pcm_hw_params_set_channels(snd_handle_, hw_params, params_.channels);
    if (ret < 0) {
        TCLOG_ERROR("%s: snd_pcm_hw_params_set_channels failed", device_.c_str());
        goto out;
    }

    rate = params_.rate;
    ret = snd_pcm_hw_params_set_rate_near(snd_handle_, hw_params, &rate, 0);
    if (ret < 0) {
        TCLOG_ERROR("%s: snd_pcm_hw_params_set_rate_near failed", device_.c_str());
        goto out;
    }
    if (static_cast<int>(rate) != params_.rate) {
        TCLOG_ERROR("%s: rate doesn't match (requested %d Hz, get %u Hz)",
                device_.c_str(), params_.rate, rate);
        ret = -EINVAL;
        goto out;
    }

    period_frames = params_.period_frames;
    ret = snd_pcm_hw_params_set_period_size_near(
            snd_handle_, hw_params, &period_frames, 0);
    if (ret < 0) {
        TCLOG_ERROR("%s: snd_pcm_hw_params_set_period_size_near failed",
                device_.c_str());
        goto out;
    }
    if (static_cast<int>(period_frames) != params_.period_frames) {
        TCLOG_WARNING("%s: period frames doesn't match "
                "(requested %d frames, get %lu frames)",
                device_.c_str(), params_.period_frames, period_frames);
        params_.period_frames = period_frames;
    }

    periods = params_.periods;
    ret = snd_pcm_hw_params_set_periods_near(snd_handle_, hw_params, &periods, 0);
    if (ret < 0) {
        TCLOG_ERROR("%s: snd_pcm_hw_params_set_periods_near failed",
                device_.c_str());
        goto out;
    }
    if (static_cast<int>(periods) != params_.periods) {
        TCLOG_WARNING("%s: periods doesn't match (requested %d, get %u)",
                device_.c_str(), params_.periods, periods);
        params_.periods = periods;
    }

    ret = snd_pcm_hw_params(snd_handle_, hw_params);
    if (ret < 0) {
        TCLOG_ERROR("%s: snd_pcm_hw_params failed", device_.c_str());
        goto out;
    }

    support_pause_ = snd_pcm_hw_params_can_pause(hw_params) ? true : false;

    ret = 0;
out:
    return ret;
}

void AlsaUtils::Release(void)
{
    if (snd_handle_ != nullptr) {
        snd_pcm_close(snd_handle_);
        snd_handle_ = nullptr;
    }
}

int AlsaUtils::ReadInterleaved(char* data, int frames)
{
    return snd_pcm_readi(snd_handle_, data, frames);
}

int AlsaUtils::WriteInterleaved(char* data, int frames)
{
    return snd_pcm_writei(snd_handle_, data, frames);
}

int AlsaUtils::Prepare(void)
{
    return snd_pcm_prepare(snd_handle_);
}

int AlsaUtils::Start(void)
{
    TCLOG_DEBUG("Start: current PCM state: %s",
            kPcmStateToStringMap.at(snd_pcm_state(snd_handle_)).c_str());

    switch (snd_pcm_state(snd_handle_)) {
    case SND_PCM_STATE_RUNNING:
    case SND_PCM_STATE_SUSPENDED:
    case SND_PCM_STATE_PREPARED:
        return 0;
    case SND_PCM_STATE_SETUP:
    case SND_PCM_STATE_XRUN:
        return snd_pcm_prepare(snd_handle_);
    case SND_PCM_STATE_PAUSED:
        return support_pause_ ?
            snd_pcm_pause(snd_handle_, 0) : snd_pcm_prepare(snd_handle_);
    default:
        TCLOG_ERROR("Not supported PCM state (current state: %s)",
                kPcmStateToStringMap.at(snd_pcm_state(snd_handle_)).c_str());
        return -1;
    }
}

int AlsaUtils::Pause(void)
{
    TCLOG_DEBUG("Pause: current PCM state: %s",
            kPcmStateToStringMap.at(snd_pcm_state(snd_handle_)).c_str());

    switch (snd_pcm_state(snd_handle_)) {
    case SND_PCM_STATE_PAUSED:
        return 0;
    case SND_PCM_STATE_RUNNING:
        return support_pause_ ?
            snd_pcm_pause(snd_handle_, 1) : snd_pcm_drop(snd_handle_);
    default:
        TCLOG_ERROR("Not supported PCM state (current state: %s)",
                kPcmStateToStringMap.at(snd_pcm_state(snd_handle_)).c_str());
        return -1;
    }
}

int AlsaUtils::Resume(void)
{
    return snd_pcm_resume(snd_handle_);
}

int AlsaUtils::Drop(void)
{
    return snd_pcm_drop(snd_handle_);
}

int AlsaUtils::Drain(void)
{
    return snd_pcm_drain(snd_handle_);
}

const char* AlsaUtils::ErrorString(int error_num)
{
    return snd_strerror(error_num);
}

int AlsaUtils::GetSoftwareParameters(uint32_t flag, SoftwareParameters* sw_params)
{
    if (!snd_handle_) {
        TCLOG_ERROR("%s: snd_handle_ is nullptr", device_.c_str());
        return -1;
    }

    int ret = 0;
    snd_pcm_sw_params_t *snd_sw_params;
    snd_pcm_sw_params_alloca(&snd_sw_params);

    ret = snd_pcm_sw_params_current(snd_handle_, snd_sw_params);
    if (ret < 0) {
        TCLOG_ERROR("%s: snd_pcm_sw_params_current failed (%s)",
                device_.c_str(), snd_strerror(ret));
        return ret;
    }

    if (flag & kAvailableFramesMinFlag) {
        snd_pcm_uframes_t avail_min;
        ret = snd_pcm_sw_params_get_avail_min(snd_sw_params, &avail_min);
        if (ret < 0) {
            TCLOG_ERROR("%s: snd_pcm_sw_params_get_avail_min failed (%s)",
                    device_.c_str(), snd_strerror(ret));
            return ret;
        }
        sw_params->available_frames_min = avail_min;
    }
    if (flag & kTimestampModeFlag) {
        snd_pcm_tstamp_t ts_mode;
        ret = snd_pcm_sw_params_get_tstamp_mode(snd_sw_params, &ts_mode);
        if (ret < 0) {
            TCLOG_ERROR("%s: snd_pcm_sw_params_get_tstamp_mode failed (%s)",
                    device_.c_str(), snd_strerror(ret));
            return ret;
        }
        sw_params->timestamp_mode = static_cast<TimestampMode>(ts_mode);
    }
    if (flag & kTimestampTypeFlag) {
        snd_pcm_tstamp_type_t ts_type;
        ret = snd_pcm_sw_params_get_tstamp_type(snd_sw_params, &ts_type);
        if (ret < 0) {
            TCLOG_ERROR("%s: snd_pcm_sw_params_get_tstamp_type failed (%s)",
                    device_.c_str(), snd_strerror(ret));
            return ret;
        }
        sw_params->timestamp_type = static_cast<TimestampType>(ts_type);
    }
    if (flag & kStartThresholdFlag) {
        snd_pcm_uframes_t start_thr;
        ret = snd_pcm_sw_params_get_start_threshold(snd_sw_params, &start_thr);
        if (ret < 0) {
            TCLOG_ERROR("%s: snd_pcm_sw_params_get_start_threshold failed (%s)",
                    device_.c_str(), snd_strerror(ret));
            return ret;
        }
        sw_params->start_threshold = start_thr;
    }
    if (flag & kStopThresholdFlag) {
        snd_pcm_uframes_t stop_thr;
        ret = snd_pcm_sw_params_get_stop_threshold(snd_sw_params, &stop_thr);
        if (ret < 0) {
            TCLOG_ERROR("%s: snd_pcm_sw_params_get_stop_threshold failed (%s)",
                    device_.c_str(), snd_strerror(ret));
            return ret;
        }
        sw_params->stop_threshold = stop_thr;
    }
    if (flag & kSilenceThresholdFlag) {
        snd_pcm_uframes_t silence_thr;
        ret = snd_pcm_sw_params_get_silence_threshold(snd_sw_params, &silence_thr);
        if (ret < 0) {
            TCLOG_ERROR("%s: snd_pcm_sw_params_get_silence_threshold failed (%s)",
                    device_.c_str(), snd_strerror(ret));
            return ret;
        }
        sw_params->silence_threshold = silence_thr;
    }

    return 0;
}

int AlsaUtils::SetSoftwareParameters(uint32_t flag, const SoftwareParameters* sw_params)
{
    if (!snd_handle_) {
        TCLOG_ERROR("%s: snd_handle_ is nullptr", device_.c_str());
        return -1;
    }

    int ret = 0;
    snd_pcm_sw_params_t *snd_sw_params;
    snd_pcm_sw_params_alloca(&snd_sw_params);

    ret = snd_pcm_sw_params_current(snd_handle_, snd_sw_params);
    if (ret < 0) {
        TCLOG_ERROR("%s: snd_pcm_sw_params_current failed (%s)",
                device_.c_str(), snd_strerror(ret));
        return ret;
    }

    if (flag & kAvailableFramesMinFlag) {
        ret = snd_pcm_sw_params_set_avail_min(snd_handle_, snd_sw_params,
                static_cast<snd_pcm_uframes_t>(sw_params->available_frames_min));
        if (ret < 0) {
            TCLOG_ERROR("%s: snd_pcm_sw_params_set_avail_min failed (%s)",
                    device_.c_str(), snd_strerror(ret));
            return ret;
        }
    }
    if (flag & kTimestampModeFlag) {
        ret = snd_pcm_sw_params_set_tstamp_mode(snd_handle_, snd_sw_params,
                static_cast<snd_pcm_tstamp_t>(sw_params->timestamp_mode));
        if (ret < 0) {
            TCLOG_ERROR("%s: snd_pcm_sw_params_set_tstamp_mode failed (%s)",
                    device_.c_str(), snd_strerror(ret));
            return ret;
        }
    }
    if (flag & kTimestampTypeFlag) {
        ret = snd_pcm_sw_params_set_tstamp_type(snd_handle_, snd_sw_params,
                static_cast<snd_pcm_tstamp_type_t>(sw_params->timestamp_type));
        if (ret < 0) {
            TCLOG_ERROR("%s: snd_pcm_sw_params_set_tstamp_type failed (%s)",
                    device_.c_str(), snd_strerror(ret));
            return ret;
        }
    }
    if (flag & kStartThresholdFlag) {
        ret = snd_pcm_sw_params_set_start_threshold(snd_handle_, snd_sw_params,
                sw_params->start_threshold);
        if (ret < 0) {
            TCLOG_ERROR("%s: snd_pcm_sw_params_set_start_threshold failed (%s)",
                    device_.c_str(), snd_strerror(ret));
            return ret;
        }
    }
    if (flag & kStopThresholdFlag) {
        ret = snd_pcm_sw_params_set_stop_threshold(snd_handle_, snd_sw_params,
                sw_params->stop_threshold);
        if (ret < 0) {
            TCLOG_ERROR("%s: snd_pcm_sw_params_set_stop_threshold failed (%s)",
                    device_.c_str(), snd_strerror(ret));
            return ret;
        }
    }
    if (flag & kSilenceThresholdFlag) {
        snd_pcm_uframes_t silence_thr;
        ret = snd_pcm_sw_params_set_silence_threshold(snd_handle_, snd_sw_params,
                sw_params->silence_threshold);
        if (ret < 0) {
            TCLOG_ERROR("%s: snd_pcm_sw_params_set_silence_threshold failed (%s)",
                    device_.c_str(), snd_strerror(ret));
            return ret;
        }
    }

    ret = snd_pcm_sw_params(snd_handle_, snd_sw_params);
    if (ret < 0) {
        TCLOG_ERROR("%s: snd_pcm_sw_params failed (%s)", device_.c_str(), snd_strerror(ret));
        return ret;
    }

    return 0;
}

int AlsaUtils::GetStatus(uint32_t flag, Status* status)
{
    if (!snd_handle_) {
        TCLOG_ERROR("%s: snd_handle_ is nullptr", device_.c_str());
        return -1;
    }

    int ret;
    snd_pcm_status_t *snd_status;

    snd_pcm_status_alloca(&snd_status);
    ret = snd_pcm_status(snd_handle_, snd_status);
    if (ret < 0) {
        TCLOG_ERROR("%s: snd_pcm_status failed (%s)", device_.c_str(), snd_strerror(ret));
        return ret;
    }

    if (flag & kStateFlag) {
        status->state = static_cast<State>(snd_pcm_status_get_state(snd_status));
    }
    if (flag & kTriggerTimestampFlag) {
        snd_pcm_status_get_trigger_tstamp(snd_status, &status->trigger_timestamp);
    }
    if (flag & kLastUpdateTimestampFlag) {
        snd_pcm_status_get_tstamp(snd_status, &status->last_update_timestamp);
    }
    if (flag & kDelayFramesFlag) {
        status->delay_frames = snd_pcm_status_get_delay(snd_status);
    }
    if (flag & kAvailableFramesFlag) {
        status->available_frames = snd_pcm_status_get_avail(snd_status);
    }
    if (flag & kAvailableFramesMaxFlag) {
        status->available_frames_max = snd_pcm_status_get_avail_max(snd_status);
    }
    if (flag & kOverrangeFramesFlag) {
        status->overrange_frames = snd_pcm_status_get_overrange(snd_status);
    }

    return 0;
}

} // namespace audio
} // namespace tconfigs
