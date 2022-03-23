include $(top_srcdir)/Makefile.inc

lib_LTLIBRARIES = libcdx_parser.la

## set the source files.
if TEST_ONLY_ENABLE_AUDIO
libcdx_parser_la_SOURCES =  CdxParser.c
else
libcdx_parser_la_SOURCES =  CdxParser.c \
			VideoSpecificData.c
endif

libcdx_parser_la_SOURCES += \
    ./id3base/Id3Base.c \
    ./id3base/StringContainer.c \
    ./id3base/CdxUtfCode.c \
    ./id3base/CdxMetaData.c

libcdx_parser_la_CFLAGS = $(CFLAGS_CDXG)

if TEST_ONLY_ENABLE_AUDIO
LOCAL_INCLUDE = -I../include \
		-I../../include \
		-I../../base/include \
		-I../../stream/include \
		-I$(top_srcdir)/external/include/adecoder \
		-I$(top_srcdir)/ \
		-I./id3base/
else
LOCAL_INCLUDE = -I../include \
		-I../../include \
		-I../../base/include \
		-I../../stream/include \
		-I$(top_srcdir)/external/include/adecoder \
		-I$(top_srcdir)/../libcedarc/include \
		-I$(top_srcdir)/ \
		-I./id3base/
endif

libcdx_parser_la_CFLAGS += $(LOCAL_INCLUDE)

libcdx_parser_la_LDFLAGS = $(LDFLAGS_CDXG)

LOCAL_LIB = -lpthread -lrt -lz

libcdx_parser_la_LIBADD =

if ASF_PARSER_ENABLE
libcdx_parser_la_LIBADD += $(top_srcdir)/libcore/parser/asf/libcdx_asf_parser.la
libcdx_parser_la_CFLAGS += -DASF_PARSER_ENABLE
endif

if MOV_PARSER_ENABLE
libcdx_parser_la_LIBADD += $(top_srcdir)/libcore/parser/mov/libcdx_mov_parser.la
libcdx_parser_la_CFLAGS += -DMOV_PARSER_ENABLE
endif

if FLV_PARSER_ENABLE
libcdx_parser_la_LIBADD += $(top_srcdir)/libcore/parser/flv/libcdx_flv_parser.la
libcdx_parser_la_CFLAGS += -DFLV_PARSER_ENABLE
endif

if AVI_PARSER_ENABLE
libcdx_parser_la_LIBADD += $(top_srcdir)/libcore/parser/avi/libcdx_avi_parser.la
libcdx_parser_la_CFLAGS += -DAVI_PARSER_ENABLE
endif

if TS_PARSER_ENABLE
libcdx_parser_la_LIBADD += $(top_srcdir)/libcore/parser/ts/libcdx_ts_parser.la
libcdx_parser_la_CFLAGS += -DTS_PARSER_ENABLE
endif

if MKV_PARSER_ENABLE
libcdx_parser_la_LIBADD += $(top_srcdir)/libcore/parser/mkv/libcdx_mkv_parser.la
libcdx_parser_la_CFLAGS += -DMKV_PARSER_ENABLE
endif

if OGG_PARSER_ENABLE
libcdx_parser_la_LIBADD += $(top_srcdir)/libcore/parser/ogg/libcdx_ogg_parser.la
libcdx_parser_la_CFLAGS += -DOGG_PARSER_ENABLE
endif

if MPG_PARSER_ENABLE
libcdx_parser_la_LIBADD += $(top_srcdir)/libcore/parser/mpg/libcdx_mpg_parser.la
libcdx_parser_la_CFLAGS += -DMPG_PARSER_ENABLE
endif

if APE_PARSER_ENABLE
libcdx_parser_la_LIBADD += $(top_srcdir)/libcore/parser/ape/libcdx_ape_parser.la
libcdx_parser_la_CFLAGS += -DAPE_PARSER_ENABLE
endif

if FLAC_PARSER_ENABLE
libcdx_parser_la_LIBADD += $(top_srcdir)/libcore/parser/flac/libcdx_flac_parser.la
libcdx_parser_la_CFLAGS += -DFLAC_PARSER_ENABLE
endif

if AMR_PARSER_ENABLE
libcdx_parser_la_LIBADD += $(top_srcdir)/libcore/parser/amr/libcdx_amr_parser.la
libcdx_parser_la_CFLAGS += -DAMR_PARSER_ENABLE
endif

if MP3_PARSER_ENABLE
libcdx_parser_la_LIBADD += $(top_srcdir)/libcore/parser/mp3/libcdx_mp3_parser.la
libcdx_parser_la_CFLAGS += -DMP3_PARSER_ENABLE
endif

if AAC_PARSER_ENABLE
libcdx_parser_la_LIBADD += $(top_srcdir)/libcore/parser/aac/libcdx_aac_parser.la
libcdx_parser_la_CFLAGS += -DAAC_PARSER_ENABLE
endif

if WAV_PARSER_ENABLE
libcdx_parser_la_LIBADD += $(top_srcdir)/libcore/parser/wav/libcdx_wav_parser.la
libcdx_parser_la_CFLAGS += -DWAV_PARSER_ENABLE
endif

if ID3V2_PARSER_ENABLE
libcdx_parser_la_LIBADD += $(top_srcdir)/libcore/parser/id3v2/libcdx_id3v2_parser.la
libcdx_parser_la_CFLAGS += -DID3V2_PARSER_ENABLE
endif

if PLS_PARSER_ENABLE
libcdx_parser_la_LIBADD += $(top_srcdir)/libcore/parser/pls/libcdx_pls_parser.la
libcdx_parser_la_CFLAGS += -DPLS_PARSER_ENABLE
endif

if HLS_PARSER_ENABLE
libcdx_parser_la_LIBADD += $(top_srcdir)/libcore/parser/hls/libcdx_hls_parser.la
libcdx_parser_la_CFLAGS += -DHLS_PARSER_ENABLE
LOCAL_LIB += -lcrypto
endif

libcdx_parser_la_LIBADD += $(top_srcdir)/libcore/stream/base/libcdx_stream.la \
					    $(top_srcdir)/libcore/base/libcdx_base.la

libcdx_parser_la_LDFLAGS += $(LOCAL_LIB)
