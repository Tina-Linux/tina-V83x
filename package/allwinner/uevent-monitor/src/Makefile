TARGET = uevent-monitor

LIBS := -lev
COMPILE_FLAGS := $(LIBS)

$(TARGET): uevent-monitor.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $^ $(COMPILE_FLAGS)

all:$(TARGET) install

install:
	mkdir -p $(CONFIG_PREFIX)/usr/bin
	cp $(TARGET) $(CONFIG_PREFIX)/usr/bin/

clean:
	rm -rf $(TARGET) $(OBJS)
