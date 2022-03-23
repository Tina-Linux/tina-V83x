TARGET = libtutu_tool.so

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))

INCLUDES += -I../include/$(TARGET_CPU_VARIANT)
LIBS +=
override LDFLAGS += -Wl,--no-undefined

all: $(TARGET) install

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -fPIC -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $^ $(LDFLAGS) $(LIBS) -shared -o $@

install:
	mkdir -p $(INSTALL_PREFIX)/usr/lib
	cp $(TARGET) $(INSTALL_PREFIX)/usr/lib

clean:
	rm -rf $(OBJS) $(TARGET)

.PHONY: all clean install
