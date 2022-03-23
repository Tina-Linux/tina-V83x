
## 1. cpu arch
ifeq ($(TARGET_ARCH_VARIANT), armv8-a)
LOCAL_CFLAGS += -DCONF_ARMV8_A
endif

ifeq ($(TARGET_ARCH_VARIANT), armv7-a-neon)
LOCAL_CFLAGS += -DCONF_ARMV7_A_NEON
endif

## 2. Android Version
##    Auto detect Android version by SDK level
SDK_LEVEL_JB42 := 17
SDK_LEVEL_KK := 19
SDK_LEVEL_L := 21
SDK_LEVEL_M := 23

ifeq ($(word 1,$(sort $(PLATFORM_SDK_VERSION) $(SDK_LEVEL_M))), $(SDK_LEVEL_M))
MARSHMALLOW_AND_NEWER = yes
LOCAL_CFLAGS += -DCONF_MARSHMALLOW_AND_NEWER
else
MARSHMALLOW_AND_NEWER = no
endif

ifeq ($(word 1,$(sort $(PLATFORM_SDK_VERSION) $(SDK_LEVEL_L))), $(SDK_LEVEL_L))
LOLLIPOP_AND_NEWER = yes
LOCAL_CFLAGS += -DCONF_LOLLIPOP_AND_NEWER
else
LOLLIPOP_AND_NEWER = no
endif

ifeq ($(word 1,$(sort $(PLATFORM_SDK_VERSION) $(SDK_LEVEL_KK))), $(SDK_LEVEL_KK))
KITKAT_AND_NEWER = yes
LOCAL_CFLAGS += -DCONF_KITKAT_AND_NEWER
else
KITKAT_AND_NEWER = no
endif

ifeq ($(word 1,$(sort $(PLATFORM_SDK_VERSION) $(SDK_LEVEL_JB42))), $(SDK_LEVEL_JB42))
JB42_AND_NEWER = yes
LOCAL_CFLAGS += -DCONF_JB42_AND_NEWER
else
JB42_AND_NEWER = no
endif

## 3. secure os
#on semelis secure os, we transform phy addr to secure os to operate the buffer,
#but we adjust on optee secure os, just transform vir addr.
ifeq ($(SECURE_OS), yes)

ifeq ($(SECURE_OS_OPTEE), yes)
    LOCAL_CFLAGS +=-DADJUST_ADDRESS_FOR_SECURE_OS_OPTEE=1
else
    LOCAL_CFLAGS +=-DADJUST_ADDRESS_FOR_SECURE_OS_OPTEE=0
endif

endif ## 'SECURE_OS == yes'
