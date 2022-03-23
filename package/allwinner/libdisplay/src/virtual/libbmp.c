/* Copyright (C)
* 2015 - ZengYajian
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#include "libbmp.h"


static int file_operator_init(struct file_operator *fops,
		const char *fname, const char *mode)
{
	fops->fd = fopen(fname, mode);
	if (!fops->fd) {
		perror("fopen error");
		return -1;
	}

	strncpy(fops->name, fname, MAX_NAME_LEN);
	return 0;
}

static char * file_operator_get_fname(struct file_operator *fops)
{
	return fops->name;
}

static int _get_short_int(char **p)
{
	int tmp;
	char *ptr = *p;

	tmp  = ((*ptr++) & 0xff);
	tmp += ((*ptr++) & 0xff)<<8;

	*p = ptr;
	return tmp;
}

static int _get_int(char **p)
{
	int tmp;
	char *ptr = *p;

	tmp  = ((*ptr++) & 0xff);
	tmp += ((*ptr++) & 0xff)<<8;
	tmp += ((*ptr++) & 0xff)<<16;
	tmp += ((*ptr++) & 0xff)<<24;

	*p = ptr;
	return tmp;
}

static int file_operator_get_image_tag(struct file_operator *fops, struct tag *t)
{
	char tmp[BMP_TAG_LEN];
	char *p = tmp;

	if (!fops->fd) return -1;

	fseek(fops->fd, BMP_TAG_OFFSET, SEEK_SET);
	fread(p, BMP_TAG_LEN, 1, fops->fd);

	/* get bmp tag */
	t->type = _get_short_int(&p);
	t->size = _get_int(&p);
	_get_short_int(&p);	/* reserved1 */
	_get_short_int(&p);	/* reserved2 */
	t->offset = _get_int(&p);

	return 0;
}

static int file_operator_get_image_hdr(struct file_operator *fops, struct header *hdr)
{
	char tmp[BMP_HDR_LEN];
	char *p = tmp;

	if (!fops->fd) return -1;

	fseek(fops->fd, BMP_HDR_OFFSET, SEEK_SET);
	fread(p, BMP_HDR_LEN, 1, fops->fd);

	/* get bmp header */
	hdr->size = _get_int(&p);
	hdr->width = _get_int(&p);
	hdr->height = _get_int(&p);
	hdr->planes = _get_short_int(&p);
	hdr->bitcount = _get_short_int(&p);
	hdr->compression = _get_int(&p);
	hdr->image_size = _get_int(&p);
	hdr->x_bixpelspermeter = _get_int(&p);
	hdr->y_bixpelspermeter = _get_int(&p);
	hdr->color_used = _get_int(&p);
	hdr->color_important = _get_int(&p);

	fops->bytes_per_line = BYTES_PER_LINE(hdr->width, hdr->bitcount);

	return 0;
}

static int file_operator_get_color_table(struct file_operator *fops,
		struct color_table *table, int bitcount)
{
	int len = 1 << bitcount;

	fseek(fops->fd, BMP_COLOR_TABLE_OFFSET, SEEK_SET);
	fread((void *)table, len*4, 1, fops->fd);

	return 0;
}

static int file_operator_get_one_line(struct file_operator *fops, char *buf, int line)
{
	int offset = fops->tag.offset + (fops->bytes_per_line * line);

	fseek(fops->fd, offset, SEEK_SET);
	fread((void *)buf, fops->bytes_per_line, 1, fops->fd);

	return 0;
}

static int file_operator_set_one_line(struct file_operator *fops, char *buf, int line)
{
	int offset = fops->tag.offset + (fops->bytes_per_line * line);

	fseek(fops->fd, offset, SEEK_SET);
	fwrite((void *)buf, fops->bytes_per_line, 1, fops->fd);

	return 0;
}

#define SET_SHORT(ptr, data) \
do { \
	*(ptr++) = (char)(data & 0xff); \
	*(ptr++) = (char)((data>>8) & 0xff); \
} while(0)

