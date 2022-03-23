#compile r6-tina static lib
obj := wfd_log.o WFDMessageQueue.o   p2pManager.o wpa_cli.o

libwpa_cli: $(obj)
	$(AR) -r libwpa_client.a  $(obj)

wfd_log.o: wfd_log.c ./include/wfd_log.h
	$(CC) -c wfd_log.c  -I ./include  

WFDMessageQueue.o: WFDMessageQueue.c   ./include/WFDMessageQueue.h
	$(CC) -c WFDMessageQueue.c      -I ./include 

p2pManager.o: p2pManager.c  ./include/wpa_cli.h ./include/p2pManager.h ./include/WFDMessageQueue.h
	$(CC) -c p2pManager.c   -I ./include  

wpa_cli.o: wpa_cli.c ./include/wpa_cli.h
	$(CC) -c wpa_cli.c  -I ./include  -MMD -O2 -Wall -g \
    -DCONFIG_CTRL_IFACE \
    -DCONFIG_CTRL_IFACE_UNIX \
    -DCONFIG_P2P \
    -DCONFIG_WIFI_DISPLAY \
    -DCONFIG_WPS \
    -DCONFIG_WPS_NFC \
    -DCONFIG_AP \
    -DCONFIG_HS20 \
    -DCONFIG_AUTOSCAN \
    -DCONFIG_INTERWORKING \
    -DIEEE8021X_EAPOL

.PHONY: clean
clean:
	$(AR) -d  libwpa_client.a $(obj); rm $(obj); rm *.d
