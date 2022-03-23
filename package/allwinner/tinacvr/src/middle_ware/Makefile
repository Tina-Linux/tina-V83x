TARGET		=  middle_ware_test
LIBS			+=   -ltplayer -lxplayer -lcdc_vdecoder -ladecoder -lcdc_sdecoder -lcdc_base -lcdc_ve -lcdc_memory \
               -lcdx_parser  -lcdx_playback -lcdx_stream -lcdx_base \
               -lpthread -ldl -luapi -lstdc++ -lrt -lm -lc -lz -lasound \
               -lawrecorder -lcdc_vencoder -laencoder \
               -lcdx_muxer -ljpegdecode  -ltmetadataretriever -lcdx_common

SRCDIRS   =  $(shell find . -maxdepth 3 -type d)
INCLUDES :=   $(INCLUDES)       \
		$(foreach dir,$(SRCDIRS),-I$(dir))  \
		-I$(STAGING_DIR)/usr/include/allwinner/include \
		-I$(STAGING_DIR)/usr/include/allwinner

SRCS	= $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))
OBJS	= $(SRCS:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

all:$(TARGET)

clean:
	rm -rf $(TARGET) *.o *.a *~
