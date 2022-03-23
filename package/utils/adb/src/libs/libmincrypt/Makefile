TARGET          = libmincrypt.a
INCLUDES        += -I.


SRCS := rsa.c rsa_e_3.c rsa_e_f4.c sha.c
OBJS    = $(SRCS:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(TARGET): $(OBJS)
	ar r $@ $(OBJS)

all:$(TARGET)

clean:
	rm -rf $(TARGET) *.o *.a *~
