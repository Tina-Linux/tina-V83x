DESCRIPTION = "libtutuclear."
SECTION = "third party"
LICENSE = "CLOSED"

PV = "16a"
PR = "r0"

SRC_URI = " file://test/ \
		file://header/ \
		file://lib/aarch64/libtutuClear_${PV}*.a \
"

PARALLEL_MAKE = "-j 1"

inherit cmake

DEPENDS += "libsensory"

#do_unpack[nostamp]="1"

do_unpack() {
	cp -rf ${THISDIR}/files/src/*   ${S}
	cp -rf ${THISDIR}/files/header/*.h  ${S}
	cp -rf ${THISDIR}/files/lib/aarch64/libtutuClear_${PV}*.a  ${S}/libtutuClear.a
	cp -rf ${THISDIR}/files/prm/*.prm  ${S}
}

do_install_append() {
    install -d ${D}/${includedir}/tutu
    install -d ${D}/${libdir}/
    install -d ${D}/${sysconfdir}/avs/
    cp -rf ${S}/*.h ${D}/${includedir}/tutu/
    cp -rf ${S}/libtutuClear.a ${D}/${libdir}/
    cp -rf ${S}/*.prm ${D}/${sysconfdir}/avs/
}

INSANE_SKIP_${PN} += "dev-deps"
INSANE_SKIP_${PN}-dev += "dev-elf"
