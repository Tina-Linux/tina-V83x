TARGET := mjson.o
SCRIPTS := mjson_fetch

.PHONY: all install

all: $(TARGET) install $(SCRIPTS)

$(TARGET):%.o:%.c
	@$(CC) $(CFLAGS) $^ -o $@

install: $(TARGET)
	@cp *.h $(PREFIX)/include

$(SCRIPTS): %:scripts/%.c
	@$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $(PREFIX)/bin/$@
