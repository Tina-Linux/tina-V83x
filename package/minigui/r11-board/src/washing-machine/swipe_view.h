#ifndef SWIPE_VIEW_H_
#define SWIPE_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "laundry_view.h"

#define SWIPE_VIEW "swipeView"
#define SWIPE_TIMER_ID 11

typedef struct {
    int currentPageIndex;
    LaundryData *laundryData[2];
} SwipeData;

BOOL RegisterSwipeView(void);
void UnregisterSwipeView(void);

#endif /* SWIPE_VIEW_H_ */
