
include $(top_srcdir)/Makefile.inc

lib_LTLIBRARIES = libcdc_base.la

libcdc_base_la_SOURCES = CdcSysinfo.c

libcdc_base_la_CFLAGS = $(CFLAGS_CDCG)
LOCAL_INCLUDE = -I./include \
				-I$(top_srcdir)/include

libcdc_base_la_CFLAGS += $(LOCAL_INCLUDE)

libcdc_base_la_LDFLAGS = $(LDFLAGS_CDCG)
#LOCAL_LIB = -lpthread
libcdc_base_la_LDFLAGS += $(LOCAL_LIB)
