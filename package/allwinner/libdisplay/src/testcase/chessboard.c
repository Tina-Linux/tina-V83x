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

#define _cell_width (20)

const struct argb_t color[2] =
{
	{0xFF, 0x00, 0x00, 0x00},
	{0xFF, 0xFF, 0xFF, 0xFF},
};

static void color_value(int x, int y, struct argb_t *out)
{
	int yodd, xodd;
	int i;

	xodd = (x / _cell_width) & 0x01;
	yodd = (y / _cell_width) & 0x01;

	i = (yodd ^ xodd);
	memcpy(out, &color[i], sizeof(struct argb_t));
}

int chessboard(struct canvas *canvas, const char *params)
{
	printf("Testcase %s\n", __func__);

	canvas->draw_by_func(canvas, color_value);
	return 0;
}

