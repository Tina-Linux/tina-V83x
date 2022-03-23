
include $(top_srcdir)/Makefile.inc

lib_LTLIBRARIES = libMemAdapter.la

libMemAdapter_la_SOURCES =  ./memoryAdapter.c \
							./ionMemory/ionAlloc.c

#			./secureMemory/secureAlloc.c
#			./sunxiMemory/sunxiAlloc.c

libMemAdapter_la_CFLAGS = $(CFLAGS_CDCG)
LOCAL_INCLUDE = -I./include \
				-I$(top_srcdir)/include \
				-I$(top_srcdir)/base/include \
				-I$(top_srcdir)/ve/include

libMemAdapter_la_CFLAGS += $(LOCAL_INCLUDE)


libMemAdapter_la_LDFLAGS = $(LDFLAGS_CDCG)

libMemAdapter_la_LIBADD = $(top_srcdir)/base/libcdc_base.la
#						$(top_srcdir)/ve/libVE.la

#LOCAL_LIB = -lpthread
LOCAL_LIB = -lVE

libMemAdapter_la_LDFLAGS += $(LOCAL_LIB)
