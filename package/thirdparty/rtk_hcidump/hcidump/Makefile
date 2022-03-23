include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/package.mk

target			= rtk_hcidump

INCLUDES      += -I. -I./include -I $(STAGING_DIR)/usr/include/glib-2.0 \
                 -I $(STAGING_DIR)/usr/include/gio-unix-2.0 -I $(CONFIG_PREFIX)/usr/include
LIBS            += -lglib-2.0 -lc

SRCS		  = tools/hcidump.c \
				tools/parser/parser.c \
				tools/parser/lmp.c \
				tools/parser/hci.c \
				tools/parser/l2cap.c \
				tools/parser/amp.c \
				tools/parser/smp.c \
				tools/parser/att.c \
				tools/parser/sdp.c \
				tools/parser/rfcomm.c \
				tools/parser/bnep.c \
				tools/parser/cmtp.c \
				tools/parser/hidp.c \
				tools/parser/hcrp.c \
				tools/parser/avdtp.c \
				tools/parser/avctp.c \
				tools/parser/avrcp.c \
				tools/parser/sap.c \
				tools/parser/obex.c \
				tools/parser/capi.c \
				tools/parser/ppp.c \
				tools/parser/tcpip.c \
				tools/parser/ericsson.c \
				tools/parser/csr.c \
				tools/parser/bpa.c

SRCS         += lib/hci.c \
		        lib/bluetooth.c

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/bin

$(target): $(SRCS)
	$(CC) $(INCLUDES) $(LDFLAGS) $^ -o $@ $(LIBS)
	@cp $@ $(CONFIG_PREFIX)/usr/bin

####################################################################
all: install $(target)
clean:
	rm -rf $(target)
