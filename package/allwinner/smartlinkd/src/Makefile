target = libsm_mg.so

ifeq ($(strip $(XRADIO_AIRKISS_CONFIG)),y)
XRADIO_AIRKISS_ROOT      =./protocol/dependent/xradio/airkiss
XRADIO_AIRKISS_SRC       =$(wildcard $(XRADIO_AIRKISS_ROOT)/*.c) \
						  $(wildcard $(XRADIO_AIRKISS_ROOT)/radiotap/*.c)

XRADIO_AIRKISS_INC_PATH  =-I$(XRADIO_AIRKISS_ROOT) \
						  -I$(XRADIO_AIRKISS_ROOT)/include \
						  -I$(XRADIO_AIRKISS_ROOT)/radiotap
XRADIO_AIRKISS_LIB_PATH  =$(XRADIO_AIRKISS_ROOT)/lib/$(CPU)
XRADIO_AIRKISS_LIB_CFLAGS=-L$(XRADIO_AIRKISS_LIB_PATH) -lairkiss_aes -DXRADIO_AIRKISS
endif

ifeq ($(strip $(ADT_CONFIG)),y)
ADT_ROOT      =./protocol/independent/soundwave
ADT_SRC       =$(wildcard $(ADT_ROOT)/*.c)
ADT_INC_PATH  =-I$(ADT_ROOT)
ADT_LIB_PATH  =$(ADT_ROOT)/$(CONFIG_LIBC)/$(CPU)
ADT_LIB_CFLAGS=-L$(ADT_LIB_PATH) -lasound -luci -lADT -DADT
endif


ifeq ($(strip $(XRADIO_SMARTCONFIG_CONFIG)),y)
XRADIO_SMARTCONFIG_ROOT      =./protocol/dependent/xradio/smartconfig
XRADIO_SMARTCONFIG_SRC       =$(wildcard $(XRADIO_SMARTCONFIG_ROOT)/*.c)
XRADIO_SMARTCONFIG_INC_PATH  =-I$(XRADIO_SMARTCONFIG_ROOT)
XRADIO_SMARTCONFIG_LIB_PATH  =$(XRADIO_SMARTCONFIG_ROOT)/lib/$(CPU)
XRADIO_SMARTCONFIG_LIB_CFLAGS=-L$(XRADIO_SMARTCONFIG_LIB_PATH) -ldecode -DXRADIO_SMARTCONFIG
endif

ifeq ($(strip $(SOFT_AP_CONFIG)),y)
SOFT_AP_ROOT       =./protocol/independent/softAP
SOFT_AP_SRC        =$(wildcard $(SOFT_AP_ROOT)/*.c)
SOFT_AP_INC_PATH   =-I$(SOFT_AP_ROOT)
SOFT_AP_LIB_PATH   =
SOFT_AP_LIB_CFLAGS = -DSOFT_AP
endif

SMG_ROOT     = ./
SMG_SRC      =$(wildcard $(SMG_ROOT)/*.c)
SMG_INC_PATH =-I$(SMG_ROOT)
SMG_LIB_PATH =$(SMG_ROOT)

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp ./*.h $(CONFIG_PREFIX)/usr/include
ifeq ($(strip $(XRADIO_AIRKISS_CONFIG)),y)
	@cp $(XRADIO_AIRKISS_LIB_PATH)/* $(CONFIG_PREFIX)/usr/lib
endif
ifeq ($(strip $(ADT_CONFIG)),y)
	@cp $(ADT_LIB_PATH)/* $(CONFIG_PREFIX)/usr/lib
endif
ifeq ($(strip $(XRADIO_SMARTCONFIG_CONFIG)),y)
	@cp $(XRADIO_SMARTCONFIG_LIB_PATH)/* $(CONFIG_PREFIX)/usr/lib
endif


INC_CFLAGS += $(XRADIO_AIRKISS_INC_PATH) $(ADT_INC_PATH) \
			  $(XRADIO_SMARTCONFIG_INC_PATH) $(SMG_INC_PATH) \
			  $(SOFT_AP_INC_PATH)

LIB_CFLAGS += $(XRADIO_AIRKISS_LIB_CFLAGS) $(ADT_LIB_CFLAGS) \
			  $(XRADIO_SMARTCONFIG_LIB_CFLAGS) $(SOFT_AP_LIB_CFLAGS)


SRC= \
	$(SMG_SRC) \
	$(XRADIO_AIRKISS_SRC) \
	$(ADT_SRC) \
	$(XRADIO_SMARTCONFIG_SRC) \
	$(SOFT_AP_SRC)


$(target): $(SRC)
	$(CC) -fPIC -shared -D_GNU_SOURCE $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(INC_CFLAGS) $(LIB_CFLAGS) $^ -o $@
	@cp $@  $(CONFIG_PREFIX)/usr/lib

####################################################################
all: install  $(target)

clean:
	rm -rf $(target)
