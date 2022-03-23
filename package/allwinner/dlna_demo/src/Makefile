
LS_CPP=$(subst $(1)/,,$(wildcard $(1)/$(2)/*.cpp))
LS_C=$(subst $(1)/,,$(wildcard $(1)/$(2)/*.c))

TARGET = dlnademo

SYS_LIBS := \
	-lpthread \
	-lcrypto

DLNA_DYNAMIC_LIBS := \
		-ljsoncpp \
		-lneptune \
		-lplatinum \
		-ldlnaHAL \
		-ldlnaservice

ifeq ($(TARGET_BOARD),$(filter $(TARGET_BOARD),r40 r18 r6 g102 r7 r11))
AWPLATER_LIBS :=  -ltinaplayer -lxplayer -lcdc_vdecoder -ladecoder -lcdc_sdecoder -lcdc_base -lcdc_ve -lcdc_memory \
               -lcdx_parser  -lcdx_playback -lcdx_stream -lcdx_base \
               -lpthread -ldl -lstdc++ -lrt -lm -lc -lz -lasound
else ifeq ($(TARGET_BOARD),$(filter $(TARGET_BOARD),r16 r58))
AWPLATER_LIBS :=  -ltinaplayer -lxplayer -lcdc_vdecoder -lcdc_adecoder -lcdc_sdecoder -lcdc_base -lcdc_ve -lcdc_memory \
               -lcdx_parser  -lplayer -lcdx_stream -lcdx_base -law_plugin \
               -lpthread -ldl -lstdc++ -lrt -lm -lc -lz -lasound
endif

LIBS := \
		$(SYS_LIBS) \
		$(AWPLATER_LIBS) \
		$(DLNA_DYNAMIC_LIBS) \
		$(AUDIO_LIBS)

COMPILE_FLAGS := $(LIBS)

#$(CXX) -o $@ $^ $(COMPILE_FLAGS) $(LDFLAGS)
$(TARGET): main.cpp
	$(CXX) -g -o $@ $^ $(COMPILE_FLAGS)

all:$(TARGET) install

install:
	mkdir -p $(CONFIG_PREFIX)/usr/bin
	cp $(TARGET) $(CONFIG_PREFIX)/usr/bin/

clean:
	rm -rf $(TARGET) $(OBJS)
