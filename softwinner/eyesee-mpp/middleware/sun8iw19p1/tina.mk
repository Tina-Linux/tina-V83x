############################################################################################
# 			eyesee-mpp-middleware for tina(OpenWrt) Linux
#
#	eyesee-mpp is designed for CDR/SDV product, focus on video/audio capturing and encoding, 
# it also can support video/audio decode.
#   eyesee-mpp-middleware provides basic libraries of mpp.
# libDisplay, etc.
#
# Version: v1.0
# Date   : 2019-1-23
# Author : PDC-PD5
############################################################################################
CUR_PATH := $(shell pwd)
include $(CUR_PATH)/config/mpp_config.mk

all:
	@echo ==================================================
	@echo build eyesee-mpp-middleware
	@echo ==================================================
	make -C media/utils -f tina.mk                                 all
	make -C media/LIBRARY/libstream -f tina.mk                    all
ifeq ($(MPPCFG_MUXER),Y)
	make -C media/LIBRARY/libFsWriter -f tina.mk                  all
	make -C media/LIBRARY/libmuxer/common/libavutil -f tina.mk    all
	make -C media/LIBRARY/libmuxer/mp3_muxer -f tina.mk           all
	make -C media/LIBRARY/libmuxer/aac_muxer -f tina.mk           all
	make -C media/LIBRARY/libmuxer/mp4_muxer -f tina.mk           all
	make -C media/LIBRARY/libmuxer/mpeg2ts_muxer -f tina.mk       all
	make -C media/LIBRARY/libmuxer/raw_muxer -f tina.mk           all
	make -C media/LIBRARY/libmuxer/muxers -f tina.mk              all
endif
ifeq ($(MPPCFG_VI),Y)
	PACKAGE_TOP=$(CUR_PATH) make -C media/LIBRARY/libisp    -f tina.mk  all
endif
ifeq ($(MPPCFG_ISE),Y)
	make -C media/LIBRARY/libISE -f tina.mk                       all
endif
	PACKAGE_TOP=$(CUR_PATH) make -C media/LIBRARY/libcedarc -f tina.mk  all
	make -C media/LIBRARY/libcedarx/config                  -f tina.mk  all
	make -C media/LIBRARY/libcedarx/libcore/base            -f tina.mk  all
	make -C media/LIBRARY/libcedarx/libcore/common          -f tina.mk  all
ifeq ($(MPPCFG_DEMUXER),Y)
	make -C media/LIBRARY/libcedarx/libcore/stream/file     -f tina.mk  all
	make -C media/LIBRARY/libcedarx/libcore/stream/base     -f tina.mk  all
	make -C media/LIBRARY/libcedarx/libcore/parser/aac      -f tina.mk  all
	make -C media/LIBRARY/libcedarx/libcore/parser/id3v2    -f tina.mk  all
	make -C media/LIBRARY/libcedarx/libcore/parser/mp3      -f tina.mk  all
	make -C media/LIBRARY/libcedarx/libcore/parser/mov      -f tina.mk  all
	make -C media/LIBRARY/libcedarx/libcore/parser/mpg      -f tina.mk  all
	make -C media/LIBRARY/libcedarx/libcore/parser/ts       -f tina.mk  all
	make -C media/LIBRARY/libcedarx/libcore/parser/base     -f tina.mk  all
	make -C media/LIBRARY/libdemux -f tina.mk                     all
endif
	make -C media/LIBRARY/AudioLib/lib -f tina.mk       all
	make -C media/LIBRARY/AudioLib/midware/encoding -f tina.mk all
	make -C media/LIBRARY/AudioLib/midware/decoding -f tina.mk all
ifeq ($(MPPCFG_TEXTENC),Y)
	make -C media/LIBRARY/textEncLib -f tina.mk all
endif
ifeq ($(MPPCFG_AEC),Y)
	make -C media/LIBRARY/aec_lib -f tina.mk all
endif

ifeq ($(MPPCFG_ANS),Y)
	make -C media/LIBRARY/ans_lib -f tina.mk all
endif

	make -C media/LIBRARY/libMODSoft -f tina.mk                     all
ifneq ($(filter Y, $(MPPCFG_ADAS_DETECT) $(MPPCFG_ADAS_DETECT_V2)),)
	make -C media/LIBRARY/libADAS -f tina.mk                     	all
