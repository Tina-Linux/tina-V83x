
DHRYSTONE_VERSION=v2.1
DHRYSTONE_DIR=dhrystone_$(DHRYSTONE_VERSION)
ifeq ($(BENCHMARKS_DHRYSTONE), y)
target += dhrystone
endif

LINPACK_VERSION=v1.0
LINPACK_DIR=linpack_$(LINPACK_VERSION)
ifeq ($(BENCHMARKS_LINPACK), y)
target += linpack
endif

COREMARK_VERSION=v1.0
COREMARK_DIR=coremark_$(COREMARK_VERSION)
PORT_DIR=linux
ifeq ($(ARCH),aarch64)
PORT_DIR=linux64
endif
ifeq ($(BENCHMARKS_COREMARK), y)
target += coremark
endif

ifeq ($(BENCHMARKS_STATIC_COMPILE), y)
LOCAL_CFLAGS=-static -lc -lgcc -lgcc_eh -lm
endif

all:install

install:$(target)
	mkdir -p $(CONFIG_PREFIX)/usr/bin
	cp $(target) $(CONFIG_PREFIX)/usr/bin

dhrystone: ./$(DHRYSTONE_DIR) Makefile
	$(CC) $(DHRYSTONE_DIR)/dhry_1.c $(DHRYSTONE_DIR)/dhry_2.c -o dhrystone $(CFLAGS) $(LOCAL_CFLAGS) -DTIME

linpack: ./$(LINPACK_DIR) Makefile
	$(CC) $(LINPACK_DIR)/linpack.c -o linpack $(CFLAGS) $(LOCAL_CFLAGS) -funsafe-math-optimizations -fno-fast-math

coremark: ./$(COREMARK_DIR) Makefile
	make link -C $(COREMARK_DIR) OPATH=../ PORT_DIR=$(PORT_DIR) XCFLAGS="-DMULTITHREAD=4 -DUSE_PTHREAD $(LOCAL_CFLAGS) -lpthread"

clean:
	rm -f dhrystone linpack
	make clean -C $(COREMARK_DIR)


