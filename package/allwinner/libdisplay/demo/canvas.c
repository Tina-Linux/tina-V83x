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

#include "display.h"
#include "framebuffer.h"
#include "virtual.h"

#define _PLATFORM_FRAMEBUFFER	(0)
#define _PLATFORM_LAYER			(1)
#define _PLATFORM_VIRTUAL		(2)

struct platform_table {
	char name[64];
	int  idx;
};

const struct platform_table platforms[] = {
	{"framebuffer", _PLATFORM_FRAMEBUFFER},
	{"layer",		_PLATFORM_LAYER},
	{"virtual",		_PLATFORM_VIRTUAL},
};

static int platform_match(const char *name)
{
	int i;
	int total = sizeof(platforms) / sizeof(platforms[0]);

	for (i=0; i<total; i++) {
		if (strcmp(platforms[i].name, name) == 0)
			return platforms[i].idx;
	}
	return -1;
}

struct canvas *create_canvas(const char *platform, const char *fbdev)
{
	void *device;
	struct canvas *canvas = NULL;
	int idx = platform_match(platform);

	switch (idx) {
	case _PLATFORM_FRAMEBUFFER: {
        const char *devpath = fbdev ? fbdev : "/dev/fb0";
		device = alloc_framebuffer_device(devpath);
		if (device) {
			canvas = &((struct framebuffer_device *)device)->fb_canvas;
			canvas->private.type = _PLATFORM_FRAMEBUFFER;
		}
        }
		break;
	case _PLATFORM_LAYER:
		/* TODO: add layer support */
		canvas = NULL;
		break;
	case _PLATFORM_VIRTUAL:
		device = alloc_virtual_device(480, 320, "virtual");
		if (device) {
			canvas = &((struct virtual_device *)device)->image_canvas;
			canvas->private.type = _PLATFORM_VIRTUAL;
		}
		break;
	default:
		break;
	}
	return canvas;
}

int destroy_canvas(struct canvas *canvas)
{
	void *device;
	int type;

	if (!canvas)
		return -1;

	device = canvas->private.device;
	type = canvas->private.type;

	switch (type) {
	case _PLATFORM_FRAMEBUFFER:
		free_framebuffer_device(device);
		break;
	case _PLATFORM_VIRTUAL:
		free_virtual_device(device);
		break;
	case _PLATFORM_LAYER:
	default:
		break;
	}
	return 0;
}


