#ifndef __PLATFORM_ADAPTER_H__
#define __PLATFORM_ADAPTER_H__

#include <memory>

#include <json-c/json.h>

#include "watcher/KeyInputEventWatcher.h"
#include "recorder/PlatformRecorder.h"
#include "mute/IMuteManager.h"
#include "button/IButtonManager.h"
#include "audiojack/IAudioJackManager.h"
#include "ledshow/IShowManager.h"

namespace AW {

class PlatformAdapter
{
public:
    static std::shared_ptr<PlatformAdapter> create(const char *config);
    ~PlatformAdapter();

    //getrecorder
    std::shared_ptr<RecorderInterface> getRecorder();

    //get detector type
    const char * getDetectorType();

    //get voice data loopback
    bool getVoiceDataLoopback();

    //Sensory
    const char * getSensoryModel();
    const char * getSensoryOperatingPoint();

    //Amazon-lite
    const char * getAmazonliteModel();
    int getAmazonliteDetectThreshold();

    std::shared_ptr<IMuteManager> getMuteManager() { return m_mute_manager; };
    std::shared_ptr<IButtonManager> getButtonManager() { return m_button_manager; };
    std::shared_ptr<IAudioJackManager> getAudioJackManager() { return m_audiojack_manager; }
    std::shared_ptr<IShowManager> getShowManager() { return m_show_manager; }

private:
    PlatformAdapter(const char *config);
    int init();

private:
    const char *m_config_file{nullptr};
    struct json_object *m_config_json{nullptr};
    struct json_object *m_platform_json{nullptr};

    std::shared_ptr<EventWatcher> m_watcher{nullptr};
    std::shared_ptr<Executor> m_executor{nullptr};

    std::shared_ptr<IMuteManager> m_mute_manager{nullptr};
    std::shared_ptr<IButtonManager> m_button_manager{nullptr};
    std::shared_ptr<IAudioJackManager> m_audiojack_manager{nullptr};
    std::shared_ptr<IShowManager> m_show_manager{nullptr};

    std::shared_ptr<RecorderInterface> m_recorder{nullptr};
};
}
#endif
