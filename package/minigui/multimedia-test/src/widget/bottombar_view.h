#ifndef SRC_WIDGET_BOTTOMBAR_VIEW_H_
#define SRC_WIDGET_BOTTOMBAR_VIEW_H_

#include <string.h>
#include "resource.h"
#include "button_view.h"

#define BOTTOM_BAR_VIEW "bottomBarView"
#define ID_BUTTON_BACK 0
#define ID_BUTTON_SETUP 2
#define ID_BUTTON_TAKE 1

typedef struct {
	ButtonView *backData;
	ButtonView *setupData;
	ButtonView *takeData;
} BottomBarView;

BOOL RegisterBottomBarView(void);
void UnregisterBottomBarView(void);
HWND BottomBarViewInit(HWND hParent, int mode);

#endif /* SRC_WIDGET_BOTTOMBAR_VIEW_H_ */
