target	= libsuspend.a

INCLUDES        += -I. -I./include

LOCAL_CFLAGS 	=$(INCLUDES)
LOCAL_CFLAGS    += -fPIC

SRCS = 	autosuspend.c \
	autosuspend_autosleep.c \
	autosuspend_wakeup_count.c \

OBJS = $(SRCS:.c=.o)

ifeq ($(USE_TINA_SUSPEND), y)
SRCS += autosuspend_tinasuspend.c
LOCAL_CFLAGS+=-DUSE_TINA_SUSPEND
else
SRCS += autosuspend_earlysuspend.c
endif

LOCAL_CFLAGS += -DCONFIG_TLOG_LEVEL=OPTION_TLOG_LEVEL_WARNING

install:$(target)
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp -r ./include/suspend $(CONFIG_PREFIX)/usr/include/

%.o:%.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ $<

$(target): $(OBJS)
	#$(CC) -fPIC -shared $(CFLAGS) $(LOCAL_CFLAGS) $(LDFLAGS) $^ -o $@ $(LIBS)
	$(AR) rcs -o $@ $(OBJS)

####################################################################
all: install $(target)
clean:
	rm -rf $(target)
