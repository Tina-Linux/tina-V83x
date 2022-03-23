TARGET = __trecordertest
INCLUDES += -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner

LoadFlags += -ltrecorder -lawrecorder -lvencoder -laencoder \
               -lcdx_base -lcdx_muxer -lcdc_base -lVE \
               -lMemAdapter  -lpthread -luapi -ldl -lrt -lz -lasound -lvdecoder -lm

$(TARGET): __trecordertest.c
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(INCLUDES) $(LoadFlags)

all:$(TARGET)


clean:
	rm -rf $(TARGET) *.o *.a *~
