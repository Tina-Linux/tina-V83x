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

  File Name: capehost.c

------------------------------------------------------------------------------*/
#include "capehostx.h"


// register_address - the slave register address for read/write
// buffer - buffer to read/write from/to
// size - size of buffer in 32 bits data
extern int  i2c_write (int register_address, unsigned  *buffer, int size);
extern int  i2c_read  (int register_address, unsigned  *buffer, int size);

// TODO: implement using the host's OS blocking sleep function
extern "C"  void sys_mdelay (unsigned int intreval_ms);

// TODO: tweak the interval for reply bit polling and its timeout
#define REPLY_POLL_INTERVAL_MSEC     1
#define REPLY_POLL_TIMEOUT_MSEC   2000


int SendCmdV (Command *cmd)
{


  int num_32b_words = cmd->num_32b_words;

  cmd->num_32b_words = (cmd->command_id&CMD_GET(0)) ? MAX_COMMAND_SIZE : num_32b_words;

  unsigned int *i2c_data = (unsigned int *)cmd;
  int size = num_32b_words + 2;

  // write words 1 to N-1 , to addresses 4 to 4+4*N-1
  i2c_write (0x4, &i2c_data[1], size-1);

  // write word 0 to address 0
  i2c_write (0x0, &i2c_data[0], 1);

  int elapsed_ms = 0;
  while (elapsed_ms < REPLY_POLL_TIMEOUT_MSEC)
  {
    // only read the first word and check the reply bit
    i2c_read (0x0, &i2c_data[0], 1);

    if (cmd->reply==1)
      break;
    sys_mdelay(REPLY_POLL_INTERVAL_MSEC);
    elapsed_ms += REPLY_POLL_INTERVAL_MSEC;
  }

  if (cmd->reply==1)
  {
      if( cmd->num_32b_words >0)
      {
            i2c_read (0x8, &i2c_data[2],cmd->num_32b_words);
      }
        return(cmd->num_32b_words);
  }
  return(-1);
}
