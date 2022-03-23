#ifndef _DISPLAY_LIB_H_
#define  _DISPLAY_LIB_H_

#include <stdio.h>
#include <stdbool.h>
#include "sunxi_display_v2.h"

int display_get_lcd_rect(disp_rectsz *rect);
int display_lcd_onoff(bool onoff);
int display_lcd_backlight_onoff(bool onoff);
int display_lcd_set_brightness(int val);

#endif
