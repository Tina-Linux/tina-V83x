target			= wifi_connect_ap_test

INCLUDES += -I$(CONFIG_PREFIX)/usr/include \
			-I ../src/core/include

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/bin

#wifi_on_off_test
wifi_on_off_test: wifi_on_off_test.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg
	@cp -f wifi_on_off_test $(CONFIG_PREFIX)/usr/bin

#wifi_connect_chinese_ap_test
wifi_connect_chinese_ap_test: wifi_connect_chinese_ap.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg
	@cp -f wifi_connect_chinese_ap_test $(CONFIG_PREFIX)/usr/bin

#wifi_scan_results_test
wifi_scan_results_test: wifi_scan_results.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg
	@cp -f wifi_scan_results_test $(CONFIG_PREFIX)/usr/bin

#wifi_list_networks_test
wifi_list_networks_test: wifi_list_networks.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg
	@cp -f wifi_list_networks_test $(CONFIG_PREFIX)/usr/bin

#wifi_remove_network_test
wifi_remove_network_test: wifi_remove_network.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg
	@cp -f wifi_remove_network_test $(CONFIG_PREFIX)/usr/bin

#wifi_connect_ap_with_netid_test
wifi_connect_ap_with_netid_test: wifi_connect_ap_with_netid.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg
	@cp -f wifi_connect_ap_with_netid_test $(CONFIG_PREFIX)/usr/bin

#wifi_get_netid_test
wifi_get_netid_test: wifi_get_netid.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg
	@cp -f wifi_get_netid_test $(CONFIG_PREFIX)/usr/bin

#wifi_longtime_test
wifi_longtime_test: wifi_longtime_test.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg
	@cp -f wifi_longtime_test $(CONFIG_PREFIX)/usr/bin

#wifi_get_connection_info_test
wifi_get_connection_info_test: wifi_get_connection_info_test.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg
	@cp -f wifi_get_connection_info_test $(CONFIG_PREFIX)/usr/bin

#wifi_remove_all_networks_test
wifi_remove_all_networks_test: wifi_remove_all_networks.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg
	@cp -f wifi_remove_all_networks_test $(CONFIG_PREFIX)/usr/bin

#wifi_wps_pbc_test
wifi_wps_pbc_test: wifi_wps_pbc.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg
	@cp -f wifi_wps_pbc_test $(CONFIG_PREFIX)/usr/bin

#wifi_longtime_scan_test
wifi_longtime_scan_test: wifi_longtime_scan.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg
	@cp -f wifi_longtime_scan_test $(CONFIG_PREFIX)/usr/bin

#wifi_disconnect_ap_test
wifi_disconnect_ap_test: wifi_disconnect_ap.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg
	@cp -f wifi_disconnect_ap_test $(CONFIG_PREFIX)/usr/bin

#wifi_reconnect_ap_test
wifi_reconnect_ap_test: wifi_reconnect_ap.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg
	@cp -f wifi_reconnect_ap_test $(CONFIG_PREFIX)/usr/bin

wifi_connect_ap_test: wifi_connect_ap.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg
	@cp -f wifi_connect_ap_test $(CONFIG_PREFIX)/usr/bin
####################################################################
all: install $(target) wifi_connect_ap_test wifi_on_off_test wifi_connect_chinese_ap_test wifi_scan_results_test wifi_list_networks_test wifi_remove_network_test wifi_connect_ap_with_netid_test wifi_get_netid_test wifi_longtime_test wifi_remove_all_networks_test wifi_wps_pbc_test wifi_longtime_scan_test wifi_disconnect_ap_test wifi_reconnect_ap_test wifi_get_connection_info_test
clean:
	rm -rf $(target) wifi_on_off_test wifi_connect_chinese_ap_test wifi_scan_results_test wifi_list_networks_test wifi_remove_network_test wifi_connect_ap_with_netid_test wifi_get_netid_test wifi_add_network_test wifi_longtime_test wifi_remove_all_networks_test $(LIB_OBJS) wifi_wps_pbc_test wifi_longtime_scan_test wifi_disconnect_ap_test wifi_reconnect_ap_test wifi_get_connection_info_test
