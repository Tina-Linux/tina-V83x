$(call inherit-product-if-exists, target/allwinner/v833-common/v833-common.mk)

PRODUCT_PACKAGES +=

PRODUCT_COPY_FILES +=

PRODUCT_AAPT_CONFIG := large xlarge hdpi xhdpi
PRODUCT_AAPT_PERF_CONFIG := xhdpi
PRODUCT_CHARACTERISTICS := musicbox

PRODUCT_BRAND := allwinner
PRODUCT_NAME := v833_lindenis
PRODUCT_DEVICE := v833-lindenis
PRODUCT_MODEL := Allwinner v833 lindenis board