endif
ifeq ($(MPPCFG_EIS),Y)
	PACKAGE_TOP=$(CUR_PATH) make -C media/LIBRARY/libVideoStabilization -f tina.mk all
endif
	make -C media/LIBRARY/lib_aw_ai_algo -f tina.mk               all
	make -C media/LIBRARY/lib_aw_ai_core -f tina.mk               all
	make -C media/LIBRARY/lib_aw_ai_mt -f tina.mk                 all
ifeq ($(MPPCFG_VO),Y)
	make -C media/librender -f tina.mk                            all
endif
ifeq ($(MPPCFG_VI),Y)
	make -C media -f tina_mpp_vi.mk                     all
	make -C media -f tina_mpp_isp.mk                    all
endif
ifeq ($(MPPCFG_ISE),Y)
	make -C media -f tina_mpp_ise.mk                    all
endif
ifeq ($(MPPCFG_EIS),Y)
	make -C media -f tina_mpp_eis.mk                    all
endif
ifeq ($(MPPCFG_VO),Y)
	make -C media -f tina_mpp_vo.mk                     all
endif
ifeq ($(MPPCFG_UVC),Y)
	make -C media -f tina_mpp_uvc.mk                    all
endif
	make -C media/component -f tina.mk                            all
	make -C media -f tina.mk                                      all
#	make -C media/isp_tool                              all
	make -C media -f tina_mpp_static.mk                 all
	make -C sample/configfileparser -f tina.mk                    all
#	make -C sample/sample_adec -f tina.mk               all
#	make -C sample/sample_aenc -f tina.mk               all
#	make -C sample/sample_ai -f tina.mk                 all
#	make -C sample/sample_ai2aenc -f tina.mk            all
#	make -C sample/sample_ai2aenc2muxer -f tina.mk      all
#	make -C sample/sample_ai2ao -f tina.mk              all
#	make -C sample/sample_ai2ao_aec -f tina.mk              all
#	make -C sample/sample_ao -f tina.mk                 all
#	make -C sample/sample_ao2ai -f tina.mk              all
#	make -C sample/sample_ao2ai_aec -f tina.mk              all
#	make -C sample/sample_demux -f tina.mk              all
#	make -C sample/sample_demux2adec -f tina.mk         all
#	make -C sample/sample_demux2adec2ao -f tina.mk      all
#	make -C sample/sample_demux2vdec -f tina.mk         all
#	make -C sample/sample_demux2vdec2vo -f tina.mk      all
#	make -C sample/sample_demux2vdec_saveFrame -f tina.mk all
##	make -C sample/sample_face_detect -f tina.mk        all
#	make -C sample/sample_fish -f tina.mk               all
#	make -C sample/sample_g2d -f tina.mk                all
#	make -C sample/sample_glog -f tina.mk               all
#	make -C sample/sample_hello -f tina.mk              all
#	make -C sample/sample_motor -f tina.mk              all
#	make -C sample/sample_nna/sample_onet -f tina.mk    all
#	make -C sample/sample_nna/sample_rnet -f tina.mk    all
#	make -C sample/sample_region -f tina.mk             all
#	make -C sample/sample_rtsp -f tina.mk                 all
#	make -C sample/sample_isposd -f tina.mk             all
#	make -C sample/sample_select -f tina.mk             all
#	make -C sample/sample_sound_controler -f tina.mk    all
#	make -C sample/sample_timelapse -f tina.mk          all
#	make -C sample/sample_UILayer -f tina.mk            all
##	make -C sample/sample_UVC/sample_uvc2vdec_vo -f tina.mk all
##	make -C sample/sample_UVC/sample_uvc2vdenc2vo -f tina.mk all
##	make -C sample/sample_UVC/sample_uvc2vo -f tina.mk  all
##	make -C sample/sample_UVC/sample_uvc_vo -f tina.mk  all
##	make -C sample/sample_uvcout -f tina.mk             all
#	make -C sample/sample_vdec -f tina.mk               all
#	make -C sample/sample_venc -f tina.mk               all
#	make -C sample/sample_venc2muxer -f tina.mk         all
#	make -C sample/sample_v459_BGA -f tina.mk           all
#	make -C sample/sample_v459_QFN -f tina.mk           all
#	make -C sample/sample_vin_isp_test -f tina.mk       all
#	make -C sample/sample_virvi -f tina.mk              all
#	make -C sample/sample_virvi2eis2venc -f tina.mk     all
#	make -C sample/sample_virvi2fish2venc -f tina.mk    all
#	make -C sample/sample_virvi2fish2vo -f tina.mk      all
#	make -C sample/sample_virvi2venc -f tina.mk         all
#	make -C sample/sample_virvi2venc2muxer -f tina.mk   all
#	make -C sample/sample_virvi2venc2ce -f tina.mk      all
#	make -C sample/sample_virvi2vo -f tina.mk           all
#	make -C sample/sample_vi_reset -f tina.mk           all
#	make -C sample/sample_vo -f tina.mk                 all
	@echo build eyesee-mpp-middleware done!

