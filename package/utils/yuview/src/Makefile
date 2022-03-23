CC = gcc
CFLAGS = -Wall
LDFLAGS = -luapi
OBJS = yuview.o

all: yuview

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

yuview: $(OBJS)
	$(CC) -o $@ $(OBJS) -luapi

clean:
	rm -f rbcfg *.o
