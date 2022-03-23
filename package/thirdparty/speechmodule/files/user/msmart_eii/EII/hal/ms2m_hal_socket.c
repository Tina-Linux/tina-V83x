/*
 * Copyright (c) 2018 - 2019 M-SMART Research Institute of Midea Group.
 * All rights reserved.
 *
 * File Name            : ms2m_hal_socket.c
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

#include "stdint.h"
#include "string.h"

#include "netdb.h"

#include "ms2m_hal_socket.h"

int ms2m_hal_get_ip_by_name(char *pname, unsigned int *pipaddress)
{
    int dot_cnt = 0;
    char *p = pname;
    int is_anyaddr_flag = 1;
    while(*p != '\0')
    {
        if(*p == '.')
        {
            dot_cnt ++;
        }
        else if(*p == '0')
        {
        }
        else
        {
            is_anyaddr_flag = 0;
            break;
        }
        p++;
    }

    if(is_anyaddr_flag && dot_cnt == 3)
    {
        *pipaddress = 0;
        return 0;
    }

    if((*pipaddress = ms2m_hal_inet_addr(pname)) == 0)
    {
        struct hostent *s_hostent = gethostbyname(pname);
        *pipaddress = (unsigned int)((struct in_addr *)(s_hostent->h_addr))->s_addr;
//        *pipaddress = (s_hostent->h_addr_list[0][0] << 24) |
//                ((0xff & s_hostent->h_addr_list[0][1]) << 16) |
//                ((0xff & s_hostent->h_addr_list[0][2]) << 8) |
//                (0xff & s_hostent->h_addr_list[0][3]);
    }
    return 0;
}

char *ms2m_hal_inet_ntoa(unsigned long n)
{
    struct in_addr addr;
    memset(&addr,0,sizeof(struct in_addr));
    addr.s_addr = n;
    return inet_ntoa(addr);
}
