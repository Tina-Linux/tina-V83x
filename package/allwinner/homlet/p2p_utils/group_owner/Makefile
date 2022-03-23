PROG_NAME := p2p_go

PWD := $(shell pwd)
INCLUDES := -I../libs/p2p_wpa/include
DEP_LIBS := -lpthread -ldl -lm
INSTALL_PREFIX ?= $(PWD)/out

all: $(PROG_NAME) install

$(PWD)/../libs/p2p_wpa/libwpa_client.a:
	make -C $(PWD)/../libs/p2p_wpa

$(PROG_NAME): GO_demo.cpp $(PWD)/../libs/p2p_wpa/libwpa_client.a $(PWD)/../libs/libwpa.a
	$(CXX) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib  $(DEP_LIBS)

####################################################################

clean:
	rm -rf  $(PROG_NAME)

install: 
	if [ ! -d "$(INSTALL_PREFIX)/usr/bin" ]; then  mkdir -p $(INSTALL_PREFIX)/usr/bin; fi
	cp $(PROG_NAME) $(INSTALL_PREFIX)/usr/bin

.PHONY: clean install