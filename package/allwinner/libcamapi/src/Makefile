TARGET = libcamapi.so

PWD := $(shell pwd)
INCLUDES += -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner -I$(PWD)/include
LIBS +=  -lpthread \

SRCS = $(PWD)/common/CameraV4l2.c $(PWD)/common/CameraParseConfig.c CameraInterface.c

OBJS = $(SRCS:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(CompileFlags) -c -o $@ $< -fPIC -shared

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ -fPIC -shared $(LDFLAGS) $(LIBS) $(INCLUDES)

all:$(TARGET)

clean:
	rm -rf $(TARGET) *.o *.a *~
	cd common && rm -f *.o *.a *.bak *~ .depend

