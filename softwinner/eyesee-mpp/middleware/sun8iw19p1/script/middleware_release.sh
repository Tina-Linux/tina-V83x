#!/bin/bash 
set -e
#set -x

TARGET_PATH=$(pwd)
RELATIVE_DIR=middleware
RELEASE_PATH=${TARGET_PATH}/${RELATIVE_DIR}
LIBRARY_PATH=${RELEASE_PATH}/lib

SOURCE_PATH=${TARGET_PATH}/..
COMPILE_SO_PATH=${TARGET_OUT}/target/usr/lib
COMPILE_A_PATH=${TARGET_OUT}/staging/usr/lib

show_help()
{
local scriptName=./`basename $0`
printf "
$scriptName -p [platform]

OPTIONS
    -h             Display help message
    -p [platform]  platform
                    V40_Linux4.4

Examples:
    V40:  $scriptName -p V40_Linux3.10
    V40:  $scriptName -p V40_Linux4.4
    V5:   $scriptName -p V5_Linux4.4

"
}

source_sdk()
{
    cd ${TARGET_PATH}/../..
    case $PLATFORM in
    V40_Linux3.10)
        source ./build/envsetup.sh
        ;;
    V40_Linux4.4)
        source ./build/v40_linux44_envsetup.sh
        ;;
    V5_Linux4.4)
        source ./build/v5_envsetup.sh
        ;;
    *)
        echo "PLATFORM:$PLATFORM is unknown!"
        source ./build/envsetup.sh
        ;;
    esac
    cd ${TARGET_PATH}
}

clean_code()
{
    cleanmpp
}

