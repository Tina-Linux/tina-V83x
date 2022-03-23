include $(TOPDIR)/rules.mk

PKG_NAME:=SuiteSparse
PKG_VERSION:=5.6.0
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=SuiteSparse
  DEPENDS:=+libstdcpp +libgfortran +libgomp +lapack +OpenBLAS
  URL:=http://faculty.cse.tamu.edu/davis/suitesparse.html
endef

define Package/$(PKG_NAME)/description
  SuiteSparse is a suite of sparse matrix algorithms.
endef

TARGET_CFLAGS+= -fPIC
TARGET_LDFLAGS+= \
	-L$(PKG_BUILD_DIR)/lib -lgfortran

CMAKE_OPTIONS += -DCMAKE_INSTALL_PREFIX=/
CMAKE_OPTIONS += -DCMAKE_BINARY_DIR=$(PKG_BUILD_DIR)
CMAKE_OPTIONS += -DCMAKE_INSTALL_BINDIR=/usr/bin
CMAKE_OPTIONS += -DCMAKE_INSTALL_LIBDIR=/usr/lib
CMAKE_OPTIONS += -DCMAKE_INSTALL_INCLUDEDIR=/usr/include

define Build/Compile
	+$(MAKE) library -C $(PKG_BUILD_DIR) \
		CC=$(TARGET_CC) \
		CXX="$(TARGET_CXX)" \
		AR="$(TARGET_AR)" \
		AS="$(TARGET_AS)" \
		RANLIB=$(TARGET_RANLIB) \
		LD=$(TARGET_LD) \
		HOSTCC=$(HOSTCC) \
		PREFIX=$(PKG_INSTALL_DIR) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CXXFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CMAKE_OPTIONS="$(CMAKE_OPTIONS)" \
		JOBS=32
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/bin

	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/* $(1)/usr/lib

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/* $(1)/usr/bin
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/bin

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/* $(1)/usr/lib

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/* $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
