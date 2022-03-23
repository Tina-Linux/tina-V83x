#target			= peq_test

INCLUDES += -I$(CONFIG_PREFIX)/usr/include

#CPP_SRCS = peq_test.c
#CPP_OBJS := $(CPP_SRCS:.c=.o)
#LIB_OBJS = $(CPP_OBJS)

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/bin


#peq_test
peq_test: peq_test.c tsound_ctrl.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lpeq -lasound
	@cp -f peq_test $(CONFIG_PREFIX)/usr/bin


#$(CPP_OBJS): %.o: %.cpp
#	$(CXX) $(CFLAGS) $(INCLUDES) -c -o $@ $<
#$(target): $(LIB_OBJS)
#	$(CXX) -o $@ $^ $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib -lpthread
#	@cp -f $(target) $(CONFIG_PREFIX)/usr/bin

####################################################################
all: install  peq_test
clean:
	rm -rf peq_test
