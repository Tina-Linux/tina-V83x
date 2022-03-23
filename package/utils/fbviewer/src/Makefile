#
# Makefile
#
# Makefile for fbviwer
#
CFLAGS = -Wall -D_GNU_SOURCE
LOCAL_LDFLAGS = -ljpeg -lpng
SOURCES	= main.c jpeg.c png.c bmp.c fb_display.c transforms.c
OBJECTS	= ${SOURCES:.c=.o}

OUT	= fbviewer

all: $(OUT)
	@echo Build DONE.

$(OUT): $(OBJECTS)
	$(CC) $(LDFLAGS) $(LOCAL_LDFLAGS) -o $(OUT) $(OBJECTS) $(LIBS)

clean:
	rm -f $(OBJECTS)

distclean: clean
	rm -f $(OUT)
