INCLUDES += -I./include -I$(CONFIG_PREFIX)/usr/include
LIB_PATH := $(CONFIG_PREFIX)/usr/lib
LIBS += -L$(LIB_PATH) -L../src/base_bluez -lbluetooth -lglib-2.0 -lsbc -lgio-2.0 -lgobject-2.0 -lasound -lbtmg -lbttool
#TARGET_PATH := $(CONFIG_PREFIX)/usr/bin

define DO_COMPILE
$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES) $(LDFLAGS) -lbtmg -lpthread
endef

target	= aw_bt_test
C_SRCS = aw_bt_test.c aw_bt_test_dev.c
C_OBJS := $(C_SRCS:.c=.o)
LIB_OBJS = $(C_OBJS)


#aw_bt_test
$(C_OBJS): %.o: %.c
	$(CXX) $(CFLAGS) $(INCLUDES) -g -c -o $@ $^
$(target): $(LIB_OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES) -g $(LIBS)  $(LDFLAGS)

GATT_LIBS = -L$(CONFIG_PREFIX)/usr/lib -lgatt-server -lbluetooth-internal -lshared-mainloop

aw_gatt_server_test: aw_gatt_server_test.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(GATT_LIBS) $(LIBS)  $(LDFLAGS)
	@cp aw_gatt_server_test  $(CONFIG_PREFIX)/usr/bin/

bt_test: bt_test.c aw_bt_test_dev.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(GATT_LIBS) $(LIBS)  $(LDFLAGS)
	@cp bt_test  $(CONFIG_PREFIX)/usr/bin/

####################################################################
all: $(target) aw_gatt_server_test bt_test
	-@mkdir -p $(CONFIG_PREFIX)/usr/bin
	@cp $(target) $(CONFIG_PREFIX)/usr/bin/
clean:
	rm -rf $(target) $(LIB_OBJS)
