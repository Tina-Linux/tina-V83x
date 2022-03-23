target = libgatt-server.so
install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/usr/bin
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	-@cp -rf ./include/* $(CONFIG_PREFIX)/usr/include
	-@cp ./*.a $(CONFIG_PREFIX)/usr/lib

INCLUDES +=-I./include -I$(STAGING_DIR)/usr/include/glib-2.0 \
	-I$(STAGING_DIR)/usr/include/gio-unix-2.0 -I$(CONFIG_PREFIX)/usr/include

LIBS = -L$(CONFIG_PREFIX)/usr/lib -lbluetooth-internal -lshared-mainloop


LIB_SRC = $(wildcard *.c)

$(target):$(LIB_SRC)
	$(CC) -fPIC -shared $(CFLAGS) $(INCLUDES) $(CPPFLAGS) $(LDFLAGS) $(LIBS) $^ -o $@
	@cp $@  $(CONFIG_PREFIX)/usr/lib
	@cp libbluetooth-internal.a  $(CONFIG_PREFIX)/usr/lib
	@cp libshared-mainloop.a  $(CONFIG_PREFIX)/usr/lib

all:install $(target)

clean:
	rm -f $(OBJS) libgatt-server.so
