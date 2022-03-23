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
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>

#include "framebuffer.h"

static void* __start;

static int fb_draw_color(struct canvas *ca, struct argb_t *color);
static int fb_draw_point(struct canvas *ca,
				struct argb_t *color, point_t *coords);
static int fb_draw_line(struct canvas *ca,
				struct argb_t *color, point_t *from, point_t *to);
static int fb_draw_rectangle(struct canvas *ca,
				struct argb_t *color, point_t *top_left, point_t *bottom_right);
static void framebuffer_flip(struct canvas *ca);

static int _device_ioctl(int fd, int request, void *args)
{
	int retval = 0;

	retval = ioctl(fd, request, args);
	if (retval < 0) {
		fprintf(stderr, "ioctl(%08x) error, %s\n", request, strerror(errno));
		return retval;
	}
	return 0;
}

struct framebuffer_device *alloc_framebuffer_device(const char *path)
{
	struct framebuffer_device *dev = NULL;
	int fd = -1;

	fd = open(path, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "could not open device '%s', %s\n",
			path, strerror(errno));
		goto _out;
	}

	dev = calloc(1, sizeof(*dev));
	if (!dev) {
		fprintf(stderr, "could not alloc memory, %s\n", strerror(errno));
		close(fd);
		goto _out;
	}

	strncpy(dev->devpath, path, PATH_MAX);
	dev->devfd = fd;
	_device_ioctl(fd, FBIOGET_VSCREENINFO, &dev->var_info);
	_device_ioctl(fd, FBIOGET_FSCREENINFO, &dev->fix_info);

	dev->length = dev->fix_info.smem_len;
	dev->base = mmap(0, dev->length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (dev->base == MAP_FAILED) {
		fprintf(stderr, "mmap framebuffer memory failed, %s\n", strerror(errno));
		dev->base = NULL;
		dev->length = 0;
	}
    __start = dev->base;
    printf("base: %p, start: %p\n", dev->base, __start);

    dump_framebuffer_device_info(dev);

	dev->fb_canvas.private.device = dev;
	dev->fb_canvas.draw_color = fb_draw_color;
	dev->fb_canvas.draw_point = fb_draw_point;
	dev->fb_canvas.draw_line = fb_draw_line;
	dev->fb_canvas.draw_rectangle = fb_draw_rectangle;
    dev->fb_canvas.flip = framebuffer_flip;

    dev->fb_canvas.width  = dev->var_info.xres;
    dev->fb_canvas.height = dev->var_info.yres;


    dev->var_info.yoffset = 0;
    _device_ioctl(dev->devfd, FBIOPUT_VSCREENINFO, &dev->var_info);

	fprintf(stdout, "alloc device success, mmap at %p, size %u\n",
		dev->base, (unsigned int)dev->length);
	return dev;

_out:
	return NULL;
}

#if 1
static void framebuffer_flip(struct canvas *ca)
{
    struct framebuffer_device *dev = ca->private.device;
	struct fb_var_screeninfo var_info;
    int offset = 0;

	_device_ioctl(dev->devfd, FBIOGET_VSCREENINFO, &var_info);

    if (var_info.yres == var_info.yoffset) {
        offset = var_info.yres;
        var_info.yoffset = 0;
    } else {
        offset = 0;
        var_info.yoffset = var_info.yres;
    }

    _device_ioctl(dev->devfd, FBIOPUT_VSCREENINFO, &var_info);
    dev->base = ((char*)__start + dev->fix_info.line_length * offset);
}
#endif

void free_framebuffer_device(struct framebuffer_device *dev)
{
	if (!dev)
		return;

    if (dev->devfd) {
        dev->var_info.yoffset = 0;
	    _device_ioctl(dev->devfd, FBIOPUT_VSCREENINFO, &dev->var_info);
    }

	if (dev->base && dev->length != 0)
		munmap(dev->base, dev->length);

	if (dev->devfd > 0)
		close(dev->devfd);

	free(dev);
}

static unsigned int argb2raw(struct fb_bitfield *transp,
	struct fb_bitfield *red, struct fb_bitfield *green,
	struct fb_bitfield *blue, struct argb_t *color)
{
	unsigned int raw = 0;

