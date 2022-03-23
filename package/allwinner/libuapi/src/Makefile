TARGET := libuapi.so
LIBS :=

commonSources:= \
	ion_alloc.c \
	yuv_rotate.c \
	videoOutPort.c \
	gpu_yuv2rgb.c

local_CFLAGS := -I./include

OBJS = $(patsubst %.c, %.c.o, $(commonSources))

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $(local_CFLAGS) -o $@ $^ -fPIC -shared

$(OBJS):%.c.o:%.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) -c $< -o $@ -fPIC -shared

all: $(TARGET) install

clean:
	-rm $(OBJS) $(TARGET)
install:
	mkdir -p $(INSTALL_PREFIX)/usr/lib
	cp $(TARGET) $(INSTALL_PREFIX)/usr/lib
	mkdir -p $(INSTALL_PREFIX)/usr/include
	cp -r include/*  $(INSTALL_PREFIX)/usr/include
