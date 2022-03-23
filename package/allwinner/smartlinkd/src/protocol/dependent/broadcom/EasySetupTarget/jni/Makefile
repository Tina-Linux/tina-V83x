CC ?= gcc

src_list := main_linux.c easy_setup_linux.c easy_setup/easy_setup.c easy_setup/scan.c proto/bcast.c proto/neeze.c proto/akiss.c proto/changhong.c
obj_list := $(src_list:%.c=%.o)

myCFLAGS := -Ieasy_setup -Iproto
myCFLAGS += -I../../../../../daemon/include -I../../../../../daemon/libclient/linux_client -I../../../../../daemon/include/platform/Tina -lpthread -L../../../../../daemon/libclient/linux_client -lsmartlinkd_client

.PHONY: setup

setup:$(obj_list)
	$(CC) $(myCFLAGS) -o $@ $^

%.o:%.c
	$(CC) -c $(CFLAGS) $(myCFLAGS) -o $@ $^
all:setup
	cp -r setup $(CONFIG_PREFIX)/usr/bin/smartlinkd_setup
clean:
	rm $(obj_list) setup
