TARGET1 := write_misc
TARGET2 := read_misc

all: $(TARGET1) $(TARGET2) install

$(TARGET1) : write_misc.c misc_message.h misc_message.c
	$(CC) $^ -o $@ -I.

$(TARGET2) : read_misc.c misc_message.h misc_message.c
	$(CC) $^ -o $@ -I.


install:
	mkdir -p $(CONFIG_PREFIX)/sbin
	cp $(TARGET1) $(CONFIG_PREFIX)/sbin
	cp $(TARGET2) $(CONFIG_PREFIX)/sbin
clean:
	rm -rf $(TARGET1) $(TARGET2)
