TARGET = libsunxicamera.so

INCLUDES += -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner -I./include
LIBS +=  -lpthread \

CompileFlags = -D__USE_VIN_ISP__
LIBS += -lm -lisp -lisp_ini -lAWIspApi

C_SRCS = camerav4l2.c
CPP_SRCS = sunxi_cam.cpp

C_OBJS = $(C_SRCS:.c=.o)
CPP_OBJS = $(CPP_SRCS:.cpp=.o)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(CompileFlags) -c -o $@ $< -fPIC -shared

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $< -fPIC -shared

$(TARGET): $(C_OBJS) $(CPP_OBJS)
	$(CXX) -o $@ $^ -fPIC -shared $(LDFLAGS) $(LIBS) $(INCLUDES)

all:$(TARGET) install

install:
	mkdir -p $(INSTALL_PREFIX)/usr/lib
	cp $(TARGET) $(INSTALL_PREFIX)/usr/lib
	mkdir -p $(INSTALL_PREFIX)/usr/include
	cp -r include/*  $(INSTALL_PREFIX)/usr/include

clean:
	rm -rf $(TARGET) *.o *~

