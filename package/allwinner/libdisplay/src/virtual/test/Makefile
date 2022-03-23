# *
# * screent test base on framebuffer device
# * Copyright (C) 2015-2018 AllwinnerTech
# *
# * This program is free software; you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation; either version 2 of the License, or
# * (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this program; if not, write to the Free Software
# * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# *

INCLUDE_FLAGS = -I../

CFLAGS = $(INCLUDE_FLAGS) -Wall -g

ifeq ($(arch),arm)
	CC	  = arm-linux-gcc
	AR	  = arm-linux-ar
	STRIP = arm-linux-strip
else
	CC	  = gcc
	AR	  = ar
endif

LFLAGS = -lm \
	-L../ -lbmp

SHELL   = /bin/bash

SUBDIRS =

TARGET = bmp

export CC
export AR

SOUCE_FILES = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SOUCE_FILES))

%.o : %.c %.h
	$(CC) -c $(CFLAGS) $<

%.o : %.c
	$(CC) -c $(CFLAGS) $<

$(TARGET): $(OBJS) libs
	$(CC) $(OBJS) -o $@ $(CFLAGS) $(LFLAGS)

libs:
	@ for subdir in $(SUBDIRS); do \
        (cd $$subdir && $(MAKE)); \
	done

.PHONY:clean strip tags

strip: $(TARGET)
	$(STRIP) $(TARGET)

tags:
	ctags -R *

clean:
	$(RM) $(TARGET) $(OBJS)
	@ for subdir in $(SUBDIRS); do \
        (cd $$subdir && $(MAKE) clean); \
    done
