LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LIB_ROOT=$(LOCAL_PATH)/../..
include $(LIB_ROOT)/../config.mk
include $(LIB_ROOT)/PARSER/config.mk

LOCAL_SRC_FILES = \
                $(notdir $(wildcard $(LOCAL_PATH)/*.c))

LOCAL_C_INCLUDES:= \
	$(LIB_ROOT)/BASE/include \
    $(LIB_ROOT)/STREAM/include \
    $(LIB_ROOT)/PARSER/include \
    $(TOP)/external/zlib \
    $(LIB_ROOT)/../CODEC/VIDEO/DECODER/include    \
    $(LIB_ROOT)/../CODEC/AUDIO/DECODER/include    \
    $(LIB_ROOT)/../CODEC/SUBTITLE/DECODER/include \
    $(LIB_ROOT)/../PLAYER/                 \
    $(LIB_ROOT)/../     \

ifeq ($(CONFIG_PRODUCT), $(OPTION_PRODUCT_TVBOX))
ifeq ($(CONFIG_CHIP), $(OPTION_CHIP_1689))
	ifeq ($(BOARD_USE_PLAYREADY), 1)
		ifeq ($(PLAYREADY_DEBUG), 1)
		PLAYREADY_DIR:= $(TOP)/vendor/playready
		include $(PLAYREADY_DIR)/config.mk
		LOCAL_CFLAGS += $(PLAYREADY_CFLAGS)
		LOCAL_C_INCLUDES += \
			$(PLAYREADY_DIR)/source/inc                 \
			$(PLAYREADY_DIR)/source/oem/common/inc      \
			$(PLAYREADY_DIR)/source/oem/ansi/inc        \
			$(PLAYREADY_DIR)/source/results             \
			$(PLAYREADY_DIR)/source/tools/shared/common
		else
		include $(TOP)/hardware/aw/playready/config.mk
		LOCAL_CFLAGS += $(PLAYREADY_CFLAGS)
		LOCAL_C_INCLUDES +=							\
			$(TOP)/hardware/aw/playready/include/inc                 \
			$(TOP)/hardware/aw/playready/include/oem/common/inc      \
			$(TOP)/hardware/aw/playready/include/oem/ansi/inc        \
			$(TOP)/hardware/aw/playready/include/results             \
			$(TOP)/hardware/aw/playready/include/tools/shared/common
		endif
	endif
endif
endif

LOCAL_CFLAGS += $(CDX_CFLAGS)

LOCAL_SHARED_LIBRARIES += libz
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := libcdx_mov_parser

ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -Wno-psabi
endif

include $(BUILD_STATIC_LIBRARY)
