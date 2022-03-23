target		= libbatinfo.so

LOCAL_CFLAGS 	= -I. -I./include

ifeq ($(BROADCAST_INFO),y)
LOCAL_CFLAGS 	+= -DBROADCAST_INFO
LIBS 		+= -ldbus-1
endif

SRCS = batinfo.c

OBJS = $(SRCS:.c=.o)

install:$(target)
	@mkdir -p $(CONFIG_PREFIX)/usr/lib
	@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp -r ./include/batinfo $(CONFIG_PREFIX)/usr/include
	@cp $(target) $(CONFIG_PREFIX)/usr/lib

%.o: %.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ $< -fPIC

$(target): $(OBJS)
	$(CC) -fPIC -shared $(INCLUDES) $(LDFLAGS) $(LIBS) $^ -o $@

####################################################################
all: install
clean:
	rm -rf $(target) $(OBJS)
