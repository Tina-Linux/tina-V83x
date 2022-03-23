TARGET = asan_demo_heap_use_after_free \
		asan_demo_heap_buffer_overflow \
		asan_demo_stack_buffer_overflow \
		asan_demo_global_buffer_overflow

all: $(TARGET) install

asan_demo_%: %.c
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $<

install:
	mkdir -p $(INSTALL_PREFIX)/usr/bin
	cp $(TARGET) $(INSTALL_PREFIX)/usr/bin

clean:
	rm -rf $(OBJS) $(TARGET)

.PHONY: all clean install
