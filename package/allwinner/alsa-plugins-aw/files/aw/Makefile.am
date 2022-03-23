asound_module_rate_awrate_LTLIBRARIES = libasound_module_rate_awrate.la

asound_module_rate_awratedir = @ALSA_PLUGIN_DIR@

AM_CFLAGS = -Wall -g @ALSA_CFLAGS@ -I./
AM_LDFLAGS = -module -avoid-version -export-dynamic -Wl,--no-undefined $(LDFLAGS_NOUNDEFINED)  -Wl,--whole-archive libs/$(host)/libsrc.a -Wl,--no-whole-archive

libasound_module_rate_awrate_la_SOURCES = rate_awrate.c

uninstall-hook:
	rm -f $(DESTDIR)@ALSA_PLUGIN_DIR@/libasound_module_rate_awrate.so


noinst_HEADERS = doResample.h aumixcom.h
