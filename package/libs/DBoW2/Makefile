include $(TOPDIR)/rules.mk

PKG_NAME:=DBoW2
PKG_VERSION:=master

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_LICENSE:=BSD

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+opencv
  TITLE:=Modified DBoW2 which can load/save binary vocabulary files
  URL:=https://github.com/shinsumicco/DBoW2.git
endef

define Package/$(PKG_NAME)/description
DBoW2 is an improved version of the DBoW library, an open source C++ library for indexing
and converting images into a bag-of-word representation. It implements a hierarchical tree
for approximating nearest neighbours in the image feature space and creating a visual vocabulary.
DBoW2 also implements an image database with inverted and direct files to index images and enabling
quick queries and feature comparisons.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
