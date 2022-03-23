#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wolfssl
PKG_VERSION:=4.1.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.wolfssl.com/
PKG_MD5SUM:=d07e6bf7ba971363f776dfa1cd5538ea

PKG_FIXUP:=libtool
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPL-2.0+

include $(BUILD_DIR)/package.mk

define Package/libcyassl
  SECTION:=libs
  SUBMENU:=SSL
  CATEGORY:=Libraries
  TITLE:=CyaSSL library
  URL:=http://www.wolfssl.com/
  MENU:=1
endef

define Package/libcyassl/description
CyaSSL is an SSL library optimized for small footprint, both on disk and for
memory use.
endef

TARGET_CFLAGS += $(FPIC) -DWOLFSSL_ALLOW_NO_SUITES

CONFIGURE_ARGS += \
	--without-zlib \
	--enable-singlethreaded \
 	--disable-extended-master \
	--disable-examples

ifdef CONFIG_CYASSL_WITH_STATIC
  CONFIGURE_ARGS += --enable-static
endif

ifdef CONFIG_CYASSL_WITH_DEBUG
  CONFIGURE_ARGS += --enable-debug
endif

ifdef CONFIG_CYASSL_WITH_AFALG
  CONFIGURE_ARGS += --enable-afalg

 ifdef CONFIG_CYASSL_WITH_ZERO_COPY
   TARGET_CFLAGS +=  -DWOLFSSL_AFALG_ZERO_COPY
 endif

endif

CONFIGURE_ARGS += $(if $(CONFIG_CYASSL_WITH_SSLV3), --enable-sslv3, --disable-sslv3) \
                  $(if $(CONFIG_CYASSL_WITH_SINGLETHREADED), \
                         	  --enable-singlethreaded, --disable-singlethreaded) \
                  $(if $(CONFIG_CYASSL_WITH_OPENSSLCOEXIST), \
                         	  --enable-opensslcoexist, --disable-opensslcoexist) \
                  $(if $(CONFIG_CYASSL_WITH_OPENSSLALL), --enable-opensslall, --disable-opensslall) \
                  $(if $(CONFIG_CYASSL_WITH_OPENSSLEXTRA), \
                         	  --enable-opensslextra, --disable-opensslextra) \
                  $(if $(CONFIG_CYASSL_WITH_FILESYSTEM), --enable-filesystem, --disable-filesystem) \
                  $(if $(CONFIG_CYASSL_WITH_IDEA),--enable-idea, --disable-idea) \
                  $(if $(CONFIG_CYASSL_WITH_ARC4), --enable-arc4, --disable-arc4) \
                  $(if $(CONFIG_CYASSL_WITH_CMAC), --enable-cmac, --disable-cmac) \
                  $(if $(CONFIG_CYASSL_WITH_AES), --enable-aes, --disable-aes) \
                  $(if $(CONFIG_CYASSL_WITH_XTS), --enable-xts, --disable-xts) \
                  $(if $(CONFIG_CYASSL_WITH_AESCBC), --enable-aescbc, --disable-aescbc) \
                  $(if $(CONFIG_CYASSL_WITH_AESGCM), --enable-aesgcm, --disable-aesgcm) \
                  $(if $(CONFIG_CYASSL_WITH_AESCTR),--enable-aesctr, --disable-aesctr) \
                  $(if $(CONFIG_CYASSL_WITH_AESCFB), --enable-aescfb, --disable-aescfb) \
                  $(if $(CONFIG_CYASSL_WITH_AESNI), --enable-aesni, --disable-aesni) \
                  $(if $(CONFIG_CYASSL_WITH_AESKEYWRAP), --enable-aeskeywrap, --disable-aeskeywrap) \
                  $(if $(CONFIG_CYASSL_WITH_HASHDRBG), --enable-hashdrbg, --disable-hashdrbg) \
                  $(if $(CONFIG_CYASSL_WITH_SHA224), --enable-sha224, --disable-sha224) \
                  $(if $(CONFIG_CYASSL_WITH_SHA3), --enable-sha3, --disable-sha3) \
                  $(if $(CONFIG_CYASSL_WITH_SHA512), --enable-sha512, --disable-sha512) \
                  $(if $(CONFIG_CYASSL_WITH_SHA384), --enable-sha384, --disable-sha384) \
                  $(if $(CONFIG_CYASSL_WITH_MD2), --enable-md2, --disable-md2) \
                  $(if $(CONFIG_CYASSL_WITH_MD4), --enable-md4, --disable-md4) \
                  $(if $(CONFIG_CYASSL_WITH_MD5), --enable-md5, --disable-md5) \
                  $(if $(CONFIG_CYASSL_WITH_SHA), --enable-sha, --disable-sha) \
                  $(if $(CONFIG_CYASSL_WITH_OAEP),--enable-oaep, --disable-oaep) \
                  $(if $(CONFIG_CYASSL_WITH_RSAVFY), --enable-rsavfy, --disable-rsavfy) \
                  $(if $(CONFIG_CYASSL_WITH_RSAPSS), --enable-rsapss, --disable-rsapss) \
                  $(if $(CONFIG_CYASSL_WITH_DH), --enable-dh, --disable-dh) \
                  $(if $(CONFIG_CYASSL_WITH_POLY1305), --enable-poly1305, --disable-poly1305) \
                  $(if $(CONFIG_CYASSL_WITH_CHACHA), --enable-chacha, --disable-chacha) \
                  $(if $(CONFIG_CYASSL_WITH_ECCSHAMIR), --enable-eccshamir, --disable-eccshamir) \
                  $(if $(CONFIG_CYASSL_WITH_PKCS7), --enable-pkcs7, --disable-pkcs7) \
                  $(if $(CONFIG_CYASSL_WITH_PKCS11), --enable-pkcs11, --disable-pkcs11) \
                  $(if $(CONFIG_CYASSL_WITH_PKCS12), --enable-pkcs12, --disable-pkcs12) \
                  $(if $(CONFIG_CYASSL_WITH_OLDTLS), --enable-oldtls, --disable-oldtls) \
                  $(if $(CONFIG_CYASSL_WITH_TLSV10), --enable-tlsv10, --disable-tlsv10) \
                  $(if $(CONFIG_CYASSL_WITH_TLS13), --enable-tls13, --disable-tls13) \
                  $(if $(CONFIG_CYASSL_WITH_TLS13_DRAFT18), \
                  	--enable-tls13_draft18, --disable-tls13_draft18) \
                  $(if $(CONFIG_CYASSL_WITH_TLS13_DRAFT22), \
                  	--enable-tls13_draft22, --disable-tls13_draft22) \
                  $(if $(CONFIG_CYASSL_WITH_TLS13_DRAFT23), \
                  	--enable-tls13_draft23, --disable-tls13_draft23) \
                  $(if $(CONFIG_CYASSL_WITH_TLS13_DRAFT26), \
                  	--enable-tls13_draft26, --disable-tls13_draft26) \
                  $(if $(CONFIG_CYASSL_WITH_TLS13_DRAFT28), \
                  			--enable-tls13_draft28, --disable-tls13_draft28)

