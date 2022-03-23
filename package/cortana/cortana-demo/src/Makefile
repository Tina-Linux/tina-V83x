TARGET = cortana-demo

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))

override CFLAGS +=
override LDFLAGS +=
INCLUDES +=
override LIBS += -liot-cortana-sdk -liot-cortana-sdk-ble-stub \
		-liot-cortana-sdk-wifi-stub -liot-cortana-sdk-ota-stub \
		-lbreakpad_client -laziotsharedutil -lcurl -lssl -luuid \
		-lcrypto -lwebsockets -lkwsapi -lsilk \
		-lgstreamer-1.0 -lasound -lm -lpthread \
		-lglib-2.0 -lgobject-2.0 -lstdc++

all: $(TARGET) install

%.o: %.c
	$(CC) -o $@ $(CFLAGS) $(INCLUDES) -c $^

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) -Wl,--start-group $(LIBS) -Wl,--end-group $(LDFLAGS)

install:
	mkdir -p $(INSTALL_DIR)
	cp $(TARGET) $(INSTALL_DIR)

clean:
	rm -rf $(TARGET) *.o
