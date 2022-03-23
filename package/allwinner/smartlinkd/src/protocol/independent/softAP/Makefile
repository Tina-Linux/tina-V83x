TARGET = smartlinkd_softap
INCLUDES += -I. \
			-I../../../daemon/include \
			-I./include \

SRCS := \
	softAP_demo.c \
	wifi_connect_ap.cpp \

SUB_LIB := \
	-L../../../daemon/libclient/linux_client \
	-lsmartlinkd_client \
	-lpthread \
	-lwifimg
$(TARGET):
	$(CC) $(LDFLAGS) $(INCLUDES) $(SRCS) -o $@ $(SUB_LIB)

all:$(TARGET) install

clean:
	rm -rf $(TARGET) *.o *.a *~

install:
	mkdir -p $(CONFIG_PREFIX)/use/bin
	cp $(TARGET) $(CONFIG_PREFIX)/usr/bin
