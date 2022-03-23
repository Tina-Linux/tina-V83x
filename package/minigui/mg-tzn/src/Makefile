TARGET = smart_ctrl

LOCAL_CFLAGS =
LOCAL_LDFLAGS = -lminigui_ths -lts -lasound -ljpeg -lpng -ldl -lm -lpthread

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
