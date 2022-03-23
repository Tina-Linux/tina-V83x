
include config.mk
include ../Makefile.inc
all:
	make -C DEMUX/
	make -C PLAYER/
	make -C ../awrecorder/
	make -C ../xplayer/
	make -C ../xmetadataretriever/
	make -C DEMO/


clean:
	make clean -C DEMUX/
	make clean -C PLAYER/
	make clean -C DEMO/
	make clean -C ../awrecorder/
	make clean -C ../xplayer/
	make clean -C ../xmetadataretriever/


cleanall:
	make cleanall -C DEMUX/
	make cleanall -C PLAYER/
	make cleanall -C DEMO/
	make cleanall -C ../awrecorder/
	make cleanall -C ../xplayer/
	make cleanall -C ../xmetadataretriever/

	-rm -rf install


install:
    ############################################################
	## copy libraries
	-mkdir -p install/lib/
	-cp DEMUX/BASE/libcdx_base.so               install/lib/
	-cp DEMUX/STREAM/libcdx_stream.so           install/lib/
	-cp DEMUX/PARSER/libcdx_parser.so           install/lib/
	-cp DEMUX/MUXER/libcdx_muxer.so             install/lib/
	-cp PLAYER/libplayer.so                     install/lib/
	-cp ../awrecorder/libawrecorder.so          install/lib/
	-cp ../xplayer/libxplayer.so                install/lib/
	-cp ../xmetadataretriever/libxmetaretriever.so    install/lib/
	-cp PLUGIN/manager/libaw_plugin.so                install/lib/

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
	-cp EXTERNAL/lib32/lgnueabi/libnormal_audio.so  install/lib/
	-cp EXTERNAL/lib32/lgnueabi/libpostprocess.so  install/lib/
	-cp EXTERNAL/lib32/lgnueabi/libcdc_adecoder.so  install/lib/
	-cp EXTERNAL/lib32/lgnueabi/libcdc_aencoder.so  install/lib/
	-cp EXTERNAL/lib32/lgnueabi/liblive555.so  install/lib/

