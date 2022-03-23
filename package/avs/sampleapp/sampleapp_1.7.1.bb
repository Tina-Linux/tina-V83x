DESCRIPTION = "avs sampleapp"
SECTION = "third party"
LICENSE = "CLOSED"

SRC_URI = " file://source/ \
"

PV = "1.7.1"
PR = "r0"

#PARALLEL_MAKE = "-j 1"

inherit cmake

EXTRA_OECMAKE = " \
	-DGSTREAMER_MEDIA_PLAYER=ON \
	-DPORTAUDIO=ON  \
	-DCMAKE_BUILD_TYPE=DEBUG \
	-DPORTAUDIO_LIB_PATH=${STAGING_DIR_TARGET}/usr/lib/libportaudio.so \
	-DPORTAUDIO_INCLUDE_DIR=${STAGING_DIR_TARGET}/usr/include \
	-DSENSORY_KEY_WORD_DETECTOR=ON \
	-DSENSORY_KEY_WORD_DETECTOR_LIB_PATH=sensory \
	-DSENSORY_KEY_WORD_DETECTOR_INCLUDE_DIR=${STAGING_DIR_TARGET}/usr/include \
	-DAMAZONLITE_KEY_WORD_DETECTOR=ON \
	-DAMAZONLITE_KEY_WORD_DETECTOR_LIB_PATH=pryon_lite \
	-DAMAZONLITE_KEY_WORD_DETECTOR_INCLUDE_DIR=${STAGING_DIR_TARGET}/usr/include \
	-DAMAZONLITE_KEY_WORD_DETECTOR_DYNAMIC_MODEL_LOADING=ON \
	-DESP_PROVIDER=ON \
	-DESP_INCLUDE_DIR=${STAGING_DIR_TARGET}/usr/include/esp \
	-DESP_LIB_PATH=esp \
	\
"
DEPENDS += "avs-sdk \
			portaudio-v19 \
			libconfigutils \
"

RDEPENS_${PN}+="avs-sdk"

#do_unpack[nostamp]="1"

do_unpack() {
	cp -rf ${THISDIR}/files/source/*   ${S}
	cp -rf ${THISDIR}/files/AlexaClientSDKConfig.json   ${S}
}

do_install_append() {
	install -d ${D}/${sysconfdir}/avs/
	cp -rf  ${S}/AlexaClientSDKConfig.json ${D}/${sysconfdir}/avs/
}

INSANE_SKIP_${PN} = "ldflags file-rdeps dev-deps"
INSANE_SKIP_${PN}-dev += "dev-elf"
