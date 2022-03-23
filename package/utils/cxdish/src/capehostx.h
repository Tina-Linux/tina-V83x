/*------------------------------------------------------------------------------
  Copyright (C) 2013 Conexant Systems Inc.
  All rights reserved.

  CONEXANT SYSTEMS, INC. CONFIDENTIAL AND PROPRIETARY

  The information contained in this source code file
  is strictly confidential and proprietary to Conexant Systems, Inc.
  ("Conexant")

  No part of this file may be possessed, reproduced or distributed, in
  any form or by any means for any purpose, without the express written
  permission of Conexant Systems Inc.

  Except as otherwise specifically provided through an express agreement
  with Conexant that governs the confidentiality, possession, use
  and distribution of the information contained in this file, CONEXANT
  PROVIDES THIS INFORMATION "AS IS" AND MAKES NO REPRESENTATIONS OR
  WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO ANY IMPLIED
  WARRANTY OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
  TITLE OR NON-INFRINGEMENT, AND SPECIFICALLY DISCLAIMS SUCH WARRANTIES
  AND REPRESENTATIONS.  IN NO EVENT WILL CONEXANT BE LIABLE FOR ANY DAMAGES
  ARISING FROM THE USE OF THE INFORMATION CONTAINED IN THIS FILE.
--------------------------------------------------------------------------------

  File Name:  capehost.h

  Description:  API for CAPE host side code
------------------------------------------------------------------------------*/

#ifndef CAPEHOST_H_
#define CAPEHOST_H_
#ifdef __cplusplus
extern "C" {
#endif
#define COMMAND_OF_SIZE(n)   \
struct {                     \
  int           num_32b_words:16;\
  unsigned int  command_id:15;   \
  unsigned int  reply:1;         \
  unsigned int  app_module_id;   \
  unsigned int  data[n] ;        \
}
#if defined(_MSC_VER)
//typedef signed   short      int16_t   ;
//typedef unsigned short      uint16_t  ;
//typedef unsigned long       uint32_t  ;
//typedef signed   long long  int64_t   ;
//typedef unsigned long long  uint64_t  ;
#endif
// The maximum number of 32-bit data elements that a command can contain
#define MAX_COMMAND_SIZE 13
//
typedef COMMAND_OF_SIZE(MAX_COMMAND_SIZE) Command ;


int SendCmdV (Command *cmd);

#define CMD_GET(item)   ((item) |  0x0100)

#define APP_ID(a,b,c,d) ((((a)-0x20)<<8)|(((b)-0x20)<<14)|(((c)-0x20)<<20)|(((d)-0x20)<<26))
#define CHAR_FROM_CAPE_ID_A(id)  (((((unsigned int)(id))>>8) & 0x3f) + 0x20)
#define CHAR_FROM_CAPE_ID_B(id)  (((((unsigned int)(id))>>14) & 0x3f) + 0x20)
#define CHAR_FROM_CAPE_ID_C(id)  (((((unsigned int)(id))>>20) & 0x3f) + 0x20)
#define CHAR_FROM_CAPE_ID_D(id)  (((((unsigned int)(id))>>26) & 0x3f) + 0x20)

/* control_ex.h */
typedef enum {
  CONTROL_APP_VERSION               =  3,
  CONTROL_APP_EXEC_FILE             =  4,
  CONTROL_APP_FW_UPGD               = 33,
  SOS_RESOURCE                      = 47,
  CONTROL_MGR_TUNED_MODES           = 85
} ControlAppCommandCode;
#define CONTROL_APP_GET_VERSION             	CMD_GET(CONTROL_APP_VERSION)

#ifdef __cplusplus
}
#endif

#endif  // CAPEHOST_H_
