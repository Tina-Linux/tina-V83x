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
include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/package.mk

INCLUDE_FLAGS = -I. -I../ -I../include
CFLAGS	      = $(INCLUDE_FLAGS) -Wall

AFLAGS		  = -r

TARGET = libbmp.a

SOUCE_FILES = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SOUCE_FILES))

%.o : %.c %.h
	$(CC) -c $(CFLAGS) $<

%.o : %.c
	$(CC) -c $(CFLAGS) $<

$(TARGET): $(OBJS)
	$(AR) $(AFLAGS) $(TARGET) $(OBJS)

all: $(TARGET)

.PHONY:clean
clean:
	$(RM) $(TARGET) $(OBJS)
