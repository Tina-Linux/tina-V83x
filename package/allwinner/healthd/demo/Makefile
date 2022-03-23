target		= healthd-demo

LOCAL_CFLAGS 	+= -I. -I../libbatinfo/include
LIBS            := -L../libbatinfo -lbatinfo -ldbus-1

SRCS = main.c

OBJS = $(SRCS:.c=.o)

install:$(target)
	@mkdir -p $(CONFIG_PREFIX)/usr/bin
	@cp $(target) $(CONFIG_PREFIX)/usr/bin

%.o: %.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ $<

$(target): $(OBJS)
	$(CC) $(INCLUDES) $(LDFLAGS) $(LIBS) $^ -o $@

####################################################################
all: install
clean:
	rm -rf $(target) $(OBJS)
