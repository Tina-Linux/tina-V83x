/*
 * fun_button_view.h
 *
 *  Created on: Oct 23, 2017
 *      Author: anruliu
 */

#ifndef SRC_WIDGET_FUN_BUTTON_VIEW_H_
#define SRC_WIDGET_FUN_BUTTON_VIEW_H_

#include <string.h>
#include "resource.h"
#include "button_view.h"

#define FUN_BUTTON_VIEW "funButtonView"

typedef struct {
	ButtonView *funData[7];
} FunButtonView;

BOOL RegisterFunButtonView(void);
void UnregisterFunButtonView(void);
HWND FunButtonViewInit(HWND hParent);

#endif /* SRC_WIDGET_FUN_BUTTON_VIEW_H_ */
