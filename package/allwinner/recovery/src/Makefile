target 		=recovery

INCLUDES 	+= -I. -I./include -DCONFIG_TLOG_LEVEL=OPTION_TLOG_LEVEL_WARNING \
				-DRECOVERY_UI_MARGIN_HEIGHT=0 \
				-DRECOVERY_UI_MARGIN_WIDTH=0 \
				-DRECOVERY_UI_TOUCH_LOW_THRESHOLD=50 \
				-DRECOVERY_UI_TOUCH_HIGH_THRESHOLD=90 \
				-DRECOVERY_UI_PROGRESS_BAR_BASELINE=259 \
				-DRECOVERY_UI_ANIMATION_FPS=30 \
				-DRECOVERY_UI_MENU_UNUSABLE_ROWS=9 \
				-DRECOVERY_UI_VR_STEREO_OFFSET=0
LIBS 		:= -Wl,--start-group -lpthread -lpng -lminui -Wl,--end-group

SRCS 		= recovery.cpp \
      		  default_device.cpp \
      		  device.cpp \
      		  screen_ui.cpp \
      		  ui.cpp

OBJS = $(SRCS:.cpp=.o)

install:$(target)
	@mkdir -p $(CONFIG_PREFIX)/bin
	@cp $(target) $(CONFIG_PREFIX)/bin

%.o: %.cpp
	$(CXX) $(CFLAGS) $(LOCAL_CFLAGS) $(INCLUDES) -c -o $@ $<

$(target): $(OBJS)
	$(CXX) $(INCLUDES) $(LDFLAGS) $^ $(LIBS) -o $@

####################################################################
all: install
clean:
	rm -rf $(target) $(OBJS)
