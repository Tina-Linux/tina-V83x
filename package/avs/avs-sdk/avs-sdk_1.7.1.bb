DESCRIPTION = "avs  devices sdk"
SECTION = "third party"
LICENSE = "CLOSED"

SRC_URI = " file://avs-device-sdk/ \
"

PV = "1.7.1"
PR = "r0"

#PARALLEL_MAKE = "-j 1"

inherit cmake

OECMAKE_CXX_FLAGS_append = " -std=c++11"

EXTRA_OECMAKE = " \
	-DBUILD_SHARED_LIBS=ON \
	-DBUILD_TESTING=ON \
	-DCMAKE_BUILD_TYPE=DEBUG \
	-DCOVERAGE=ON \
	-DBUILD_GTEST=ON \
	-DGSTREAMER_MEDIA_PLAYER=ON \
	-DPORTAUDIO=ON \
	-DPORTAUDIO_LIB_PATH=portaudio \
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

DEPENDS += "\
	curl \
	sqlite3 \
	gstreamer1.0 \
	gstreamer1.0-plugins-base \
	gstreamer1.0-plugins-aw \
	gstreamer1.0-plugins-good \
	gstreamer1.0-plugins-bad \
	gstreamer1.0-libav \
	libsensory \
	libpryon-lite \
	glib-networking \
	ca-certificates \
	portaudio-v19 \
    ntp \
    libesp \
"

#do_unpack[nostamp]="1"

do_unpack() {
	cp -rf ${THISDIR}/files/avs-device-sdk/*   ${S}
}

PACKAGES = "${PN} ${PN}-dev ${PN}-dbg"

#INSANE_SKIP_${PN}-dev += "dev-elf"

INSANE_SKIP_${PN} += "dev-deps"

INSANE_SKIP_${PN}-dev += "dev-elf"

FILES_${PN}+="${libdir}/*.so"
FILES_${PN}+="${sysconfdir}/*"

