#ifndef __IBUTTION_MANAGER_H__
#define __IBUTTION_MANAGER_H__

namespace AW {

class IButtonManager
{
public:
    class Observer
    {
    public:
        virtual ~Observer() = default;
        virtual void onVolumeUp() = 0;
        virtual void onVolumeDown() = 0;
        virtual void onMute() = 0;
        virtual void onPlayPause(){};
        virtual void onPlay(){};
        virtual void onPause(){};
        virtual void onAudioJackPlugIn(){};
        virtual void onAudioJackPlugOut(){};
    };
    virtual void release() = 0;
    virtual void addButtonObserver(std::shared_ptr<Observer> observer) = 0;
    virtual void removeButtonObserver(std::shared_ptr<Observer> observer) = 0;
};

}
#endif