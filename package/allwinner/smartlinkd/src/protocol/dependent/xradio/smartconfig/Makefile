# xr_sc.o	\
# detect.o	\
# reorder.o \
#################### configure area begin ###########
PLATFORM	=ARM
AES_VER		=y
REALEASE_VER	=n
TARGET		=smartlinkd_xrsc
#################### configure area  end ############


#AR=ar

LDFLAGS=
LIBS = -lpthread -lcrypto  -ldl \
	-lpthread \
	-L../../../../daemon/libclient/linux_client \
	-lsmartlinkd_client \
	-L./lib/$(CPU) -ldecode

SCFLAGS += -Wall -Winline

#TODO xr_sc.o should be made into a shared lib
OBJS = main.o	\
      scan.o	\
      control.o	\
	  wifi_cntrl.o \
	  time_out.o

ifeq ("$(PLATFORM)", "ARM")
INCLUDES += -I. \
			-I../../../../daemon/include \
			-I../../../../daemon/libclient/linux_client
SCFLAGS += -O2
else
SCFLAGS += -O0 -g
endif

ifeq ("$(AES_VER)", "y")
SCFLAGS += -DUSE_AES_
endif

#ifneq ("$(RELEASE_VER)", "y")
#SCFLAGS += -DUSE_CTRL_
#endif

all:$(TARGET)

%.o: %.c
	$(CC) $(SCFLAGS) $(INCLUDES) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $(TARGET)
	cp smartlinkd_xrsc $(CONFIG_PREFIX)/usr/bin/
check:
	file $(TARGET)
	ls -l $(TARGET)

clean:
	rm -f *.o $(TARGET)
