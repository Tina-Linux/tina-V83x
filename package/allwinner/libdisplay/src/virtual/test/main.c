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

#include <stdio.h>
#include <stdlib.h>

#include "libbmp.h"

int main(int argc, char **argv)
{
	struct libbmp *image;
	int x, y;
	char *buffer;
	int i,j;

	if (argc < 4) return 0;

	sscanf(argv[2], "%d", &x);
	sscanf(argv[3], "%d", &y);

	printf("image: %s, %dx%d\n", argv[1], x, y);
	image = alloc_bmp();
	image->create_image(image, x, y, 32, argv[1]);
	image->dump_image_info(image);

	buffer = image->get_image_buffer(image);

	for (i=0; i<y; i++)
		for (j=0; j<x*4; j++)
			*buffer++ = i*j;

	image->save_image(image);

	return 0;
}
