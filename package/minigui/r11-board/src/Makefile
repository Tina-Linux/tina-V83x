TARGET = r11-board

SRCDIRS   =  $(shell find . -maxdepth 3 -type d)

INCLUDES	:=  $(foreach dir,$(SRCDIRS),-I$(dir))            \
		   -I$(STAGING_DIR)/usr/include/allwinner/include \
		   -I$(STAGING_DIR)/usr/include/allwinner

SRCS			= $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))
OBJS			= $(SRCS:.c=.o)

#-lwifimg
LOCAL_CFLAGS =
LOCAL_LDFLAGS = -lminigui_ths -lts -lwifimg -lasound -ljpeg -lpng -ldl -lm -lpthread -lmgi \
		-ltrecorder -lpthread -ldl -lminigui_ths -lpng  -ljpeg -lfreetype -lz -lm \
		-ltplayer -lxplayer -lvdecoder -ladecoder -lsubdecoder -lcdc_base -lVE -lMemAdapter \
		-lcdx_parser  -lcdx_playback -lcdx_stream -lcdx_base \
		-luapi -lrt -lc  -lasound \
		-lawrecorder -lvencoder -laencoder \
		-lcdx_muxer -ljpegdecode  -ltmetadataretriever -lcdx_common  -luapi \
		-lmgplus -lmgncs -lmgncs4touch -lmgeff -lsqlite3 -lxml2 -lchipmunk -lmgutils -lstdc++

%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -g

all:install

install:$(TARGET)
	@mkdir -p $(CONFIG_PREFIX)/bin
	@cp $(TARGET) $(CONFIG_PREFIX)/bin

clean:
	-rm -rf $(OBJS) $(TARGET)
.PHONY:all clean $(TARGET) install
