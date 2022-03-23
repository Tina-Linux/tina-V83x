LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LIB_ROOT=$(LOCAL_PATH)/../..
include $(LIB_ROOT)/../config.mk
include $(LIB_ROOT)/PARSER/config.mk

LOCAL_SRC_FILES = \
                $(notdir $(wildcard $(LOCAL_PATH)/*.c))

LOCAL_C_INCLUDES:= $(LIB_ROOT)/BASE/include \
        $(LIB_ROOT)/STREAM/include \
        $(LIB_ROOT)/PARSER/include \
        $(LIB_ROOT)/../CODEC/VIDEO/DECODER/include    \
        $(LIB_ROOT)/../CODEC/AUDIO/DECODER/include    \
        $(LIB_ROOT)/../CODEC/SUBTITLE/DECODER/include \
        $(LIB_ROOT)/../PLAYER/include                 \
        $(LIB_ROOT)/../     \

LOCAL_CFLAGS += $(CDX_CFLAGS)

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

LOCAL_MODULE:= libcdx_parser

LOCAL_SHARED_LIBRARIES = libcdx_stream libcdx_base libaw_wvm

LOCAL_SHARED_LIBRARIES += libicuuc libutils libcutils libz libdl libssl libcrypto

#LOCAL_WHOLE_STATIC_LIBRARIES = libcdx_remux_parser libcdx_asf_parser
LOCAL_STATIC_LIBRARIES = \
	libcdx_remux_parser \
	libcdx_asf_parser \
	libcdx_avi_parser \
	libcdx_flv_parser \
	libcdx_ts_parser \
	libcdx_mov_parser \
	libcdx_mms_parser \
	libcdx_dash_parser \
	libcdx_hls_parser \
	libcdx_mkv_parser \
	libcdx_mpg_parser \
	libcdx_bd_parser \
	libcdx_pmp_parser \
	libcdx_ogg_parser \
	libcdx_m3u9_parser \
	libcdx_playlist_parser \
	libcdx_wav_parser \
	libcdx_ape_parser \
	libcdx_flac_parser \
	libcdx_amr_parser \
	libcdx_atrac_parser \
	libcdx_mp3_parser \
	libcdx_aac_parser \
	libcdx_mmshttp_parser\
	libcdx_awts_parser\
	libcdx_sstr_parser \
	libcdx_caf_parser \
	libcdx_g729_parser \
	libcdx_id3_parser \
	libcdx_dsd_parser \
	libcdx_awrawstream_parser\
	libcdx_awspecialstream_parser

LOCAL_STATIC_LIBRARIES += libxml2

ifeq ($(BOARD_USE_PLAYREADY), 1)
LOCAL_SHARED_LIBRARIES += libaw_env libMemAdapter libplayreadypk
else
LOCAL_STATIC_LIBRARIES += libaw_env
endif

ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -Wno-psabi
endif

include $(BUILD_SHARED_LIBRARY)
