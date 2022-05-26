TARGET = update_dtb

.PHONY: all
all:$(TARGET)

.PHONY: clean
clean:
	rm -rf $(TARGET)

$(TARGET): update_dtb.c
	$(CC) -o $(TARGET) update_dtb.c
	$(STRIP) $(TARGET)
