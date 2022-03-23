#ifndef LAUNDRY_PART_VIEW_H_
#define LAUNDRY_PART_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define LAUNDRY_PART_VIEW "laundryPartView"
#define LAUNDRY_PART_ENABLE_CLICK_YES 1
#define LAUNDRY_PART_ENABLE_CLICK_NO 0
#define MSG_LAUNDRY_PART_CHANGE_VALUE (MSG_USER + 3)

typedef struct {
    HWND laundryPartHwnd;
    BITMAP bmpBg;
    PLOGFONT logfontUnit;
    PLOGFONT logfontDes;
    char *valueUnitDes[3];
    int enableClick;
} LaundryPartData;

BOOL RegisterLaundryPartView(void);
void UnregisterLaundryPartView(void);

#endif /* STATIC_VIEW_H_ */
