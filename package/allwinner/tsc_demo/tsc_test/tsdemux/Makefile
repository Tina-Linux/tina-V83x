ROOT		= .
TARGET_DIR  = ../lib

TARGET      = $(TARGET_DIR)/libtsdemux.so

SRCCS     = tsdemux.c \

LOCAL_FLAGS     := -fPIC -Wall -O2 -DOS_LINUX
INCLUDES		:= -I./ -I./../include -I./../include/base/ -I./../ -I./../include/demux/ -I./../tsc/

LIBS += -ltsc -L../lib

all:$(SRCCS)
	$(CC) -shared $(CFLAGS) $(LOCAL_FLAGS) $(INCLUDES) $(LIBS) -o $(TARGET) $^

clean:
	-rm $(TARGET)
