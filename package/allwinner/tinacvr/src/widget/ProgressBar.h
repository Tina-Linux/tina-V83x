#ifndef __PROGRESS_BAR_H__
#define __PROGRESS_BAR_H__
#include "CvrInclude.h"

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define CTRL_CDRPROGRESSBAR		"CDRPROGRESSBAR"

typedef struct {
  unsigned char sec;
  unsigned char min;
}PGBTime_t;

typedef struct {
	gal_pixel bgcWidget;
	gal_pixel fgcWidget;
}ProgressBarData_t;

int RegisterPGBControls(void);
void UnRegisterPGBControls(void);

#endif
