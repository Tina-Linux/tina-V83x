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

#ifndef __libbmp_h__
#define __libbmp_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#define alignment_down(a, size) (a & (~(size-1)) )
#define alignment_up(a, size)   ((a+size-1) & (~ (size-1)))

#define BYTES_PER_LINE(width, bitcount)	(((width)*(bitcount) + 31)/32*4)

#define BMP_TAG_OFFSET	(0)
#define BMP_TAG_LEN		(14)
#define BMP_HDR_OFFSET	(BMP_TAG_OFFSET + BMP_TAG_LEN)
#define BMP_HDR_LEN		(40)
#define BMP_COLOR_TABLE_OFFSET	(BMP_HDR_OFFSET + BMP_HDR_LEN)

#define MAX_NAME_LEN 128

struct tag {
	int type;	/* bmp image must be 0x424d */
	int size;	/* file size in bytes */
	int reserved1, reserved2;
	int offset;	/* bitmap data start offset */
};

struct header {
	int size;			/* size of the bmp header, byte 15~18 */
	int width;			/* width of image, in pix, byte 19~22 */
	int height;			/* height of image, in pix, byte 23~26 */
	int planes;			/* must be 1, byte 27~28 */
	int bitcount;		/* bytes of each piex, 1/4/8/16/24 , byte 29~30 */
	int compression;	/* must be 0/1/2 , byte 31~34 */
	int image_size;		/* in bytes, byte 35~38 */
	int x_bixpelspermeter;	/* byte 39-42 */
	int y_bixpelspermeter; 	/* byte 43~46 */
	int color_used;			/* total number in color table, byte 47~50 */
	int color_important;	/* byte 51~54 */
};

struct rgb_t {
	unsigned char blue;
	unsigned char green;
	unsigned char red;
	unsigned char reserved;
};

struct color_table {
	struct rgb_t item[256];
};

struct file_operator
{
	FILE *fd;
	char name[MAX_NAME_LEN];

	struct tag tag;
	struct header hdr;
	struct color_table pallet;
	char *image;

	int bytes_per_line;

	char *(*get_fname)(struct file_operator *fops);
	int (*init)(struct file_operator *fops,
			const char* fname, const char *mode);

	int (*load_image)(struct file_operator *fops);
	int (*dump_image_info)(struct file_operator *fops);
	int (*dump_color_table)(struct file_operator *fops);

	int (*get_image_tag)(struct file_operator *fops, struct tag *t);
	int (*get_image_hdr)(struct file_operator *fops, struct header *hdr);
	int (*get_color_table)(struct file_operator *fops,
			struct color_table *table, int size);

	int (*set_image_tag)(struct file_operator *fops, struct tag *t);
	int (*set_image_hdr)(struct file_operator *fops, struct header *hdr);
	int (*set_color_table)(struct file_operator *fops,
			struct color_table *table, int size);

	int (*get_one_line)(struct file_operator *fops, char *buf, int line);
	int (*set_one_line)(struct file_operator *fops, char *buf, int line);

	int (*fsync)(struct file_operator *fops);
};

struct libbmp
{
	struct file_operator *raw;

	int (*create_image)(struct libbmp *image,
			int x, int y, int bpp, const char *name);
	char *(*get_image_buffer)(struct libbmp *image);
	int (*get_image_buffer_size)(struct libbmp *image);
	int (*get_image_buffer_stride)(struct libbmp *image);
	int (*save_image)(struct libbmp *image);
	int (*dump_image_info)(struct libbmp *image);
};

struct file_operator *alloc_file_operator(void);
struct libbmp *alloc_bmp(void);

#endif
