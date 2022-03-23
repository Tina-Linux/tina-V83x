TARGET =libsec_key.so

INCLUDES += -I. -I./include

SRCS := api.c \
		crc.c \
		common.c \
		private_bare.c \
		secure_storage_ioctl.c \
		secure_storage.c

OBJS = $(patsubst %.c, %.o, $(SRCS))


%.o: %.c
	$(CC) -fPIC $(CFLAGS) $(LDFLAGS) $(INCLUDES) $(LIBS) -c -o $@ $<

$(TARGET):$(OBJS)
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LDFLAGS) $(INCLUDES) $(LIBS) -fPIC -shared

all: $(TARGET)

clean:
	rm -rf $(TARGET)

