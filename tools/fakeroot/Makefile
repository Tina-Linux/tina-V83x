#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=fakeroot
PKG_VERSION:=1.20.2

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.bz2
PKG_SOURCE_URL:=http://snapshot.debian.org/archive/debian/20141005T221953Z/pool/main/f/fakeroot
PKG_HASH:=7c0a164d19db3efa9e802e0fc7cdfeff70ec6d26cdbdc4338c9c2823c5ea230c

include $(BUILD_DIR)/host-build.mk

HOST_CONFIGURE_VARS += \
	ac_cv_header_sys_capability_h=no \
	ac_cv_func_capset=no

$(eval $(call HostBuild))
