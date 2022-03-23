
libvopdir = $(libdir)
libvop_PROGRAMS = libsunxivop.so

libsunxivop_so_LDFLAGS  = -shared -fpic
libsunxivop_so_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/src/edid -fPIC
libsunxivop_so_LDADD    = -L./edid -lsunxiedid

libsunxivop_so_SOURCES  = sunxi_vop.c

# install header
nobase_include_HEADERS  = sunxi_vop.h


