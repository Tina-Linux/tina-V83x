/*****************************************************************************
**
**  Name:           bta_ma_co.h
**
**  Description:    This is the interface file for the Message Access Profile
**                  common call-out functions.
**
**  Copyright (c) 2009, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_MA_CO_H
#define BTA_MA_CO_H

#include <stdio.h>

/*****************************************************************************
**
** Utility functions for converting types to strings.
**
*****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         bta_ma_co_open
**
** Description      Open a file.
**
** Parameters       p_path - Full path of file to open.
**                  oflags - file open flags.
**
** Returns          file descriptor.  BTA_FS_INVALID_FD if open fails.
**
*******************************************************************************/
    BTA_API extern int bta_ma_co_open(const char *p_path, int oflags);

/*******************************************************************************
**
** Function         bta_ma_co_write
**
** Description      Write data to file.
**
** Parameters       fd - file descriptor.
**                  buffer - data to write.
**                  size - size of data to write (in bytes).
**
** Returns          Number of bytes written.
**
*******************************************************************************/
    BTA_API extern  int bta_ma_co_write(int fd, const void *buffer, int size);

/*******************************************************************************
**
** Function         bta_ma_co_read
**
** Description      Read data from file.
**
** Parameters       fd - file descriptor.
**                  buffer - to receive data.
**                  size - amount of data to read (in bytes).
**
** Returns          Number of bytes read.
**
*******************************************************************************/
    BTA_API extern int bta_ma_co_read(int fd, void *buffer, int size);

/*******************************************************************************
**
** Function         bta_ma_co_close
**
** Description      Close the file.
**
** Parameters       fd - file descriptor.
**
** Returns          void
**
*******************************************************************************/
    BTA_API extern void bta_ma_co_close(int fd);

#ifdef __cplusplus
}
#endif

#endif /* BTA_MA_FILE_H */
