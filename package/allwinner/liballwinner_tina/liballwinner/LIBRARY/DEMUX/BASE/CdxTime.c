/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxTime.c
 * Description : Time
 * History :
 *
 */

#include <CdxTypes.h>

#include <sys/time.h>

#include <CdxTime.h>

cdx_int64 CdxGetNowUs(cdx_void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (cdx_int64)tv.tv_usec + tv.tv_sec * 1000000ll;
}
