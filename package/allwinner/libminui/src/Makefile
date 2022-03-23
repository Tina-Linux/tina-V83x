LOCAL_CFLAGS += -I./include -DOVERSCAN_PERCENT=0 -I.
LOCAL_LDFLAGS =

SRCS += ${wildcard *.cpp}
OBJS = $(patsubst %.cpp, %.cpp.o, $(SRCS))
TARGET = libminui.so

ifneq ($(findstring DRECOVERY_RGBX,$(CFLAGS)),DRECOVERY_RGBX)
SRCS := $(filter-out graphics_drm.cpp,$(SRCS))
endif

all:$(TARGET) install

$(TARGET):$(OBJS)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -o $@ $^ -fPIC -shared

$(OBJS):%.cpp.o:%.cpp
	$(CXX) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -c $^ -o $@ -fPIC -shared

clean:
	-rm -rf $(OBJS) $(TARGET)

install:
	-mkdir -p $(CONFIG_PREFIX)/lib
	-mkdir -p $(CONFIG_PREFIX)//usr/include/minui/
	-cp $(TARGET) $(CONFIG_PREFIX)/lib/
	-cp include/minui/*.h $(CONFIG_PREFIX)/usr/include/minui/

.PHONY:all clean $(TARGET) install
