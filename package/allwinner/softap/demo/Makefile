#target			= softap_test

INCLUDES += -I$(CONFIG_PREFIX)/usr/include

#CPP_SRCS = softap_test.c
#CPP_OBJS := $(CPP_SRCS:.c=.o)
#LIB_OBJS = $(CPP_OBJS)

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/bin


#softap_test
softap_test: softap_test.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lsoftap -lssl
	@cp -f softap_test $(CONFIG_PREFIX)/usr/bin

#softap_up
softap_up: softap_up.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lsoftap -lssl
	@cp -f softap_up $(CONFIG_PREFIX)/usr/bin

#softap_down
softap_down: softap_down.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lsoftap -lssl
	@cp -f softap_down $(CONFIG_PREFIX)/usr/bin

#softap_longtime_test
softap_longtime_test: softap_longtime_test.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lsoftap -lssl
	@cp -f softap_longtime_test $(CONFIG_PREFIX)/usr/bin

#wifi_connect_ap_test
#$(CPP_OBJS): %.o: %.cpp
#	$(CXX) $(CFLAGS) $(INCLUDES) -c -o $@ $<
#$(target): $(LIB_OBJS)
#	$(CXX) -o $@ $^ $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lwifimg -lpthread
#	@cp -f $(target) $(CONFIG_PREFIX)/usr/bin

####################################################################
all: install  softap_test softap_up softap_down softap_longtime_test
clean:
	rm -rf softap_test softap_up softap_down softap_longtime_test
