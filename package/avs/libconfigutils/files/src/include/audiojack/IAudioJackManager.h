#ifndef __IAUDIO_JACK_MANAGER_H__
#define __IAUDIO_JACK_MANAGER_H__

namespace AW {

class IAudioJackManager
{
public:
    virtual int doAudioJackPlugIn() = 0;
    virtual int doAudioJackPlugOut() = 0;
};

}

#endif