clean:
	@echo ==================================================
	@echo clean eyesee-mpp-middleware
	@echo ==================================================
	make -C media/utils -f tina.mk                                clean
	make -C media/LIBRARY/libstream -f tina.mk                    clean
	make -C media/LIBRARY/libFsWriter -f tina.mk                  clean
	make -C media/LIBRARY/libmuxer/common/libavutil -f tina.mk    clean
	make -C media/LIBRARY/libmuxer/mp3_muxer -f tina.mk           clean
	make -C media/LIBRARY/libmuxer/aac_muxer -f tina.mk           clean
	make -C media/LIBRARY/libmuxer/mp4_muxer -f tina.mk           clean
	make -C media/LIBRARY/libmuxer/mpeg2ts_muxer -f tina.mk       clean
	make -C media/LIBRARY/libmuxer/raw_muxer -f tina.mk           clean
	make -C media/LIBRARY/libmuxer/muxers -f tina.mk              clean
	PACKAGE_TOP=$(CUR_PATH) make -C media/LIBRARY/libisp    -f tina.mk  clean
	make -C media/LIBRARY/libISE -f tina.mk                       clean
	PACKAGE_TOP=$(CUR_PATH) make -C media/LIBRARY/libcedarc -f tina.mk  clean
	make -C media/LIBRARY/libcedarx/config -f tina.mk   clean
	make -C media/LIBRARY/libcedarx/libcore/base -f tina.mk clean
	make -C media/LIBRARY/libcedarx/libcore/common -f tina.mk clean
	make -C media/LIBRARY/libcedarx/libcore/stream/file -f tina.mk clean
	make -C media/LIBRARY/libcedarx/libcore/stream/base -f tina.mk clean
	make -C media/LIBRARY/libcedarx/libcore/parser/aac -f tina.mk clean
	make -C media/LIBRARY/libcedarx/libcore/parser/id3v2 -f tina.mk clean
	make -C media/LIBRARY/libcedarx/libcore/parser/mp3 -f tina.mk clean
	make -C media/LIBRARY/libcedarx/libcore/parser/mov -f tina.mk clean
	make -C media/LIBRARY/libcedarx/libcore/parser/mpg -f tina.mk clean
	make -C media/LIBRARY/libcedarx/libcore/parser/ts -f tina.mk clean
	make -C media/LIBRARY/libcedarx/libcore/parser/base -f tina.mk clean
	make -C media/LIBRARY/libdemux -f tina.mk                     clean
	make -C media/LIBRARY/AudioLib/lib -f tina.mk       clean
	make -C media/LIBRARY/aec_lib -f tina.mk clean
	make -C media/LIBRARY/ans_lib -f tina.mk clean
	make -C media/LIBRARY/AudioLib/midware/encoding -f tina.mk clean
	make -C media/LIBRARY/textEncLib -f tina.mk clean
	make -C media/LIBRARY/AudioLib/midware/decoding -f tina.mk clean
	make -C media/LIBRARY/libMODSoft -f tina.mk                   clean
	make -C media/LIBRARY/libADAS -f tina.mk                   clean
	PACKAGE_TOP=$(CUR_PATH) make -C media/LIBRARY/libVideoStabilization -f tina.mk  clean
	make -C media/LIBRARY/lib_aw_ai_algo -f tina.mk               clean
	make -C media/LIBRARY/lib_aw_ai_core -f tina.mk               clean
	make -C media/LIBRARY/lib_aw_ai_mt -f tina.mk                 clean
	make -C media/librender -f tina.mk                            clean
	make -C media -f tina_mpp_vi.mk                     clean
	make -C media -f tina_mpp_isp.mk                    clean
	make -C media -f tina_mpp_ise.mk                    clean
	make -C media -f tina_mpp_eis.mk                    clean
	make -C media -f tina_mpp_vo.mk                     clean
	make -C media -f tina_mpp_uvc.mk                    clean
	make -C media/component -f tina.mk                            clean
	make -C media -f tina.mk                                      clean
