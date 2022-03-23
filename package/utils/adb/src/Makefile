TARGET          = adbd
INCLUDES        += -I. \
                   -I./libs/libcutils \
				   -I./libs/libmincrypt

SRCS := \
	adb.c \
	backup_service.c \
	fdevent.c \
	transport.c \
	transport_local.c \
	transport_usb.c \
	adb_auth_client.c \
	sockets.c \
	services.c \
	file_sync_service.c \
	jdwp_service.c \
	framebuffer_service.c \
	remount_service.c \
	usb_linux_client.c \
	log_service.c \
	utils.c

SUB_LIB := \
		libmincrypt.a \
		libcutils.a \
		-lpthread

OBJS    = $(SRCS:.c=.o)

LOCAL_CFLAGS := -O2 -g -DADB_HOST=0 -Wall -Wno-unused-parameter -DALLOW_ADBD_ROOT=1 -DHAVE_FORKEXEC
LOCAL_CFLAGS += -D_XOPEN_SOURCE -D_GNU_SOURCE

%.o: %.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) $(INCLUDES) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(SUB_LIB)  -o $@

#all:$(TARGET)

clean:
	rm -rf $(TARGET) *.o *.a *~
