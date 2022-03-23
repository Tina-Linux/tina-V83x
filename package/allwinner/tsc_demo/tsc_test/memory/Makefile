ROOT		= .
TARGET_DIR  = ../lib

TARGET      = $(TARGET_DIR)/libMemAdapter.so

SRCCS     = ion_alloc.c \
			memoryAdapter.c

LOCAL_FLAGS     := -fPIC -Wall -O2 -D__OS_LINUX

INCLUDES		:= -I./

all:$(SRCCS)
	$(CC) -shared $(CFLAGS) $(LOCAL_FLAGS) $(INCLUDES) -o $(TARGET) $^

clean:
	-rm $(TARGET)
