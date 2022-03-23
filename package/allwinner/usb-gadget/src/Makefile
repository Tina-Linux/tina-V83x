TARGET1 = libusb-gadget.so
TARGET2 = usb-gadget-test

LIBS :=
COMPILE_FLAGS := $(LIBS)

$(TARGET1): usb-gadget.c
	$(CC) -shared -fpic -o $@ $^ $(COMPILE_FLAGS)

$(TARGET2): usb-gadget.c usb-gadget-test.c
	$(CC) -o $@ $^ $(COMPILE_FLAGS)

all:$(TARGET1) $(TARGET2) install

install:
	mkdir -p $(CONFIG_PREFIX)/usr/lib
	cp $(TARGET1) $(CONFIG_PREFIX)/usr/lib/

clean:
	rm -rf $(TARGET1) $(TARGET2) $(OBJS)
