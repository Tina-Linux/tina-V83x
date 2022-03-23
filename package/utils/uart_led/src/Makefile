TARGET = uart_led

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, %.c.o, $(SRCS))

INCLUDES +=-I./include
LIBS += -lpthread
all:$(TARGET)

$(OBJS):%.c.o:%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@

$(TARGET): $(OBJS)
	$(CC) $^ $(LIBS) $(LDFLAGS) $(LOCAL_LDFLAGS) -o $@

clean:
	rm -rf $(TARGET) $(OBJS)

