#include "common.h"
#include <stdio.h>
#include <stdarg.h>

uint16_t little_endian_u8_to_u16(const uint8_t *buf)
{
    uint16_t value = 0;
    int i;
    for (i = 0; i < 2; ++i)
        value += (uint16_t)(*(buf + i)) << (8 * i);
    return value;
}

uint32_t little_endian_u8_to_u32(const uint8_t *buf)
{
    uint32_t value = 0;
    int i;
    for (i = 0; i < 4; ++i)
        value += (uint32_t)(*(buf + i)) << (8 * i);
    return value;
}

void u16_to_little_endian_u8(uint16_t value, uint8_t *buf)
{
    int i;
    for (i = 0; i < 2; ++i)
        buf[i] = (uint8_t)(value >> (8 * i) & 0xff);
}

void u32_to_little_endian_u8(uint32_t value, uint8_t *buf)
{
    int i;
    for (i = 0; i < 4; ++i)
        buf[i] = (uint8_t)(value >> (8 * i) & 0xff);
}

void audiotest_stderr(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "[ERROR] [audiotest] ");
    vfprintf(stderr, fmt, va);
    va_end(va);
}

void audiotest_stdout(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    fprintf(stdout, "[audiotest] ");
    vfprintf(stdout, fmt, va);
    va_end(va);
}
