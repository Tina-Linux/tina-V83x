TARGET = libalsa-ucm-aw.so

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))

INCLUDES +=
LIBS += -lasound

all: $(TARGET) install

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -fPIC -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $^ $(LDFLAGS) $(LIBS) -shared -o $@

install:
	mkdir -p $(INSTALL_PREFIX)/usr/lib
	cp $(TARGET) $(INSTALL_PREFIX)/usr/lib
	mkdir -p $(INSTALL_PREFIX)/usr/include
	cp alsa-ucm-aw.h $(INSTALL_PREFIX)/usr/include

clean:
	rm -rf $(OBJS) $(TARGET)

.PHONY: all clean install
