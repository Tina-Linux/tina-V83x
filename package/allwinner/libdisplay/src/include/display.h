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

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

struct argb_t {
	unsigned char transp;
	unsigned char red;
	unsigned char green;
	unsigned char blue;
};

struct point {
	int x;
	int y;
};
typedef struct point point_t;

typedef void (*color_function)(int x, int y, struct argb_t *out);

struct canvas_private {
	void *device;
	int type;
};

struct canvas {
	struct canvas_private private;
	int width;
	int height;
	int (*draw_color)(struct canvas *ca, struct argb_t *color);
	int (*draw_point)(struct canvas *ca, struct argb_t *color, point_t *coords);
	int (*draw_line)(struct canvas *ca, struct argb_t *color, point_t *from, point_t *to);
	int (*draw_rectangle)(struct canvas *ca, struct argb_t *color, point_t *top_left, point_t *bottom_right);
	int (*draw_by_func)(struct canvas *ca, color_function func);
	void (*flip)(struct canvas *ca);
};

struct canvas *create_canvas(const char *platform, const char *fbdev);
int destroy_canvas(struct canvas *canvas);

#endif
