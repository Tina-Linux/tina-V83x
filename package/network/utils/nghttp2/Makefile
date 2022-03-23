include $(TOPDIR)/rules.mk

PKG_NAME:=nghttp2
PKG_VERSION:=1.24.0
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2

PKG_LICENSE:=MIT

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

#CONFIGURE_ARGS += --enable-lib-only

define Package/nghttp2/description/default
  nghttp2 is an implementation of HTTP/2 and its header compression algorithm HPACK in C.

  The framing layer of HTTP/2 is implemented as a form of reusable C library. On top of
  that, we have implemented HTTP/2 client, server and proxy. We have also developed load
  test and benchmarking tool for HTTP/2 and SPDY.

  We have participated in httpbis working group since HTTP/2 draft-04, which is the first
  implementation draft. Since then we have updated nghttp2 library constantly to latest
  specification and nghttp2 is now one of the most mature HTTP/2 implementations.

  All C APIs are fully documented.

  HTTP/2 utilizes header compression method called HPACK. We offer HPACK encoder and decoder
  are available as public API.

  nghttp2 library itself is a bit low-level. The experimental high level C++ API is also available.

  We have Python binding of this libary, but we have not covered everything yet.
endef

define Package/libnghttp2
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=HTTP/2 C Library
endef

define Package/libnghttp2/description
  $(call Package/nghttp2/description/default)
endef

define Package/nghttp-tools
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:= +libopenssl +libev +zlib +libcares +libnghttp2 +@OPENSSL_WITH_EC
  TITLE:= tools in nghttp2
endef

define Package/libnghttp2/description
  tools in nghttp2, eg: nghttp nghttpd nghttpx h2load
  details see libnghttp2
endef

ifeq ($(CONFIG_PACKAGE_nghttp-tools),y)
  CMAKE_OPTIONS=-DENABLE_APP=1
else
  CMAKE_OPTIONS=-DENABLE_LIB_ONLY=1
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/nghttp2 $(1)/usr/lib $(1)/usr/share $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/nghttp2/* $(1)/usr/include/nghttp2/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnghttp2.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/* $(1)/usr/lib/pkgconfig/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share
endef

define Package/libnghttp2/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnghttp2.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libnghttp2))
$(eval $(call BuildPackage,nghttp-tools))
