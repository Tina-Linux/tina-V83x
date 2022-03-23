#ifndef __EVENTLISTENER_H__
#define __EVENTLISTENER_H__

#include "Info.h"

namespace softwinner{
class EventListener{
public:
    ~EventListener(){};
    /* 播放，以及相关播放信息 */
    virtual void onPlay(std::string url, DLNAMediaInfo info) = 0;
    virtual void onPause() = 0;
    virtual void onStop() = 0;

    /* 请求设置音量（0~100），管理端设置完音量，
    无论成功失败，调用updateVolume 通知DLNA服务 */
    virtual void onSetVolume(int volume) = 0;

};
} /* namespace softwinner */

#endif /* __EVENTLISTENER_H__ */