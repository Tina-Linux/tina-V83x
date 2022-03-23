TARGET          = ShaderYUVtoRGB
INCLUDES        +=
LIBS            +=

SRCS    = ShaderYUVtoRGB.c ShaderYUVtoRGB_test.c
OBJS    = ShaderYUVtoRGB.c ShaderYUVtoRGB_test.o

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

.PHONY: all clean

all: $(TARGET)

clean:
	rm -rf $(TARGET) *.o

