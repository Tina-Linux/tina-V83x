/*
 * Copyright (c) 2018 - 2019 M-SMART Research Institute of Midea Group.
 * All rights reserved.
 *
 * File Name            : ms2m_hal_info.c
 * Introduction         :
 *
 * Current Version      : v1.0
 * Author               : Zachary Chau <zhouzh6@midea.com.cn>
 * Create Time          : 2018/07/10
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

#include "stdio.h"
#include "sys/ioctl.h"
//#include "net/if.h"
#include "unistd.h"
#include "netinet/in.h"
#include "string.h"
#include "linux/wireless.h"

#include "ms2m_hal_info.h"

static struct ifreq *_wlan_if_get(int sock)
{
    struct ifconf ifc;
    struct ifreq ifr;
    struct iwreq iwr;
    char buf[2048];

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1)
    {
        printf("ioctl error\n");
        return NULL;
    }

    struct ifreq* eth_it = NULL;
    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));
    for (; it != end; ++it)
    {
        strcpy(iwr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIWNAME, &iwr) == -1)
        {
            strcpy(ifr.ifr_name, it->ifr_name);
            if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0)
            {
                if(!(ifr.ifr_flags & IFF_LOOPBACK) && (ifr.ifr_flags & IFF_RUNNING) && (ifr.ifr_flags & IFF_UP))
                {
                    eth_it = it;
                }
            }
            continue;
        }
        if(strcmp("IEEE 802.11",iwr.u.name) != 0)
        {
            continue;
        }

        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0)
        {
            if(ifr.ifr_flags & IFF_LOOPBACK)
            {// don't count loopback
                continue;
            }
        }
        return it;
    }

    if(eth_it != NULL)
    {
        return eth_it;
    }
    return NULL;
}

MS2M_STATUS ms2m_hal_info_mac(uint8_t *mac)
{
    struct ifreq ifr;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1)
    {
        printf("socket error\n");
        return M_ERROR;
    }
    struct ifreq* it = _wlan_if_get(sock);
    if(it == NULL)
    {
        close(sock);
        return M_ERROR;
    }

    strcpy(ifr.ifr_name, it->ifr_name);
    //    if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0)
    //    {
    //        if (! (ifr.ifr_flags & IFF_LOOPBACK))
    //        {
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0)
    {
        unsigned char *ptr ;
        ptr = (unsigned char  *)&ifr.ifr_ifru.ifru_hwaddr.sa_data[0];
        memcpy(mac,ptr,6);

        close(sock);
        return M_OK;
    }
    //        }
    //    }
    //    else
    //    {
    //        printf("get mac info error\n");
    //        close(sock);
    //        return M_ERROR;
    //    }
    close(sock);
    return M_ERROR;
}

MS2M_STATUS ms2m_hal_info_ip(unsigned int *ip)
{
    struct ifreq ifr;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1)
    {
        printf("socket error\n");
        return M_ERROR;
    }
    struct ifreq* it = _wlan_if_get(sock);
    if(it == NULL)
    {
        close(sock);
        return M_ERROR;
    }

    strcpy(ifr.ifr_name, it->ifr_name);

    // if error: No such device
    if (ioctl(sock, SIOCGIFADDR, &ifr) < 0)
    {
        printf("ioctl error\n");
        close(sock);
        return M_ERROR;
    }

    *ip = (unsigned int)(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr);
    //memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    //snprintf(ip, IP_SIZE, "%s", inet_ntoa(sin.sin_addr));

    close(sock);
    return M_OK;
}
