#
# Copyright (C) 2006-2103 OpenWrt.org
# Copyright (C) 2014 Adam Porter
# Copyright (C) 2015 Gabe Rodriguez, Jeff Waugh
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=systemd
PKG_VERSION:=225
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.freedesktop.org/software/systemd/
PKG_MD5SUM:=2355cb77fdc8dd22439fd1941a6c7f22

PKG_BUILD_DEPENDS:=intltool/host gperf/host
PKG_BUILD_PARALLEL:=1

include systemd-common.mk
include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/version.mk

PKG_INSTALL:=1

TARGET_CFLAGS += --std=gnu99

SYSTEMD_DISABLE_PKG = $(if $(CONFIG_PACKAGE_systemd-$(1)),,--disable-$(2))

CONFIGURE_ARGS_FAST_BUILD = \
	--disable-gtk-doc \
	--disable-gtk-doc-html \
	--disable-manpages \
	--disable-tests \
	--without-python

CONFIGURE_ARGS_EMBEDDED = \
	--disable-backlight \
	--disable-binfmt \
	--disable-efi \
	--disable-firstboot \
	--disable-hibernate \
	--disable-importd \
	--disable-ldconfig \
	--disable-logind \
	--disable-machined \
	--disable-microhttpd \
	--disable-nls \
	--disable-pam \
	--disable-qrencode \
	--disable-quotacheck \
	--disable-vconsole \
	--disable-xkbcommon

CONFIGURE_ARGS += \
	--prefix=/usr \
	--sysconfdir=/etc \
	$(CONFIGURE_ARGS_FAST_BUILD) \
	$(CONFIGURE_ARGS_EMBEDDED) \
	--disable-apparmor \
	--disable-introspection \
	--disable-gudev \
	--disable-chkconfig \
	--disable-ima \
	--disable-selinux \
	--disable-sysusers \
	--disable-seccomp \
	--disable-audit \
	--disable-smack \
	--disable-xz \
	--disable-acl \
	--disable-gcrypt \
	--disable-myhostname \
	--disable-libcryptsetup \
	--disable-polkit \
	--disable-elfutils \
	--disable-gnutls \
	--disable-libcurl \
	$(call SYSTEMD_DISABLE_PKG,bootchart,bootchart) \
	$(call SYSTEMD_DISABLE_PKG,coredump,coredump) \
	$(call SYSTEMD_DISABLE_PKG,hostnamed,hostnamed) \
	$(call SYSTEMD_DISABLE_PKG,localed,localed) \
	$(call SYSTEMD_DISABLE_PKG,modules-load,kmod) \
	$(call SYSTEMD_DISABLE_PKG,networkd,networkd) \
	$(call SYSTEMD_DISABLE_PKG,random-seed,randomseed) \
	$(call SYSTEMD_DISABLE_PKG,resolved,resolved) \
	$(call SYSTEMD_DISABLE_PKG,rfkill,rfkill) \
	$(call SYSTEMD_DISABLE_PKG,timedated,timedated) \
	$(call SYSTEMD_DISABLE_PKG,timesyncd,timesyncd) \
	$(call SYSTEMD_DISABLE_PKG,tmpfiles,tmpfiles) \
	$(call SYSTEMD_DISABLE_PKG,update-utmp,utmp)

define Build/Configure
	(cd $(PKG_BUILD_DIR); ./autogen.sh;);
	$(call Build/Configure/Default,)
endef

define Package/systemd/Default
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=System and Service Manager
  URL:=http://www.freedesktop.org/wiki/Software/systemd/
  MAINTAINER:=Jeff Waugh <jdub@bethesignal.org>
endef

define Package/systemd/description/Default
systemd is a suite of basic building blocks for a Linux system
endef

define Package/systemd
$(call Package/systemd/Default)
	DEPENDS:=+@BUSYBOX_DEFAULT_SULOGIN +libcap +libmount +libkmod +dbus +agetty \
		+PACKAGE_kmod-fs-autofs4 +@KERNEL_CGROUPS +@KERNEL_NET_CLS_CGROUP \
		+@KERNEL_LXC_MISC +@KERNEL_DEVPTS_MULTIPLE_INSTANCES +@KERNEL_POSIX_MQUEUE \
		+@KERNEL_NAMESPACES +@KERNEL_NET_NS \
		+@KERNEL_DEVTMPFS +@KERNEL_FHANDLE +@KERNEL_FANOTIFY +@KERNEL_BLK_DEV_BSG
	MENU:=1
