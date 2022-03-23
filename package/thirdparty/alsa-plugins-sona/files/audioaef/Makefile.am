asound_module_pcm_sona_audioaef_LTLIBRARIES = libasound_module_pcm_sona_audioaef.la

asound_module_pcm_sona_audioaefdir = @ALSA_PLUGIN_DIR@

AM_CFLAGS = -Wall -g @ALSA_CFLAGS@ -I./
AM_LDFLAGS = -module -avoid-version -export-dynamic -Wl,--no-undefined $(LDFLAGS_NOUNDEFINED) \
			 -lsona_audioaef

libasound_module_pcm_sona_audioaef_la_SOURCES = pcm_sona_audioaef.c

uninstall-hook:
	rm -f $(DESTDIR)@ALSA_PLUGIN_DIR@/libasound_module_pcm_sona_audioaef.so

noinst_HEADERS = audioaef_ext.h
