BUILDPATH=.

CFLAGS=-Wall
CFLAGS+=-I./
CFLAGS+=-O0 -g
LOCAL_SRC = pcm_mixer.c tinyplay_earphone_plug.c

all:
	$(CC) $(CFLAGS) $(EXT_CFLAGS)  $(LOCAL_SRC) $(LDFLAGS)  -lpthread -o test_hp_plug
clean:
	rm -f $(LIB) $(LIBOBJ)
