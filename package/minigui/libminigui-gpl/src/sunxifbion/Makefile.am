
## Makefile.am for the sunxifb video driver

AM_CPPFLAGS = -I$(abs_top_srcdir)/src/include -I$(abs_top_srcdir)/include \
			            -I$(abs_top_srcdir)/src/newgal/

noinst_LTLIBRARIES = libvideo_sunxifbion.la

# The sunxifb video driver sources
SRC_FILES =             \
    sunxifbion.c

HDR_FILES= \
    sunxifbion.h

libvideo_sunxifbion_la_SOURCES = $(SRC_FILES) $(HDR_FILES)

EXTRA_DIST=makefile.ng makefile.msvc

LIB_NAME=libvideo_sunxifbion
