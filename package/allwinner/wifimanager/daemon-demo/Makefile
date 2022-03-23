INCLUDES += -I$(CONFIG_PREFIX)/usr/include \
			-I ../src/include

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/bin


wifid: wifid_cmd.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifid -lwifimg
	@cp -f wifid $(CONFIG_PREFIX)/usr/bin

####################################################################
all: install wifid
clean:
	rm -rf  wifid
