TARGET := libsysutils.so

commonSources:= \
           	  SocketListener.cpp      \
                  FrameworkListener.cpp   \
                  NetlinkListener.cpp     \
                  NetlinkEvent.cpp        \
                  FrameworkCommand.cpp    \
                  SocketClient.cpp        \
                  ServiceManager.cpp
#                  EventLogTags.logtags

local_CFLAGS := -I./include

OBJS = $(patsubst %.cpp, %.cpp.o, $(commonSources))

$(TARGET):$(OBJS)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) -o $@ $^ -fPIC -shared

$(OBJS):%.cpp.o:%.cpp
	$(CXX) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) -c $< -o $@ -fPIC -shared

all: $(TARGET) install

clean:
	-rm $(OBJS) $(TARGET)
install:
	mkdir -p $(INSTALL_PREFIX)/usr/lib
	cp $(TARGET) $(INSTALL_PREFIX)/usr/lib
	mkdir -p $(INSTALL_PREFIX)/usr/include
	cp -r include/*  $(INSTALL_PREFIX)/usr/include
