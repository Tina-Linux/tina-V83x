include ../config.mk
all:
#	make -C demoPlayer/
	make -C demoVdecoder/
#	make -C xplayerdemo/
	make -C xmetademo/
	make -C encodedemo/
ifneq ($(CONFIG_PRODUCT),$(OPTION_PRODUCT_LOUDSPEAKER))
	make -C recoderdemo/
	make -C muxtest/
endif
	make -C jpegdemo/

clean:
	make clean -C demoPlayer/
	make clean -C recoderdemo/
	make clean -C demoVdecoder/
	make clean -C xplayerdemo/
	make clean -C xmetademo/
	make clean -C encodedemo/
	make clean -C jpegdemo/
	make clean -C muxtest/
cleanall:
	make cleanall -C demoPlayer/
	make cleanall -C recoderdemo/
	make cleanall -C xplayerdemo/
	make cleanall -C demoVdecoder/
	make cleanall -C xmetademo/
	make cleanall -C encodedemo/
	make cleanall -C jpegdemo/
	make cleanall -C muxtest/
.PHONY: all clean cleanall
