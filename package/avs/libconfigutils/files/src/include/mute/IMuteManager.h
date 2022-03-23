#ifndef __IMUTEMANGER_H__
#define __IMUTEMANGER_H__

namespace AW {

class IMuteManager
{
public:
    virtual int privacyMute(bool is_mute) = 0;
};

}
#endif /*__IMUTEMANGER_H__*/