#ifndef STATIC_VIEW_H_
#define STATIC_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "resource.h"

#define STATIC_VIEW "staticView"

BOOL RegisterStaticView(void);
void UnregisterStaticView(void);

#endif /* STATIC_VIEW_H_ */
