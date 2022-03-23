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

#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include <stdint.h>
#include <linux/fb.h>
#include <dirent.h>

#include "display.h"

struct framebuffer_device {
	char devpath[PATH_MAX];
	int devfd;

	struct fb_fix_screeninfo fix_info;
	struct fb_var_screeninfo var_info;

	void *base;
	size_t length;

	struct canvas fb_canvas;
};


struct framebuffer_device *alloc_framebuffer_device(const char *path);
void free_framebuffer_device(struct framebuffer_device *dev);

void dump_framebuffer_device_info(struct framebuffer_device *dev);

#endif
