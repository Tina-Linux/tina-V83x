install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp ./*.h $(CONFIG_PREFIX)/usr/include
	@cp ./$(CONFIG_LIBC)/libbtmg.so $(CONFIG_PREFIX)/usr/lib
	@cp ./$(CONFIG_LIBC)/libbttool.so $(CONFIG_PREFIX)/usr/lib

all:install

clean:
	rm -rf *.h *.a
