target			= aec-xt-demo

LOCAL_CFLAGS:= -mfpu=neon-vfpv4
INCLUDES += -I$(CONFIG_PREFIX)/usr/include

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/bin
	@cp -f aec-xt-demo $(CONFIG_PREFIX)/usr/bin/


aec-xt-demo: main_demo.c
	$(CC) -o $@ $^ $(CFLAGS) $(LOCAL_CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lQoSound_aes_ncs


all: aec-xt-demo install

clean:
	rm -rf aec-xt-demo
