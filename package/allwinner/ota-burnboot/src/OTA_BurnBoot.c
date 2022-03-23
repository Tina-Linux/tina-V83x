#include <stdio.h>
#include "Utils.h"
#include "BurnNandBoot.h"
#include "BurnSpinor.h"
#include "BurnSdBoot.h"

int OTA_burnboot0(const char *img_path)
{
    BufferExtractCookie cookie;
    if (getBufferExtractCookieOfFile(img_path, &cookie) < 0) {
        ob_error("getBufferExtractCookieOfFile failed\n");
        return -1;
    }

    initInfo();

    switch (getFlashType()) {
    case FLASH_TYPE_UNKNOW:
        ob_error("getFlashType failed\n");
        return -1;
    case FLASH_TYPE_NOR:
        return burnSpinorBoot0(&cookie);
    case FLASH_TYPE_NAND:
    case FLASH_TYPE_SPINAND:
        return burnNandBoot0(&cookie);
    default:
        return burnSdBoot0(&cookie);
    }
}

int OTA_burnuboot(const char *img_path)
{
    BufferExtractCookie cookie;
    if (getBufferExtractCookieOfFile(img_path, &cookie) < 0) {
        ob_error("getBufferExtractCookieOfFile failed\n");
        return -1;
    }

    initInfo();

    switch (getFlashType()) {
    case FLASH_TYPE_UNKNOW:
        ob_error("getFlashType failed\n");
        return -1;
    case FLASH_TYPE_NOR:
        return burnSpinorUboot(&cookie);
    case FLASH_TYPE_NAND:
    case FLASH_TYPE_SPINAND:
        return burnNandUboot(&cookie);
    default:
        return burnSdUboot(&cookie);
    }
}
