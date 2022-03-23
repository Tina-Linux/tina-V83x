
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "virtual.h"

static int draw_by_func(struct canvas *ca, color_function func);
static int draw_color(struct canvas *ca, struct argb_t *color);
static int draw_point(struct canvas *ca, struct argb_t *color, point_t *coords);
static int draw_line(struct canvas *ca, struct argb_t *color, point_t *from, point_t *to);
static int draw_rectangle(struct canvas *ca, struct argb_t *color, point_t *top_left, point_t *bottom_right);

struct virtual_device *alloc_virtual_device(int width, int height, const char *name)
{
	char fname[64] = {0};
	struct virtual_device *dev;
	int bpp = 32;

	strcat(fname, name);
	strcat(fname, ".bmp");
	dev = malloc(sizeof(*dev));
	dev->bmp = alloc_bmp();
	dev->bmp->create_image(dev->bmp, width, height, bpp, fname);

	dev->bpp   = bpp;
	dev->width  = width;
	dev->height = height;
	dev->base   = dev->bmp->get_image_buffer(dev->bmp);
	dev->size   = dev->bmp->get_image_buffer_size(dev->bmp);
	dev->stride = dev->bmp->get_image_buffer_stride(dev->bmp);

	dev->image_canvas.width = dev->width;
	dev->image_canvas.height = dev->height;
	dev->image_canvas.private.device = dev;;
	dev->image_canvas.draw_color = draw_color;
	dev->image_canvas.draw_by_func = draw_by_func;
	dev->image_canvas.draw_point = draw_point;
	dev->image_canvas.draw_line  = draw_line;
	dev->image_canvas.draw_rectangle = draw_rectangle;

	return dev;
}

void free_virtual_device(struct virtual_device *dev)
{
	if (!dev)
		return;

	dev->bmp->save_image(dev->bmp);
	free(dev);
}

static unsigned int argb2raw(struct argb_t *color)
{
	unsigned int raw = 0;

	raw = (color->blue << 0)
			| (color->green << 8)
			| (color->red << 16)
			| (color->transp << 24);

	return raw;
}

static int draw_color(struct canvas *ca, struct argb_t *color)
{
	struct virtual_device *dev = ca->private.device;
	uint32_t *pline_start = dev->base;
	uint32_t *pcurrent = NULL;
	unsigned int line_stride;
	unsigned int raw;
	int x, y;

	fprintf(stdout, "%s\n", __func__);
	if (!pline_start) {
		fprintf(stderr, "unknow buffer address\n");
		return -1;
	}

	raw = argb2raw(color);

	line_stride = dev->stride/4;
	for (y=0; y<dev->height; y++) {
		pcurrent = pline_start;
		pline_start += line_stride;
		for (x=0; x<dev->width; x++)
			*pcurrent++ = raw;
	}
	return 0;
}

static int _check_coords(struct canvas *ca, point_t *coords)
{
	struct virtual_device *dev = ca->private.device;

	if ((coords->x < 0)
			|| (coords->x >= dev->width)
			|| (coords->y < 0)
			|| (coords->y >= dev->height)) {
		return -1;
	}
	return 0;
}

static int draw_point(struct canvas *ca, struct argb_t *color, point_t *coords)
{
	struct virtual_device *dev = ca->private.device;
	char *p;
	uint32_t *pixel;

	if (_check_coords(ca, coords)) {
		fprintf(stderr, "%s input params out of range\n", __func__);
		return -1;
	}

	p = (char *)dev->base
			+ (coords->y * dev->stride)
			+ (coords->x * dev->bpp / 8);

	 pixel = (uint32_t *)p;
	*pixel = argb2raw(color);

	return 0;
}

static int draw_line(struct canvas *ca,
	struct argb_t *color, point_t *from, point_t *to)
{
	point_t *tmp;

	if (_check_coords(ca, from) || _check_coords(ca, to)) {
		fprintf(stderr, "invalid coords to draw line\n");
		return -1;
	}

	if (from->x != to->x && from->y != to->y) {
		fprintf(stderr,
			"draw line error, only Vertical or Parallel line support\n");
		return -1;
	}

	if (from->x > to->x || from->y > to->y) {
		tmp = from;
		from = to;
		to = tmp;
	}
	draw_rectangle(ca, color, from, to);

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

static int draw_rectangle(struct canvas *ca,
	struct argb_t *color, point_t *top_left, point_t *bottom_right)
{
	struct virtual_device *dev = ca->private.device;
	char *from;
	int stride, lines;
	int value, i;

	if (_check_coords(ca, top_left)
			|| _check_coords(ca, bottom_right)
			|| (top_left->x > bottom_right->x)
			|| (top_left->y > bottom_right->y)) {
		fprintf(stderr, "%s input params out of range\n", __func__);
		return -1;
	}

	value = argb2raw(color);
	lines = bottom_right->y - top_left->y;
	stride = (bottom_right->x - top_left->x) * dev->bpp / 8;
	from = (char *)dev->base
				+ (top_left->y * dev->stride)
				+ (top_left->x * dev->bpp / 8);

	for (i=0; i<=lines; i++) {
		_memset32((uint32_t *)from, value, stride);
		from += dev->stride;
	}

	return 0;
}

static int draw_by_func(struct canvas *ca, color_function func)
{
	int i, j;
	struct argb_t color;
	struct virtual_device *dev = ca->private.device;
	char *lines_start;
	char *from;

	lines_start = (char *)dev->base;
	for (i=0; i<dev->height; i++) {
		from = lines_start;
		for (j=0; j<dev->width; j++) {
			func(j, i, &color);
			_memset32((uint32_t *)from, argb2raw(&color), 4);
			from += 4;
		}
		lines_start += dev->stride;
	}

	return 0;
}
