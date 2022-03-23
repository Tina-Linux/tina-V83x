TARGET = recovery-mode

LOCAL_CFLAGS =
LOCAL_LDFLAGS =

SRCS += ${wildcard *.c}
OBJS = $(patsubst %.c, %.c.o, $(SRCS))

all:install

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -o $@ $^

$(OBJS):%.c.o:%.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -c $^ -o $@

install:$(TARGET)
	@mkdir -p $(CONFIG_PREFIX)/bin
	@cp $(TARGET) $(CONFIG_PREFIX)/bin

clean:
	-rm -rf $(OBJS) $(TARGET)
.PHONY:all clean $(TARGET) install
