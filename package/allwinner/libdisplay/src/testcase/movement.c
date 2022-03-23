/*
 * screent test base on framebuffer device
 * Copyright (C) 2015-2018 AllwinnerTech
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "screen-test.h"

#define _BAR_NUM (8)
static const struct argb_t color[_BAR_NUM] = {
	{0xFF, 0xFF, 0xFF, 0xFF}, /* White   */
	{0xFF, 0xFF, 0xFF, 0x00}, /* Yellow  */
	{0xFF, 0x00, 0xFF, 0xFF}, /* Cyan    */
	{0xFF, 0x00, 0x80, 0x00}, /* Green   */
	{0xFF, 0xFF, 0x00, 0xFF}, /* Magenta */
	{0xFF, 0xFF, 0x00, 0x00}, /* Red     */
	{0xFF, 0x00, 0x00, 0xFF}, /* Blue    */
	{0xFF, 0x00, 0x00, 0x00}, /* Black   */
};

void clear(struct canvas *canvas)
{
	point_t top_left;
	point_t bottom_right;

	top_left.x = 0;
	top_left.y = 0;
	bottom_right.x = canvas->width - 1;
	bottom_right.y = canvas->height - 1;

    canvas->draw_rectangle(canvas, (struct argb_t *)&color[7], &top_left, &bottom_right);
}

int movement(struct canvas *canvas, const char *params)
{
	int bar_width = 0;
	point_t top_left;
	point_t bottom_right;
    int period = 16;
    int flip = 0;

	printf("Testcase %s\n", __func__);

    if (params != NULL) {
        sscanf(params, "%d,%d", &period, &flip);
        printf("Period: %d ms, flip: %d", period, flip);
    }

__restart:
	bar_width = canvas->width / _BAR_NUM;
	top_left.x = 0;
	top_left.y = 0;
	bottom_right.x = bar_width - 1;
	bottom_right.y = canvas->height - 1;


    while (bottom_right.x < canvas->width) {
        clear(canvas);
	    canvas->draw_rectangle(canvas, (struct argb_t *)&color[5], &top_left, &bottom_right);
        top_left.x += 4;
        bottom_right.x +=4;

        if (flip)
            canvas->flip(canvas);

        usleep(period * 1000);
    }

    goto __restart;

	return 0;
}

