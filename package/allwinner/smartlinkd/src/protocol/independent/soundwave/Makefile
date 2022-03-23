TARGET = smartlinkd_soundwave
INCLUDES += -I. \
			-I../../../daemon/include \
			-I../../../daemon/libclient/linux_client\
			-I../../../daemon/include/platform/Tina \

SRCS := \
	sound.c \

SUB_LIB := \
	-lpthread \
	-L../../../daemon/libclient/linux_client \
	-lsmartlinkd_client \
	-lasound \
	-lwifimg \
	-luci

ADTLIB=$(CONFIG_LIBC)/$(CPU)/libADT.a
ifeq ($(CONFIG_LIBC),glibc)
	SUB_LIB +=-lm
endif

$(TARGET):$(ADTLIB)
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) $(SRCS)  -o $@ $^ $(SUB_LIB)

all:$(TARGET)
	cp smartlinkd_soundwave $(CONFIG_PREFIX)/usr/bin/
clean:
	rm -rf $(TARGET)
