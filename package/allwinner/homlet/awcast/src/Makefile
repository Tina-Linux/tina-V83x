
all: awcast

awcast: awcast.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib 
	#@cp -f wifid $(CONFIG_PREFIX)/usr/bin

####################################################################

clean:
	rm -rf  awcast

.PHONY: clean