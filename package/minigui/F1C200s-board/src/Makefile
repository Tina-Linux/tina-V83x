TARGET = F1C200s-board

SRCDIRS   =  $(shell find . -maxdepth 3 -type d)

INCLUDES	:=  $(foreach dir,$(SRCDIRS),-I$(dir))            \
		   -I$(STAGING_DIR)/usr/include/allwinner/include \
		   -I$(STAGING_DIR)/usr/include/allwinner

SRCS			= $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))
OBJS			= $(SRCS:.c=.o)

#LOCAL_LDFLAGS = -lminigui_ths -lts -ljpeg -lpng -lpthread -lmgi -lmgplus -lfreetype \
#    -lasound -ldl -lm -lz -luapi -lrt -lc -lcdx_common\
#		-ltrecorder -ltplayer -lxplayer -lcdc_vdecoder -ladecoder -lcdc_sdecoder -lcdc_base -lcdc_ve -lcdc_memory \
#		-lcdx_parser  -lcdx_playback -lcdx_stream -lcdx_base -lawrecorder -laencoder \
#		-lcdx_muxer -ljpegdecode  -ltmetadataretriever

LOCAL_LDFLAGS = -lminigui_ths -lts -ljpeg -lpng -lpthread -lmgi -lmgplus -lfreetype \
    -ldl -lm -lz -luapi -lrt -lc -luapi

%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS)

all:install

install:$(TARGET)
	@mkdir -p $(CONFIG_PREFIX)/bin
	@cp $(TARGET) $(CONFIG_PREFIX)/bin

clean:
	-rm -rf $(OBJS) $(TARGET)
.PHONY:all clean $(TARGET) install
