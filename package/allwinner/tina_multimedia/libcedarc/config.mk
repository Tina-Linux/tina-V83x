
MODULE_TOP = $(TOP)/frameworks/av/media/libcedarc

product = $(TARGET_BOARD_PLATFORM)

########## configure CONF_ANDROID_VERSION ##########
android_version = $(shell echo $(PLATFORM_VERSION) | cut -c 1)

config_file = null

ifeq ($(product), r58)
    ifeq ($(android_version), 5)
        config_file = config_pad_A83_lollipop.mk
    else ifeq ($(android_version), 7)
	config_file = config_pad_A83_Nougat.mk
    endif
else ifeq ($(product), r16)
    ifeq ($(android_version), 7)
        config_file = config_pad_A33_Nougat.mk
    endif
else ifeq ($(product), r18)
    ifeq ($(android_version), 5)
        config_file = config_pad_A64_lollipop.mk
    else ifeq ($(android_version), 6)
        config_file = config_pad_A64_Marshmallow.mk
    else ifeq ($(android_version), 7)
        config_file = config_pad_A64_Nougat.mk
    else ifeq ($(android_version), 8)
        config_file = config_pad_A64_Oreo.mk
    endif
else ifeq ($(product), h3)
    ifeq ($(android_version), 4)
        config_file = config_box_H3_kitkat.mk
    else ifeq ($(android_version), 7)
	config_file = config_box_H3_Nougat.mk
    endif
else ifeq ($(product), dolphin)
    ifeq ($(android_version), 4)
        config_file = config_box_H3_kitkat.mk
    else ifeq ($(android_version), 7)
	config_file = config_box_H3_Nougat.mk
    endif
else ifeq ($(product), rabbit)
    ifeq ($(android_version), 5)
        config_file = config_box_H64_lollipop.mk
    endif
else ifeq ($(product), cheetah)
    ifeq ($(android_version), 4)
        config_file = config_box_H5_kitkat.mk
    else ifeq ($(android_version), 5)
        config_file = config_box_H5_lollipop.mk
    else ifeq ($(android_version), 7)
        config_file = config_box_H5_Nougat.mk
    endif
else ifeq ($(product), eagle)
    ifeq ($(android_version), 5)
        config_file = config_box_H8_kitkat.mk
    else ifeq ($(android_version), 7)
        config_file = config_box_H8_Nougat.mk
    endif
else ifeq ($(product), t3)
    ifeq ($(android_version), 6)
        config_file = config_cdr_T3_Marshmallow.mk
	else ifeq ($(android_version), 7)
        config_file = config_cdr_T3_Nougat.mk
    endif
else ifeq ($(product), t7)
	ifeq ($(android_version), 7)
        config_file = config_cdr_T7_Nougat.mk
    endif
else ifeq ($(product), kylin)
    ifeq ($(android_version), 7)
        config_file = config_vr_A80_Nougat.mk
    endif
else ifeq ($(product), h6)
    ifeq ($(android_version), 4)
        config_file = config_box_H6_kitkat.mk
    else ifeq ($(android_version), 7)
        config_file = config_box_H6_Nougat.mk
    else ifeq ($(android_version), 8)
        config_file = config_box_H6_Oreo.mk
    endif
else ifeq ($(product), neptune)
    ifeq ($(android_version), 7)
        config_file = config_vr_A63_Nougat.mk
    endif
else ifeq ($(product), uranus)
    ifeq ($(android_version), 7)
        config_file = config_pad_A63_Nougat.mk
    else ifeq ($(android_version), 8)
        config_file = config_pad_A63_Oreo.mk
    endif
else ifeq ($(product), t8)
    ifeq ($(android_version), 7)
        config_file = config_pad_A83_Nougat.mk
     else ifeq ($(android_version), 8)
         config_file = config_pad_T8_Oreo.mk
     endif
else ifeq ($(product), venus)
    ifeq ($(android_version), 8)
        config_file = config_pad_A50_Oreo.mk
    endif
endif

ifeq ($(config_file), null)
    $(warning "can not find config_file: product=$(product), androd_version=$(android_version)")
else
    $(warning "config file: $(config_file)")
    include $(MODULE_TOP)/config/$(config_file)
endif
