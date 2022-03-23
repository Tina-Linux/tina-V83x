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

#ifndef __SCREEN_TEST_H__
#define __SCREEN_TEST_H__

#include "display.h"

typedef int (*testcase_entry)(struct canvas *canvas, const char *params);

/*
 * name: testcase name
 * describe: the brief of the testcase
 * entry : testcase function entry
 */
struct testcase_info {
	const char *name;
	const char *describe;
	testcase_entry entry;
};

#endif
