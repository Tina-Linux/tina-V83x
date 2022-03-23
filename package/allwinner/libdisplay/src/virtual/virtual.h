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

#ifndef __VIRTUAL_H__
#define __VIRTUAL_H__

#include <unistd.h>
#include <stdint.h>
#include "display.h"
#include "libbmp.h"

struct virtual_device {
	struct libbmp *bmp;

	void *base;
	size_t size;
	int width;
	int height;
	int stride;
	int bpp;

	struct canvas image_canvas;
};

struct virtual_device *alloc_virtual_device(int x, int y, const char *name);
void free_virtual_device(struct virtual_device *dev);

#endif
