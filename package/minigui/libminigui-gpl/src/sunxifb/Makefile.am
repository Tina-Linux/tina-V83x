
## Makefile.am for the sunxifb video driver

AM_CPPFLAGS = -I$(abs_top_srcdir)/src/include -I$(abs_top_srcdir)/include \
			            -I$(abs_top_srcdir)/src/newgal/

noinst_LTLIBRARIES = libvideo_sunxifb.la

# The sunxifb video driver sources
SRC_FILES =             \
    sunxifb.c

HDR_FILES= \
    sunxifb.h

libvideo_sunxifb_la_SOURCES = $(SRC_FILES) $(HDR_FILES)

EXTRA_DIST=makefile.ng makefile.msvc

LIB_NAME=libvideo_sunxifb
