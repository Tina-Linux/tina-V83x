#
# Copyright (C) 2007-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=youtube-dl
PKG_VERSION:=2018.07.04
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://yt-dl.org/downloads/$(PKG_VERSION)/
PKG_HASH:=c65209af93d8524c0da2cd7d8228830816da96471957325597653639080d0d43
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)

PKG_LICENSE:=Unlicense
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Adrian Panella <ianchi74@outlook.com>

PKG_BUILD_DEPENDS:=python/host zip/host

include $(BUILD_DIR)/package.mk

define Package/youtube-dl
  SECTION:=multimedia
  CATEGORY:=Multimedia
  TITLE:=utility to download videos from YouTube.com
  DEPENDS:=+python-openssl +python-email +python-xml +python-codecs +python-ctypes +ca-certificates
  URL:=https://youtube-dl.org
endef

define Package/youtube-dl/description
  youtube-dl is a small command-line program to download videos
  from YouTube.com and a few more sites.
  It requires the Python interpreter.
endef

# Without recompiling, the files extracted from the tarball can be executed directly, provided that python is installed.
#define Build/Compile
#	(cd $(PKG_BUILD_DIR); $(MAKE) clean)
#	(cd $(PKG_BUILD_DIR); $(MAKE) youtube-dl)
#endef

define Package/youtube-dl/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/youtube-dl $(1)/usr/bin/
endef

$(eval $(call BuildPackage,youtube-dl))
