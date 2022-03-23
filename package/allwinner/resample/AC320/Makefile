

ifneq ($(LIBPATH),wildcard($(LIBPATH)))
a := $(shell mkdir -p $(LIBPATH))
endif
ifneq ($(BUILDPATH),wildcard($(BUILDPATH)))
a := $(shell mkdir -p $(BUILDPATH))
endif

CFLAGS+=-Wall
CFLAGS+= -I./ -I$(LIBBASE)/
CFLAGS+=-O1 -D__OS_LINUX  -DARM_ADS

ifeq ($(STATIC_AUDIOMIX),1)
LIB=$(LIBPATH)/libresamplerate.a
CFLAGS+=-fPIC
else
    ifeq ($(CONFIG_ARCH), "aarch64")
        LIB=$(LIBPATH)/libresamplerate_64.so
    else
        LIB=$(LIBPATH)/libresamplerate.so
    endif
CFLAGS+=-fPIC
endif

## define commands for make, sush as all, clean
.PHONY: all clean cleanall
all:
	cp -f ./*.so $(LIBPATH)/

clean:
	rm -rf $(LIBPATH)/

cleanall: clean
	-rm -rf $(BUILDPATH)
