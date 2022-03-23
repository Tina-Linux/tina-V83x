TARGET = audiotest_wav_play_time

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))

SRCS += $(wildcard ../local/*.c)
INCLUDES += -I../local
LIBS += -lasound

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
