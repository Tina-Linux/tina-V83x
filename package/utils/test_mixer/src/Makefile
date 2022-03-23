BUILDPATH=.

CFLAGS=-Wall
CFLAGS+=-I./
CFLAGS+=-O0 -g
LOCAL_SRC = pcm_mixer.c tinymix.c

all:
	$(CC) $(CFLAGS) $(EXT_CFLAGS)  $(LOCAL_SRC) $(LDFLAGS)  -lpthread -o tinymix
clean:
	rm -f $(LIB) $(LIBOBJ)
