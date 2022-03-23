TARGET := libawresample.so

SRC :=  ./do_audioresample.c

$(TARGET) : $(SRC)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) libresamplerate.a -shared -fPIC

test : ./test.c
	$(CC) -o $@ $^ -lawresample -L.

install:
	mkdir -p $(INSTALL_PREFIX)/usr/lib
	mkdir -p $(INSTALL_PREFIX)/usr/include
	cp $(TARGET) $(INSTALL_PREFIX)/usr/lib
	cp do_audioresample.h $(INSTALL_PREFIX)/usr/include

all: $(TARGET) test install
