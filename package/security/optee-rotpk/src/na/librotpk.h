#ifndef _LIBROTPK_H__
#define _LIBROTPK_H__

#include <string.h>
#include <stdio.h>
#include <stdint.h>

/**
 * sunxi_dump() - dump buf.
 * @buf: input buf.
 * @ttl_len: dump length.
 *
 * return value: zero, write success; non-zero, write failed.
 */
void sunxi_dump(uint8_t *buf, int ttl_len);

/**
 * write_rotpk_hash() - write rotpk hash to efuse.
 * @buf: input c-style string, should be 32byte hash, with a nul terminated.
 *
 * return value: zero, write success; non-zero, write failed.
 */
int write_rotpk_hash(const char *buf);

/**
 * read_rotpk_hash() - read rotpk hash from efuse.
 * @buf: buf used to contain the rotpk hash value.
 *
 * return value: size of hash length.
 */
int read_rotpk_hash(char *buf);

#endif

