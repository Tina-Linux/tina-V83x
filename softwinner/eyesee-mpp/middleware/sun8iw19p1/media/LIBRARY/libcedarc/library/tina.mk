# Makefile for eyesee-mpp/middleware/media/LIBRARY/libcedarc/library
CUR_PATH := .
PACKAGE_TOP := $(PACKAGE_TOP)
EYESEE_MPP_INCLUDE:=$(STAGING_DIR)/usr/include/eyesee-mpp
EYESEE_MPP_LIBDIR:=$(STAGING_DIR)/usr/lib/eyesee-mpp
# STAGING_DIR is exported in rules.mk, so it can be used directly here.
# STAGING_DIR:=.../tina-v316/out/v316-perfnor/staging_dir/target

include $(PACKAGE_TOP)/config/mpp_config.mk
include $(CUR_PATH)/../config.mk
#set source files here.
SRCCS :=

#include directories
INCLUDE_DIRS :=

LOCAL_SHARED_LIBS :=

LOCAL_STATIC_LIBS :=

LOCAL_PREBUILT_LIBS :=
ifeq ($(MPPCFG_COMPILE_DYNAMIC_LIB), Y)
    LOCAL_PREBUILT_LIBS += \
        $(CUR_PATH)/libVE.so
  ifeq ($(MPPCFG_VENC),Y)
    LOCAL_PREBUILT_LIBS += \
        $(CUR_PATH)/libvenc_codec.so
  endif
  ifeq ($(MPPCFG_VDEC),Y)
    LOCAL_PREBUILT_LIBS += \
        $(CUR_PATH)/libawh264.so \
        $(CUR_PATH)/libawh265.so \
        $(CUR_PATH)/libawmjpegplus.so \
        $(CUR_PATH)/libvideoengine.so
  endif
endif
ifeq ($(MPPCFG_COMPILE_STATIC_LIB), Y)
    LOCAL_PREBUILT_LIBS += \
        $(CUR_PATH)/libVE.a
  ifeq ($(MPPCFG_VENC),Y)
    LOCAL_PREBUILT_LIBS += \
        $(CUR_PATH)/libvenc_codec.a
  endif
  ifeq ($(MPPCFG_VDEC),Y)
    LOCAL_PREBUILT_LIBS += \
        $(CUR_PATH)/libawh264.a \
        $(CUR_PATH)/libawh265.a \
        $(CUR_PATH)/libawmjpegplus.a \
        $(CUR_PATH)/libvideoengine.a
  endif
endif

#set dst file name: shared library, static library, execute bin.
LOCAL_TARGET_DYNAMIC :=
LOCAL_TARGET_STATIC :=
LOCAL_TARGET_BIN :=

#generate include directory flags for gcc.
inc_paths := $(foreach inc,$(filter-out -I%,$(INCLUDE_DIRS)),$(addprefix -I, $(inc))) \
                $(filter -I%, $(INCLUDE_DIRS))
#Extra flags to give to the C compiler
LOCAL_CFLAGS := $(CFLAGS) $(inc_paths) -fPIC -Wall
#Extra flags to give to the C++ compiler
LOCAL_CXXFLAGS := $(CXXFLAGS) $(inc_paths) -fPIC -Wall
#Extra flags to give to the C preprocessor and programs that use it (the C and Fortran compilers).
LOCAL_CPPFLAGS := $(CPPFLAGS)
#target device arch: x86, arm
LOCAL_TARGET_ARCH := $(ARCH)
#Extra flags to give to compilers when they are supposed to invoke the linker,‘ld’.
LOCAL_LDFLAGS := $(LDFLAGS)

LOCAL_DYNAMIC_LDFLAGS := $(LOCAL_LDFLAGS) -shared \
    -L $(EYESEE_MPP_LIBDIR) \
    -Wl,-Bstatic \
    -Wl,--start-group $(foreach n, $(LOCAL_STATIC_LIBS), -l$(patsubst lib%,%,$(patsubst %.a,%,$(notdir $(n))))) -Wl,--end-group \
    -Wl,-Bdynamic \
    $(foreach y, $(LOCAL_SHARED_LIBS), -l$(patsubst lib%,%,$(patsubst %.so,%,$(notdir $(y)))))

#generate object files
OBJS := $(SRCCS:%=%.o) #OBJS=$(patsubst %,%.o,$(SRCCS))

#add dynamic lib name suffix and static lib name suffix.
target_dynamic := $(if $(LOCAL_TARGET_DYNAMIC),$(LOCAL_TARGET_DYNAMIC).so,)
target_static := $(if $(LOCAL_TARGET_STATIC),$(LOCAL_TARGET_STATIC).a,)

#generate exe file.
.PHONY: all
all: $(target_dynamic) $(target_static)
	if [ -n "$(LOCAL_PREBUILT_LIBS)" ]; then \
		mkdir -p $(CUR_PATH)/out; \
		cp -f $(LOCAL_PREBUILT_LIBS) $(CUR_PATH)/out; \
	fi
	@echo ===================================
	@echo build eyesee-mpp-middleware-media-LIBRARY-libcedarc-library done
	@echo ===================================

$(target_dynamic): $(OBJS)
	$(CXX) $+ $(LOCAL_DYNAMIC_LDFLAGS) -o $@
	@echo ----------------------------
	@echo "finish target: $@"
#	@echo "object files:  $+"
#	@echo "source files:  $(SRCCS)"
	@echo ----------------------------

$(target_static): $(OBJS)
	$(AR) -rcs -o $@ $+
	@echo ----------------------------
	@echo "finish target: $@"
#	@echo "object files:  $+"
#	@echo "source files:  $(SRCCS)"
	@echo ----------------------------

#patten rules to generate local object files
%.cpp.o: %.cpp
	$(CXX) $(LOCAL_CXXFLAGS) $(LOCAL_CPPFLAGS) -c -o $@ $<
%.cc.o: %.cc
	$(CXX) $(LOCAL_CXXFLAGS) $(LOCAL_CPPFLAGS) -c -o $@ $<

%.c.o: %.c
	$(CC) $(LOCAL_CFLAGS) $(LOCAL_CPPFLAGS) -c -o $@ $<

# clean all
.PHONY: clean
clean:
	-rm -f $(OBJS) $(target_dynamic) $(target_static)
	-rm -rf $(CUR_PATH)/out