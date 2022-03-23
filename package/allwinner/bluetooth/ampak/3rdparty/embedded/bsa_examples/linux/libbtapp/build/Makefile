#######################################
#  This makefile is written for a GNU make or equivalent.
#  For example:
#   make clean
#   make
# Copyright (c) 2010, Broadcom Corp., All Rights Reserved.
# Broadcom Bluetooth Core. Proprietary and confidential.
#######################################
# Basic path definitions
GLOBALPATH := ../../../../../..
customer := bsa_examples
project := linux
application := libbtapp
os_name := linux
BLUETOOTH_3RD_DIR := $(GLOBALPATH)/3rdparty

ifeq ($(findstring arm,$(CC)), arm)
	CPU = arm
else
ifeq ($(findstring aarch64, $(CC)), aarch64)
	CPU = arm64
endif
endif

target := libbtapp.so

# include common makefile rules
include $(BLUETOOTH_3RD_DIR)/embedded/$(customer)/$(project)/app_common/build/make.common

# extra source list list
srcdirs :=
appsources :=

#GLOBAL_DEFINE += -DCUSTOM_MAC_ENABLE
#GLOBAL_DEFINE += -DRANDOM_MAC_ENABLE

# add ENABLE_ALSA=TRUE at the command line to enable
ENABLE_ALSA ?= FALSE
ifeq ($(strip $(ENABLE_ALSA)), TRUE)
  GLOBAL_DEFINE += -DPCM_ALSA
	LINKLIBS += -lasound -lresamplerate
endif

ifeq ($(strip $(SYSTEM_INIT_PROCD)),y)
	GLOBAL_DEFINE += -DSYSTEM_INIT_PROCD
endif

LINKLIBS += -luci

