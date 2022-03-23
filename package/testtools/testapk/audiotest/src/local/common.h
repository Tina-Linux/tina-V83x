#ifndef __AUDIOTEST_COMMON_H__
#define __AUDIOTEST_COMMON_H__

#include <stdint.h>

uint16_t little_endian_u8_to_u16(const uint8_t *buf);
uint32_t little_endian_u8_to_u32(const uint8_t *buf);
void u16_to_little_endian_u8(uint16_t value, uint8_t *buf);
void u32_to_little_endian_u8(uint32_t value, uint8_t *buf);

void audiotest_stderr(const char *fmt, ...);
void audiotest_stdout(const char *fmt, ...);

#endif /* ifndef __AUDIOTEST_COMMON_H__ */
