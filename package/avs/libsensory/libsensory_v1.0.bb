DESCRIPTION = "libsensory"
SECTION = "third party"
LICENSE = "CLOSED"

SRC_URI = " file://wrapper/ \
	    file://test/ \
	    file://CMakeLists.txt \
"

PV = "1.0"
PR = "r0"

PARALLEL_MAKE = "-j 1"

inherit cmake

EXTRA_OECMAKE = " \
	-DARCH=aarch64 \
	\
"
DEPENDS += "alsa-lib"

#do_unpack[nostamp]="1"

do_unpack() {
	cp -rf ${THISDIR}/files/wrapper/   ${S}
	cp -rf ${THISDIR}/files/test/  ${S}
	cp -rf ${THISDIR}/files/CMakeLists.txt  ${S}
}

do_install_append() {
    install -d ${D}/${sysconfdir}/avs/
    cp -rf ${D}/usr/models/thfft_alexa_a_enus_v3_1mb_search_8.snsr ${D}/${sysconfdir}/avs/
    cp -rf ${D}/usr/models/thfft_alexa_a_enus_v3_1mb.snsr ${D}/${sysconfdir}/avs/
}

PACKAGES = "${PN} ${PN}-dev ${PN}-dbg"

INSANE_SKIP_${PN} += "dev-deps"
INSANE_SKIP_${PN}-dev += "dev-elf"

FILES_${PN}+="${libdir}/lib*.so \
	/usr/models \
        /usr/models/thfft_alexa_a_enus_v3_1mb_search_8.snsr \
        /usr/models/thfft_alexa_a_enus_v3_1mb.snsr \
"
