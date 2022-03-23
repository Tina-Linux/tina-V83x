TARGET			 =  tinacvr

SRCDIRS   =  $(shell find . -maxdepth 3 -type d)

INCLUDES	:= $(INCLUDES)       \
						 $(foreach dir,$(SRCDIRS),-I$(dir))  \
						 -I$(STAGING_DIR)/usr/include/allwinner/include \
						 -I$(STAGING_DIR)/usr/include/allwinner

SRCS			= $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))
OBJS			= $(SRCS:.c=.o)
LIBS			+= -ltrecorder -lpthread -ldl -lminigui_ths -lpng  -ljpeg -lfreetype -lz -lm \
						 -ltplayer -lxplayer -lvdecoder -ladecoder -lsubdecoder -lcdc_base -lVE -lMemAdapter \
             -lcdx_parser  -lcdx_playback -lcdx_stream -lcdx_base \
             -luapi -lrt -lc  -lasound \
             -lawrecorder -lvencoder -laencoder \
             -lcdx_muxer -ljpegdecode  -ltmetadataretriever -lcdx_common

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

all:$(TARGET)

clean:
	rm -rf $(TARGET) *.o *.a *~

# -lstdc++ //c++¿â
#TARGET			 =  tinacvr
#LIBS			  +=  -lpthread -ldl -lminigui_ths -lpng  -ljpeg -lfreetype -lz -lm -lasound
#
#SRCDIRS   =  $(shell find . -maxdepth 3 -type d)
#INCLUDES :=   $(INCLUDES)       \
#                    $(foreach dir,$(SRCDIRS),-I$(dir))
#
#SRCS	= $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))
#OBJS	= $(SRCS:.c=.o)
#
#%.o: %.c
#	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
#
#$(TARGET): $(OBJS)
#	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)
#
#all:$(TARGET)
#
#clean:
#	rm -rf $(TARGET) *.o *.a *~
#
#
#
