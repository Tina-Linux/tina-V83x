#######################################
#
#  This makefile is written for a GNU make or equivalent.
#  For example:
#
#   make clean
#   make
#
#######################################
# Basic path definitions
GLOBALPATH := ../../../../../..
customer := bsa_examples
project := linux
os_name := linux
BLUETOOTH_3RD_DIR := $(GLOBALPATH)/3rdparty

# include common makefile rules
include $(BLUETOOTH_3RD_DIR)/embedded/$(customer)/$(project)/app_common/build/make.common

#######################################
#
# BSA Library reference
#
#######################################

BSA_LIB_ROOT_PATH = ../../libbsa

bsa_lib_name = bsa

ifeq ($(BSASHAREDLIB), TRUE)
bsa_lib_path = $(BSA_LIB_ROOT_PATH)/build/$(CPU)/sharedlib
else
bsa_lib_path = $(BSA_LIB_ROOT_PATH)/build/$(CPU)
endif

bsa_lib_inc_path = $(BSA_LIB_ROOT_PATH)/include

#######################################
#
# Target
#
#######################################

PROGNAME = hello_bsa

#######################################
#
# specify which libraries to link
#
#######################################

LINKFLAGS = -L$(bsa_lib_path) $(LINKCPUFLAGS)
LINKLIBS = -l$(bsa_lib_name) -lpthread -lrt

#######################################
#
# Build Rules
#
#######################################

SRCS = hello_bsa.c

OBJS = hello_bsa.o

#.c.obj:
%.o: %.c
	$(CC) -I$(bsa_lib_inc_path) $(CCFLAGS) -c -o $@ $<

all: $(PROGNAME)

$(PROGNAME) : $(OBJS)
	$(CC) $(LINKFLAGS) -o $@ $(OBJS) $(LINKLIBS)

clean:
	rm -f $(OBJS) $(PROGNAME) core
