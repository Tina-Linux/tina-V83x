TARGET = camerademo
INCLUDES += -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner
LIBS +=  -lpthread -ldl -lrt \

ifeq ($(USE_VIN_ISP),y)
	CompileFlags += -D__USE_VIN_ISP__

	LIBS += -lm -lisp -lisp_ini -lAWIspApi
endif

SRCS = camerademo.c add_water.c water_mark.c convert.c common.c
OBJS = $(SRCS:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(CompileFlags) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

all:$(TARGET)

clean:
	rm -rf $(TARGET) *.o *.a *~
	cd common && rm -f *.o *.a *.bak *~ .depend
