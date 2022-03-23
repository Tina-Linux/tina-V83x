library := libota-burnboot.so
lib_src := \
	BurnNandBoot.c \
	BurnSpinor.c \
	BurnSdBoot.c \
	OTA_BurnBoot.c \
	Utils.c

target := ota-burnboot0 ota-burnuboot

.PHONY: all

all: $(library) $(target)

%: %.c
	$(CC) -L. -lota-burnboot $(CFLAGS) $(LDFLAGS) $^ -o $@

$(target): ${library}

$(library): $(lib_src)
	$(CC) -fPIC -shared $(LDFLAGS) $^ -o $(library)

clean:
	rm ${library} $(target)
