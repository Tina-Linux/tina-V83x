#ifndef __AUDIO_JACK_MANAGER_H__
#define __AUDIO_JACK_MANAGER_H__

#include <memory>
#include <list>
#include <functional>

#include "ActiveHandler.h"

#include <json-c/json.h>

#include "audiojack/IAudioJackManager.h"

namespace AW {

class DummyAudioJackManager : public IAudioJackManager
{
public:
    static std::shared_ptr<DummyAudioJackManager> create(struct json_object *config){
        return std::make_shared<DummyAudioJackManager>();
    };

    int doAudioJackPlugIn(){};
    int doAudioJackPlugOut(){};
};

class AudioJackManager : public IAudioJackManager
{
public:
    static std::shared_ptr<AudioJackManager> create(struct json_object *config);
    ~AudioJackManager();

    int doAudioJackPlugIn();
    int doAudioJackPlugOut();
private:
    AudioJackManager(){};
    int init(struct json_object *config);
    int init_gpio_ctr(struct json_object *config);
    int init_amixer_ctr(struct json_object *config);
    void release();

    std::function<void()> m_do_audio_jack_plugout;
    std::function<void()> m_do_audio_jack_plugin;

    std::list<std::shared_ptr<ActiveHandler>> m_active_list;
};

}

#endif
