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
static const struct argb_t gray[_BAR_NUM] = {
	{0xFF, 0x00, 0x00, 0x00}, /* Black       */
	{0xFF, 0x69, 0x69, 0x69}, /* Dim gray    */
	{0xFF, 0x80, 0x80, 0x80}, /* Gray        */
	{0xFF, 0xA9, 0xA9, 0xA9}, /* Dark gray   */
	{0xFF, 0xC0, 0xC0, 0xC0}, /* Sliver      */
	{0xFF, 0xD3, 0xD3, 0xD3}, /* Light gray  */
	{0xFF, 0xDC, 0xDC, 0xDC}, /* Gainsboro   */
	{0xFF, 0xF5, 0xF5, 0xF5}, /* White smoke */
};

int grayscale(struct canvas *canvas, const char *params)
{
	int i;
	int bar_width = 0;
	point_t top_left;
	point_t bottom_right;

	printf("Testcase %s\n", __func__);

	bar_width = canvas->width / _BAR_NUM;
	top_left.x = 0;
	top_left.y = 0;
	bottom_right.x = bar_width - 1;
	bottom_right.y = canvas->height - 1;

	for (i=0; i<_BAR_NUM; i++) {
		canvas->draw_rectangle(canvas, (struct argb_t *)&gray[i], &top_left, &bottom_right);
		top_left.x = bottom_right.x + 1;
		bottom_right.x += bar_width;
	}
	return 0;
}

