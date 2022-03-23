##############################################
# OpenWrt Makefile for eyesee-mpp-middleware
#
#
# Most of the variables used here are defined in
# the include directives below. We just need to
# specify a basic description of the package,
# where to build our program, where to find
# the source files, and where to install the
# compiled program on the router.
#
# Be very careful of spacing in this file.
# Indents should be tabs, not spaces, and
# there should be no trailing whitespace in
# lines that are not commented.
#
##############################################
include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

# Name and release number of this package
PKG_NAME:=eyesee-mpp-middleware
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_MAINTAINER:=AW-PDC-PD5
# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)
SRC_COMPILE_DIR := $(PKG_BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(TOPDIR)/softwinner/eyesee-mpp/middleware
ifeq ($(TARGET_CHIP), sun8iw12p1)
    SRC_CODE_DIR := $(SRC_CODE_DIR)/v5
else ifeq ($(TARGET_CHIP), sun8iw16p1)
    SRC_CODE_DIR := $(SRC_CODE_DIR)/v316
else ifeq ($(TARGET_CHIP), sun8iw19p1)
    SRC_CODE_DIR := $(SRC_CODE_DIR)/sun8iw19p1
else
#    $(error fatal error! $(TARGET_CHIP) is not support by eyesee-mpp!)
endif

# This specifies the directory where file depends. when files in it update, increasing compile will be enable.
PKG_FILE_DEPENDS:= $(SRC_CODE_DIR)

include $(BUILD_DIR)/package.mk

export TARGET_BOARD_PLATFORM
export LICHEE_PLATFORM_OS:=tina
ifeq ($(CONFIG_imx317),y)
    SENSOR_NAME:=imx317
else ifeq ($(CONFIG_imx258),y)
    SENSOR_NAME:=imx258
else ifeq ($(CONFIG_imx278),y)
    SENSOR_NAME:=imx278
else ifeq ($(CONFIG_imx386),y)
    SENSOR_NAME:=imx386
else ifeq ($(CONFIG_sc2232),y)
    SENSOR_NAME:=sc2232
else ifeq ($(CONFIG_imx335),y)
    SENSOR_NAME:=imx335
else ifeq ($(CONFIG_gc2053),y)
    SENSOR_NAME:=gc2053
else
    SENSOR_NAME:=
endif
export SENSOR_NAME

############################################################################
#below items are used for mpp configuration of previous EyeseeLinux,
#for smooth migration to tina, we reserve these config items, define them here and export.
#ref to mpp_config.mk of EyeseeLinux.
#[config item], valid in makefile.
#config chip version: AW1721
ifeq ($(TARGET_CHIP), sun8iw12p1)
    MPPCFG_CHIP := AW1721
else ifeq ($(TARGET_CHIP), sun8iw16p1)
    MPPCFG_CHIP := AW1816
else ifeq ($(TARGET_CHIP), sun8iw19p1)
    MPPCFG_CHIP := AW1817
else
#    $(error fatal error! $(TARGET_CHIP) is not support by eyesee-mpp!)
endif
export MPPCFG_CHIP
#config write file method: LINUX_VFS, DIRECT_IO
ifeq ($(CONFIG_mpp_fwrite_method_vfs),y)
    MPPCFG_FILE_SYSTEM := LINUX_VFS
else ifeq ($(CONFIG_mpp_fwrite_method_directIO),y)
    MPPCFG_FILE_SYSTEM := DIRECT_IO
else
    MPPCFG_FILE_SYSTEM := LINUX_VFS
endif
export MPPCFG_FILE_SYSTEM
#config if use hw display method: true, false
ifeq ($(CONFIG_mpp_hw_display),y)
    MPPCFG_HW_DISPLAY := true
else
    MPPCFG_HW_DISPLAY := false
endif
export MPPCFG_HW_DISPLAY
#config if enable mpp_vi
ifeq ($(CONFIG_mpp_vi),y)
    MPPCFG_VI := Y
else
    MPPCFG_VI := N
endif
export MPPCFG_VI
#config if enable mpp_vo
ifeq ($(CONFIG_mpp_vo),y)
    MPPCFG_VO := Y
else
    MPPCFG_VO := N
endif
export MPPCFG_VO
#config if enable mpp_textEnc
ifeq ($(CONFIG_mpp_textenc),y)
    MPPCFG_TEXTENC := Y
else
    MPPCFG_TEXTENC := N
endif
export MPPCFG_TEXTENC
#config if enable mpp_venc
ifeq ($(CONFIG_mpp_venc),y)
    MPPCFG_VENC := Y
else
    MPPCFG_VENC := N
endif
export MPPCFG_VENC
#config if enable mpp_venc
ifeq ($(CONFIG_mpp_vdec),y)
    MPPCFG_VDEC := Y
else
    MPPCFG_VDEC := N
endif
export MPPCFG_VDEC
#config if enable mpp_muxer
ifeq ($(CONFIG_mpp_muxer),y)
    MPPCFG_MUXER := Y
else
    MPPCFG_MUXER := N
endif
export MPPCFG_MUXER
#config if enable mpp_demuxer
ifeq ($(CONFIG_mpp_demuxer),y)
    MPPCFG_DEMUXER := Y
else
    MPPCFG_DEMUXER := N
endif
export MPPCFG_DEMUXER
#config if include libmpp_ise.so and enable ise function.
ifeq ($(CONFIG_mpp_ise),y)
    MPPCFG_ISE := Y
else
    MPPCFG_ISE := N
endif
export MPPCFG_ISE
#config if include libmpp_ise.so and enable ise mo module.
ifeq ($(CONFIG_mpp_ise_mo),y)
    MPPCFG_ISE_MO := Y
else
    MPPCFG_ISE_MO := N
endif
export MPPCFG_ISE_MO
#config if include libmpp_ise.so and enable ise bi module.
ifeq ($(CONFIG_mpp_ise_bi),y)
    MPPCFG_ISE_BI := Y
else
    MPPCFG_ISE_BI := N
endif
export MPPCFG_ISE_BI
#config if include libmpp_ise.so and enable ise bi soft module.
ifeq ($(CONFIG_mpp_ise_bi_soft),y)
    MPPCFG_ISE_BI_SOFT := Y
else
    MPPCFG_ISE_BI_SOFT := N
endif
export MPPCFG_ISE_BI_SOFT
#config if include libmpp_ise.so and enable ise sti module.
ifeq ($(CONFIG_mpp_ise_sti),y)
    MPPCFG_ISE_STI := Y
else
    MPPCFG_ISE_STI := N
endif
export MPPCFG_ISE_STI
#config if include libmpp_eis.so and enable eis function.(Electronic Image Stabilization)
ifeq ($(CONFIG_mpp_eis),y)
    MPPCFG_EIS := Y
else
    MPPCFG_EIS := N
endif
export MPPCFG_EIS
#config if include libai_MOD.so and enable MOD(Motion Object Detection) detect.
ifeq ($(CONFIG_mpp_mod),y)
    MPPCFG_MOD := Y
else
    MPPCFG_MOD := N
endif
export MPPCFG_MOD
#config if include libai_VLPR.so and enable VLPR(Vehicle License Plate Recognition) detect.
ifeq ($(CONFIG_mpp_vlpr),y)
    MPPCFG_VLPR := Y
else
    MPPCFG_VLPR := N
endif
export MPPCFG_VLPR
#config if include libawmd.so and enable motion  detect.
ifeq ($(CONFIG_mpp_motion_detect_soft),y)
    MPPCFG_MOTION_DETECT_SOFT := Y
else
    MPPCFG_MOTION_DETECT_SOFT := N
endif
export MPPCFG_MOTION_DETECT_SOFT
#config if include libadas.so and enable adas  detect.
ifeq ($(CONFIG_mpp_adas_detect),y)
    MPPCFG_ADAS_DETECT := Y
else
    MPPCFG_ADAS_DETECT := N
endif
export MPPCFG_ADAS_DETECT
#config if include libadas-v2.so and enable adas-v2  detect.
ifeq ($(CONFIG_mpp_adas_detect_v2),y)
    MPPCFG_ADAS_DETECT_V2 := Y
else
    MPPCFG_ADAS_DETECT_V2 := N
endif
export MPPCFG_ADAS_DETECT_V2
#config if include libai_HCNT.so and enable HCNT(human count) detect.
ifeq ($(CONFIG_mpp_hcnt),y)
    MPPCFG_HCNT := Y
else
    MPPCFG_HCNT := N
endif
export MPPCFG_HCNT
#config if include libai_BDII.so and enable BDII() detect.
ifeq ($(CONFIG_mpp_bdii),y)
    MPPCFG_BDII := Y
else
    MPPCFG_BDII := N
endif
export MPPCFG_BDII
#config if include libeve_face.so and enable face detect.
ifeq ($(CONFIG_mpp_eveface),y)
    MPPCFG_EVEFACE := Y
else
    MPPCFG_EVEFACE := N
endif
export MPPCFG_EVEFACE
#config if enable mpp_aec
ifeq ($(CONFIG_mpp_aec),y)
    MPPCFG_AEC := Y
else
    MPPCFG_AEC := N
endif
export MPPCFG_AEC
#config if enable mpp_ans
ifeq ($(CONFIG_mpp_ans),y)
    MPPCFG_ANS := Y
else
    MPPCFG_ANS := N
endif
export MPPCFG_ANS

#config ans library of webrtc if enable mpp_ans_libwebrtc
ifeq ($(CONFIG_mpp_ans_libwebrtc),y)
    MPPCFG_ANS_LIB := libwebrtc
endif

#config ans library of lstm if enable mpp_ans_liblstm
ifeq ($(CONFIG_mpp_ans_liblstm),y)
    MPPCFG_ANS_LIB := liblstm
endif
export MPPCFG_ANS_LIB

#config if include libmpp_uvc.so
ifeq ($(CONFIG_mpp_uvc),y)
    MPPCFG_UVC := Y
else
    MPPCFG_UVC := N
endif
export MPPCFG_UVC

#config if enable iommu
ifeq ($(CONFIG_mpp_iommu),y)
    MPPCFG_USE_IOMMU := Y
else
    MPPCFG_USE_IOMMU := N
endif
export MPPCFG_USE_IOMMU
#prebuild all AI libs and component by static
ifeq ($(CONFIG_mpp_compile_static_lib),y)
    MPPCFG_COMPILE_STATIC_LIB := Y
else
    MPPCFG_COMPILE_STATIC_LIB := N
endif
export MPPCFG_COMPILE_STATIC_LIB
#build component to dynamic so if possible
ifeq ($(CONFIG_mpp_compile_dynamic_lib),y)
    MPPCFG_COMPILE_DYNAMIC_LIB := Y
else
    MPPCFG_COMPILE_DYNAMIC_LIB := N
endif
export MPPCFG_COMPILE_DYNAMIC_LIB
ifeq ($(CONFIG_mpp_systrace),y)
    MPPCFG_SYSTRACE := Y
else
    MPPCFG_SYSTRACE := N
endif
export MPPCFG_SYSTRACE
# Specify package information for this program.
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/$(PKG_NAME)
  SECTION:=eyesee-mpp-middleware
  CATEGORY:=Allwinner
  SUBMENU:=eyesee-mpp
  TITLE:=eyesee-mpp-middleware for Tina Linux
  DEPENDS:= +eyesee-mpp-system +zlib +alsa-lib +mpp_systrace:awsystrace
endef

define Package/$(PKG_NAME)/config
source "$(SOURCE)/Config.in"
endef

# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/$(PKG_NAME)/description
  eyesee-mpp-middleware contain eyesee-mpp basic libraries.
endef

# Specify what needs to be done to prepare for building the package.
# In our case, we need to copy the source files to the build directory.
# This is NOT the default.  The default uses the PKG_SOURCE_URL and the
# PKG_SOURCE which is not defined here to download the source from the web.
# In order to just build a simple program that we have just written, it is
# much easier to do it this way.

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	if [ -d $(SRC_COMPILE_DIR) ]; then \
		rm -rf $(SRC_COMPILE_DIR); \
	fi
	# copy custom mpp_config.mk to source code directory.
	#if [ -f $(TOPDIR)/target/allwinner/$(BOARD)/custom-config/mpp_config.mk ]; then \
	#	echo "copy happen!"; \
	#	$(CP) $(TOPDIR)/target/allwinner/$(BOARD)/custom-config/mpp_config.mk $(SRC_CODE_DIR)/config/mpp_config.mk; \
	#fi
	ln -s $(SRC_CODE_DIR) $(SRC_COMPILE_DIR)
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(SRC_COMPILE_DIR) -f tina.mk \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CXXFLAGS="$(TARGET_CXXFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers" \
		all
endef

define Build/Clean
	if [ -d $(SRC_COMPILE_DIR) ]; then \
		MPPCFG_COMPILE_DYNAMIC_LIB=Y \
		MPPCFG_COMPILE_STATIC_LIB=Y \
		$(MAKE) -C $(SRC_COMPILE_DIR) -f tina.mk \
			ARCH="$(TARGET_ARCH)" \
			AR="$(TARGET_AR)" \
			CC="$(TARGET_CC)" \
			CXX="$(TARGET_CXX)" \
			CFLAGS="$(TARGET_CFLAGS)" \
			CXXFLAGS="$(TARGET_CXXFLAGS)" \
			CPPFLAGS="$(TARGET_CPPFLAGS)" \
			LDFLAGS="$(TARGET_LDFLAGS)" \
			clean; \
	else \
		echo "$(SRC_COMPILE_DIR) is not exist!"; \
	fi
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib/eyesee-mpp/
	$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware
	$(CP) -r $(SRC_COMPILE_DIR)/include $(1)/usr/include/eyesee-mpp/middleware
	$(CP) -r $(SRC_COMPILE_DIR)/config $(1)/usr/include/eyesee-mpp/middleware
	$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media
	$(CP) -r $(SRC_COMPILE_DIR)/media/include $(1)/usr/include/eyesee-mpp/middleware/media
	$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/include_ai_common ]; then \
		$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/include_ai_common $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY; \
	fi
	$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/include_demux $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/include_eve_common ]; then \
		$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/include_eve_common $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY; \
	fi
	$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/include_FsWriter $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY
	$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/include_muxer $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY
	$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/include_stream $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY
	if [ -f $(SRC_COMPILE_DIR)/media/utils/libmedia_utils.so ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/utils/libmedia_utils.so $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/utils/libmedia_utils.a ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/utils/libmedia_utils.a $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libstream/libcedarxstream.so ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libstream/libcedarxstream.so $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libstream/libcedarxstream.a ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libstream/libcedarxstream.a $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libFsWriter/libFsWriter.a ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libFsWriter/libFsWriter.a $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libmuxer/common/libavutil/libffavutil.a ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libmuxer/common/libavutil/libffavutil.a $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libmuxer/mp3_muxer/libmp3_muxer.a ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libmuxer/mp3_muxer/libmp3_muxer.a $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libmuxer/aac_muxer/libaac_muxer.a ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libmuxer/aac_muxer/libaac_muxer.a $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libmuxer/mp4_muxer/libmp4_muxer.a ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libmuxer/mp4_muxer/libmp4_muxer.a $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libmuxer/mpeg2ts_muxer/libmpeg2ts_muxer.a ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libmuxer/mpeg2ts_muxer/libmpeg2ts_muxer.a $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libmuxer/raw_muxer/libraw_muxer.a ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libmuxer/raw_muxer/libraw_muxer.a $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libmuxer/muxers/libmuxers.a ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libmuxer/muxers/libmuxers.a $(1)/usr/lib/eyesee-mpp; \
	fi
	$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libisp
	$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/libisp/include $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libisp
	$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/libisp/*.h $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libisp
	$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libisp/isp_tuning
	$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/libisp/isp_tuning/*.h $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libisp/isp_tuning
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libisp/out/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libisp/out/out \( -name "*.a" -o -name "*.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libisp/iniparser/libiniparser.a $(1)/usr/lib/eyesee-mpp
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libisp/isp_cfg/libisp_ini.so ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libisp/isp_cfg/libisp_ini.so $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libisp/isp_cfg/libisp_ini.a ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libisp/isp_cfg/libisp_ini.a $(1)/usr/lib/eyesee-mpp; \
	fi
	$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libisp/isp_dev/libisp_dev.a $(1)/usr/lib/eyesee-mpp
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libisp/libISP.so ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libisp/libISP.so $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libisp/libISP.a ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libisp/libISP.a $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libMODSoft ]; then \
		$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libMODSoft; \
		$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/libMODSoft/include $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libMODSoft; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libMODSoft/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libMODSoft/out \( -name "*.a" -o -name "*.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libADAS ]; then \
		$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libADAS; \
		$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/libADAS/include $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libADAS; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libADAS/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libADAS/out \( -name "*.a" -o -name "*.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libISE
	$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/libISE/include $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libISE
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libISE/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libISE/out \( -name "*.a" -o -name "*.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/aec_lib ]; then \
		$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/aec_lib; \
		$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/aec_lib/include $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/aec_lib; \
	fi	
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/aec_lib/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/aec_lib/out \( -name "*.a" -o -name "*.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/ans_lib ]; then \
		$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/ans_lib; \
		if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/ans_lib/include ]; then \
			$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/ans_lib/include $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/ans_lib; \
		fi \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/ans_lib/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/ans_lib/out \( -name "*.a" -o -name "*.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi

	$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libcedarc
	$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/include $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libcedarc
	find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/base \( -name "libcdc_base.a" -o -name "libcdc_base.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/ve ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/ve \( -name "libVE.a" -o -name "libVE.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/library/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/library/out \( -name "*.a" -o -name "*.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/memory \( -name "libMemAdapter.a" -o -name "libMemAdapter.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vencoder/base \( -name "libvenc_base.a" -o -name "libvenc_base.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vencoder/libcodec ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vencoder/libcodec \( -name "libvenc_codec.a" -o -name "libvenc_codec.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vencoder -maxdepth 1 \( -name "libvencoder.a" -o -name "libvencoder.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vdecoder/videoengine ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vdecoder/videoengine \( -name "libvideoengine.a" -o -name "libvideoengine.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vdecoder -maxdepth 1 \( -name "libvdecoder.a" -o -name "libvdecoder.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vdecoder/videoengine/h264 ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vdecoder/videoengine/h264 \( -name "libawh264.a" -o -name "libawh264.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vdecoder/videoengine/h265 ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vdecoder/videoengine/h265 \( -name "libawh265.a" -o -name "libawh265.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vdecoder/videoengine/mjpegplus ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vdecoder/videoengine/mjpegplus \( -name "libawmjpegplus.a" -o -name "libawmjpegplus.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/base/libcdx_base.so $(1)/usr/lib/eyesee-mpp
	$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libcedarx/libcore/common/iniparser
	$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/common/iniparser/*.h $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libcedarx/libcore/common/iniparser
	$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/common/libcdx_common.so $(1)/usr/lib/eyesee-mpp
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/stream/file/libcdx_file_stream.a ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/stream/file/libcdx_file_stream.a $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/stream/base/libcdx_stream.so ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/stream/base/libcdx_stream.so $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/parser/aac/libcdx_aac_parser.a ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/parser/aac/libcdx_aac_parser.a $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/parser/id3v2/libcdx_id3v2_parser.a ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/parser/id3v2/libcdx_id3v2_parser.a $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/parser/mp3/libcdx_mp3_parser.a ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/parser/mp3/libcdx_mp3_parser.a $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/parser/mov/libcdx_mov_parser.a ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/parser/mov/libcdx_mov_parser.a $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/parser/mpg/libcdx_mpg_parser.a ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/parser/mpg/libcdx_mpg_parser.a $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/parser/ts/libcdx_ts_parser.a ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/parser/ts/libcdx_ts_parser.a $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/parser/base/libcdx_parser.so ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/parser/base/libcdx_parser.so $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libdemux/libcedarxdemuxer.a ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libdemux/libcedarxdemuxer.a $(1)/usr/lib/eyesee-mpp; \
	fi
	$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/AudioLib
	$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/AudioLib/osal $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/AudioLib
	$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/AudioLib/midware/encoding
	$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/AudioLib/midware/encoding/include $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/AudioLib/midware/encoding
	$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/AudioLib/midware/encoding/*.h $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/AudioLib/midware/encoding
	find $(SRC_COMPILE_DIR)/media/LIBRARY/AudioLib/midware/encoding -maxdepth 1 \( -name "libcedarx_aencoder.a" -o -name "libcedarx_aencoder.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/AudioLib/midware/decoding
	$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/AudioLib/midware/decoding/include $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/AudioLib/midware/decoding
	find $(SRC_COMPILE_DIR)/media/LIBRARY/AudioLib/midware/decoding -maxdepth 1 \( -name "libadecoder.a" -o -name "libadecoder.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/AudioLib/lib/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/AudioLib/lib/out \( -name "*.a" -o -name "*.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libai_common/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libai_common/out \( -name "*.a" -o -name "*.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/textEncLib
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/textEncLib/include ]; then \
		$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/textEncLib/include $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/textEncLib; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/textEncLib ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/textEncLib -maxdepth 1 \( -name "libcedarx_tencoder.a" -o -name "libcedarx_tencoder.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libaiMOD ]; then \
		$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libaiMOD; \
		$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/libaiMOD/include $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libaiMOD; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libaiMOD/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libaiMOD/out \( -name "*.a" -o -name "*.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libADAS ]; then \
		$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libADAS; \
		$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/libADAS/include $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libADAS; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libADAS/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libADAS/out \( -name "*.a" -o -name "*.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi

	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libaiHCNT ]; then \
		$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libaiHCNT; \
		$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/libaiHCNT/include $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libaiHCNT; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libaiHCNT/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libaiHCNT/out \( -name "*.a" -o -name "*.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libaiBDII ]; then \
		$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libaiBDII; \
		$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/libaiBDII/include $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libaiBDII; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libaiBDII/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libaiBDII/out \( -name "*.a" -o -name "*.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libVLPR ]; then \
		$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libVLPR; \
		$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/libVLPR/include $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libVLPR; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libVLPR/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libVLPR/out \( -name "*.a" -o -name "*.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libeveface ]; then \
		$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libeveface; \
		$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/libeveface/include $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/libeveface; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libeveface/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libeveface/out \( -name "*.a" -o -name "*.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libevekernel/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libevekernel/out \( -name "*.a" -o -name "*.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libVideoStabilization/libEIS.a ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libVideoStabilization/libEIS.a $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libVideoStabilization/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libVideoStabilization/out \( -name "*.a" -o -name "*.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/lib_aw_ai_algo/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/lib_aw_ai_algo/out \( -name "*.a" -o -name "*.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/lib_aw_ai_core ]; then \
		$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/lib_aw_ai_core/include; \
		$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/lib_aw_ai_core/include/* $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/lib_aw_ai_core/include; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/lib_aw_ai_core/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/lib_aw_ai_core/out \( -name "*.a" -o -name "*.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/lib_aw_ai_mt ]; then \
		$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/lib_aw_ai_mt/include; \
		$(CP) -r $(SRC_COMPILE_DIR)/media/LIBRARY/lib_aw_ai_mt/include/* $(1)/usr/include/eyesee-mpp/middleware/media/LIBRARY/lib_aw_ai_mt/include; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/lib_aw_ai_mt/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/lib_aw_ai_mt/out \( -name "*.a" -o -name "*.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	find $(SRC_COMPILE_DIR)/media/librender -maxdepth 1 \( -name "libcedarxrender.a" -o -name "libcedarxrender.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	find $(SRC_COMPILE_DIR)/media -maxdepth 1 -name "libmpp_vi.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	find $(SRC_COMPILE_DIR)/media -maxdepth 1 -name "libmpp_isp.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	find $(SRC_COMPILE_DIR)/media -maxdepth 1 -name "libmpp_ise.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	find $(SRC_COMPILE_DIR)/media -maxdepth 1 -name "libmpp_eis.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	find $(SRC_COMPILE_DIR)/media -maxdepth 1 -name "libmpp_vo.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	find $(SRC_COMPILE_DIR)/media -maxdepth 1 -name "libmpp_uvc.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	find $(SRC_COMPILE_DIR)/media/component -maxdepth 1 -name "libmpp_component.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	find $(SRC_COMPILE_DIR)/media -maxdepth 1 -name "libmedia_mpp.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	find $(SRC_COMPILE_DIR)/media -maxdepth 1 -name "libaw_mpp.a" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/middleware/sample/configfileparser
	$(CP) -r $(SRC_COMPILE_DIR)/sample/configfileparser/*.h $(1)/usr/include/eyesee-mpp/middleware/sample/configfileparser
	find $(SRC_COMPILE_DIR)/sample/configfileparser -maxdepth 1 -name "libsample_confparser.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
endef

# We do not need to define Build/Configure or Build/Compile directives
# The defaults are appropriate for compiling a simple program such as this one


# Specify where and how to install the program. Since we only have one file,
# the helloworld executable, install it by copying it to the /bin directory on
# the router. The $(1) variable represents the root directory on the router running
# OpenWrt. The $(INSTALL_DIR) variable contains a command to prepare the install
# directory if it does not already exist.  Likewise $(INSTALL_BIN) contains the
# command to copy the binary file from its current location (in our case the build
# directory) to the install directory.
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/eyesee-mpp
	$(INSTALL_DIR) $(1)/usr/bin
	if [ -f $(SRC_COMPILE_DIR)/media/utils/libmedia_utils.so ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/utils/libmedia_utils.so $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libstream/libcedarxstream.so ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libstream/libcedarxstream.so $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libisp/out/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libisp/out/out -name "*.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libisp/isp_cfg/libisp_ini.so ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libisp/isp_cfg/libisp_ini.so $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libisp/libISP.so ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libisp/libISP.so $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libMODSoft/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libMODSoft/out -name "*.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libADAS/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libADAS/out -name "*.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libISE/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libISE/out -name "*.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/base -name "libcdc_base.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/ve ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/ve -name "libVE.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/library/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/library/out -name "*.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/memory -name "libMemAdapter.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vencoder/base -name "libvenc_base.so"  -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vencoder/libcodec ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vencoder/libcodec -name "libvenc_codec.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vencoder -maxdepth 1 -name "libvencoder.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vdecoder/videoengine ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vdecoder/videoengine -name "libvideoengine.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vdecoder -maxdepth 1 -name "libvdecoder.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vdecoder/videoengine/h264 ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vdecoder/videoengine/h264 -name "libawh264.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vdecoder/videoengine/h265 ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vdecoder/videoengine/h265 -name "libawh265.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vdecoder/videoengine/mjpegplus ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarc/vdecoder/videoengine/mjpegplus -name "libawmjpegplus.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	$(INSTALL_DIR) $(1)/etc
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/config/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/config/out -name "cedarx.conf" -exec $(CP) {} $(1)/etc \;; \
	fi
	$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/base/libcdx_base.so $(1)/usr/lib/eyesee-mpp
	$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/common/libcdx_common.so $(1)/usr/lib/eyesee-mpp
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/stream/base/libcdx_stream.so ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/stream/base/libcdx_stream.so $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -f $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/parser/base/libcdx_parser.so ]; then \
		$(CP) $(SRC_COMPILE_DIR)/media/LIBRARY/libcedarx/libcore/parser/base/libcdx_parser.so $(1)/usr/lib/eyesee-mpp; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/AudioLib/lib/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/AudioLib/lib/out -name "*.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	find $(SRC_COMPILE_DIR)/media/LIBRARY/AudioLib/midware/encoding -maxdepth 1 -name "libcedarx_aencoder.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	find $(SRC_COMPILE_DIR)/media/LIBRARY/AudioLib/midware/decoding -maxdepth 1 -name "libadecoder.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libai_common/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libai_common/out -name "*.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi

	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/textEncLib ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/textEncLib -maxdepth 1 -name "libcedarx_tencoder.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi	

	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libaiMOD/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libaiMOD/out -name "*.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libADAS/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libADAS/out -name "*.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libaiHCNT/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libaiHCNT/out -name "*.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libaiBDII/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libaiBDII/out -name "*.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libVLPR/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libVLPR/out -name "*.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libeveface/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libeveface/out \( -name "*.so" -o -name "*.ld" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libevekernel/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libevekernel/out -name "*.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/libVideoStabilization/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/libVideoStabilization/out -name "*.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/lib_aw_ai_algo/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/lib_aw_ai_algo/out -name "*.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/lib_aw_ai_core/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/lib_aw_ai_core/out -name "*.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	if [ -d $(SRC_COMPILE_DIR)/media/LIBRARY/lib_aw_ai_mt/out ]; then \
		find $(SRC_COMPILE_DIR)/media/LIBRARY/lib_aw_ai_mt/out -name "*.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;; \
	fi
	find $(SRC_COMPILE_DIR)/media/librender -maxdepth 1 -name "libcedarxrender.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	find $(SRC_COMPILE_DIR)/media -maxdepth 1 -name "libmpp_vi.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	find $(SRC_COMPILE_DIR)/media -maxdepth 1 -name "libmpp_isp.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	find $(SRC_COMPILE_DIR)/media -maxdepth 1 -name "libmpp_ise.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	find $(SRC_COMPILE_DIR)/media -maxdepth 1 -name "libmpp_eis.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	find $(SRC_COMPILE_DIR)/media -maxdepth 1 -name "libmpp_vo.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	find $(SRC_COMPILE_DIR)/media -maxdepth 1 -name "libmpp_uvc.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	find $(SRC_COMPILE_DIR)/media/component -maxdepth 1 -name "libmpp_component.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	find $(SRC_COMPILE_DIR)/media -maxdepth 1 -name "libmedia_mpp.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	find $(SRC_COMPILE_DIR)/sample/configfileparser -maxdepth 1 -name "libsample_confparser.so" -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;
	#	$(INSTALL_BIN) $(SRC_COMPILE_DIR)/logcat/alogcat.sh $(1)/bin/alogcat
endef

#define Package/$(PKG_NAME)/InstallDev
#endef

# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.

$(eval $(call BuildPackage,$(PKG_NAME)))
