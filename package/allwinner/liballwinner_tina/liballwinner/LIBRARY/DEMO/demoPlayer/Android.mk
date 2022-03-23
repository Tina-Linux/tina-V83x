LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include $(LOCAL_PATH)/../../config.mk

################################################################################
## set flags for golobal compile and link setting.
################################################################################

CONFIG_FOR_COMPILE =
CONFIG_FOR_LINK =

LOCAL_CFLAGS += $(CONFIG_FOR_COMPILE)
LOCAL_MODULE_TAGS := optional

################################################################################
## set the source files
################################################################################
## set the source path to VPATH.
SourcePath = $(shell find $(LOCAL_PATH) -type d)
SvnPath = $(shell find $(LOCAL_PATH) -type d | grep ".svn")
SourcePath := $(filter-out $(SvnPath), $(SourcePath))


## set the source files.
tmpSourceFiles  = $(foreach dir,$(SourcePath),$(wildcard $(dir)/*.cpp))
SourceFiles  = $(foreach file,$(tmpSourceFiles),$(subst $(LOCAL_PATH)/,,$(file)))

## set the include path for compile flags.
LOCAL_SRC_FILES:= $(SourceFiles)
LOCAL_C_INCLUDES := $(SourcePath)                               \
                    $(LOCAL_PATH)/../../                        \
                    $(LOCAL_PATH)/../../PLAYER/include                  \
                    $(LOCAL_PATH)/../../VE/include                      \
                    $(LOCAL_PATH)/../../MEMORY/include                  \
                    $(LOCAL_PATH)/../../CODEC/VIDEO/DECODER/include     \
                    $(LOCAL_PATH)/../../CODEC/AUDIO/DECODER/include     \
                    $(LOCAL_PATH)/../../CODEC/SUBTITLE/DECODER/include  \
                    $(LOCAL_PATH)/../../DEMUX/BASE/include      \
                    $(LOCAL_PATH)/../../DEMUX/STREAM/include    \
                    $(LOCAL_PATH)/../../DEMUX/PARSER/include    \

LOCAL_SHARED_LIBRARIES := \
            libcutils       \
            libutils        \
            libplayer       \
            libvdecoder     \
            libadecoder     \
            libnormal_audio \
            libsdecoder     \
            libcdx_base     \
            libcdx_stream   \
            libcdx_parser   \
            libVE           \
            libMemAdapter

#libve
LOCAL_MODULE:= demoPlayer

include $(BUILD_EXECUTABLE)
