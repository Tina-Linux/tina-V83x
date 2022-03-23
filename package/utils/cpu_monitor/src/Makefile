target			= cpu_monitor

INCLUDES        += -I.

SRCS = cpu_test.c cpu_lib.c
OBJS = $(SRCS:.c=.o)

install:$(target)
	@mkdir -p $(CONFIG_PREFIX)/usr/bin
	@cp $(target) $(CONFIG_PREFIX)/usr/bin

%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(target): $(OBJS)
	$(CC) $(INCLUDES) $(LDFLAGS) $^ -o $@

####################################################################
all: install
clean:
	rm -rf $(target) $(OBJS)
