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

#include "screen-test.h"

struct palette {
	struct argb_t color;
	char *name;
};

static const struct palette table[] = {
	{{0xFF, 0xFF, 0xFF, 0xFF}, "White"   },/* White   */
	{{0xFF, 0xFF, 0xFF, 0x00}, "Yellow"  },/* Yellow  */
	{{0xFF, 0x00, 0xFF, 0xFF}, "Cyan"    },/* Cyan    */
	{{0xFF, 0x00, 0x80, 0x00}, "Green"   },/* Green   */
	{{0xFF, 0xFF, 0x00, 0xFF}, "Magenta" },/* Magenta */
	{{0xFF, 0xFF, 0x00, 0x00}, "Red"     },/* Red     */
	{{0xFF, 0x00, 0x00, 0xFF}, "Blue"    },/* Blue    */
	{{0xFF, 0x00, 0x00, 0x00}, "Black"   },/* Black   */
};

int solid_color(struct canvas *canvas, const char *params)
{
	int i;
	struct argb_t *color = NULL;
	int size = sizeof(table) / sizeof(*table);

	for (i=0; i<size; i++) {
		if (strcmp(table[i].name, params) == 0) {
			color = (struct argb_t *)&table[i].color;
			break;
		}
	}

	if (color)
		canvas->draw_color(canvas, color);

	return 0;
}

