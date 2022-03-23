#ifndef ROTATE_ADD_VIEW_H_
#define ROTATE_ADD_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define ROTATE_ADD_VIEW "rotateAddView"

typedef struct {
	BITMAP bmpBgMin;
	BITMAP bmpBgBig;
	BITMAP bmpAnim[2];
} RotateAddView;

BOOL RegisterRotateAddView(void);
void UnregisterRotateAddView(void);

#endif /* ROTATE_ADD_VIEW_H_ */