	raw = (color->red << red->offset)
			| (color->green << green->offset)
			| (color->blue << blue->offset);

	if (transp->length)
		raw |= (color->transp << transp->offset);

	return raw;
}


static int fb_draw_color(struct canvas *ca, struct argb_t *color)
{
	struct framebuffer_device *dev = ca->private.device;
	uint32_t *pline_start = dev->base;
	uint32_t *pcurrent = NULL;
	unsigned int line_stride;
	unsigned int raw;
	int x, y;

	fprintf(stdout, "%s\n", __func__);
	if (!pline_start) {
		fprintf(stderr, "unknow framebuffer address\n");
		return -1;
	}

	raw = argb2raw(&dev->var_info.transp, &dev->var_info.red,
			&dev->var_info.green, &dev->var_info.blue, color);

	line_stride = dev->fix_info.line_length/4;
	for (y=0; y<dev->var_info.yres; y++) {
		pcurrent = pline_start;
		pline_start += line_stride;
		for (x=0; x<dev->var_info.xres; x++)
			*pcurrent++ = raw;
	}

	return 0;
}

static int fb_check_coords(struct canvas *ca, point_t *coords)
{
	struct framebuffer_device *dev = ca->private.device;

	if ((coords->x < 0)
			|| (coords->x >= dev->var_info.xres)
			|| (coords->y < 0)
			|| (coords->y >= dev->var_info.yres)) {
		return -1;
	}
	return 0;
}

static int fb_draw_point(struct canvas *ca,
			struct argb_t *color, point_t *coords)
{
	struct framebuffer_device *dev = ca->private.device;
	unsigned long long start = (unsigned long long)dev->base;
	uint32_t *pbuff = 0;
	unsigned int raw;

	if (start == 0)
		return -1;

	if (fb_check_coords(ca, coords)) {
		fprintf(stderr, "invalid coords to draw point\n");
		return -1;
	}

	raw = argb2raw(&dev->var_info.transp, &dev->var_info.red,
			&dev->var_info.green, &dev->var_info.blue, color);

	start += (coords->y * dev->fix_info.line_length)
				+ (coords->x * (dev->var_info.bits_per_pixel/8));

	pbuff = (void *)start;
	*pbuff = raw;

	return 0;
}

static int fb_draw_line(struct canvas *ca,
			struct argb_t *color, point_t *from, point_t *to)
{

#if 0
	struct framebuffer_device *dev = ca->private.device;
	unsigned int start = (unsigned int)dev->base;
	uint32_t *pbuff = 0;
	unsigned int raw;

	if (fb_check_coords(ca, from) || fb_check_coords(ca, to)) {
		fprintf(stderr, "invalid coords to draw line\n");
		return -1;
	}

	if (from->x != to->x && from->y != to->y) {
		fprintf(stderr,
			"draw line error, only Vertical or Parallel line support\n");
		return -1;
	}

	raw = argb2raw(&dev->var_info.transp, &dev->var_info.red,
			&dev->var_info.green, &dev->var_info.blue, color);
#endif
	return 0;
}

static inline int _memset32(uint32_t *from, uint32_t value, size_t size)
{
	uint32_t *end = 0;

	if (!from || size<0)
		return -1;

	end = from + (size / 4);
	while (from <= end) {
		*from++ = value;
	}
	return 0;
}

static int fb_draw_rectangle(struct canvas *ca,
			struct argb_t *color, point_t *top_left, point_t *bottom_right)
{
	struct framebuffer_device *dev = ca->private.device;
	char *from;
	int stride, lines;
	int value, i;

	if (fb_check_coords(ca, top_left)
			|| fb_check_coords(ca, bottom_right)
			|| (top_left->x > bottom_right->x)
			|| (top_left->y > bottom_right->y)) {
		fprintf(stderr, "%s input params out of range\n", __func__);
		return -1;
	}

	value = argb2raw(&dev->var_info.transp, &dev->var_info.red,
			&dev->var_info.green, &dev->var_info.blue, color);

