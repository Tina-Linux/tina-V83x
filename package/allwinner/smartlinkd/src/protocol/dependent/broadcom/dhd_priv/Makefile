#CC = $(CROSS_COMPILE)gcc
CC ?= g++

TARGET = dhd_priv

INCLUDES += -I. \

SRCS := \
	main.c \

SUB_LIB := \
	-lpthread \

$(TARGET):
	$(CC) $(LDFLAGS) $(INCLUDES) $(SRCS)  -o $@ $(SUB_LIB)


all:$(TARGET) install

clean:
	rm -rf $(TARGET) *.o *.a *~

install:
	mkdir -p $(CONFIG_PREFIX)/usr/bin
	cp $(TARGET) $(CONFIG_PREFIX)/usr/bin




#IFLAGS := -g -Wall

#DFLAGS += -DLIB

#DFLAGS += -DIL_BIGENDIAN

#obj-dhd_priv = dhd_priv.o

#DHD_PRIV := dhd_priv

#CFLAGS += $(IFLAGS) $(DFLAGS)

#all:  $(DHD_PRIV)

#$(DHD_PRIV): $(obj-dhd_priv)
#	$(CC) -o $@ $(obj-dhd_priv)

#clean:
#	-rm -f $(DHD_HPRIV) *.o