#define SET_LONG(ptr, data) \
do { \
	*(ptr++) = (char)(data & 0xff); \
	*(ptr++) = (char)((data>>8) & 0xff); \
	*(ptr++) = (char)((data>>16) & 0xff); \
	*(ptr++) = (char)((data>>24) & 0xff); \
} while(0)

static int file_operator_set_image_tag(struct file_operator *fops, struct tag *t)
{
	char buf[BMP_TAG_LEN];
	char *p = buf;

	memset(p, 0, BMP_TAG_LEN);

	SET_SHORT(p, t->type);
	 SET_LONG(p, t->size);
	SET_SHORT(p, 0);
	SET_SHORT(p, 0);
	 SET_LONG(p, t->offset);

	fwrite((void *)buf, BMP_TAG_LEN, 1, fops->fd);
	fops->fsync(fops);

	return 0;
}

static int file_operator_set_image_hdr(struct file_operator *fops, struct header *hdr)
{
	char buf[BMP_HDR_LEN];
	char *p = buf;

	memset(p, 0, BMP_HDR_LEN);
	SET_LONG(p, hdr->size);
	SET_LONG(p, hdr->width);
	SET_LONG(p, hdr->height);
   SET_SHORT(p, hdr->planes);
   SET_SHORT(p, hdr->bitcount);
	SET_LONG(p, hdr->compression);
	SET_LONG(p, hdr->image_size);
	SET_LONG(p, hdr->x_bixpelspermeter);
	SET_LONG(p, hdr->y_bixpelspermeter);
	SET_LONG(p, hdr->color_used);
	SET_LONG(p, hdr->color_important);

	fwrite((void *)buf, BMP_HDR_LEN, 1, fops->fd);
	fops->fsync(fops);

	return 0;
}

static int file_operator_set_color_table(struct file_operator *fops,
		struct color_table *table, int bitcount)
{
	int len = 1 << bitcount;

	fseek(fops->fd, BMP_COLOR_TABLE_OFFSET, SEEK_SET);
	fwrite((void *)table, len*4, 1, fops->fd);
	fops->fsync(fops);

	return 0;
}

static int file_operator_fsync(struct file_operator *fops)
{
	fflush(fops->fd);
	return 0;
}

static int file_operator_dump_image_info(struct file_operator *fops)
{
	struct tag *tag = &fops->tag;
	struct header *hdr = &fops->hdr;

	printf("* image: %s\n", fops->get_fname(fops));
	printf("              type: 0x%04x\n", tag->type);
	printf("              size: %d bytes\n", tag->size);
	printf("            offset: 0x%08x\n", tag->offset);
	printf("--------------------------\n");
	printf("              size: %d\n", hdr->size);
	printf("             width: %d\n", hdr->width);
	printf("            height: %d\n", hdr->height);
	printf("            planes: %d\n", hdr->planes);
	printf("          bitcount: %d\n", hdr->bitcount);
	printf("       compression: %d\n", hdr->compression);
	printf("        image_size: %d\n", hdr->image_size);
	printf(" x_bixpelspermeter: %d\n", hdr->x_bixpelspermeter);
	printf(" y_bixpelspermeter: %d\n", hdr->y_bixpelspermeter);
	printf("        color_used: %d\n", hdr->color_used);
	printf("   color_important: %d\n", hdr->color_important);

	printf("--------------------------\n");
	printf("    bytes per line: %d\n", fops->bytes_per_line);
	printf(" total data length: %d\n", fops->bytes_per_line * fops->hdr.height);

	return 0;
}

static int file_operator_dump_color_table(struct file_operator *fops)
{
	int len = 1 << fops->hdr.bitcount;
	int i;

	if (fops->hdr.bitcount>8)
		return 0;

	for (i=0; i<len; i++) {
		printf("[%3d, %3u, %3u, %3u] ",
			fops->pallet.item[i].blue,
			fops->pallet.item[i].green,
			fops->pallet.item[i].red,
			fops->pallet.item[i].reserved);

		if (((i+1)&0x3) == 0)
			printf("\n");
	}

	return 0;
}

