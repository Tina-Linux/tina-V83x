TARGET := rwcheck
SRCS = \
   rwcheck.c \
   crc16.c \
   fops.c

OBJS     = $(SRCS:.c=.o)

.PHONY:all clean

all:$(OBJS)
	$(CC) $(LDFLAGS) $(SUB_LIB) -o $(TARGET)  $(OBJS)

%.o:%.c
	$(CC) -c $(CFLAGS) -Wall -I. $< -o $@

clean:
	rm -rf $(SRCS) $(OBJS) $(TARGET)
