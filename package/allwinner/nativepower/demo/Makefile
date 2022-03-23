target	= nativepower_client

INCLUDES        += -I. -I./include -I$(CONFIG_PREFIX)/usr/include
LIBS            += -lscenemanager

SRCS =	main.c

LOCAL_CFLAGS = $(INCLUDES)

ifeq ($(USE_DBUS), y)
LOCAL_CFLAGS 	+= -DUSE_DBUS
LIBS		+= -lnativepower
endif

LOCAL_CFLAGS += -DCONFIG_TLOG_LEVEL=OPTION_TLOG_LEVEL_WARNING

install:


$(target): $(SRCS)
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib $^ -o $@ $(LIBS)
	@cp $@ $(CONFIG_PREFIX)/usr/lib

####################################################################
all: install $(target)
clean:
	rm -rf $(target)
