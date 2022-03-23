/*
 * Copyright (c) 2018 - 2019 M-SMART Research Institute of Midea Group.
 * All rights reserved.
 *
 * File Name            : ms2m_hal_socket.h
 * Introduction         :
 *
 * Current Version      : v1.0
 * Author               : Zachary Chau <zhouzh6@midea.com.cn>
 * Create Time          : 2018/07/11
 * Change Log           :
 *
 * All software, firmware and related documentation here in ("M-Smart Software") are
 * intellectual property of M-SMART Research Institute of Midea Group and protected
 * by law, including, but not limited to, copyright law and international treaties.
 *
 * Any use, modification, reproduction, retransmission, or republication of all
 * or part of M-Smart Software is expressly prohibited, unless prior written
 * permission has been granted by M-Smart.
 */

#ifndef __MS2M_HAL_SOCKET_H__
#define __MS2M_HAL_SOCKET_H__

#include "unistd.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "arpa/inet.h"

typedef fd_set ms2m_fd_set;

typedef int ms2m_socket;

#define ms2m_hal_inet_addr inet_addr
char *ms2m_hal_inet_ntoa(unsigned long n);

int ms2m_hal_get_ip_by_name(char *pname, unsigned int *pipaddress);

#define ms2m_hal_socket_create(a, b, c) socket(a, b, c)

#define ms2m_hal_socket_close(a) close(a)

#define ms2m_hal_socket_bind(a,b,c) bind(a,b,c)
#define ms2m_hal_socket_getname(a,b,c) getsockname(a,b,(socklen_t *)c)

#define ms2m_hal_socket_setopt(a,b,c,d,e) setsockopt(a,b,c,d,e)

#define ms2m_hal_socket_recv(a,b,c,d) recv(a,b,c,d)
#define ms2m_hal_socket_send(a,b,c,d) send(a,b,c,d)
#define ms2m_hal_socket_sendto(a,b,c,d,e,f) sendto(a,b,c,d,e,f)
#define ms2m_hal_socket_recvfrom(a,b,c,d,e,f) recvfrom(a,b,c,d,e,(socklen_t *)f)

#define ms2m_hal_select(a,b,c,d,e) select(a,b,c,d,e)
#define ms2m_hal_fd_clean(a) FD_ZERO(a)

#define ms2m_hal_fd_set(a,b) FD_SET(a,b)
#define ms2m_hal_fd_release(a,b) FD_CLR(a,b)
#define ms2m_hal_fd_is_set(a,b) FD_ISSET(a,b)

#endif /* __MS2M_HAL_SOCKET_H__ */