# include variable definitions for each component
include $(BLUETOOTH_APPLICATION_DIR)/build/def/*.def

# incdirs uses srcdirs
incdirs := $(incdirs) $(srcdirs)

# When not specified, search the files in the source directories
vpath %.c $(srcdirs)
vpath %.s $(srcdirs)
vpath %.cpp $(srcdirs)

# Sample applications also need to include the exported BSA headers
incdirs += $(bsalibincdir)

#######################################
#
# BSA client library
#
#######################################
#ifeq ($(BSASHAREDLIB), TRUE)
bsalib := $(bsasharedlib)
#else
#bsalib := $(bsastaticlib)
#endif

#######################################
#
# Objects
#
#######################################
c_src := $(filter %.c, $(appsources))
c_objs := $(patsubst %.c,$(objdir)/%.o, $(c_src))
cpp_src := $(filter %.cpp, $(appsources))
cpp_objs := $(patsubst %.cpp,$(objdir)/%.o,$(cpp_src))
objs := $(c_objs) $(cpp_objs)
app_objs := $(patsubst %.s,$(objdir)/%.o,$(objs))

objlist := $(app_objs)

#######################################
#
# Targets
#
#######################################

.PHONY: all clean delivery

all: $(target)
	@$(STRIP) ./../../server/build/$(CPU)/$(CONFIG_LIBC)/bsa_server
	@$(CP) -f ./../../server/build/$(CPU)/$(CONFIG_LIBC)/bsa_server ./bsa_server
	@$(ECHO) Done building $@

%.s:%.S
install:
	-@$(MKDIR) -p $(CONFIG_PREFIX)/usr/lib
	-@$(MKDIR) -p $(CONFIG_PREFIX)/usr/include
	@$(CP) ./../include/bluetooth_socket.h $(CONFIG_PREFIX)/usr/include
	@$(CP) ./../include/bluetooth_disc.h $(CONFIG_PREFIX)/usr/include
	@$(CP) $(PKG_PREFIX)/ampak/3rdparty/embedded/bsa_examples/linux/libbsa/build/$(CPU)/$(CONFIG_LIBC)/sharedlib/libbsa.so $(CONFIG_PREFIX)/usr/lib

$(builddir):
	@$(ECHO) CPU is $(CPU)
	@$(ECHO) Creating application directory '$(builddir)'.
	-@$(MKDIR) -p $(builddir)

$(objdir):
	@$(ECHO) Creating object directory '$(objdir)'.
	@$(ECHO) c_objs '$(c_objs)'
	@$(ECHO) cpp_objs '$(cpp_objs)'
	-@$(MKDIR) -p $(objdir)

$(c_objs):$(objdir)/%.o: %.c
	@$(ECHO) Compiling [$(CC)] $<
	$(CC) -fPIC -c -MD -DBUILD_LIB $(CCFLAGS) $(GLOBAL_DEFINE) $(TARGET_DEF) $(CCINC) -o $@ $<

$(cpp_objs):$(objdir)/%.o: %.cpp
	@$(ECHO) Compiling [$(CPP)] $<
	$(CXX) -fPIC -c -MD -DBUILD_LIB $(CCFLAGS) $(GLOBAL_DEFINE) $(TARGET_DEF) $(CCINC) -o $@ $<

# include the dependencies (if they exist)
-include $(objlist:.o=.d)

$(target): install $(builddir) $(objdir) $(objlist)
ifneq ($(strip $(bsalib)),)
	@$(ECHO) Linking application $@ with $(notdir $(bsalib))
	$(LINK) $(LINKFLAGS) -shared $(TARGET_LINK_FLAGS) -o $@ $(objlist) $(LINKLIBS)
else
	@$(ECHO) Linking application $@
	$(LINK) $(LINKFLAGS) -shared $(TARGET_LINK_FLAGS) -o $@ $(objlist) $(LINKLIBS)
endif
	@$(CP) $(target) $(CONFIG_PREFIX)/usr/lib
	@$(CP) $(bsalib) $(CONFIG_PREFIX)/usr/lib

clean:
	@$(ECHO) Cleaning up : $(objdir) $(builddir)
	@$(RM) -rf $(objdir) $(builddir)
	@$(ECHO) Finished cleaning up

delivery:
	@$(ECHO) Creating Delivery in $(DELIVERY_CURRENT_FOLDER)
	@$(MKDIR) -p $(DELIVERIES_MAIN_FOLDER)
	@$(MKDIR) -p $(DELIVERY_CURRENT_FOLDER)
	@$(MKDIR) -p $(DELIVERY_CURRENT_FOLDER)/3rdparty/embedded/$(customer)/$(project)
	@$(CP) -r --remove-destination $(BLUETOOTH_3RD_DIR)/embedded/$(customer)/$(project)/$(application) $(DELIVERY_CURRENT_FOLDER)/3rdparty/embedded/$(customer)/$(project)/$(application)
	@find $(DELIVERY_CURRENT_FOLDER) -name "*.o" | xargs rm -f
	@find $(DELIVERY_CURRENT_FOLDER) -name "*.d" | xargs rm -f
	@find $(DELIVERY_CURRENT_FOLDER) -name ".copyarea.db" | xargs rm -f
	@find $(DELIVERY_CURRENT_FOLDER) -name "*~" | xargs rm -f
	@find $(DELIVERY_CURRENT_FOLDER) -name "core" | xargs rm -f
	@find $(DELIVERY_CURRENT_FOLDER) -name ".*.cmd" | xargs rm -f
	@find $(DELIVERY_CURRENT_FOLDER) -name "*.ko" | xargs rm -f
	@find $(DELIVERY_CURRENT_FOLDER) -name "*.mod.c" | xargs rm -f
	@find $(DELIVERY_CURRENT_FOLDER) -name ".tmp_versions" | xargs rm -f -r
	@find $(DELIVERY_CURRENT_FOLDER) -name "Module.markers" | xargs rm -f -r
	@find $(DELIVERY_CURRENT_FOLDER) -name "Module.symvers" | xargs rm -f -r
	@find $(DELIVERY_CURRENT_FOLDER) -name "modules.order" | xargs rm -f -r
	@$(MKDIR) -p $(DELIVERY_CURRENT_FOLDER)/release_notes
	@$(MKDIR) -p $(DELIVERY_CURRENT_FOLDER)/release_notes/$(customer)
	@if [ -f $(GLOBALPATH)/release_notes/$(customer)/Release_$(application).txt ]; then \
		$(CP) $(GLOBALPATH)/release_notes/$(customer)/Release_$(application).txt $(DELIVERY_CURRENT_FOLDER)/release_notes/$(customer); \
	elif [ -f $(DELIVERY_CURRENT_FOLDER)/3rdparty/embedded/$(customer)/$(project)/$(application)/build/Release_$(application).txt ]; then \
		$(MV) $(DELIVERY_CURRENT_FOLDER)/3rdparty/embedded/$(customer)/$(project)/$(application)/build/Release_$(application).txt $(DELIVERY_CURRENT_FOLDER)/release_notes/$(customer); \
	else \
		echo "    Nothing to do for release notes"; \
	fi
	@$(ECHO) Delivery done
