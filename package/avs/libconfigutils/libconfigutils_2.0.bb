DESCRIPTION = "libconfigutils"
SECTION = "third party"
LICENSE = "CLOSED"

SRC_URI = " file://src/ \
"

PV = "2.0"
PR = "r0"

#PARALLEL_MAKE = "-j 1"

inherit cmake

OECMAKE_CXX_FLAGS_append = " -std=c++11"

DEPENDS += "json-c libev avs-sdk libugpio resample libtutuclear"

#do_unpack[nostamp]="1"

do_unpack() {
    cp -rf ${THISDIR}/files/src/*   ${S}
    cp -rf ${THISDIR}/files/config.json   ${S}
}

do_install_append() {
    install -d ${D}/${sysconfdir}/avs/
    cp -rf  ${S}/config.json ${D}/${sysconfdir}/avs/
}

INSANE_SKIP_${PN} += "dev-deps"
INSANE_SKIP_${PN}-dev += "dev-elf"
INSANE_SKIP_${PN}-dev += "file-rdeps"
