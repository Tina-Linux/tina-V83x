############################################################################################
# 			eyesee-mpp-system for tina(OpenWrt) Linux
#
#	eyesee-mpp is designed for CDR/SDV product, focus on video/audio capturing and encoding, 
# it also can support video/audio decode.
#   eyesee-mpp-system is lower level for providing basic libraries such as libion, liblog,
# libDisplay, etc.
#
# Version: v1.0
# Date   : 2019-1-18
# Author : PDC-PD5
############################################################################################

all:
	@echo ==================================================
	@echo build eyesee-mpp-system
	@echo ==================================================
	make -C liblog -f tina.mk              all
	make -C logcat -f tina.mk              all
	make -C libion -f tina.mk              all
	make -C libcutils -f tina.mk            all
	make -C display -f tina.mk             all

	make -C newfs_msdos -f tina.mk         all
	make -C reboot_efex -f tina.mk         all
	make -C luaconfig -f tina.mk           all
	make -C wifi/wpa_supplicant -f tina.mk all
	make -C wifi -f tina.mk      all
	make -C smartlink -f tina.mk           all
	make -C rgb_ctrl -f tina.mk            all
	make -C ntpclient -f tina.mk           all
	make -C rtsp -f tina.mk           all
	@echo build eyesee-mpp-system done!

clean:
	@echo ==================================================
	@echo clean eyesee-mpp-system
	@echo ==================================================
	make -C liblog -f tina.mk              clean
	make -C logcat -f tina.mk              clean
	make -C libion -f tina.mk              clean
	make -C libcutils -f tina.mk           clean
	make -C display -f tina.mk             clean

	make -C newfs_msdos -f tina.mk         clean
	make -C reboot_efex -f tina.mk         clean
	make -C luaconfig -f tina.mk           clean
	make -C wifi/wpa_supplicant -f tina.mk clean
	make -C wifi -f tina.mk -f tina.mk     clean
	make -C smartlink -f tina.mk           clean
	make -C rgb_ctrl -f tina.mk            clean
	make -C ntpclient -f tina.mk           clean
	make -C rtsp -f tina.mk           clean
	@echo clean eyesee-mpp-system done!