static int file_operator_load_image(struct file_operator *fops)
{
	fops->get_image_tag(fops, &fops->tag);
	fops->get_image_hdr(fops, &fops->hdr);

	if (fops->hdr.bitcount <= 8)
		fops->get_color_table(fops, &fops->pallet, fops->hdr.bitcount);

	return 0;
}

struct file_operator *alloc_file_operator(void)
{
	struct file_operator *fops = NULL;

	fops = malloc(sizeof(*fops));
	if (!fops) {
		perror("alloc file operator error");
		return NULL;
	}

	memset(fops, 0, sizeof(*fops));
	fops->init = file_operator_init;
	fops->get_fname = file_operator_get_fname;
	fops->load_image = file_operator_load_image;

	fops->dump_image_info = file_operator_dump_image_info;
	fops->dump_color_table = file_operator_dump_color_table;

	fops->get_image_tag = file_operator_get_image_tag;
	fops->get_image_hdr = file_operator_get_image_hdr;
	fops->get_color_table = file_operator_get_color_table;

	fops->set_image_tag = file_operator_set_image_tag;
	fops->set_image_hdr = file_operator_set_image_hdr;
	fops->set_color_table = file_operator_set_color_table;

	fops->get_one_line = file_operator_get_one_line;
	fops->set_one_line = file_operator_set_one_line;
	fops->fsync = file_operator_fsync;

	return fops;
}

static int create_image(struct libbmp *image, int x, int y, int bpp, const char *name)
{
	struct file_operator *raw;

	raw = alloc_file_operator();
	image->raw = raw;

	raw->init(raw, name, "wb+");
	raw->bytes_per_line = alignment_up((x*bpp/8), 4);

	raw->tag.type  = 0x4D42;
	raw->tag.size  = raw->bytes_per_line * y + 54;
	raw->tag.offset = 0x36;

	raw->hdr.size              = 40;
	raw->hdr.width             = x;
	raw->hdr.height            = y;
	raw->hdr.planes            = 1;
	raw->hdr.bitcount          = bpp;
	raw->hdr.compression       = 0;
	raw->hdr.image_size        = raw->bytes_per_line * y;
	raw->hdr.x_bixpelspermeter = 0;
	raw->hdr.y_bixpelspermeter = 0;
	raw->hdr.color_used        = 0;
	raw->hdr.color_important   = 0;

	raw->image = (char *)malloc(raw->hdr.image_size);
	if (!raw->image) {
		perror("can not alloc image buffer");
		return -1;
	}
	memset(raw->image, 0, raw->hdr.image_size);
	return 0;
}

static char * get_image_buffer(struct libbmp *image)
{
	return image->raw->image;
}

static int get_image_buffer_size(struct libbmp *image)
{
	return image->raw->hdr.image_size;
}

static int get_image_buffer_stride(struct libbmp *image)
{
	return image->raw->bytes_per_line;
}

static int save_image(struct libbmp *image)
{
	int i;
	struct file_operator *raw = image->raw;

	raw->set_image_tag(raw, &raw->tag);
	raw->set_image_hdr(raw, &raw->hdr);

	char *line = raw->image;
	for (i=0; i<raw->hdr.height; i++) {
		raw->set_one_line(raw, line, i);
		line += raw->bytes_per_line;
	}
	return 0;
}

static int dump_image_info(struct libbmp *image)
{
	image->raw->dump_image_info(image->raw);
	return 0;
}

struct libbmp *alloc_bmp(void)
{
	struct libbmp *bmp = NULL;

	bmp = malloc(sizeof(*bmp));
	if (!bmp) {
		perror("alloc bmp error");
		return NULL;
	}

	memset(bmp, 0, sizeof(*bmp));
	bmp->create_image     = create_image;
	bmp->get_image_buffer = get_image_buffer;
	bmp->get_image_buffer_size = get_image_buffer_size;
	bmp->get_image_buffer_stride = get_image_buffer_stride;

	bmp->save_image       = save_image;
	bmp->dump_image_info  = dump_image_info;

	return bmp;
}
