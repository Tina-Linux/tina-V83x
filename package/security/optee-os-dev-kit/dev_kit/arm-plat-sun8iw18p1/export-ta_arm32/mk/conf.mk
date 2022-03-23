sm := ta_arm32
sm-ta_arm32 := y
CFG_TA_FLOAT_SUPPORT := n
CFG_ARM32_ta_arm32 := y
ta_arm32-platform-cppflags := -DARM32=1 -D__ILP32__=1
ta_arm32-platform-cflags := -mcpu=cortex-a7 -Os -g3 -fpie -mthumb -mthumb-interwork -fno-short-enums -fno-common -mno-unaligned-access -mfloat-abi=soft -funwind-tables
ta_arm32-platform-aflags := -g -pipe -mcpu=cortex-a7 -mfpu=neon
CROSS_COMPILE ?= arm-linux-gnueabihf-
CROSS_COMPILE32 ?= $(CROSS_COMPILE)
CROSS_COMPILE_ta_arm32 ?= $(CROSS_COMPILE32)

CFG_SUNXI_TA_ENCRYPT_SUPPORT ?= y
