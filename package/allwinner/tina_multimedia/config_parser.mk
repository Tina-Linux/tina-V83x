#config the parsers which will be compiled
ifeq ($(CONFIG_CEDAR_PARSER_AAC),y)
	CONFIG_PARSER += --enable-aac-parser=yes
else
	CONFIG_PARSER += --enable-aac-parser=no
endif

ifeq ($(CONFIG_CEDAR_PARSER_MP3),y)
	CONFIG_PARSER += --enable-mp3-parser=yes
else
	CONFIG_PARSER += --enable-mp3-parser=no
endif

ifeq ($(CONFIG_CEDAR_PARSER_WAV),y)
	CONFIG_PARSER += --enable-wav-parser=yes
else
	CONFIG_PARSER += --enable-wav-parser=no
endif

ifeq ($(CONFIG_CEDAR_PARSER_ID3V2),y)
	CONFIG_PARSER += --enable-id3v2-parser=yes
else
	CONFIG_PARSER += --enable-id3v2-parser=no
endif

ifeq ($(CONFIG_CEDAR_PARSER_FLAC),y)
	CONFIG_PARSER += --enable-flac-parser=yes
else
	CONFIG_PARSER += --enable-flac-parser=no
endif

ifeq ($(CONFIG_CEDAR_PARSER_APE),y)
	CONFIG_PARSER += --enable-ape-parser=yes
else
	CONFIG_PARSER += --enable-ape-parser=no
endif

ifeq ($(CONFIG_CEDAR_PARSER_AMR),y)
	CONFIG_PARSER += --enable-amr-parser=yes
else
	CONFIG_PARSER += --enable-amr-parser=no
endif

ifeq ($(CONFIG_CEDAR_PARSER_OGG),y)
	CONFIG_PARSER += --enable-ogg-parser=yes
else
	CONFIG_PARSER += --enable-ogg-parser=no
endif

ifeq ($(CONFIG_CEDAR_PARSER_TS),y)
	CONFIG_PARSER += --enable-ts-parser=yes
else
	CONFIG_PARSER += --enable-ts-parser=no
endif

ifeq ($(CONFIG_CEDAR_PARSER_MKV),y)
	CONFIG_PARSER += --enable-mkv-parser=yes
else
	CONFIG_PARSER += --enable-mkv-parser=no
endif

ifeq ($(CONFIG_CEDAR_PARSER_MOV),y)
	CONFIG_PARSER += --enable-mov-parser=yes
else
	CONFIG_PARSER += --enable-mov-parser=no
endif

ifeq ($(CONFIG_CEDAR_PARSER_AVI),y)
	CONFIG_PARSER += --enable-avi-parser=yes
else
	CONFIG_PARSER += --enable-avi-parser=no
endif

ifeq ($(CONFIG_CEDAR_PARSER_FLV),y)
	CONFIG_PARSER += --enable-flv-parser=yes
else
	CONFIG_PARSER += --enable-flv-parser=no
endif

ifeq ($(CONFIG_CEDAR_PARSER_ASF),y)
	CONFIG_PARSER += --enable-asf-parser=yes
else
	CONFIG_PARSER += --enable-asf-parser=no
endif

ifeq ($(CONFIG_CEDAR_PARSER_HLS),y)
	CONFIG_PARSER += --enable-hls-parser=yes
else
	CONFIG_PARSER += --enable-hls-parser=no
endif

ifeq ($(CONFIG_CEDAR_PARSER_MPG),y)
	CONFIG_PARSER += --enable-mpg-parser=yes
else
	CONFIG_PARSER += --enable-mpg-parser=no
endif

ifeq ($(CONFIG_CEDAR_PARSER_PLS),y)
	CONFIG_PARSER += --enable-pls-parser=yes
else
	CONFIG_PARSER += --enable-pls-parser=no
endif
