TARGET           = logwrapper
INCLUDES        +=
LIBS            += -lpthread -lm -lrt

SRCS    = logwrapper.c
OBJS    = $(SRCS:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

all:$(TARGET) install

install:
	mkdir -p $(CONFIG_PREFIX)/usr/bin
	cp $(TARGET) $(CONFIG_PREFIX)/usr/bin/


clean:
	rm -rf $(TARGET) *.o *.a *~
