BUILDPATH=.

CFLAGS=-Wall
CFLAGS+=-I./
CFLAGS+=-O0 -g
LOCAL_SRC = gpio_ir_test.c

all:
	$(CC) $(CFLAGS) $(EXT_CFLAGS)  $(LOCAL_SRC) $(LDFLAGS)  -lpthread -o gpio_ir_test

clean:
	rm -f $(LIB) $(LIBOBJ)
