TARGET := decodertest
INCLUDES += -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner
LIBS +=	-lcdx_base -lcdx_common -lcdx_parser -lcdx_stream -lcdx_muxer -lvdecoder \
		-lMemAdapter -lpthread -ldl -lm

SRCS = decodertest.c

OBJS = $(SRCS:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) -g $(INCLUDES) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

all:$(TARGET)


clean:
	rm -rf $(TARGET) *.o *.a *~
	cd common && rm -f *.o *.a *.bak *~ .depend
