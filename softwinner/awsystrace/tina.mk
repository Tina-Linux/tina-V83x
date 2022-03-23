############################################################################################
# 			awsystrace for tina(OpenWrt) Linux
#
#	awsystrace is designed to debug system running info.
#
# Version: v1.0
# Date   : 2019-7-1
# Author :
############################################################################################
all:
	@echo ==================================================
	@echo build awsystrace
	@echo ==================================================
	make -C external -f tina_awtrace.mk                 all
	make -C external -f tina_atrace.mk                  all
#	make -C external -f tina_awtrace_example.mk         all
	@echo build awsystrace done!

clean:
	@echo ==================================================
	@echo clean awsystrace
	@echo ==================================================
	make -C external -f tina_awtrace.mk                 clean
	make -C external -f tina_atrace.mk                  clean
#	make -C external -f tina_awtrace_example.mk         clean
	@echo clean awsystrace done!
