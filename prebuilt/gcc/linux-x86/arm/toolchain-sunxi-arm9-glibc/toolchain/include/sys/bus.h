/* Copyright (C) 1991, 1995, 1996, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _SYS_BUS_H
#define _SYS_BUS_H      1

#include <sys/cdefs.h>

/* Get the definition of the macro to define the common sockaddr members.  */
#include <bits/sockaddr.h>

__BEGIN_DECLS

/* 'protocol' to use in socket(AF_BUS, SOCK_SEQPACKET, protocol) */
#define BUS_PROTO_NONE  0
#define BUS_PROTO_DBUS  1
#define BUS_PROTO_MAX   1

/* setsockopt() operations */
#define SOL_BUS         282
#define BUS_ADD_ADDR        1
#define BUS_JOIN_BUS        2
#define BUS_DEL_ADDR        3
#define BUS_SET_EAVESDROP   4
#define BUS_UNSET_EAVESDROP 5

/* Bus address */
struct bus_addr
  {
    __uint64_t s_addr; /* 16-bit prefix + 48-bit client address */
  };

/* Structure describing an AF_BUS socket address. */
struct sockaddr_bus
  {
    __SOCKADDR_COMMON (sbus_);    /* AF_BUS */
    struct bus_addr    sbus_addr; /* bus address */
    char sbus_path[108];          /* pathname */
  };

__END_DECLS

#endif /* sys/bus.h */
