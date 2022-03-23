target	= libscenemanager.so

INCLUDES        += -I. -I./include
LIBS            += -luci

SRCS = scene_manager.c \
	np_scene_utils.c \
	np_uci_config.c \
	power.c

LOCAL_CFLAGS = $(INCLUDES) $(CFLAGS)

LOCAL_CFLAGS += -D$(shell echo $(TARGET_CHIP) | tr a-z A-Z)

LOCAL_CFLAGS += -DLOG_TAG="\"libscenemanager\""
LOCAL_CFLAGS += -DCONFIG_TLOG_LEVEL=OPTION_TLOG_LEVEL_WARNING

install: $(target)
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp $(target) $(CONFIG_PREFIX)/usr/lib
	@cp ./include/scene_manager.h $(CONFIG_PREFIX)/usr/include

$(target): $(SRCS)
	$(CC) -fPIC -shared $(LOCAL_CFLAGS) $(LDFLAGS) $^ -o $@ $(LIBS)

####################################################################
all: install $(target)
clean:
	rm -rf $(target)
