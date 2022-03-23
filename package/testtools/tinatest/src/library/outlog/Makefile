TARGET := outlog.o

.PHONY: all install

all: $(TARGET) install

$(TARGET): $(TARGET:.o=.c)
	@$(CC) $(CFLAGS) $^ -o $@

install: $(TARGET)
	@$(CP) *.h $(PREFIX)/include
