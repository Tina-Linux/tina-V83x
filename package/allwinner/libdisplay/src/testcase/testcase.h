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

#ifndef __TESTCASE_H__
#define __TESTCASE_H__

#include "screen-test.h"

void dump_available_testcase(void);
const struct testcase_info *search_testcase_by_name(const char *name);

int solid_color(struct canvas *canvas, const char *params);
int colorbar(struct canvas *canvas, const char *params);
int grayscale(struct canvas *canvas, const char *params);
int chessboard(struct canvas *canvas, const char *params);
int gradient(struct canvas *canvas, const char *params);
int movement(struct canvas *canvas, const char *params);

#endif
