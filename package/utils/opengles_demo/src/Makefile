TARGET          = opengles_demo
INCLUDES        +=
LIBS           	+=

SRCS    = opengles_demo.c
OBJS    = opengles_demo.o

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

.PHONY: all clean

all: $(TARGET)

clean:
	rm -rf $(TARGET) *.o

