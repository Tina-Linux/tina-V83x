
libediddir = $(libdir)
libedid_PROGRAMS = libsunxiedid.so

libsunxiedid_so_LDFLAGS  = -shared -fpic
libsunxiedid_so_CPPFLAGS = -I$(top_srcdir)/src -fPIC

libsunxiedid_so_SOURCES  = cea_vic.cpp edid.cpp sunxi_edid.cpp
