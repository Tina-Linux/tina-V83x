BUILDPATH=.

CFLAGS=-Wall
CFLAGS+=-I./
CFLAGS+=-O0 -g
LOCAL_SRC = recorder.c

all:
	$(CC) $(CFLAGS) $(EXT_CFLAGS)  $(LOCAL_SRC) $(LDFLAGS) -lpthread -lspeex -lasound -o test_record
clean:
	rm -f $(LIB) $(LIBOBJ)
