CC ?= gcc
AR ?= ar
RM = rm -f

TARGET = rtw_ak

LOCAL_CFLAGS = -g -Wall #-ansi
LOCAL_CFLAGS += -DCONFIG_IOCTL_CFG80211 	#Using iw command, not iwconfig command.
#LOCAL_CFLAGS += -DCONFIG_NOFCS		#for 8812  family

LOCAL_CFLAGS += -I../../include -Ibase

src_list := main.c base/connect.c
obj_list := $(src_list:%.c=%.o)

%.o:%.c
	$(CC) -c $(CFLAGS) $(LOCAL_CFLAGS) -o $@ $^

$(TARGET): $(obj_list)
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -o $(TARGET) $(obj_list) libairkiss.a -lpthread


.PHONY : $(TARGET)

all: $(TARGET)
clean:
	$(RM) $(TARGET) *.o base/connect.o
