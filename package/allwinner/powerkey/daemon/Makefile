target	= powerkey_daemon

INCLUDES        += -I./include -I$(CONFIG_PREFIX)/include -I$(CONFIG_PREFIX)/usr/include
LIBS		+= -lpthread


SRCS = main.c \
	np_input.c

LOCAL_CFLAGS = $(INCLUDES)


LOCAL_CFLAGS += -DCONFIG_TLOG_LEVEL=OPTION_TLOG_LEVEL_WARNING

install:

$(target): $(SRCS)
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib $^ -o $@ $(LIBS)

####################################################################
all: install $(target)
clean:
	rm -rf $(target)