mkdir_release_path()
{
    rm -rf ${TARGET_PATH}/middleware.tgz
    rm -rf ${RELEASE_PATH}
    mkdir -p ${LIBRARY_PATH}
    mkdir -p ${RELEASE_PATH}/media/LIBRARY/audioDecLib
    mkdir -p ${RELEASE_PATH}/media/LIBRARY/audioEncLib
    mkdir -p ${RELEASE_PATH}/media/LIBRARY/libcedarc
    mkdir -p ${RELEASE_PATH}/media/LIBRARY/libcedarx/libcore/common/iniparser
    mkdir -p ${RELEASE_PATH}/media/LIBRARY/libcedarx/libcore/parser
    mkdir -p ${RELEASE_PATH}/media/LIBRARY/libaiBDII
    mkdir -p ${RELEASE_PATH}/media/LIBRARY/libaiHCNT
    mkdir -p ${RELEASE_PATH}/media/LIBRARY/libaiMOD
    mkdir -p ${RELEASE_PATH}/media/LIBRARY/libeveface
    mkdir -p ${RELEASE_PATH}/media/LIBRARY/libevekernel    
    mkdir -p ${RELEASE_PATH}/media/LIBRARY/libISE
    mkdir -p ${RELEASE_PATH}/media/LIBRARY/libisp
    mkdir -p ${RELEASE_PATH}/media/LIBRARY/libisp/iniparser/src
    mkdir -p ${RELEASE_PATH}/media/LIBRARY/libisp/isp_dev
    mkdir -p ${RELEASE_PATH}/media/LIBRARY/libisp/isp_tuning
    mkdir -p ${RELEASE_PATH}/media/LIBRARY/libkfc
    mkdir -p ${RELEASE_PATH}/media/LIBRARY/libVideoStabilization
    mkdir -p ${RELEASE_PATH}/media/LIBRARY/libVLPR
    cp -rf ${SOURCE_PATH}/config                                                ${RELEASE_PATH}
    cp -rf ${SOURCE_PATH}/include                                               ${RELEASE_PATH}
    cp -rf ${SOURCE_PATH}/media/include                                         ${RELEASE_PATH}/media
    cp -rf ${SOURCE_PATH}/media/LIBRARY/audioDecLib/include                     ${RELEASE_PATH}/media/LIBRARY/audioDecLib
    cp -rf ${SOURCE_PATH}/media/LIBRARY/audioEffectLib/include                  ${RELEASE_PATH}/media/LIBRARY/audioEffectLib
    cp -rf ${SOURCE_PATH}/media/LIBRARY/audioEncLib/include                     ${RELEASE_PATH}/media/LIBRARY/audioEncLib
    cp -rf ${SOURCE_PATH}/media/LIBRARY/include_ai_common                       ${RELEASE_PATH}/media/LIBRARY
    cp -rf ${SOURCE_PATH}/media/LIBRARY/include_eve_common                      ${RELEASE_PATH}/media/LIBRARY
    cp -rf ${SOURCE_PATH}/media/LIBRARY/include_demux                           ${RELEASE_PATH}/media/LIBRARY
    cp -rf ${SOURCE_PATH}/media/LIBRARY/include_FsWriter                        ${RELEASE_PATH}/media/LIBRARY
    cp -rf ${SOURCE_PATH}/media/LIBRARY/include_muxer                           ${RELEASE_PATH}/media/LIBRARY
    cp -rf ${SOURCE_PATH}/media/LIBRARY/include_stream                          ${RELEASE_PATH}/media/LIBRARY
    cp -rf ${SOURCE_PATH}/media/LIBRARY/libai_common                            ${RELEASE_PATH}/media/LIBRARY
    cp -rf ${SOURCE_PATH}/media/LIBRARY/libaiBDII                               ${RELEASE_PATH}/media/LIBRARY
    cp -rf ${SOURCE_PATH}/media/LIBRARY/libaiHCNT                               ${RELEASE_PATH}/media/LIBRARY
    cp -rf ${SOURCE_PATH}/media/LIBRARY/libaiMOD                                ${RELEASE_PATH}/media/LIBRARY
    cp -rf ${SOURCE_PATH}/media/LIBRARY/libcedarc/include                       ${RELEASE_PATH}/media/LIBRARY/libcedarc
    cp -rf ${SOURCE_PATH}/media/LIBRARY/libcedarx/*.h                           ${RELEASE_PATH}/media/LIBRARY/libcedarx
    cp -rf ${SOURCE_PATH}/media/LIBRARY/libcedarx/*.mk                          ${RELEASE_PATH}/media/LIBRARY/libcedarx
    cp -rf ${SOURCE_PATH}/media/LIBRARY/libcedarx/config                        ${RELEASE_PATH}/media/LIBRARY/libcedarx
    cp -rf ${SOURCE_PATH}/media/LIBRARY/libcedarx/libcore/common/iniparser/*.h  ${RELEASE_PATH}/media/LIBRARY/libcedarx/libcore/common/iniparser
    cp -rf ${SOURCE_PATH}/media/LIBRARY/libcedarx/libcore/parser/include        ${RELEASE_PATH}/media/LIBRARY/libcedarx/libcore/parser
    cp -rf ${SOURCE_PATH}/media/LIBRARY/libeveface                              ${RELEASE_PATH}/media/LIBRARY
    cp -rf ${SOURCE_PATH}/media/LIBRARY/libevekernel                            ${RELEASE_PATH}/media/LIBRARY
    cp -rf ${SOURCE_PATH}/media/LIBRARY/libISE/*.h                              ${RELEASE_PATH}/media/LIBRARY/libISE
    cp -rf ${SOURCE_PATH}/media/LIBRARY/libisp/include                          ${RELEASE_PATH}/media/LIBRARY/libisp
    cp -rf ${SOURCE_PATH}/media/LIBRARY/libisp/iniparser/src/*.h                ${RELEASE_PATH}/media/LIBRARY/libisp/iniparser/src
    cp -rf ${SOURCE_PATH}/media/LIBRARY/libisp/isp_cfg                          ${RELEASE_PATH}/media/LIBRARY/libisp
    cp -rf ${SOURCE_PATH}/media/LIBRARY/libisp/isp_dev/*.h                      ${RELEASE_PATH}/media/LIBRARY/libisp/isp_dev
    cp -rf ${SOURCE_PATH}/media/LIBRARY/libisp/isp_tuning/*.h                   ${RELEASE_PATH}/media/LIBRARY/libisp/isp_tuning
    cp -rf ${SOURCE_PATH}/media/LIBRARY/libkfc                                  ${RELEASE_PATH}/media/LIBRARY
    cp -rf ${SOURCE_PATH}/media/LIBRARY/libVideoStabilization/*.h*              ${RELEASE_PATH}/media/LIBRARY/libVideoStabilization
    cp -rf ${SOURCE_PATH}/media/LIBRARY/libVLPR                                 ${RELEASE_PATH}/media/LIBRARY
    cp -rf ${SOURCE_PATH}/sample                                                ${RELEASE_PATH}
    cp -rf ${SOURCE_PATH}/app_mpp_sample                                        ${RELEASE_PATH}
    cp -rf ${TARGET_PATH}/modules_release.mk                                    ${RELEASE_PATH}/modules.mk
    cp -rf ${TARGET_PATH}/build_lib.mk                                          ${LIBRARY_PATH}/build.mk
}

build_code()
{
    mkmpp -Bj
}

copy_libs()
{
    find ${COMPILE_SO_PATH} -name "libmedia_mpp.so"         | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libmpp_ise.so"           | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libmpp_isp.so"           | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libaw_mpp.a"             | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libmpp_vi.so"            | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libmpp_vo.so"            | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_SO_PATH} -name "libmpp_component.so"     | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_SO_PATH} -name "libnormal_audio.so"      | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_SO_PATH} -name "libadecoder.so"          | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_A_PATH}  -name "libadecoder.a"           | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_SO_PATH} -name "libAudioAec.so"          | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_A_PATH}  -name "libaudioaec.a"           | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_SO_PATH} -name "libAudioEq.so"           | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_A_PATH}  -name "libAwEq.a"               | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_SO_PATH} -name "libAudioGain.so"         | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_A_PATH}  -name "libAudioGain.a"          | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_SO_PATH} -name "libAudioResample.so"     | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_A_PATH}  -name "libaudioresample.a"      | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_SO_PATH} -name "libAudioDrc.so"          | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_A_PATH}  -name "libAwDrc.a"              | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_SO_PATH} -name "libAudioNosc.so"         | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_A_PATH}  -name "libAwNosc.a"             | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_SO_PATH} -name "libcedarx_aencoder.so"   | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_A_PATH}  -name "libcedarx_aencoder.a"    | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_A_PATH}  -name "libaacenc.a"  		    | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_A_PATH}  -name "libadpcmenc.a"           | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_A_PATH}  -name "libg711enc.a"            | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_A_PATH}  -name "libg726enc.a"            | xargs -I{} cp -rf {} ${LIBRARY_PATH}
	find ${COMPILE_A_PATH}  -name "libmp3enc.a"             | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libcdc_base.so"          | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libcdc_base.a"           | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libawh264.so"            | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libawh264.a"             | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libawh265.so"            | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libawh265.a"             | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libawmjpegplus.so"       | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libawmjpegplus.a"        | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libVE.so"                | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libVE.a"                 | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libvencoder.so"          | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libvencoder.a"           | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libvenc_codec.so"        | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libvenc_codec.a"         | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libvenc_base.so"         | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libvenc_base.a"          | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libvideoengine.so"       | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libvideoengine.a"        | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libMemAdapter.so"        | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libMemAdapter.a"         | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libvdecoder.so"          | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libvdecoder.a"           | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libcdx_base.so"          | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libcdx_common.so"        | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libcdx_parser.so"        | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libcdx_stream.so"        | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libcedarxdemuxer.a"      | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libFsWriter.a"           | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "lib_ise_bi.so"           | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "lib_ise_bi_soft.so"      | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "lib_ise_bi.a"            | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "lib_ise_mo.so"           | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "lib_ise_mo.a"            | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "lib_ise_sti.so"          | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "lib_ise_sti.a"           | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libISP.so"               | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libISP.a"                | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libiniparser.a"          | xargs -I{} cp -rf {} ${LIBRARY_PATH}
#    find ${COMPILE_A_PATH}  -name "libisp_ini.a"            | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libisp_dev.a"            | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libisp_ae.a"             | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libisp_af.a"             | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libisp_afs.a"            | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libisp_awb.a"            | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libisp_base.a"           | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libisp_gtm.a"            | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libisp_iso.a"            | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libisp_math.a"           | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libisp_md.a"             | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libisp_pltm.a"           | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libisp_rolloff.a"        | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libmatrix.a"             | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libaac_muxer.a"          | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libffavutil.a"           | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libmp3_muxer.a"          | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libmp4_muxer.a"          | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libmpeg2ts_muxer.a"      | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libmuxers.a"             | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libraw_muxer.a"          | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libcedarxstream.so"      | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libcedarxstream.a"       | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libIRIDALABS_ViSta.so"   | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libcedarxrender.so"      | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libcedarxrender.a"       | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_SO_PATH} -name "libmedia_utils.so"       | xargs -I{} cp -rf {} ${LIBRARY_PATH}
    find ${COMPILE_A_PATH}  -name "libmedia_utils.a"        | xargs -I{} cp -rf {} ${LIBRARY_PATH}
}

tar_files()
{
    tar -czf ${TARGET_PATH}/middleware.tgz -C ${TARGET_PATH} ${RELATIVE_DIR}
    rm -rf ${RELEASE_PATH}
}

main()
{
    source_sdk
    COMPILE_SO_PATH=${TARGET_OUT}/target/usr/lib
    COMPILE_A_PATH=${TARGET_OUT}/staging/usr/lib
    clean_code
    mkdir_release_path
    build_code
    copy_libs
    clean_code
    tar_files

    echo -e "\033[40;31m finish: release packet middleware.tgz\033[0m"
    echo -e "\033[40;31m success! \033[0m"
    exit 0
}

##
# start to execute
#(1)get the option
while getopts :hp: OPTION
do
    echo args: $OPTION -- [$OPTARG]
    case $OPTION in
    h) 
        show_help
        exit 0
        ;;
    p) 
        PLATFORM=$OPTARG
        ;;
    *) 
        show_help
        exit 1
        ;;
    esac
done

#(2)check PLATFORM option.
if [ -z "$PLATFORM" ]; then
    show_help
    exit 1
fi

#(3)execute main
main $@
