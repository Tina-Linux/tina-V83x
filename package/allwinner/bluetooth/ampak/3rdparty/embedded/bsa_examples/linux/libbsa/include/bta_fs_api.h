/*****************************************************************************
**
**  Name:           bta_fs_api.h
**
**  Description:    This is the public interface file for the
**                  file system of BTA, Widcomm's
**                  Bluetooth application layer for mobile phones.
**
**  Copyright (c) 2003 - 2009, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_FS_API_H
#define BTA_FS_API_H

#include "bta_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/

/* Configuration structure */
typedef struct
{
    UINT16  max_file_len;           /* Maximum size file name */
    UINT16  max_path_len;           /* Maximum path length (includes appended file name) */
    char    path_separator;         /* 0x2f ('/'), or 0x5c ('\') */
} tBTA_FS_CFG;

extern tBTA_FS_CFG * p_bta_fs_cfg;

#endif /* BTA_FS_API_H */
