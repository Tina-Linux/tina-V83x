TARGET=MtpDaemon
MTP_UPDATE_TOOLS=MtpTools

SRCS = main.c \
	MtpServer.c \
	MtpPacket.c \
	MtpDataBase.c \
	MtpProperty.c \
	MtpStorage.c \
	MtpObjectInfo.c \
	MtpCommon.c \

SRCS += Disk/Disk.c \
	Disk/Dir.c \
	Disk/File.c \
	Disk/DiskDebug.c \

LOCAL_CFLAGS += -I./ -I./Disk

LOCAL_CFLAGS +=-DMTP_SERVER_DAEMON

LOCAL_CFLAGS += -DFORCE_DEBUG

LOCAL_CFLAGS += -DNO_DEBUG=1

#LOCAL_CFLAGS += -DMEM_DEBUG

LIBS = -lpthread

OBJS := $(SRCS:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -o $@ -c $<

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
$(MTP_UPDATE_TOOLS): mtp_update_tools.o
	$(CC) $(CFLAGS) -o $(MTP_UPDATE_TOOLS) mtp_update_tools.o

all:$(TARGET) Makefile $(MTP_UPDATE_TOOLS) install

install:
	mkdir -p $(CONFIG_PREFIX)/usr/sbin
	mkdir -p $(CONFIG_PREFIX)/usr/bin
	cp $(TARGET) $(CONFIG_PREFIX)/usr/sbin/
	cp $(MTP_UPDATE_TOOLS) $(CONFIG_PREFIX)/usr/bin/
dump:
	$(OBJDUMP) -D $(TARGET) > dump

clean:
	rm -rf $(TARGET) $(MTP_UPDATE_TOOLS) $(OBJS) mtp_update_tools.o
