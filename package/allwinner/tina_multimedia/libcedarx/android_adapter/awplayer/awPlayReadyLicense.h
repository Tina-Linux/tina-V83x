#ifndef AW_PLAYREADY_LICENSE_H
#define AW_PLAYREADY_LICENSE_H

#include <utils/Errors.h>
#include <binder/IInterface.h>

using namespace android;

status_t PlayReady_Drm_Invoke(const Parcel &request, Parcel *reply);

#endif /* AW_PLAYREADY_LICENSE_H */
