#
# Copyright (C) 2006-2015 OpenWrt.org
#
# The files in this repository are licensed under the zlib license, except for the files under 'Extras' and examples/ThirdPartyLibs.
#
# Bullet Continuous Collision Detection and Physics Library
# http://bulletphysics.org
#
# This software is provided 'as-is', without any express or implied warranty.
# In no event will the authors be held liable for any damages arising from the use of this software.
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it freely,
# subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software.
#    If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bullet3
PKG_VERSION:=2.87
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/bulletphysics/bullet3/archive/2.87.tar.gz
PKG_MD5SUM:=7566fc00d925a742e6f7ec7ba2d352de
PKG_MAINTAINER:=erwin.coumans@gmail.com

PKG_LICENSE:=Zlib
PKC_LICENSE_FILES:=LICENSE

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

PKG_BUILD_DEPENDS:=python/host

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/bullet
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libstdcpp
  TITLE:=Official C++ source code repository of the Bullet Physics SDK
  URL:=http://github.com/bulletphysics/bullet3
endef

define Package/bullet/description
Real-time collision detection and multi-physics simulation for VR, games, visual effects, robotics, machine learning etc.
endef

CMAKE_OPTIONS += \
	-DBUILD_SHARED_LIBS=ON \
	-DBUILD_PYBULLET=ON \
	-DBUILD_PYBULLET_NUMPY=OFF \
	-DUSE_DOUBLE_PRECISION=ON \
	-DCMAKE_BUILD_TYPE=Release \
	-DBUILD_UNIT_TESTS=OFF \
	-DBUILD_BULLET2_DEMOS=OFF \
	-DBUILD_BULLET3=OFF \
	-DPYTHON_INCLUDE_DIR=$(STAGING_DIR_HOST)/include/python2.7 \
	-DPYTHON_LIBRARY=$(STAGING_DIR_HOST)/lib/python2.7

define Build/InstallDev
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

define Package/bullet/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,bullet))
