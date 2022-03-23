IPCLIB			= libipc.so

INCLUDES        += -I.
LIBS            += -lpthread

LIB_SRCS = transaction/interface.cpp
LIB_OBJS = $(LIB_SRCS:.cpp=.o)

#libipc.so
$(LIB_OBJS):%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $< -fPIC -shared
$(IPCLIB): $(LIB_OBJS)
	$(CC) -o $@ $^ $(LIBS) $(LDFLAGS) -fPIC -shared


####################################################################
all:  $(IPCLIB) install
install:
	mkdir -p $(CONFIG_PREFIX)/usr/lib
	mkdir -p $(CONFIG_PREFIX)/usr/include
	cp $(IPCLIB) $(CONFIG_PREFIX)/usr/lib
	cp transaction/*.h $(CONFIG_PREFIX)/usr/include
clean:
	rm -rf $(IPCLIB) $(LIB_OBJS)
