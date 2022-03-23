TARGET := sysinfo.o

SCRIPTS := \
	get_boot_media \
	get_kernel_version \
	get_target \
	sysinfo

.PHONY: all install

all: $(TARGET) install

$(TARGET): $(TARGET:.o=.c)
	@$(CC) $(CFLAGS) $^ -o $@

install:
	@$(CP) *.h $(PREFIX)/include

$(SCRIPTS):
	@$(CP) ./scripts/$@ $(PREFIX)/bin
