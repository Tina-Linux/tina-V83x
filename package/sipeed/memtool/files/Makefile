TARGET = memtool

.PHONY: all
all: $(TARGET)
	@echo build $(TARGET) done

.PHONY: clean
clean:
	rm -rf $(TARGET)

$(TARGET):
	$(CC) -o $(TARGET) mem_show.c