ifdef CONFIG_CYASSL_WITH_ASN

  ifdef CONFIG_CYASSL_WITH_RSA
    CONFIGURE_ARGS += --enable-rsa
  else
    CONFIGURE_ARGS += --disable-rsa
  endif

  ifdef CONFIG_CYASSL_WITH_DSA
    CONFIGURE_ARGS += --enable-dsa
  else
    CONFIGURE_ARGS += --disable-dsa
  endif

  ifdef CONFIG_CYASSL_WITH_ECC
    CONFIGURE_ARGS += --enable-ecc
  else
    CONFIGURE_ARGS += --disable-ecc
  endif

  CONFIGURE_ARGS += --enable-asn
else

  CONFIGURE_ARGS += --disable-asn
  CONFIGURE_ARGS += --disable-rsa
  CONFIGURE_ARGS += --disable-dsa
  CONFIGURE_ARGS += --disable-ecc
  CONFIGURE_ARGS += --enable-psk

endif

ifneq ($(CONFIG_TARGET_x86),)
	CONFIGURE_ARGS += --enable-intelasm
endif
ifneq ($(CONFIG_TARGET_x86_64),)
	CONFIGURE_ARGS += --enable-intelasm
endif


define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
ifndef CONFIG_CYASSL_WITH_STATIC
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libwolfssl.{so*,la} $(1)/usr/lib/
	ln -s libwolfssl.so $(1)/usr/lib/libcyassl.so
else
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libwolfssl.{a*,la} $(1)/usr/lib/
	ln -s libwolfssl.a $(1)/usr/lib/libcyassl.a
endif
	ln -s libwolfssl.la $(1)/usr/lib/libcyassl.la
endef

define Package/libcyassl/install
	$(INSTALL_DIR) $(1)/usr/lib
ifndef CONFIG_CYASSL_WITH_STATIC
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libwolfssl.so* $(1)/usr/lib/
	ln -s libwolfssl.so $(1)/usr/lib/libcyassl.so
endif
endef

define Package/libcyassl/config
source "$(SOURCE)/Config.in"
endef

$(eval $(call BuildPackage,libcyassl))