ifeq ($(LINUX_VERSION),$(LINUX_VERSION_3_10))
	-cp libcedarc/library/lib32/lgnueabi_linux310/*.so      install/lib/
else
	-cp libcedarc/library/lib32/lgnueabi/*.so      install/lib/
endif

	-mkdir -p install/external/
	-cp EXTERNAL/lib32/lgnueabi/*.so                  install/external/
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABIHF))
	-cp EXTERNAL/lib32/lgnueabihf/libnormal_audio.so  install/lib/
	-cp EXTERNAL/lib32/lgnueabihf/libpostprocess.so  install/lib/
	-cp EXTERNAL/lib32/lgnueabihf/libcdc_adecoder.so  install/lib/
	-cp EXTERNAL/lib32/lgnueabihf/libcdc_aencoder.so  install/lib/
	-cp EXTERNAL/lib32/lgnueabihf/liblive555.so  install/lib/

ifeq ($(LINUX_VERSION),$(LINUX_VERSION_3_10))
	-cp libcedarc/library/lib32/lgnueabihf_linux310/*.so      install/lib/
else
	-cp libcedarc/library/lib32/lgnueabihf/*.so      install/lib/
endif

	-mkdir -p install/external/
	-cp EXTERNAL/lib32/lgnueabihf/*.so                  install/external/
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCGNUEABI))
	-cp EXTERNAL/lib32/uclgnueabi/libnormal_audio.so  install/lib/
	-cp EXTERNAL/lib32/uclgnueabi/libpostprocess.so  install/lib/
	-cp EXTERNAL/lib32/uclgnueabi/liblive555.so  install/lib/
	-cp EXTERNAL/lib32/uclgnueabi/libcdc_adecoder.so  install/lib/
	-cp EXTERNAL/lib32/uclgnueabi/libcdc_aencoder.so  install/lib/
	-cp libcedarc/library/lib32/uclgnueabi/*.so      install/lib/

	-mkdir -p install/external/
	-cp EXTERNAL/lib32/uclgnueabi/*.so                  install/external/
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_UCGNUEABI))
	-cp EXTERNAL/lib32/linuxgnueabi/libnormal_audio.so  install/lib/
	-cp EXTERNAL/lib32/linuxgnueabi/libpostprocess.so  install/lib/
	-cp EXTERNAL/lib32/linuxgnueabi/libcdc_adecoder.so  install/lib/
	-cp EXTERNAL/lib32/linuxgnueabi/libcdc_aencoder.so  install/lib/
	-cp EXTERNAL/lib32/linuxgnueabi/liblive555.so  install/lib/
	-cp libcedarc/library/lib32/linuxgnueabi/*.so      install/lib/

	-mkdir -p install/external/
	-cp EXTERNAL/lib32/linuxgnueabi/*.so                  install/lib/
	-cp EXTERNAL/lib32/linuxgnueabi/*.so                  install/external/
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_MUSLGNUEABI))
	-cp EXTERNAL/lib32/muslgnueabi/libnormal_audio.so  install/lib/
	-cp EXTERNAL/lib32/muslgnueabi/libpostprocess.so  install/lib/
	-cp EXTERNAL/lib32/muslgnueabi/libcdc_adecoder.so  install/lib/
	-cp EXTERNAL/lib32/muslgnueabi/libcdc_aencoder.so  install/lib/
	-cp EXTERNAL/lib32/muslgnueabi/liblive555.so  install/lib/

ifeq ($(LINUX_VERSION),$(LINUX_VERSION_3_10))
	-cp libcedarc/library/lib32/muslgnueabi_linux310/*.so      install/lib/
else
	-cp libcedarc/library/lib32/muslgnueabi/*.so      install/lib/
endif

	-mkdir -p install/external/
	-cp EXTERNAL/lib32/muslgnueabi/*.so                  install/lib/
	-cp EXTERNAL/lib32/muslgnueabi/*.so                  install/external/
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_MUSLGNUEABI64))
	-cp EXTERNAL/lib64/muslgnueabi64/libnormal_audio.so  install/lib/
	-cp EXTERNAL/lib64/muslgnueabi64/libpostprocess.so  install/lib/
	-cp EXTERNAL/lib64/muslgnueabi64/libcdc_adecoder.so  install/lib/
	-cp EXTERNAL/lib64/muslgnueabi64/libcdc_aencoder.so  install/lib/
	-cp EXTERNAL/lib64/muslgnueabi64/liblive555.so  install/lib/

	-cp libcedarc/library/lib64/muslgnueabi_64_linux440/*.so      install/lib/

	-mkdir -p install/external/
	-cp EXTERNAL/lib64/muslgnueabi64/*.so                  install/lib/
	-cp EXTERNAL/lib64/muslgnueabi64/*.so                  install/external/
endif

	#################################################################
	## copy demos.
	-mkdir -p install/bin/
	-cp DEMO/demoPlayer/demoPlayer             install/bin/
	-cp DEMO/recoderdemo/recoderdemo           install/bin/
	-cp DEMO/xplayerdemo/playerdemo            install/bin/
	-cp DEMO/xmetademo/retriverdemo            install/bin/
	-cp DEMO/muxtest/muxerdemo                 install/bin/
	-cp DEMO/jpegdemo/jpegdemo                 install/bin/
	#####################################################################
	## copy header files.
	-mkdir -p install/include/
	-cp cdx_config.h                                install/include/
	-cp log.h                                   install/include/
	-cp version.h                               install/include/
	## DEMUX
	-mkdir -p install/include/DEMUX/BASE/include
	-cp -r DEMUX/BASE/include                   install/include/DEMUX/BASE/include
	-mkdir -p install/include/DEMUX/STREAM/include
	-cp -r DEMUX/STREAM/include                 install/include/DEMUX/STREAM/include
	-mkdir -p install/include/DEMUX/PARSER/include
	-cp -r DEMUX/PARSER/include                 install/include/DEMUX/PARSER/include
	-mkdir -p install/include/DEMUX/MUXER/include
	-cp -r DEMUX/MUXER/include                 install/include/DEMUX/MUXER/include
	## CODEC
	-mkdir -p install/include/libcedarc/include
	-cp libcedarc/include/*.h          install/include/libcedarc/include
	## PLAYER
	-mkdir -p install/include/PLAYER/include
	-cp PLAYER/include/player.h                         install/include/PLAYER/include

	#########################################################################################
	## copy crack lib
	-mkdir -p install/crack

	$(STRIP) install/lib/*.so
	$(STRIP) install/lib/*
	$(STRIP) install/bin/*

.PHONY: all clean cleanall install
