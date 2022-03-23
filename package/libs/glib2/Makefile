#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=glib2
PKG_VERSION:=2.50.1
PKG_RELEASE:=1

PKG_SOURCE:=glib-$(PKG_VERSION).tar.xz
PKG_BUILD_DIR:=$(COMPILE_DIR)/glib-$(PKG_VERSION)
PKG_SOURCE_URL:=http://ftp.gnome.org/pub/gnome/sources/glib/$(PKG_VERSION)
PKG_HASH:=

PKG_BUILD_PARALLEL:=1
HOST_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=pkgconf/host glib2/host gettext/host
HOST_BUILD_DEPENDS:= gettext-full/host libiconv/host libffi/host libpcre/host
PKG_INSTALL:=1
PKG_USE_MIPS16:=0

PKG_FIXUP:=autoreconf

HOST_BUILD_DIR:=$(COMPILE_DIR_HOST)/glib-$(PKG_VERSION)

include $(BUILD_DIR)/host-build.mk
include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/nls.mk
export LD_LIBRARY_PATH=$(STAGING_DIR)/host/lib

define Package/glib2
  SUBMENU:=gnome
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=$(ICONV_DEPENDS) $(INTL_DEPENDS) +zlib +libpthread +libffi +libattr +libpcre
  TITLE:=glib 2.0
  MAINTAINER:=Peter Wagner <tripolar@gmx.at>
  URL:=http://www.gtk.org/
endef

define Package/glib2/description
  The GLib library of C routines
endef

HOST_CONFIGURE_ARGS += \
	--disable-coverage \
	--disable-dtrace \
	--disable-fam \
	--disable-libelf \
	--disable-selinux \
	--disable-systemtap \
	--disable-xattr	\
	--with-pcre=system \
	--enable-libmount=no \
	--with-libiconv=gnu

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--enable-debug=yes \
	--disable-selinux \
	--enable-libmount=no \
	--disable-fam \
	--with-pcre=system \
	--disable-libelf \
	--with-libiconv=gnu \
	--with-gio-module-dir=/usr/lib/gio/modules

CONFIGURE_VARS += \
		ac_cv_func_posix_getpwuid_r=yes \
	glib_cv_stack_grows=no \
	glib_cv_uscore=no \
	ac_cv_func_strtod=yes \
	ac_fsusage_space=yes \
	fu_cv_sys_stat_statfs2_bsize=yes \
	ac_cv_func_closedir_void=no \
	ac_cv_func_getloadavg=no \
	ac_cv_lib_util_getloadavg=no \
	ac_cv_lib_getloadavg_getloadavg=no \
	ac_cv_func_getgroups=yes \
	ac_cv_func_getgroups_works=yes \
	ac_cv_func_chown_works=yes \
	ac_cv_have_decl_euidaccess=no \
	ac_cv_func_euidaccess=no \
	ac_cv_have_decl_strnlen=yes \
	ac_cv_func_strnlen_working=yes \
	ac_cv_func_lstat_dereferences_slashed_symlink=yes \
	ac_cv_func_lstat_empty_string_bug=no \
	ac_cv_func_stat_empty_string_bug=no \
	vb_cv_func_rename_trailing_slash_bug=no \
	ac_cv_have_decl_nanosleep=yes \
	jm_cv_func_nanosleep_works=yes \
	gl_cv_func_working_utimes=yes \
	ac_cv_func_utime_null=yes \
	ac_cv_have_decl_strerror_r=yes \
	ac_cv_func_strerror_r_char_p=no \
	jm_cv_func_svid_putenv=yes \
	ac_cv_func_getcwd_null=yes \
	ac_cv_func_getdelim=yes \
	ac_cv_func_mkstemp=yes \
	utils_cv_func_mkstemp_limitations=no \
	utils_cv_func_mkdir_trailing_slash_bug=no \
	jm_cv_func_gettimeofday_clobber=no \
	gl_cv_func_working_readdir=yes \
	jm_ac_cv_func_link_follows_symlink=no \
	utils_cv_localtime_cache=no \
	ac_cv_struct_st_mtim_nsec=no \
	gl_cv_func_tzset_clobber=no \
	gl_cv_func_getcwd_null=yes \
	gl_cv_func_getcwd_path_max=yes \
	ac_cv_func_fnmatch_gnu=yes \
	am_getline_needs_run_time_check=no \
	am_cv_func_working_getline=yes \
	gl_cv_func_mkdir_trailing_slash_bug=no \
	gl_cv_func_mkstemp_limitations=no \
	ac_cv_func_working_mktime=yes \
	jm_cv_func_working_re_compile_pattern=yes \
	ac_use_included_regex=no \
	gl_cv_c_restrict=no \
	ac_cv_path_GLIB_GENMARSHAL=$(STAGING_DIR_HOSTPKG)/usr/bin/glib-genmarshal \
	ac_cv_prog_F77=no \
	ac_cv_func_posix_getgrgid_r=no \
	glib_cv_long_long_format=ll \
	ac_cv_func_printf_unix98=yes \
	ac_cv_func_vsnprintf_c99=yes \
	ac_cv_func_newlocale=no \
	ac_cv_func_uselocale=no \
	ac_cv_func_strtod_l=no \
	ac_cv_func_strtoll_l=no \
	ac_cv_func_strtoull_l=no \
	gt_cv_c_wchar_t=no

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/glib-2.0 \
		$(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/glib-2.0/include/*.h \
		$(1)/usr/include/glib-2.0/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/gio-unix-2.0 \
		$(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/glib-2.0 \
		$(1)/usr/lib/

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.{so*,a,la} \
		$(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc \
		$(1)/usr/lib/pkgconfig

	$(INSTALL_DIR) $(2)/share/aclocal/
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/share/aclocal/*.m4  \
		$(2)/share/aclocal/
endef

define Package/glib2/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,glib2))
