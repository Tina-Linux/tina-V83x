target	= nativepower_daemon

INCLUDES        += -I./include -I$(CONFIG_PREFIX)/include -I$(CONFIG_PREFIX)/usr/include
LIBS            += -L ../libsuspend
LIBS		+= -lpthread -lsuspend -lscenemanager


SRCS = main.c \
	power_manager.c \
	np_input.c

LOCAL_CFLAGS = $(INCLUDES)

ifeq ($(USE_DBUS), y)
LIBS		+= -ldbus-1
SRCS		+= dbus_server.c
LOCAL_CFLAGS	+= -DUSE_DBUS
endif

ifeq ($(USE_TINA_SUSPEND), y)
LOCAL_CFLAGS += -DUSE_TINA_SUSPEND
endif

LOCAL_CFLAGS += -DCONFIG_TLOG_LEVEL=OPTION_TLOG_LEVEL_WARNING

install:

$(target): $(SRCS)
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib $^ -o $@ $(LIBS)

####################################################################
all: install $(target)
clean:
	rm -rf $(target)
