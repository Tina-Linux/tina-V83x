TARGET := \
	interact-core.o \
	interact-actor.o \
	interact-capi.o

SCRIPTS := $(basename $(shell ls scripts))

.PHONY: all install

all: $(TARGET) install

$(TARGET): %.o:%.c
	@$(CC) $(CFLAGS) $^ -o $@

install: $(TARGET)
	@$(CP) interact.h $(PREFIX)/include

$(SCRIPTS): %:scripts/%.c
	@$(CC) $(CFLAGS) $(LDFLAGS) ./scripts/$(@:%=%.c) -o $(PREFIX)/bin/$@
