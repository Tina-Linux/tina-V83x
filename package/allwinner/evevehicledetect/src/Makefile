Target = evevehicledetect
## set the include path for compile flags.
PWD := $(shell pwd)
SourceIncludePath := -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner -I$(PWD)/include
CompileFlags = $(CFLAGS) $(SourceIncludePath)
LoadFlags += -leve_event -lm -lstdc++ -lcamapi -lisp -lisp_ini -lAWIspApi -lopencv_core -lopencv_highgui -lopencv_imgproc -luapi

$(Target): evevehicledetect.cpp
	$(CC) -g -o $@ $^ $(CompileFlags) $(LDFLAGS) $(LoadFlags)
