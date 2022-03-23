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

#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "screen-test.h"

#define _cell_width (20)

static int ox;
static int oy;
static int denominator = 0;

static void color_value(int x, int y, struct argb_t *out)
{
	int c;
	int dx,dy;

#if 0
	c = 180 * ceil(sqrt(x*x + y*y)) / denominator;
#else
	dx = (x-ox)*(x-ox);
	dy = (y-oy)*(y-oy);
	c = 200 * ceil(sqrt(abs(dx + dy))) / ox;
#endif

	out->red   = c;
	out->green = c;
	out->blue  = c;
}

int gradient(struct canvas *canvas, const char *params)
{
	float tmp;
	printf("Testcase %s\n", __func__);

	tmp = (canvas->width * canvas->width) + (canvas->height * canvas->height);
	denominator = ceil(sqrt(tmp));
	ox = canvas->width  / 2;
	oy = canvas->height / 2;

	canvas->draw_by_func(canvas, color_value);
	return 0;
}

