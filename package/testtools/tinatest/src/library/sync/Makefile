TARGET := sync.o

.PHONY: all install

all: $(TARGET) install

$(TARGET):%.o:%.c
	@$(CC) $(CFLAGS) $^ -o $@

install: $(TARGET)
	@$(CP) *.h $(PREFIX)/include
