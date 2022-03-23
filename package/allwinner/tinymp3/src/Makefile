DIR_CUR := $(shell pwd)

# $1: string
# $2: start pos
# $3: length
define substring
$(shell echo `expr substr $(1) $(2) $(3)`)
endef

BT ?= 0
VR ?= 0
LAPSULE ?= 0
DMR ?= 0
DMS ?= 0
AIRPLAY ?= 0
LOCALPLAYER ?= 0

#AR = $(AR)
#CC = $(CC)
#CXX = $(CXX)

INSTALL = install
DESTDIR =
PREFIX = /usr
INITDIR = /etc/init.d/
SBINDIR = $(PREFIX)/sbin

MOLIB_DIR=../../output/molib/app
MOLIB_UPDATER_DIR=../../output/molib/updater
MOZART_DIR=../../output/mozart/app

# common opts.
#CFLAGS = -fPIC -Wall -g -O2 -I. -I$(MOLIB_DIR)/usr/include -I$(MOLIB_UPDATER_DIR)/usr/include -I$(MOZART_DIR)/usr/include
LOCAL_CFLAGS = -fPIC -Wall -I. -I./include -g -I$(MOLIB_DIR)/usr/include -I$(MOLIB_UPDATER_DIR)/usr/include -I$(MOZART_DIR)/usr/include
#LDFLAGS = -g -lmad -lstdc++ -L. -L$(MOLIB_DIR)/usr/lib -L$(MOLIB_UPDATER_DIR)/usr/lib -L$(MOZART_DIR)/usr/lib
LOCAL_LDFLAGS = -g -lasound -lmad -lpthread -L. -L$(MOLIB_DIR)/usr/lib -L$(MOLIB_UPDATER_DIR)/usr/lib -L$(MOZART_DIR)/usr/lib
#LDFLAGS += -lutils -lini

##set link flag path
ifeq ($(BOARD_PLATFORM),r18)
LocalAudioGain += ./libplayer/MUSLGNUEABI64/libAudioGain.a
else ifeq ($(BOARD_PLATFORM),r6)
ifeq ($(C_LIB_TYPE),musl)
LocalAudioGain += ./libplayer/MUSLGNUEABI_ARM9/libAudioGain.a
else
LocalAudioGain += ./libplayer/GLIBCGNUEABI_ARM9/libAudioGain.a
endif
else
LocalAudioGain += ./libplayer/MUSLGNUEABI/libAudioGain.a
endif

SRCS += ${wildcard ./libplayer/*.c ./libplayer/libaudio/*.c}
OBJS = $(patsubst %.c, %.c.o, $(SRCS))
TARGET = libtinymp3.so

all:$(TARGET) tinyplayer install

tinyplayer:$(TARGET)
	$(CC) -o tinyplayer tinyplayer.c -g -Wall -ltinymp3 -I. -I./libplayer -L. $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS)

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -fPIC -shared $(LocalAudioGain)

$(OBJS):%.c.o:%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $^ -o $@ -fPIC -shared

clean:
	-rm -rf $(OBJS) $(TARGET) tinyplayer

install:
	-mkdir -p $(CONFIG_PREFIX)/bin
	-mkdir -p $(CONFIG_PREFIX)/lib
	-mkdir -p $(CONFIG_PREFIX)/include
	-cp $(TARGET) $(CONFIG_PREFIX)/lib/
	-cp tinyplayer $(CONFIG_PREFIX)/bin/
	-cp libplayer/mp3player.h $(CONFIG_PREFIX)/include/

.PHONY:all clean $(TARGET) install
