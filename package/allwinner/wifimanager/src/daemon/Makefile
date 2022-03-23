INCLUDES += -I$(CONFIG_PREFIX)/usr/include \
			-I ../core/include \
			-I .

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/bin
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp wifid_cmd.h $(CONFIG_PREFIX)/usr/include

#wifi_longtime_test
wifi_daemon: wifi_daemon.c wifid_ctrl.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg
	@cp -f wifi_daemon $(CONFIG_PREFIX)/usr/bin

libwifid.so: wifid_cmd_handle.c wifid_cmd_iface.c
	$(CC) -fPIC -shared $(INCLUDES) $(LDFLAGS) $^ -o $@ $(LIBS)
	@cp $@ $(CONFIG_PREFIX)/usr/lib

####################################################################
all: install wifi_daemon libwifid.so
clean:
	rm -rf  wifi_daemon wifi
