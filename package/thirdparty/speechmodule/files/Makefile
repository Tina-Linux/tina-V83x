TARGET=bin/msmart_net

#WITH_FACTORY_TEST=y #delete by 926 @20190618 for debug
WITH_FACTORY_TEST=n

DEFS = -DOSTYPE_LINUX  -DMS_WIFI_ENABLE
ifeq ($(WITH_FACTORY_TEST),y)
DEFS += -DWITH_FACTORY_TEST
endif
#CFLAGS += $(DEFS)
MIDEA_CFLAGS = $(DEFS)
MIDEA_CFLAGS += $(LDFLAGS)

INSTALL = install
MAKE = make
INSTALL_BIN:=install -m0755
INSTALL_DIR:=install -d -m0755
INSTALL_DATA:=install -m0644

# module name
MSMART_LIB		= msmart
INCDIR                  = -I./msm_inc -I./msm_hal/inc

# bin targets
BINSRCS_NET		= ${wildcard user/net/*.c} ${wildcard msm_hal/src/*.c}
BIN_NET			= bin/msmart_net
BINOBJS_NET		= $(BINSRCS_NET:%.c=%.o)

BINSRCS_BT_CONFIG	= ${wildcard user/bt_config/*.c} ${wildcard msm_hal/src/*.c}
BIN_BT_CONFIG		= bin/msmart_bt_config
BINOBJS_BT_CONFIG	= $(BINSRCS_BT_CONFIG:%.c=%.o)

BINSRCS_LOCAL_NET	= ${wildcard user/localnet/*.c} ${wildcard msm_hal/src/*.c}
BIN_LOCAL_NET		= bin/msmart_local_net
BINOBJS_LOCAL_NET	= $(BINSRCS_LOCAL_NET:%.c=%.o)

BINSRCS_UART		= ${wildcard user/uart/*.c} ${wildcard msm_hal/src/*.c}
BIN_UART		= bin/msmart_uart
BINOBJS_UART		= $(BINSRCS_UART:%.c=%.o)

BINSRCS_DEAMON		= ${wildcard user/deamon/*.c} ${wildcard msm_hal/src/*.c}
BIN_DEAMON		= bin/deamon
BINOBJS_DEAMON		= $(BINSRCS_DEAMON:%.c=%.o)


BIN_MSPEECH		= bin/mspeech
INCDIR_MSPEECH          = -I./user/mspeech/include \
                          -I./user/mspeech/include/lua \
                          -I./user/mspeech/third/json \
                          -I./user/mspeech/third/kfifo \
                          -I./user/mspeech/third/kmsg \
                          -I./user/mspeech/third/recorder_x86-64 \
                          -I./user/mspeech/third/mongoose  \
                          -I./demo

BINSRCS_MSPEECH         = ${wildcard ./user/mspeech/demo/*.c} \
                          ${wildcard user/mspeech/third/json/*.c} \
                          ${wildcard user/mspeech/third/kfifo/*.c} \
                          ${wildcard user/mspeech/third/kmsg/*.c} \
                          ${wildcard user/mspeech/third/recorder_x86-64/*.c} \
                          ${wildcard user/mspeech/third/mongoose/*.c} \
			  ${wildcard msm_hal/src/*.c}
BINOBJS_MSPEECH         = $(BINSRCS_MSPEECH:%.c=%.o)



# flags
MIDEA_CFLAGS += -fPIC  -Wall  -DLINUX -lpthread -I./user/localnet/
LDFLAGS_MSMART +=  -fPIC -L./lib -l$(MSMART_LIB)
MSPEECH_CFLAGS	:= -rdynamic -g -Wall -DMG_ENABLE_THREADS  -Wno-unused-variable -fPIC

export  DUILIBS= -lauth_r328 -lduilite-r328-0.11.0-39-20190506153403-release
MSPEECH_CFLAGS += -DAISPEECH_2MIC
MSPEECH_CFLAGS += -DLOCAL_ASR -DLOCAL_ASR_AC
MSPEECH_LDFLAGS += $(DUILIBS) -lplayer_r328 -lmspeech -lcurl -lpthread -luuid -lrt -lasound -llua -lm -lssl -lcrypto -Wl,-rpath=$(STAGING_DIR)/rootfs/mnt/app/libs/:-L$(shell pwd)/user/mspeech/libs/


all: | makedir $(BIN_NET) $(BIN_LOCAL_NET) $(BIN_BT_CONFIG) $(BIN_UART) $(BIN_DEAMON) $(BIN_MSPEECH) $(INSTALL)

# mspeech
$(BIN_MSPEECH):$(BINOBJS_MSPEECH)
	$(CC) $(MIDEA_CFLAGS) $(MSPEECH_CFLAGS) -o $@ $^ $(MSPEECH_LDFLAGS)

# deamon
$(BIN_DEAMON):$(BINOBJS_DEAMON)
	$(CC) $(MIDEA_CFLAGS) -o $@ $^

# net
$(BIN_NET):$(BINOBJS_NET)
	$(CC) $(MIDEA_CFLAGS) -o $@ $^ $(LDFLAGS_MSMART)

# localnet
$(BIN_LOCAL_NET):$(BINOBJS_LOCAL_NET)
	$(CC) $(MIDEA_CFLAGS) -o $@ $^ $(LDFLAGS_MSMART)

# bt_config
$(BIN_BT_CONFIG):$(BINOBJS_BT_CONFIG)
	$(CC) $(MIDEA_CFLAGS) -o $@ $^ $(LDFLAGS_MSMART)

# uart
$(BIN_UART):$(BINOBJS_UART)
	$(CC) $(MIDEA_CFLAGS) -o $@ $^

#$(OBJS):%.c.o:%.c
#        $(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@

%.o:%.c
	$(CC) $(MIDEA_CFLAGS) $(MSPEECH_CFLAGS) -c -o $@ $< $(INCDIR) $(INCDIR_MSPEECH)

makedir:
	-mkdir -p bin lib ./oem ./oem/AC ./oem/libs ./oem/lua ./oem/res ./oem/tts ./oem/factorytest ./oem/cfg

DESTDIR = $(shell pwd)/oem/

clean:
	-rm -f  $(BINOBJS_NET) $(BIN_NET) \
		$(BINOBJS_LOCAL_NET) $(BIN_LOCAL_NET)  \
		$(BINOBJS_UART) $(BIN_UART) $(BINOBJS_DEAMON) \
		$(BIN_DEAMON) $(BINOBJS_MSPEECH) $(BIN_MSPEECH)\
		$(BINOBJS_BT_CONFIG) $(BIN_BT_CONFIG)	\
		$(DESTDIR)
	@make -C user/factorytest clean

install:
	$(INSTALL_BIN)  $(BIN_NET)           ${DESTDIR}msmart_net
	$(INSTALL_BIN)  $(BIN_LOCAL_NET)     ${DESTDIR}msmart_local_net
	$(INSTALL_BIN)  $(BIN_BT_CONFIG)     ${DESTDIR}msmart_bt_config
	$(INSTALL_BIN)  $(BIN_UART)          ${DESTDIR}msmart_uart
	$(INSTALL_BIN)  $(BIN_DEAMON)        ${DESTDIR}deamon
	$(INSTALL_BIN)  $(BIN_MSPEECH)       ${DESTDIR}mspeech
	#$(INSTALL_DATA) user/mspeech/libs/*  ${DESTDIR}libs
	$(INSTALL_DATA) user/mspeech/tts/*   ${DESTDIR}tts
	$(INSTALL_DATA) user/mspeech/lua/*   ${DESTDIR}lua
	$(INSTALL_DATA) user/mspeech/res/*   ${DESTDIR}res
	$(INSTALL_DATA) Lunch.sh             ${DESTDIR}cfg
	$(INSTALL_DATA) user/mspeech/AC/*    ${DESTDIR}AC
ifeq ($(WITH_FACTORY_TEST),y)
#	cp -rf user/factorytest/*           ${DESTDIR}factorytest
	@make -C user/factorytest CATEGORY=AC
	@make -C user/factorytest install FT_INSTALL_DIR=${DESTDIR}factorytest CATEGORY=AC
#	touch ${DESTDIR}/.initial
endif
