target 		=healthd

INCLUDES 	+= -I. -I./include -I../libbatinfo/include -DCONFIG_TLOG_LEVEL=OPTION_TLOG_LEVEL_WARNING
LIBS 		:= -L../libbatinfo -lpthread -lbatinfo -luci

ifeq ($(BROADCAST_INFO),y)
LOCAL_CFLAGS 	+= -DBROADCAST_INFO
LIBS 		+= -ldbus-1
endif
ifeq ($(CHARGE_ANIMATION),y)
LOCAL_CFLAGS 	+= -DCHARGE_ANIMATION
LIBS 		+= -lminui -lpng
endif
ifeq ($(SHUTDOWN_CHARGER),y)
LOCAL_CFLAGS 	+= -DSHUTDOWN_CHARGER
endif


SRCS 		= healthd.cpp \
      		  healthd_mode_tina.cpp \
      		  BatteryMonitor.cpp

ifeq ($(SHUTDOWN_CHARGER),y)
ifeq ($(CHARGE_ANIMATION),y)
SRCS 		+= healthd_mode_charger.cpp
else
SRCS 		+= healthd_mode_charger_without_animation.cpp
endif
endif

OBJS = $(SRCS:.cpp=.o)

install:$(target)
	@mkdir -p $(CONFIG_PREFIX)/bin
	@cp $(target) $(CONFIG_PREFIX)/bin

%.o: %.cpp
	$(CXX) $(CFLAGS) $(LOCAL_CFLAGS) $(INCLUDES) -c -o $@ $<

$(target): $(OBJS)
	$(CXX) $(INCLUDES) $(LDFLAGS) $(LIBS) $^ -o $@

####################################################################
all: install
clean:
	rm -rf $(target) $(OBJS)
