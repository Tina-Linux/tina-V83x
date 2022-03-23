LOCAL_CFLAGS += -DRECOVERY_BGRA -DOVERSCAN_PERCENT=0 -DBOARD_RECOVERY_ROTATION=0 -I.
LOCAL_LDFLAGS =

SRCS += ${wildcard *.c}
OBJS = $(patsubst %.c, %.c.o, $(SRCS))
TARGET = libspeex-lite.so

all:$(TARGET) install

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -o $@ $^ -fPIC -shared

$(OBJS):%.c.o:%.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -c $^ -o $@ -fPIC -shared

clean:
	-rm -rf $(OBJS) $(TARGET)

install:
	-mkdir -p $(CONFIG_PREFIX)/bin
	-mkdir -p $(CONFIG_PREFIX)/lib
	-mkdir -p $(CONFIG_PREFIX)/include
	-cp $(TARGET) $(CONFIG_PREFIX)/lib/
	-cp *.h $(CONFIG_PREFIX)/include/

.PHONY:all clean $(TARGET) install