#	make -C media/isp_tool                              clean
	make -C media -f tina_mpp_static.mk                 clean
	make -C sample/configfileparser -f tina.mk                    clean
#	make -C sample/sample_adec -f tina.mk               clean
#	make -C sample/sample_aenc -f tina.mk               clean
#	make -C sample/sample_ai -f tina.mk                 clean
#	make -C sample/sample_ai2aenc -f tina.mk            clean
#	make -C sample/sample_ai2aenc2muxer -f tina.mk      clean
#	make -C sample/sample_ai2ao -f tina.mk              clean
#	make -C sample/sample_ai2ao_aec -f tina.mk          clean
#	make -C sample/sample_ao -f tina.mk                 clean
#	make -C sample/sample_ao2ai -f tina.mk              clean
#	make -C sample/sample_ao2ai_aec -f tina.mk          clean
#	make -C sample/sample_demux -f tina.mk              clean
#	make -C sample/sample_demux2adec -f tina.mk         clean
#	make -C sample/sample_demux2adec2ao -f tina.mk      clean
#	make -C sample/sample_demux2vdec -f tina.mk         clean
#	make -C sample/sample_demux2vdec2vo -f tina.mk      clean
#	make -C sample/sample_demux2vdec_saveFrame -f tina.mk clean
##	make -C sample/sample_face_detect -f tina.mk        clean
#	make -C sample/sample_fish -f tina.mk               clean
#	make -C sample/sample_g2d -f tina.mk                clean
#	make -C sample/sample_glog -f tina.mk               clean
#	make -C sample/sample_hello -f tina.mk              clean
#	make -C sample/sample_motor -f tina.mk              clean
#	make -C sample/sample_nna/sample_onet -f tina.mk    clean
#	make -C sample/sample_nna/sample_rnet -f tina.mk    clean
#	make -C sample/sample_region -f tina.mk             clean
#	make -C sample/sample_rtsp -f tina.mk                 clean
#	make -C sample/sample_isposd -f tina.mk             clean
#	make -C sample/sample_select -f tina.mk             clean
#	make -C sample/sample_sound_controler -f tina.mk    clean
#	make -C sample/sample_timelapse -f tina.mk          clean
#	make -C sample/sample_UILayer -f tina.mk            clean
##	make -C sample/sample_UVC/sample_uvc2vdec_vo -f tina.mk clean
##	make -C sample/sample_UVC/sample_uvc2vdenc2vo -f tina.mk clean
##	make -C sample/sample_UVC/sample_uvc2vo -f tina.mk  clean
##	make -C sample/sample_UVC/sample_uvc_vo -f tina.mk  clean
##	make -C sample/sample_uvcout -f tina.mk             clean
#	make -C sample/sample_v459_BGA -f tina.mk           clean
#	make -C sample/sample_v459_QFN -f tina.mk           clean
#	make -C sample/sample_vdec -f tina.mk               clean
#	make -C sample/sample_venc -f tina.mk               clean
#	make -C sample/sample_venc2muxer -f tina.mk         clean
#	make -C sample/sample_vin_isp_test -f tina.mk       clean
#	make -C sample/sample_virvi -f tina.mk              clean
#	make -C sample/sample_virvi2eis2venc -f tina.mk     clean
#	make -C sample/sample_virvi2fish2venc -f tina.mk    clean
#	make -C sample/sample_virvi2fish2vo -f tina.mk      clean
#	make -C sample/sample_virvi2venc -f tina.mk         clean
#	make -C sample/sample_virvi2venc2muxer -f tina.mk   clean
#	make -C sample/sample_virvi2venc2ce -f tina.mk      clean
#	make -C sample/sample_virvi2vo -f tina.mk           clean
#	make -C sample/sample_vi_reset -f tina.mk           clean
#	make -C sample/sample_vo -f tina.mk                 clean
	@echo clean eyesee-mpp-middleware done!
