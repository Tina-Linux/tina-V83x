TARGET = midea_uart

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, %.c.o, $(SRCS))

INCLUDES +=-I./include
LIBS += -lpthread -luvoice -ltinyalsa -luv_se_bf -luvoicekws -luvoice_wakeup
all:$(TARGET)

$(OBJS):%.c.o:%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@

$(TARGET): $(OBJS)
	$(CC) $^ $(LIBS) $(LDFLAGS) $(LOCAL_LDFLAGS) -o $@

clean:
	rm -rf $(TARGET) $(OBJS)

