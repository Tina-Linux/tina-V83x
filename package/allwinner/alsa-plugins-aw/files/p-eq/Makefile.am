include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/package.mk

#asound_module_pcm_awequal_LTLIBRARIES = libasound_module_pcm_awequal.la

asound_module_pcm_awequaldir = @ALSA_PLUGIN_DIR@

AM_CFLAGS = -Wall -g @ALSA_CFLAGS@ -I./
AM_LDFLAGS = -module -avoid-version -export-dynamic -Wl,--no-undefined $(LDFLAGS_NOUNDEFINED) -lm

libasound_module_pcm_awequal_la_SOURCES = #eq.c

install-exec-hook:
	mkdir -p $(DESTDIR)@ALSA_PLUGIN_DIR@/
	cp ./lib/$(CONFIG_LIBC)/libasound_module_pcm_awequal.so $(DESTDIR)@ALSA_PLUGIN_DIR@/
	chmod 755 $(DESTDIR)@ALSA_PLUGIN_DIR@/libasound_module_pcm_awequal.so
#	$(LN_S) libasound_module_pcm_awequal.so $(DESTDIR)@ALSA_PLUGIN_DIR@/libasound_module_pcm_awequal.so

uninstall-hook:
	rm -f $(DESTDIR)$(libdir)/alsa-lib/libasound_module_pcm_awequal.so

libasound_module_pcm_awequal_la_LIBADD = @ALSA_LIBS@

noinst_HEADERS = eq.h
