#ifndef __ISHOW_MANAGER_H__
#define __ISHOW_MANAGER_H__

namespace AW {

enum class Profile
{
    IDLE = 0,
    LISTENING,
    THINKING,
    SPEAKING,
    CONNECTING,
    MUTE,
    UNMUTE,
    WAKEUPTEST
};

enum class ProfileFlag
{
    REPLACE,
    APPEND
};

class IShowManager
{
public:
    virtual void release() = 0;
    virtual int enableShow(Profile profile, ProfileFlag flag) = 0;
};

}
#endif
