
INCLUDES += -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner
LIBS +=	 -lpthread -lsunxicamera

C_SRCS = convert.c
CPP_SRCS = demo.cpp

OBJS = $(C_SRCS:.c=.o)
OBJS += $(CPP_SRCS:.cpp=.o)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)


all:$(TARGET)


clean:
	rm -rf $(TARGET) *.o *.a *~

