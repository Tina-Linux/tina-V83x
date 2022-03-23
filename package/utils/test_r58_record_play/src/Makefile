BUILDPATH=.

CFLAGS=-Wall
CFLAGS+=-I./
CFLAGS+=-O0 -g
LOCAL_SRC = sine_detector.c pcm_mixer.c test_record_play.c

all:
	$(CC) $(CFLAGS) $(EXT_CFLAGS)  $(LOCAL_SRC) $(LDFLAGS) -lpthread -o test_record_play
clean:
	rm -f $(LIB) $(LIBOBJ)
