#ifndef DOUBLE_TEXT_VIEW_H_
#define DOUBLE_TEXT_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define Double_TEXT_VIEW "doubleTextView"
#define ENABLE_CLICK_YES 0
#define ENABLE_CLICK_NO 1
#define PAGE_ONE 0
#define PAGE_TOW 1

typedef struct {
	BITMAP bmpBg;
	PLOGFONT logfontUnit;
	PLOGFONT logfontDes;
	char *valueUnitDes[6];
	int currentPage;
	int slideDistance;
	int enableClick[2];
} DoubleTextView;

BOOL RegisterDoubleTextView(void);
void UnregisterDoubleTextView(void);
void ChangeDoubleTextPage(HWND hwnd);
BOOL ChangeValue(HWND hwnd, char *value);

#endif /* DOUBLE_TEXT_VIEW_H_ */
