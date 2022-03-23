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

  File Name: command.h

  Description: The Command structure and related definitions

  Created Jun 27, 2012

-------------------------------------------------------------------------------



------------------------------------------------------------------------------*/

#ifndef CAPE_PUBLIC_CODE_INCLUDE_COMMAND_H_
#define CAPE_PUBLIC_CODE_INCLUDE_COMMAND_H_

#define COMMAND_OF_SIZE(n)   \
struct {                     \
  int32_t   num_32b_words:16;\
  uint32_t  command_id:15;   \
  uint32_t  reply:1;         \
  uint32_t  app_module_id;   \
  uint32_t  data[n] ;        \
}

// The maximum number of 32-bit data elements that a command can contain
#define MAX_COMMAND_SIZE 13

#define CMD_SET(item)   ((item) & ~0x0100)
#define CMD_GET(item)   ((item) |  0x0100)
#define CMD_MASK        (~(CMD_SET(0)|CMD_GET(0)))
#define CMD_ITEM(cmd)   ((cmd) & CMD_MASK)

#define CMD_REPLY 1
#define APP_ID(a,b,c,d) ((((a)-0x20)<<8)|(((b)-0x20)<<14)|(((c)-0x20)<<20)|(((d)-0x20)<<26))

// Retrieve the app and module id from an app_module_id
#define GET_APP_ID(app_module_id)    ((app_module_id)&~0xFF)
#define GET_MODULE_ID(app_module_id) ((app_module_id)& 0xFF)

// Reserved App IDs
#define APP_ID_BROADCAST     0xFFFFFF00 // to broadcast commands to all apps

// Reserved module IDs
#define MODULE_ID_APP        0    // to send commands to the app
#define MODULE_ID_BROADCAST  0xFF // to broadcast commands to all modules

// The Command type may be used to point to commands of arbitrary
// sizes, for example:
// COMMAND_OF_SIZE(5) cmd
// Command *ptr = (Command *)&cmd;
typedef COMMAND_OF_SIZE(MAX_COMMAND_SIZE) Command ;

#endif // CAPE_PUBLIC_CODE_INCLUDE_COMMAND_H_
