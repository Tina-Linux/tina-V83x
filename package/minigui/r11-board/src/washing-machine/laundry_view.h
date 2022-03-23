#ifndef LAUNDRY_VIEW_H_
#define LAUNDRY_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "laundry_part_view.h"

#define LAUNDRY_VIEW "laundryView"
#define ID_LAUNDRY_VIEW_ONE 1
#define ID_LAUNDRY_VIEW_TOW 2
#define ID_LAUNDRY_VIEW_THR 3
#define MSG_LAUNDRY_CLICK (MSG_USER + 2)

typedef struct {
    LaundryPartData *laundryPartData[3];
} LaundryData;

BOOL RegisterLaundryView(void);
void UnregisterLaundryView(void);

#endif /* LAUNDRY_VIEW_H_ */
