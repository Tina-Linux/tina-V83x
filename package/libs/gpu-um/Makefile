#
# Copyright (C) 2018 Allwinner Technology Limited. All rights reserved
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# Author: Albert Yu <yuxyun@allwinnertech.com>
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gpu
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)/Default
	SECTION:=libs
	CATEGORY:=Libraries
	SUBMENU:=GPU Libraries
endef

define Package/$(PKG_NAME)/description/Default
endef

ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), r16 r40 r18 r311 t7))
	GPU_TYPE:=mali400
else ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), r30))
	GPU_TYPE:=mali-t760
else ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), r58))
	GPU_TYPE:=sgx544
else ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), mr813))
	GPU_TYPE:=ge8300
endif

define Build/Configure
endef

define Build/Compile
endef

ifeq ($(CONFIG_WESTON_DRM),y)
	WINDOW_SYSTEM_TYPE:=wayland
else
	WINDOW_SYSTEM_TYPE:=fbdev
endif

GPU_3RDPARTY_INCLUDE_DIR:=$(GPU_TYPE)/3rdparty/include/khronos
GPU_INCLUDE_DIR:=$(GPU_TYPE)/$(WINDOW_SYSTEM_TYPE)/include
ifeq ($(CONFIG_COMPLILE_KERNEL64_USER64),y)
GPU_LIB_DIR:=$(GPU_TYPE)/$(WINDOW_SYSTEM_TYPE)/$(CONFIG_LIBC)/lib64
else ifeq ($(TARGET_ARCH),aarch64)
GPU_LIB_DIR:=$(GPU_TYPE)/$(WINDOW_SYSTEM_TYPE)/$(CONFIG_LIBC)/lib64
else ifeq ($(TARGET_ARCH),arm)
GPU_LIB_DIR:=$(GPU_TYPE)/$(WINDOW_SYSTEM_TYPE)/$(CONFIG_LIBC)/lib
else
$(error Invalid TARGET_ARCH $(TARGET_ARCH))
endif

#####################################################################
#For Mali-400 platforms
define Package/mali400-um
	$(Package/$(PKG_NAME)/Default)
	TITLE:=Mali-400 userspace libraries
	DEPENDS:=+kmod-mali-utgard-km +WESTON_DRM:wayland +WESTON_DRM:libdrm
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include
	$(CP) -v $(GPU_3RDPARTY_INCLUDE_DIR)/* $(1)/usr/include
ifeq ($(filter $(TARGET_PLATFORM), r58 mr813),)
	$(CP) -v $(GPU_INCLUDE_DIR)/* $(1)/usr/include
endif

	$(CP) -uvd $(GPU_LIB_DIR)/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) -v pkgconfig/*.pc $(1)/usr/lib/pkgconfig
ifeq ($(WINDOW_SYSTEM_TYPE),wayland)
	$(CP) -v $(GPU_TYPE)/wayland/pkgconfig/*.pc $(1)/usr/lib/pkgconfig
endif
endef

define Package/mali400-um/install
	$(INSTALL_DIR) $(1)/usr/lib

	$(CP) -uvd $(GPU_LIB_DIR)/* $(1)/usr/lib
endef

#####################################################################
#For Mali-T760 platforms
define Package/mali-t760-um
	$(Package/$(PKG_NAME)/Default)
	TITLE:=Mali-T760 userspace libraries
	DEPENDS:=+kmod-mali-midgard-km
endef

define Package/mali-t760-um/install
	$(INSTALL_DIR) $(1)/usr/lib

	$(CP) -uvd $(GPU_LIB_DIR)/* $(1)/usr/lib
endef

#####################################################################
#For SGX-544 platforms
define Package/sgx544-um
	$(Package/$(PKG_NAME)/Default)
	TITLE:=SGX-544 userspace libraries
	DEPENDS:=+kmod-sgx544-km +libstdcpp
endef

define Package/sgx544-um/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib

	$(CP) -uvd $(GPU_LIB_DIR)/* $(1)/usr/lib
	$(CP) -uv $(GPU_LIB_DIR)/../bin/* $(1)/usr/bin
endef

#####################################################################

#####################################################################
#For GE8300 platforms
define Package/ge8300-um
	$(Package/$(PKG_NAME)/Default)
	TITLE:=GE8300 userspace libraries
	DEPENDS:=+kmod-ge8300-km +libstdcpp
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include
	$(CP) -v $(GPU_3RDPARTY_INCLUDE_DIR)/* $(1)/usr/include

	$(CP) -uvd $(GPU_LIB_DIR)/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) -v pkgconfig/*.pc $(1)/usr/lib/pkgconfig
	$(INSTALL_DIR) $(1)/lib/firmware
	$(CP) $(GPU_LIB_DIR)/firmware/* $(1)/lib/firmware
endef

define Package/ge8300-um/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib
	$(INSTALL_DIR) $(1)/lib/firmware
	$(CP) -uvd $(GPU_LIB_DIR)/* $(1)/usr/lib
	$(CP) -uv $(GPU_LIB_DIR)/../bin/* $(1)/usr/bin
	$(CP) $(GPU_LIB_DIR)/firmware/* $(1)/lib/firmware
endef
#####################################################################

ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), r16 r40 r18 r311 t7))
$(eval $(call BuildPackage,mali400-um))
else ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), r30))
$(eval $(call BuildPackage,mali-t760-um))
else ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), r58))
$(eval $(call BuildPackage,sgx544-um))
else ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), mr813))
$(eval $(call BuildPackage,ge8300-um))
endif
