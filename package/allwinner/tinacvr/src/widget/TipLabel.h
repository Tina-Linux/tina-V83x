#ifndef __TIP_LABEL_H__
#define __TIP_LABEL_H__

#include "CvrInclude.h"

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

enum labelIndex {
	LABEL_NO_TFCARD = 0,
	LABEL_TFCARD_FULL,
	LABEL_TFCARD_FULL1,
	LABEL_PLAYBACK_FAIL,
	LABEL_LOW_POWER_SHUTDOWN,
	LABEL_10S_SHUTDOWN,
	LABEL_SHUTDOWN_NOW,
	LABEL_30S_NOWORK_SHUTDOWN,
	LABEL_FILELIST_EMPTY,
	LABEL_TFCARD_FORMATTING,
	LABEL_IMPACT_NUM,
	LABEL_LABEL_OTA_UPDATE,
	LABEL_INVAILD_TFCATD,
	LABEL_TFCARD_FORMAT,
};

int ShowTipLabel(HWND hParent, enum labelIndex index, bool endLabelKeyUp, unsigned int timeoutMs);
int CloseTipLabel(void);

#endif