endef

define Package/systemd/description
$(call Package/systemd/description/Default)
This package contains the core systemd /sbin/init daemon and supporting tools.
endef

define Package/systemd/conffiles
/etc/hostname
/etc/machine-id
/etc/systemd/system.conf
/etc/systemd/user.conf
endef

define Package/systemd/install
	$(INSTALL_DIR) $(1)/etc/systemd
	$(CP) \
		$(PKG_INSTALL_DIR)/etc/systemd/{journald,system,user}.conf \
		$(1)/etc/systemd

	$(INSTALL_DIR) $(1)/etc/systemd/system/multi-user.target.wants
	ln -sf \
		/usr/lib/systemd/system/remote-fs.target \
		$(1)/etc/systemd/system/multi-user.target.wants/.

	$(INSTALL_DIR) $(1)/etc/systemd/network
	$(INSTALL_DIR) $(1)/etc/systemd/user
	$(INSTALL_DIR) $(1)/etc/network

	$(INSTALL_DIR) $(1)/etc/dbus-1/system.d
	$(CP) \
		$(PKG_INSTALL_DIR)/etc/dbus-1/system.d/org.freedesktop.systemd1.conf \
		$(1)/etc/dbus-1/system.d

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(addprefix $(PKG_INSTALL_DIR)/usr/bin/, \
			$(SYSTEMD_BIN)) \
		$(1)/usr/bin

	ln -sf \
		/usr/lib/systemd/systemd-bus-proxyd \
		$(1)/usr/bin/systemd-stdio-bridge

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libsystemd.so* \
		$(1)/usr/lib

	$(INSTALL_DIR) $(1)/etc/sysctl.d
	$(INSTALL_DIR) $(1)/usr/lib/sysctl.d
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/sysctl.d/50-default.conf \
		$(1)/usr/lib/sysctl.d

	#$(INSTALL_DIR) $(1)/usr/lib/sysusers.d
	#$(INSTALL_DATA) \
		#$(PKG_INSTALL_DIR)/usr/lib/sysusers.d/*.conf \
		#$(1)/usr/lib/sysusers.d

	$(INSTALL_DIR) $(1)/usr/lib/systemd
	$(INSTALL_BIN) \
		$(addprefix $(PKG_INSTALL_DIR)/usr/lib/systemd/, \
			$(SYSTEMD_LIB)) \
		$(1)/usr/lib/systemd

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system
	$(CP) \
		$(addprefix $(PKG_INSTALL_DIR)/usr/lib/systemd/system/, \
			$(SYSTEMD_SYSTEM_TARGET_WANTS) \
			$(SYSTEMD_SYSTEM_TARGETS) \
			$(SYSTEMD_SYSTEM_SLICES) \
			$(SYSTEMD_SYSTEM_SOCKETS) \
			$(SYSTEMD_SYSTEM_PATHS) \
			$(SYSTEMD_SYSTEM_SERVICES) \
			$(SYSTEMD_SYSTEM_MOUNTS)) \
		$(1)/usr/lib/systemd/system

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system-shutdown
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system-sleep
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system-generators
	$(INSTALL_DIR) $(1)/usr/lib/systemd/user-generators

	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/system-generators/systemd-* \
		$(1)/usr/lib/systemd/system-generators

	$(INSTALL_DIR) $(1)/usr/lib/systemd/catalog
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/catalog/systemd.catalog \
		$(1)/usr/lib/systemd/catalog

	$(INSTALL_DIR) $(1)/usr/lib/systemd/user
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/user/* \
		$(1)/usr/lib/systemd/user

	$(INSTALL_DIR) $(1)/usr/share/dbus-1/services
	$(INSTALL_DIR) $(1)/usr/share/dbus-1/system-services
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/share/dbus-1/system-services/org.freedesktop.systemd1.service \
		$(1)/usr/share/dbus-1/system-services
	ln -sf \
		/usr/share/dbus-1/system-services/org.freedesktop.systemd1.service \
		$(1)/usr/share/dbus-1/services/.

	$(INSTALL_DIR) $(1)/var/log/journal
	$(INSTALL_DIR) $(1)/etc/systemd/system/getty.target.wants
	$(CP) \
		$(PKG_INSTALL_DIR)/etc/systemd/system/getty.target.wants/getty@tty1.service \
		$(1)/etc/systemd/system/getty.target.wants

	$(INSTALL_DIR) $(1)/sbin
	ln -sf \
		/usr/lib/systemd/systemd \
		$(1)/sbin/init

	$(CP) ./files/* $(1)/
	$(VERSION_SED) $(1)/etc/os-release

	$(INSTALL_DIR) $(1)/run
	ln -sf /run $(1)/var/run

	# FIXME: make DEFAULT_TARGET option
	ln -sf \
		/usr/lib/systemd/system/initrd.target \
		$(1)/etc/systemd/system/default.target

	# FIXME: ifeq ($(DEFAULT_TARGET),initrd)
	touch $(1)/etc/initrd-release
	$(INSTALL_DIR) $(1)/sysroot
endef


define Package/systemd-coredump
$(call Package/systemd/Default)
	TITLE:=systemd coredump support
	DEPENDS+= systemd +@KERNEL_ELF_CORE
endef

define Package/systemd-coredump/description
$(call Package/systemd/description/Default)
This package contains the systemd coredump support.
endef

define Package/systemd-coredump/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/coredumpctl \
		$(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/lib/systemd
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/systemd-coredump \
		$(1)/usr/lib/systemd

	$(INSTALL_DIR) $(1)/etc/systemd
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/etc/systemd/coredump.conf \
		$(1)/etc/systemd

	$(INSTALL_DIR) $(1)/usr/lib/sysctl.d
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/sysctl.d/50-coredump.conf \
		$(1)/usr/lib/sysctl.d
endef


define Package/systemd-nspawn
$(call Package/systemd/Default)
	TITLE:=systemd namespace container support
	DEPENDS+= systemd +libiptc
endef

define Package/systemd-nspawn/description
$(call Package/systemd/description/Default)
This package contains the systemd namespace container support.
endef

define Package/systemd-nspawn/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/systemd-nspawn \
		$(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/system/systemd-nspawn@.service \
		$(1)/usr/lib/systemd/system
endef


define Package/systemd-tmpfiles
$(call Package/systemd/Default)
	TITLE:=systemd temporary file management support
	DEPENDS+= systemd
endef

define Package/systemd-tmpfiles/description
$(call Package/systemd/description/Default)
This package contains the systemd temporary file management support.
endef

define Package/systemd-tmpfiles/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/systemd-tmpfiles \
		$(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system
	$(INSTALL_DATA) \
		$(wildcard $(PKG_INSTALL_DIR)/usr/lib/systemd/system/systemd-tmpfiles-*) \
		$(1)/usr/lib/systemd/system

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/sysinit.target.wants
	$(CP) \
		$(addprefix $(PKG_INSTALL_DIR)/usr/lib/systemd/system/sysinit.target.wants/,
			systemd-tmpfiles-setup.service \
			systemd-tmpfiles-setup-dev.service ) \
		$(1)/usr/lib/systemd/system/sysinit.target.wants

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/timers.target.wants
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/system/timers.target.wants/systemd-tmpfiles-clean.timer \
		$(1)/usr/lib/systemd/system/timers.target.wants

	$(INSTALL_DIR) $(1)/etc/tmpfiles.d	
	$(INSTALL_DIR) $(1)/usr/lib/tmpfiles.d
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/tmpfiles.d/*.conf \
		$(1)/usr/lib/tmpfiles.d
endef


define Package/systemd-udev-hwdb
$(call Package/systemd/Default)
  TITLE+= (udev hwdb)
  DEPENDS:=+systemd-udevd
endef

define Package/systemd-udev-hwdb/description
$(call Package/systemd/description/Default)
  This package contains the udev hardware database files.
endef

define Package/systemd-udev-hwdb/install
	$(INSTALL_DIR) $(1)/etc/udev/hwdb.d
	$(INSTALL_DIR) $(1)/usr/lib/udev/hwdb.d
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/udev/hwdb.d/*.hwdb \
		$(1)/usr/lib/udev/hwdb.d
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libudev.so* $(1)/usr/lib
#	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgudev-1.0.so* $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsystemd.so* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/share/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/share/pkgconfig/*.pc $(1)/usr/share/pkgconfig
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc $(1)/usr/lib/pkgconfig
endef


define SystemdBuildLib
  define Package/systemd-$(1)
    $(call Package/systemd/Default)
    TITLE:=systemd $(2)
    DEPENDS+= systemd $(3)
  endef

  define Package/systemd-$(1)/description
  $(call Package/systemd/description/Default)
  This package contains the systemd $(2).
  endef

  define Package/systemd-$(1)/install
	# daemon
	$(INSTALL_DIR) $$(1)/usr/lib/systemd
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/systemd-$(1)* \
		$$(1)/usr/lib/systemd
	# service (systemd)
	$(INSTALL_DIR) $$(1)/usr/lib/systemd/system
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/system/systemd-$(1)*.service \
		$$(1)/usr/lib/systemd/system
	# custom
	$(call Plugin/$(1)/install,$$(1))
  endef

  $$(eval $$(call BuildPackage,systemd-$(1)))
endef

define SystemdDbusService
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/system/dbus-org.freedesktop.$(2).service \
		$(1)/usr/lib/systemd/system

	$(INSTALL_DIR) $(1)/etc/dbus-1/system.d
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/etc/dbus-1/system.d/org.freedesktop.$(2).conf \
		$(1)/etc/dbus-1/system.d

	$(INSTALL_DIR) $(1)/usr/share/dbus-1/system-services
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/share/dbus-1/system-services/org.freedesktop.$(2).service \
		$(1)/usr/share/dbus-1/system-services
endef


define Plugin/bootchart/install
	$(INSTALL_DIR) $(1)/etc/systemd
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/etc/systemd/bootchart.conf \
		$(1)/etc/systemd
endef


define Plugin/hostnamed/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/hostnamectl \
		$(1)/usr/bin

	$(call SystemdDbusService,$$(1),hostname1)
endef


define Plugin/localed/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/localectl \
		$(1)/usr/bin

	$(call SystemdDbusService,$$(1),locale1)
endef

define Plugin/modules-load/install
	$(INSTALL_DIR) $(1)/etc/modules-load.d
	$(INSTALL_DIR) $(1)/usr/lib/modules-load.d

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/sysinit.target.wants
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/system/sysinit.target.wants/systemd-modules-load.service \
		$(1)/usr/lib/systemd/system/sysinit.target.wants
endef

define Plugin/networkd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/networkctl \
		$(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system
	$(INSTALL_DATA) \
		$(addprefix $(PKG_INSTALL_DIR)/usr/lib/systemd/system/, \
			network.target \
			network-pre.target \
			network-online.target \
			systemd-networkd.socket ) \
		$(1)/usr/lib/systemd/system

	$(INSTALL_DIR) $(1)/usr/lib/systemd/network
	$(INSTALL_DATA) \
		$(addprefix $(PKG_INSTALL_DIR)/usr/lib/systemd/network/, \
			80-container-host0.network \
			80-container-ve.network \
			99-default.link ) \
		$(1)/usr/lib/systemd/network

	$(INSTALL_DIR) $(1)/etc/systemd/system/multi-user.target.wants
	$(CP) \
		$(PKG_INSTALL_DIR)/etc/systemd/system/multi-user.target.wants/systemd-networkd.service \
		$(1)/etc/systemd/system/multi-user.target.wants

	$(INSTALL_DIR) $(1)/etc/systemd/system/network-online.target.wants
	$(CP) \
		$(PKG_INSTALL_DIR)/etc/systemd/system/network-online.target.wants/systemd-networkd-wait-online.service \
		$(1)/etc/systemd/system/network-online.target.wants

	$(INSTALL_DIR) $(1)/etc/systemd/system/sockets.target.wants
	$(CP) \
		$(PKG_INSTALL_DIR)/etc/systemd/system/sockets.target.wants/systemd-networkd.socket \
		$(1)/etc/systemd/system/sockets.target.wants

	$(call SystemdDbusService,$$(1),network1)
endef

define Plugin/random-seed/install
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/sysinit.target.wants
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/system/sysinit.target.wants/systemd-random-seed.service \
		$(1)/usr/lib/systemd/system/sysinit.target.wants
endef

define Plugin/resolved/install
	$(INSTALL_DIR) $(1)/etc/systemd
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/etc/systemd/resolved.conf \
		$(1)/etc/systemd

	$(INSTALL_DIR) $(1)/etc/systemd/system/multi-user.target.wants
	$(CP) \
		$(PKG_INSTALL_DIR)/etc/systemd/system/multi-user.target.wants/systemd-resolved.service \
		$(1)/etc/systemd/system/multi-user.target.wants

	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/lib/libnss_resolve.so* \
		$(1)/lib

	$(INSTALL_DIR) $(1)/usr/lib/systemd
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/systemd-resolve-host \
		$(1)/usr/lib/systemd

	$(call SystemdDbusService,$$(1),resolve1)
endef

define Package/systemd-resolved/conffiles
/etc/systemd/resolved.conf
endef

define Plugin/timedated/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/timedatectl \
		$(1)/usr/bin

	$(call SystemdDbusService,$$(1),timedate1)
endef

define Plugin/timesyncd/install
	$(INSTALL_DIR) $(1)/etc/systemd
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/etc/systemd/timesyncd.conf \
		$(1)/etc/systemd

	$(INSTALL_DIR) $(1)/etc/systemd/system/sysinit.target.wants
	$(CP) \
		$(PKG_INSTALL_DIR)/etc/systemd/system/sysinit.target.wants/systemd-timesyncd.service \
		$(1)/etc/systemd/system/sysinit.target.wants
endef

define Package/systemd-timesyncd/conffiles
/etc/systemd/timesyncd.conf
endef

define Plugin/udevd/install
	$(INSTALL_DIR) $(1)/etc/udev/rules.d
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/etc/udev/udev.conf \
		$(1)/etc/udev

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/udevadm \
		$(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/systemd-hwdb \
		$(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libudev.so* \
		$(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system
	$(INSTALL_DATA) \
		$(addprefix $(PKG_INSTALL_DIR)/usr/lib/systemd/system/, \
			$(SYSTEMD_UDEVD_SYSTEM_SERVICES) \
			$(SYSTEMD_UDEVD_SYSTEM_SOCKETS)) \
		$(1)/usr/lib/systemd/system

	$(INSTALL_DIR) $(1)/usr/lib/udev
	$(INSTALL_BIN) \
		$(addprefix $(PKG_INSTALL_DIR)/usr/lib/udev/, \
			$(SYSTEMD_UDEVD_LIBS)) \
		$(1)/usr/lib/udev

	$(INSTALL_DIR) $(1)/usr/lib/udev/rules.d
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/udev/rules.d/* \
		$(1)/usr/lib/udev/rules.d

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/sysinit.target.wants
	$(CP) \
		$(addprefix $(PKG_INSTALL_DIR)/usr/lib/systemd/system/sysinit.target.wants/, \
			systemd-udevd.service \
			systemd-udev-trigger.service ) \
		$(1)/usr/lib/systemd/system/sysinit.target.wants

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/sockets.target.wants
	$(CP) \
		$(addprefix $(PKG_INSTALL_DIR)/usr/lib/systemd/system/sockets.target.wants/, \
			systemd-udevd-control.socket \
			systemd-udevd-kernel.socket ) \
		$(1)/usr/lib/systemd/system/sockets.target.wants
endef

define Package/systemd-udevd/conffiles
/etc/udev/udev.conf
endef

define Plugin/update-utmp/install
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/sysinit.target.wants
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/system/sysinit.target.wants/systemd-update-utmp.service \
		$(1)/usr/lib/systemd/system/sysinit.target.wants

# FIXME: /usr/lib/systemd/system/runlevel$(seq 1 5).target.wants/systemd-update-utmp-runlevel.service
# directories and symlinks
#	$(INSTALL_DIR) \
#		$(foreach D, $(shell seq 1 5), $(1)/usr/lib/systemd/system/runlevel$(D).target.wants)
endef

$(eval $(call BuildPackage,systemd))
$(eval $(call BuildPackage,systemd-coredump))
$(eval $(call BuildPackage,systemd-nspawn))
#$(eval $(call BuildPackage,systemd-tmpfiles))
$(eval $(call BuildPackage,systemd-udev-hwdb))
$(eval $(call SystemdBuildLib,bootchart,bootchart tool,))
$(eval $(call SystemdBuildLib,hostnamed,hostname daemon,))
$(eval $(call SystemdBuildLib,localed,locale daemon,))
$(eval $(call SystemdBuildLib,modules-load,module loader,+libkmod))
$(eval $(call SystemdBuildLib,networkd,network daemon,+libiptc))
$(eval $(call SystemdBuildLib,random-seed,randomseed tools,))
$(eval $(call SystemdBuildLib,resolved,name resolver,))
$(eval $(call SystemdBuildLib,rfkill,rfkill tools,+@USE_RFKILL))
$(eval $(call SystemdBuildLib,timedated,timedate daemon,))
$(eval $(call SystemdBuildLib,timesyncd,network time synchronization,))
$(eval $(call SystemdBuildLib,udevd,device manager,))
$(eval $(call SystemdBuildLib,update-utmp,utmp/wtmp log handling,))
