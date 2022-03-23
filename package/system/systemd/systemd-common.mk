SYSTEMD_BIN = \
	busctl \
	journalctl \
	systemctl \
	systemd-analyze \
	systemd-ask-password \
	systemd-cat \
	systemd-cgls \
	systemd-cgtop \
	systemd-delta \
	systemd-detect-virt \
	systemd-escape \
	systemd-machine-id-setup \
	systemd-notify \
	systemd-path \
	systemd-run \
	systemd-tty-ask-password-agent

SYSTEMD_LIB = \
	systemd \
	systemd-ac-power \
	systemd-activate \
	systemd-bus-proxyd \
	systemd-cgroups-agent \
	systemd-fsck \
	systemd-initctl \
	systemd-journald \
	systemd-machine-id-commit \
	systemd-remount-fs \
	systemd-reply-password \
	systemd-shutdown \
	systemd-sleep \
	systemd-socket-proxyd \
	systemd-sysctl \
	systemd-update-done

SYSTEMD_SYSTEM_TARGET_WANTS = \
	sockets.target.wants \
	runlevel1.target.wants \
	runlevel2.target.wants \
	runlevel3.target.wants \
	runlevel4.target.wants \
	runlevel5.target.wants \
	multi-user.target.wants \
	local-fs.target.wants \
	sysinit.target.wants

SYSTEMD_SYSTEM_TARGETS = \
	basic.target \
	bluetooth.target \
	ctrl-alt-del.target \
	default.target \
	emergency.target \
	final.target \
	getty.target \
	graphical.target \
	halt.target \
	initrd-fs.target \
	initrd-root-fs.target \
	initrd-switch-root.target \
	initrd.target \
	kexec.target \
	local-fs-pre.target \
	local-fs.target \
	machines.target \
	multi-user.target \
	network-online.target \
	nss-lookup.target \
	nss-user-lookup.target \
	paths.target \
	poweroff.target \
	printer.target \
	reboot.target \
	remote-fs-pre.target \
	remote-fs.target \
	rescue.target \
	rpcbind.target \
	runlevel0.target \
	runlevel1.target \
	runlevel2.target \
	runlevel3.target \
	runlevel4.target \
	runlevel5.target \
	runlevel6.target \
	shutdown.target \
	sigpwr.target \
	sleep.target \
	slices.target \
	smartcard.target \
	sockets.target \
	sound.target \
	suspend.target \
	swap.target \
	sysinit.target \
	system-update.target \
	timers.target \
	time-sync.target \
	umount.target

SYSTEMD_SYSTEM_SLICES = \
	-.slice \
	system.slice

SYSTEMD_SYSTEM_SOCKETS = \
	syslog.socket \
	systemd-initctl.socket \
	systemd-journald.socket \
	systemd-journald-audit.socket \
	systemd-journald-dev-log.socket \

SYSTEMD_SYSTEM_PATHS = \
	systemd-ask-password-console.path \
	systemd-ask-password-wall.path

SYSTEMD_SYSTEM_SERVICES = \
	autovt@.service \
	console-getty.service \
	console-shell.service \
	container-getty@.service \
	debug-shell.service \
	emergency.service \
	getty@.service \
	halt-local.service \
	initrd-cleanup.service \
	initrd-parse-etc.service \
	initrd-switch-root.service \
	quotaon.service \
	rc-local.service \
	rescue.service \
	serial-getty@.service \
	systemd-ask-password-console.service \
	systemd-ask-password-wall.service \
	systemd-fsck-root.service \
	systemd-fsck@.service \
	systemd-halt.service \
	systemd-initctl.service \
	systemd-journald.service \
	systemd-journal-catalog-update.service \
	systemd-journal-flush.service \
	systemd-kexec.service \
	systemd-machine-id-commit.service \
	systemd-poweroff.service \
	systemd-reboot.service \
	systemd-remount-fs.service \
	systemd-suspend.service \
	systemd-sysctl.service \
	systemd-update-done.service \
	user@.service

SYSTEMD_SYSTEM_MOUNTS = \
	dev-hugepages.mount \
	dev-mqueue.mount \
	sys-fs-fuse-connections.mount \
	sys-kernel-config.mount \
	sys-kernel-debug.mount \
	tmp.mount

SYSTEMD_UDEVD_SYSTEM_SERVICES = \
	initrd-udevadm-cleanup-db.service \
	systemd-hwdb-update.service \
	systemd-udev-settle.service \
	systemd-udev-trigger.service

SYSTEMD_UDEVD_SYSTEM_SOCKETS = \
	systemd-udevd-control.socket \
	systemd-udevd-kernel.socket

SYSTEMD_UDEVD_LIBS = \
	ata_id \
	cdrom_id \
	collect \
	mtd_probe \
	scsi_id \
	v4l_id
