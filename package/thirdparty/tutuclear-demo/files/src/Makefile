TARGET = tutuclear-demo

SRCS = $(wildcard *.cpp)

override INCLUDES +=
override LDFLAGS +=
override LIBS += -ltconfigs

all: $(TARGET) install

$(TARGET): $(SRCS)
	$(CXX) $^ $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(LIBS) -o $@

install:
	mkdir -p $(INSTALL_PREFIX)/usr/bin
	cp $(TARGET) $(INSTALL_PREFIX)/usr/bin

clean:
	rm -rf $(TARGET)

.PHONY: all clean install
