noinst_LTLIBRARIES = libwpa_cli.la

libwpa_cli_la_SOURCES = \
    wpa_cli.c\
    wfd_log.c \
    WFDMessageQueue.c\
    p2pManager.c

libwpa_cli_la_CFLAGS = -MMD -O2 -Wall -g \
    -DCONFIG_CTRL_IFACE \
    -DCONFIG_CTRL_IFACE_UNIX \
    -DCONFIG_P2P \
    -DCONFIG_WIFI_DISPLAY \
    -DCONFIG_WPS \
    -DCONFIG_WPS_NFC \
    -DCONFIG_AP \
    -DCONFIG_HS20 \
    -DCONFIG_AUTOSCAN \
    -DCONFIG_INTERWORKING \
    -DIEEE8021X_EAPOL

libwpa_cli_la_CFLAGS += \
    -I$(srcdir) \
    -I$(srcdir)/include

libwpa_cli_la_LIBADD = \
    $(srcdir)/libwpa_client.a
