LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LIB_ROOT=$(LOCAL_PATH)/../..
include $(LIB_ROOT)/../config.mk
include $(LIB_ROOT)/PARSER/config.mk

LOCAL_SRC_FILES = \
    CdxMp3Parser.c

LOCAL_C_INCLUDES:= \
	$(LIB_ROOT)/BASE/include \
    $(LIB_ROOT)/STREAM/include \
    $(LIB_ROOT)/PARSER/include \
    $(LIB_ROOT)/../CODEC/VIDEO/DECODER/include    \
    $(LIB_ROOT)/../CODEC/AUDIO/DECODER/include    \
    $(LIB_ROOT)/../CODEC/SUBTITLE/DECODER/include \
    $(LIB_ROOT)/../PLAYER/                 \
    $(LIB_ROOT)/../                 \

LOCAL_CFLAGS += $(CDX_CFLAGS) -Wno-unused-parameter

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

LOCAL_MODULE:= libcdx_mp3_parser

include $(BUILD_STATIC_LIBRARY)
