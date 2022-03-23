ifeq ($(strip $(XRADIO_AIRKISS_CONFIG)),y)
XRADIO_AIRKISS_LIB_CFLAGS=-lairkiss_aes -lcrypto
endif

ifeq ($(strip $(ADT_CONFIG)),y)
ADT_LIB_CFLAGS=-lasound -luci -lADT
ifeq ($(LIBC_CONFIG), glibc)
ADT_LIB_CFLAGS+=-lm
endif
endif


ifeq ($(strip $(XRADIO_SMARTCONFIG_CONFIG)),y)
XRADIO_SMARTCONFIG_LIB_CFLAGS= -ldecode
endif

ifeq ($(strip $(SOFT_AP_CONFIG)),y)
SOFT_AP_LIB_CFLAGS =
endif


DEMO_ROOT  = ./
DEMO_SRC   = $(wildcard $(DEMO_ROOT)/*.c)


SRC = \
	  $(DEMO_SRC)

INC_CFLAGS += -I$(CONFIG_PREFIX)/usr/include

LIB_CFLAGS += -L$(CONFIG_PREFIX)/usr/lib -lsm_mg -lwifimg \
			  $(XRADIO_AIRKISS_LIB_CFLAGS) $(ADT_LIB_CFLAGS) \
			  $(XRADIO_SMARTCONFIG_LIB_CFLAGS)


TARGET = smartlink_demo


$(TARGET):
	$(CC) $(LDFLAGS) $(INC_CFLAGS) $(LIB_CFLAGS) $(SRC) -o $@

install:
	mkdir -p $(CONFIG_PREFIX)/usr/bin
	@cp $(TARGET) $(CONFIG_PREFIX)/usr/bin


all: $(TARGET) install

clean:
	rm -rf $(TARGET) *.o *.a *~
