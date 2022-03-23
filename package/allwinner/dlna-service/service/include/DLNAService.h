#ifndef __DLNA_SERVICE_H__
#define __DLNA_SERVICE_H__

#include "Info.h"
#include "EventListener.h"
namespace softwinner{

class CedarXPlayerAdapter;
class DLNAController;

class DLNAService{
public:
    DLNAService();
    ~DLNAService();

    void setDeviceInfo(DLNADeviceInfo info);
    int startDMR();
    int stopDMR();

    void setEventListener(EventListener* l);
    void updateVolume(int volume);

	void setPlayer(void* player);

private:

    static const int STATUS_START = 0;
    static const int STATUS_STOP = 1;

    EventListener* mEventListener;

    DLNADeviceInfo mDeviceInfo;
    int mStatus;
	void* mPlayer;
	CedarXPlayerAdapter* mCedarXPlayerAdapter;
	DLNAController* mDlnaController;
};

}/* namespace softwinner */
#endif /* __DLNA_SERVICE_H__ */