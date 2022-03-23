#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mg-samples
PKG_VERSION:=3.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=http://www.minigui.org/downloads/
PKG_MD5SUM:=e14dc09754a20c11e88ddbfcf3fdb04f

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/mg-samples
  SECTION:=gui
  CATEGORY:=Minigui
  DEPENDS:= +libminigui-gpl +libmgeff +libmgncs +libmgutils +mgncs4touch
  TITLE:=mg-smaples
  MAINTAINER:=tracewong <tracewong@outlook.com>
  URL:=http://www.minigui.org/
endef

define Package/mg-samples/description
  The minigui test case
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	(cd $(PKG_BUILD_DIR) && ./autogen.sh)
endef

CONFIGURE_ARGS += \
	--with-lang=zhcn \

LDFLAGS+=-L$(STAGING_DIR)/usr/lib/ -lmgplus
LDADD+=-lmgplus
LIBADD+=-L$(STAGING_DIR)/usr/lib/ -lmgeff -lmgplus -lmgncs4touch -lmgutils
ifeq ($(CONFIG_G2D_SUPPORT),y)
TARGET_LDFLAGS+= -luapi
LDFLAGS+=-L$(STAGING_DIR)/usr/lib/ -luapi
endif
define Package/mg-samples/install
	$(INSTALL_DIR) $(1)/usr/bin/ctrl_dlg
	$(INSTALL_DIR) $(1)/usr/bin/ctrl_dlg/res
	$(CP) \
		$(PKG_BUILD_DIR)/ctrl_dlg/ctrl_dlg \
		$(1)/usr/bin/ctrl_dlg/
	$(CP) \
		$(PKG_BUILD_DIR)/ctrl_dlg/*.jpg \
		$(1)/usr/bin/ctrl_dlg/
	$(CP) \
		$(PKG_BUILD_DIR)/ctrl_dlg/res/*.{gif,ico,bmp,jpg} \
		$(1)/usr/bin/ctrl_dlg/res/

	$(INSTALL_DIR) $(1)/usr/bin/dbuff
	$(INSTALL_DIR) $(1)/usr/bin/dbuff/res
	$(CP) \
		$(PKG_BUILD_DIR)/dbuff \
		$(1)/usr/bin/dbuff/
	$(CP) \
		$(PKG_BUILD_DIR)/dbuff/res/*.png \
		$(1)/usr/bin/dbuff/res/

	$(INSTALL_DIR) $(1)/usr/bin/font_text
	$(CP) \
		$(PKG_BUILD_DIR)/font_text/*.png \
		$(1)/usr/bin/dbuff/
	$(CP) \
		$(PKG_BUILD_DIR)/font_text/font_text \
		$(1)/usr/bin/font_text/

	$(INSTALL_DIR) $(1)/usr/bin/graphics
	$(INSTALL_DIR) $(1)/usr/bin/graphics/res
	$(CP) \
		$(PKG_BUILD_DIR)/graphics/res/*.bmp \
		$(1)/usr/bin/graphics/res/
	$(CP) \
		$(PKG_BUILD_DIR)/graphics/graphics \
		$(1)/usr/bin/graphics/
	$(CP) \
		$(PKG_BUILD_DIR)/graphics/graphicsex \
		$(1)/usr/bin/graphics/

	$(INSTALL_DIR) $(1)/usr/bin/housekeeper
	$(INSTALL_DIR) $(1)/usr/bin/housekeeper/res
	$(CP) \
		$(PKG_BUILD_DIR)/housekeeper/housekeeper \
		$(1)/usr/bin/housekeeper/
	$(CP) \
		$(PKG_BUILD_DIR)/housekeeper/res/*.gif \
		$(1)/usr/bin/housekeeper/res/

	$(INSTALL_DIR) $(1)/usr/bin/lf_skin
	#$(INSTALL_DIR) $(1)/usr/bin/lf_skin/res
	$(CP) \
		$(PKG_BUILD_DIR)/lf_skin/lf_skin \
		$(1)/usr/bin/lf_skin/
	$(CP) \
		$(PKG_BUILD_DIR)/lf_skin/res \
		$(1)/usr/bin/lf_skin/

	$(INSTALL_DIR) $(1)/usr/bin/look_feel
	$(INSTALL_DIR) $(1)/usr/bin/look_feel/res
	$(CP) \
		$(PKG_BUILD_DIR)/look_feel/look_feel \
		$(1)/usr/bin/look_feel/
	$(CP) \
		$(PKG_BUILD_DIR)/look_feel/res/*.{bmp,ico,png,bmp} \
		$(1)/usr/bin/look_feel/res/

ifeq ($(CONFIG_MINIGUI_PROC),y)
	$(INSTALL_DIR) $(1)/usr/bin/mginit
	$(INSTALL_DIR) $(1)/usr/bin/mginit/res
	$(CP) \
		$(PKG_BUILD_DIR)/mginit/mginit \
		$(1)/usr/bin/mginit/
	$(CP) \
		$(PKG_BUILD_DIR)/mginit/mginit.rc \
		$(1)/usr/bin/mginit/
	$(CP) \
		$(PKG_BUILD_DIR)/mginit/res/*.{png,gif} \
		$(1)/usr/bin/mginit/res/
endif

	$(INSTALL_DIR) $(1)/usr/bin/minesweeper
	$(INSTALL_DIR) $(1)/usr/bin/minesweeper/res
	$(CP) \
		$(PKG_BUILD_DIR)/minesweeper/minesweeper \
		$(1)/usr/bin/minesweeper/
	$(CP) \
		$(PKG_BUILD_DIR)/minesweeper/res/*.bmp \
		$(1)/usr/bin/minesweeper/res/

	$(INSTALL_DIR) $(1)/usr/bin/same
	$(INSTALL_DIR) $(1)/usr/bin/same/res
	$(CP) \
		$(PKG_BUILD_DIR)/same/same \
		$(1)/usr/bin/same/
	$(CP) \
		$(PKG_BUILD_DIR)/same/res/*.gif \
		$(1)/usr/bin/same/res/

	$(INSTALL_DIR) $(1)/usr/bin/mgeff/
	$(INSTALL_DIR) $(1)/usr/bin/mgeff/res
	$(CP) \
		$(PKG_BUILD_DIR)/mgeff/{animation/animation_async,animation/animation_callback,animation/animation_control,animation/animation_curve,animation/animation_setcontext,animation/animation_setproperty,animation/animation_sync,animation/animation_window,animation/effector,animation/effector_custom,animation/group_animation_parallel,animation/group_animation_sequential,ball/ball,banner/banner,btnshow/btnshow,curve/curve,custom-effector/custom-effector,image-box/image-box,particle-sys/particle-sys} \
		$(1)/usr/bin/mgeff/
	$(CP) \
		$(PKG_BUILD_DIR)/mgeff/curve/res/*.png \
		$(1)/usr/bin/mgeff/res
	$(CP) \
		$(PKG_BUILD_DIR)/mgeff/custom-effector/res/*.png \
		$(1)/usr/bin/mgeff/res
	$(CP) \
		$(PKG_BUILD_DIR)/mgeff/effector/res/*.png \
		$(1)/usr/bin/mgeff/res
	$(CP) \
		$(PKG_BUILD_DIR)/mgeff/image-box/res/*.jpg \
		$(1)/usr/bin/mgeff/res
	$(CP) \
		$(PKG_BUILD_DIR)/mgeff/particle-sys/*.png \
		$(1)/usr/bin/mgeff/
	$(INSTALL_DIR) $(1)/usr/bin/mgncs
	$(INSTALL_DIR) $(1)/usr/bin/mgncs/res
	$(CP) \
		$(PKG_BUILD_DIR)/mgncs/{button,checkbutton,colorbutton,combobox,container,data_binding,data_source,edit,event_listener,helloworld,helloworld_dlgbox,helloworld_domodal,iconview,listbox,listview,menubutton,monthcalender,panel,progressbar,propsheet,radiogroup,resmgr,scrollbar,scrollview,spinbox,spinner,timer,trackbar,wnd_template} \
		$(1)/usr/bin/mgncs/
	$(CP) \
		$(PKG_BUILD_DIR)/mgncs/res/*.png \
		$(1)/usr/bin/mgncs/res
	$(INSTALL_DIR) $(1)/usr/bin/mgncs4touch/
	$(INSTALL_DIR) $(1)/usr/bin/mgncs4touch/res
	$(CP) \
		$(PKG_BUILD_DIR)/mgncs4touch/{balloontip,btnnavbar,commdraw,datepicker,exlist,hscrollview,iconflow,imgnavbar,itembar,name2value,newtrackbar,picker,settingtableview,switchbutton,tableview,timepicker} \
		$(1)/usr/bin/mgncs4touch/
	$(CP) \
		$(PKG_BUILD_DIR)/mgncs4touch/res/*.{png,jpg,bmp} \
		$(1)/usr/bin/mgncs4touch/res

	$(INSTALL_DIR) $(1)/usr/bin/mgutils/
	$(INSTALL_DIR) $(1)/usr/bin/mgutils/picview/res
	$(CP) \
		$(PKG_BUILD_DIR)/mgutils/{mywins/mywins,notebook/notebook,painter/painter,skin/skindemo,templates/colorseldlg,templates/fileopendlg,templates/fontseldlg,templates/infoshowdlg,vcongui/vcongui} \
		$(1)/usr/bin/mgutils
	$(CP) \
		$(PKG_BUILD_DIR)/mgutils/skin/*.png \
		$(1)/usr/bin/mgutils
	$(INSTALL_DIR) $(1)/usr/bin/mgplus/
	$(INSTALL_DIR) $(1)/usr/bin/mgplus/res
	$(CP) \
		$(PKG_BUILD_DIR)/mgplus/{brush/brushtest,colorcomposite/colorcompositetest,glyphoutline/glyphoutline,path/pathtest,pen/pentest,worldtransform/worldtransform,src/blinds,src/centersplit,src/coverflow,src/cubicrotate,src/gradientdemo,src/lion_test,src/mgcompositedemo,src/mgglyph_outline,src/mgplus_efficiency,src/mgplustest} \
		$(1)/usr/bin/mgplus/
	$(CP) \
		$(PKG_BUILD_DIR)/mgplus/{brush/res/*.jpg,colorcomposite/res/*.bmp,src/res/*.{png,jpg,bmp}} \
		$(1)/usr/bin/mgplus/res

	$(INSTALL_DIR) $(1)/usr/bin/src
	$(INSTALL_DIR) $(1)/usr/bin/src/res
	$(CP) \
		$(PKG_BUILD_DIR)/src/{animation,menubutton,edit,bitblt,scrollview,monthcal,bmpbkgnd,fminigui,scrollwnd,scrollwnd2,mycontrol,button,gridview,newtoolbar,gridview_ext,capture,simplekey,nr_window,caretdemo,painter,helloworld,spinbox,combobox,iconview,static,coolbar,progressbar,input,stretchblt,createicon,propsheet,listbox,timeeditor,cursordemo,listview,scalerbmp,trackbar,dialogbox,loadbmp,treeview,scrollbar,scrollbar_ctrl,drawicon} \
		$(1)/usr/bin/src/
	$(CP) \
		$(PKG_BUILD_DIR)/src/res/*.{gif,jpg,png} \
		$(1)/usr/bin/src/res/

endef

$(eval $(call BuildPackage,mg-samples))
