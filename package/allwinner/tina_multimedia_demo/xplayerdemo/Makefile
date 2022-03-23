include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=xplayerdemo
PKG_VERSION:=1
PKG_RELEASE:=1


PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SUBMENU:=tina_multimedia_demo
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=use xplayerdemo in tina_multimedia
  DEPENDS:=+libcedarx
endef

define Package/$(PKG_NAME)/description
	CedarX2.8 xplayerdemo
endef

define Build/Prepare

endef

define Build/Compile
endef


define Package/$(PKG_NAME)/install
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
