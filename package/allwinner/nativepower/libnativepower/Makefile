target	= libnativepower.so

INCLUDES        += -I. -I./include
LIBS            += -ldbus-1

SRCS = power_manager_client.c \
	dbus_client.c \

LOCAL_CFLAGS = $(INCLUDES)
LOCAL_CFLAGS += -DCONFIG_TLOG_LEVEL=OPTION_TLOG_LEVEL_WARNING

install:$(target)
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp $(target) $(CONFIG_PREFIX)/usr/lib
	@cp power_manager_client.h $(CONFIG_PREFIX)/usr/include/

$(target): $(SRCS)
	$(CC) -fPIC -shared $(CFLAGS) $(INCLUDES) $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib $^ -o $@ $(LIBS)

####################################################################
all: install $(target)
clean:
	rm -rf $(target)
