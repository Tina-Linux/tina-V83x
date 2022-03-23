C ?= g++

TARGET = smartlink_raw

INCLUDES += -I. \

SRCS := \
	raw.c \

SUB_LIB := \
	-lpthread \

$(TARGET):
	$(CC) $(LDFLAGS) $(INCLUDES) $(SRCS)  -o $@ $(SUB_LIB)


all:$(TARGET) install

clean:
	rm -rf $(TARGET) *.o *.a *~

install:
	mkdir -p $(CONFIG_PREFIX)/usr/bin
	cp $(TARGET) $(CONFIG_PREFIX)/usr/bin