	lines = bottom_right->y - top_left->y;
	stride = (bottom_right->x - top_left->x) * (dev->var_info.bits_per_pixel/8);
	from = (char *)dev->base
				+ (top_left->y * dev->fix_info.line_length)
				+ (top_left->x * (dev->var_info.bits_per_pixel/8));

	for (i=0; i<=lines; i++) {
		_memset32((uint32_t *)from, value, stride);
		from += dev->fix_info.line_length;
	}

	return 0;
}

void dump_framebuffer_device_info(struct framebuffer_device *dev)
{
	if (!dev)
		return;

	fprintf(stdout, "\nFramebuffer device information:\n");
	fprintf(stdout, "\n*fix screen information:\n");
	fprintf(stdout, "%16s: %s\n", "id"         , dev->fix_info.id         );
	fprintf(stdout, "%16s: %p\n", "smem_start" , (void *)dev->fix_info.smem_start );
	fprintf(stdout, "%16s: %u\n", "smem_len"   , dev->fix_info.smem_len   );
	fprintf(stdout, "%16s: %u\n", "type"       , dev->fix_info.type       );
	fprintf(stdout, "%16s: %u\n", "type_aux"   , dev->fix_info.type_aux   );
	fprintf(stdout, "%16s: %u\n", "visual"     , dev->fix_info.visual     );
	fprintf(stdout, "%16s: %u\n", "xpanstep"   , dev->fix_info.xpanstep   );
	fprintf(stdout, "%16s: %u\n", "ypanstep"   , dev->fix_info.ypanstep   );
	fprintf(stdout, "%16s: %u\n", "ywrapstep"  , dev->fix_info.ywrapstep  );
	fprintf(stdout, "%16s: %u\n", "line_length", dev->fix_info.line_length);
	fprintf(stdout, "%16s: %p\n", "mmio_start" , (void *)dev->fix_info.mmio_start );
	fprintf(stdout, "%16s: %u\n", "mmio_len"   , dev->fix_info.mmio_len   );
	fprintf(stdout, "%16s: %u\n", "accel"      , dev->fix_info.accel      );

	fprintf(stdout, "\n*var screen information:\n");
	fprintf(stdout, "%16s: %u\n", "xres"          , dev->var_info.xres          );
	fprintf(stdout, "%16s: %u\n", "yres"          , dev->var_info.yres          );
	fprintf(stdout, "%16s: %u\n", "xres_virtual"  , dev->var_info.xres_virtual  );
	fprintf(stdout, "%16s: %u\n", "yres_virtual"  , dev->var_info.yres_virtual  );
	fprintf(stdout, "%16s: %u\n", "xoffset"       , dev->var_info.xoffset       );
	fprintf(stdout, "%16s: %u\n", "yoffset"       , dev->var_info.yoffset       );
	fprintf(stdout, "%16s: %u\n", "bits_per_pixel", dev->var_info.bits_per_pixel);
	fprintf(stdout, "%16s: %u\n", "grayscale"     , dev->var_info.grayscale     );
	fprintf(stdout, "%16s: %u\n", "nonstd"        , dev->var_info.nonstd        );
	fprintf(stdout, "%16s: %u\n", "activate"      , dev->var_info.activate      );
	fprintf(stdout, "%16s: %u\n", "height"        , dev->var_info.height        );
	fprintf(stdout, "%16s: %u\n", "width"         , dev->var_info.width         );
	fprintf(stdout, "%16s: %u\n", "accel_flags"   , dev->var_info.accel_flags   );

	fprintf(stdout, "%16s: length %d, offset %d\n", "red",
		dev->var_info.red.length, dev->var_info.red.offset);
	fprintf(stdout, "%16s: length %d, offset %d\n", "green",
		dev->var_info.green.length, dev->var_info.green.offset);
	fprintf(stdout, "%16s: length %d, offset %d\n", "blue",
		dev->var_info.blue.length, dev->var_info.blue.offset);
	fprintf(stdout, "%16s: length %d, offset %d\n", "transp",
		dev->var_info.transp.length, dev->var_info.transp.offset);
}
