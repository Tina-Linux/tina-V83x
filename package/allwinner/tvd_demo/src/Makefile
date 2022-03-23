TARGET			=	tvd_demo
INCLUDES		+=	-I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner
LIBS			+=  -lpthread -ldl -lrt -luapi

SRCS	= display.c camera_video.c camera_disp.c convert.c common.c

OBJS	= $(SRCS:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) -g $(INCLUDES) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

all:$(TARGET)


clean:
	rm -rf $(TARGET) *.o *.a *~
	cd common && rm -f *.o *.a *.bak *~ .depend
