TARGET = alsa-ucm-aw

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))

override INCLUDES +=
override LIBS += -lalsa-ucm-aw

all: $(TARGET) install

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $^ $(LDFLAGS) $(LIBS) -o $@

install:
	mkdir -p $(INSTALL_PREFIX)/usr/bin
	cp $(TARGET) $(INSTALL_PREFIX)/usr/bin

clean:
	rm -rf $(OBJS) $(TARGET)

.PHONY: all clean install
