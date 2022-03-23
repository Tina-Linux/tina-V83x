TARGET := libcutils.so

commonSources := \
	hashmap.c \
	native_handle.c \
	config_utils.c \
	cpu_info.c \
	load_file.c \
	open_memstream.c \
	strdup16to8.c \
	strdup8to16.c \
	record_stream.c \
	process_name.c \
	threads.c \
	sched_policy.c \
	iosched_policy.c \
	str_parms.c

commonSources += \
	fs.c \
	multiuser.c \
	socket_inaddr_any_server.c \
	socket_local_client.c \
	socket_local_server.c \
	socket_loopback_client.c \
	socket_loopback_server.c \
	socket_network_client.c \
	sockets.c \
	android_reboot.c \
	debugger.c \
	klog.c \
	memory.c \
	misc_rw.c \
	partition_utils.c \
	properties.c \
	qtaguid.c \
	uevent.c

local_CFLAGS := -I./include -DANDROID_SMP=1 -DHAVE_PTHREADS \
	-DHAVE_SYS_SOCKET_H=1 -D_GNU_SOURCE -Werror -Wno-error=unused-result\
	-fkeep-inline-functions

OBJS = $(patsubst %.c, %.c.o, $(commonSources))

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) -o $@ $^ -fPIC -shared

$(OBJS):%.c.o:%.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) -c $^ -o $@ -fPIC -shared

all: $(TARGET) install

clean:
	-rm $(OBJS) $(TARGET)
install:
	mkdir -p $(INSTALL_PREFIX)/usr/lib
	cp $(TARGET) $(INSTALL_PREFIX)/usr/lib

	mkdir -p $(INSTALL_PREFIX)/usr/include
	cp -r include/* $(INSTALL_PREFIX)/usr/include
