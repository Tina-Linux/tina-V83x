#ifndef __CRC16_H
#define __CRC16_H

#include <stdbool.h>

unsigned short get_crc16_buf(const char *buf, unsigned long long numbytes);
int get_crc16_fd(int fd, unsigned long long numBytes, unsigned short *crc);
int get_crc16_path(char *path, unsigned short *crc);
bool check_crc16(const char *buf, unsigned long long numbytes);

/*
 * step 1: get_crc16_start
 * step 2: get_crc16_continue
 * step 3: get_crc16_end
 * step 4: check_crc16_by_crc
 */
unsigned short get_crc16_start();
unsigned short get_crc16_continue(unsigned short oldcrc,
        const char *buf, unsigned long long numBytes);
unsigned short get_crc16_end(unsigned short oldcrc);
bool check_crc16_by_crc(unsigned short crc);

#endif
