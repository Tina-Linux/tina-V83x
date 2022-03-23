############################################################################################
# 			eyesee-mpp-framework for tina(OpenWrt) Linux
#
#	eyesee-mpp is designed for CDR/SDV product, focus on video/audio capturing and encoding, 
# it also can support video/audio decode.
#   eyesee-mpp-framework is designed for our apps. It wraps the use of mpp-components, 
#   providing simple classes such as camera, recorder and player to apps to use.
#
# Version: v1.0
# Date   : 2019-2-18
# Author : PDC-PD5
############################################################################################
all:
	@echo ==================================================
	@echo build eyesee-mpp-framework
	@echo ==================================================
	make -C utils -f tina.mk                  all
	make -C media/camera -f tina.mk           all
	make -C media/ise -f tina.mk              all
	make -C media/eis -f tina.mk              all
	make -C media/recorder -f tina.mk         all
	make -C media/player -f tina.mk           all
	make -C media/thumbretriever -f tina.mk   all
	make -C media/motion -f tina.mk           all
	make -C media/usbcamera -f tina.mk        all
	make -C media/videoresizer -f tina.mk     all
	make -C media/bdii -f tina.mk             all
#	make -C demo/sample_ADAS  -f tina.mk      all
#	make -C demo/sample_Camera -f tina.mk     all
#	make -C demo/sample_EncodeResolutionChange -f tina.mk all
	@echo build eyesee-mpp-framework done!

clean:
	@echo ==================================================
	@echo clean eyesee-mpp-framework
	@echo ==================================================
	make -C utils -f tina.mk                  clean
	make -C media/camera -f tina.mk           clean
	make -C media/ise -f tina.mk              clean
	make -C media/eis -f tina.mk              clean
	make -C media/recorder -f tina.mk         clean
	make -C media/player -f tina.mk           clean
	make -C media/thumbretriever -f tina.mk   clean
	make -C media/motion -f tina.mk           clean
	make -C media/usbcamera -f tina.mk        clean
	make -C media/videoresizer -f tina.mk     clean
	make -C media/bdii -f tina.mk             clean
#	make -C demo/sample_ADAS  -f tina.mk	  clean
#	make -C demo/sample_Camera -f tina.mk     clean
#	make -C demo/sample_EncodeResolutionChange -f tina.mk clean
	@echo clean eyesee-mpp-framework done!
