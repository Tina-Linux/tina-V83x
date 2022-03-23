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

#include "testcase.h"

#define ARRAY_SIZE(_) (sizeof (_) / sizeof (*_))

const struct testcase_info testcase_array[] = {
	{	"solid_color",
		"Pure color display",
		solid_color,
	},
	{	"colorbar",
		"Sequence: White, Yellow, Cyan, Green, Magentam, Red, Blue, Black",
		colorbar,
	},
	{
		"grayscale",
		"From black(0,0,0) to white(255,255,255)",
		grayscale,
	},
	{
		"chessboard",
		"Black and White chessboard",
		chessboard,
	},
	{
		"gradient",
		"Gray gradient image",
		gradient,
	},
	{
		"movement",
		"Movement test",
		movement,
	}
};

const struct testcase_info *search_testcase_by_name(const char *name)
{
	int i;
	int size = ARRAY_SIZE(testcase_array);
	const struct testcase_info *retval = NULL;

	for (i=0; i<size; i++) {
		if (strcmp(testcase_array[i].name, name) == 0) {
			retval = &testcase_array[i];
			break;
		}
	}
	return retval;
}

void dump_available_testcase(void)
{
	int i;
	int size = ARRAY_SIZE(testcase_array);

	fprintf(stderr, "\nAvailable testcase(%d):\n", size);
	for (i=0; i<size; i++) {
		fprintf(stderr, "\t%12s - %s\n",
			testcase_array[i].name,
			testcase_array[i].describe);
	}
}

