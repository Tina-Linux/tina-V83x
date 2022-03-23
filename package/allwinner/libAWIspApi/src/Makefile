TARGET := libAWIspApi.so
LIBS :=

commonSources:= \
	AWIspApi.c

local_CFLAGS := \
	-I./libisp/include/V4l2Camera \
    -I./libisp/include/device \
    -I./libisp/include \
    -I./libisp/isp_dev \
    -I./libisp/isp_tuning \
    -I./libisp \
    -I./


OBJS = $(patsubst %.c, %.c.o, $(commonSources))

$(TARGET):$(OBJS) libisp.so libisp_ini.so
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $(local_CFLAGS) -o $@ $^ -fPIC -shared

libisp.so:
	+$(MAKE) -C ./libisp all CPU_VARIANT="$(CPU_VARIANT)" C_LIB_TYPE="$(C_LIB_TYPE)"

libisp_ini.so:
	+$(MAKE) -C ./libisp/isp_cfg all

$(OBJS):%.c.o:%.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) -c $< -o $@ -fPIC

all: $(TARGET) install

clean:
	-rm $(OBJS) $(TARGET)
install:
	mkdir -p $(INSTALL_PREFIX)/usr/lib
	install -m0755 $(TARGET) $(INSTALL_PREFIX)/usr/lib
	mkdir -p $(INSTALL_PREFIX)/usr/include
	cp -r ./*.h  $(INSTALL_PREFIX)/usr/include
	cp -r ./libisp/include/V4l2Camera/sunxi_camera_v2.h  $(INSTALL_PREFIX)/usr/include
