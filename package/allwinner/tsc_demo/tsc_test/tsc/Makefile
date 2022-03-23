ROOT		= .
TARGET_DIR  = ../lib

TARGET      = $(TARGET_DIR)/libtsc.so

SRCCS     = tsc_dev.c \
			tsc_hal.c

LOCAL_FLAGS     := -fPIC -Wall -O2 -DOS_LINUX
INCLUDES		:= -I./ -I./../include -I./../include/base/ -I./../ -I./../include/demux/

all:$(SRCCS)
	$(CC) -shared $(CFLAGS) $(LOCAL_FLAGS) $(INCLUDES) -o $(TARGET) $^

clean:
	-rm $(TARGET)
