TARGET = libsocket_db.so
SRCS := \
	init_entry.c \
	queue_main.c \
	socket_main.c \
	xml_packet.c

OBJS = $(SRCS:.c=.o)

%.o: %.c
	$(CC) -c $(CFLAGS) -fpic -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -shared -o $@ -lxml2


.PHONY: all install clean

all: $(TARGET)

clean:
	rm -rf $(TARGET) *.o
