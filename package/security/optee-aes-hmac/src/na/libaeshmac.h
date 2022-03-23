#ifndef __LIBAESHMAC_
#define __LIBAESHMAC_

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>

/**
 * sunxi_dump() - dump buf.
 * @buf: input buf.
 * @ttl_len: dump length.
 *
 * return value: zero, write success; non-zero, write failed.
 */
void data_dump(uint8_t *buf, int ttl_len);

/**
 * aes_hmac_init() - initialize tee context & open session.
 *
 * return value: zero, init success; non-zero, init failed.
 */
int aes_hmac_init(void);

/**
 * aes_hmac_finalize() - close session & finalize tee context.
 *
 */
void aes_hmac_finalize(void);

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

/**
 * generate_unique_key() - derive unique key by chip id.
 *
 * return value: zero, success; non-zero, failed.
 */
int generate_unique_key(void);

/**
 * ui_encrypt_secretaes1_by_unique_key() - encrypt/decrypt data_in with unique_key.
 * @pid:
 * @data_in: source data address.
 * @data_in_data: source data len.
 * @data_out: encrypt/decrypt data address.
 * @data_out_len: data out length.
 * @encrypt: 0 encrypt, 1 decrypt.
 *
 * return value: zero, encrypt/decrypt success; non-zero, encrypt/decrypt failed.
 */
int ui_encrypt_secretaes1_by_unique_key(pid_t pid, char* data_in,
		int data_in_len, char *data_out, int *data_out_len, int encrypt);

/**
 * set_info() - genrate key1 by unique_key.
 * @pid:
 * @in_data: data read from nvram
 * @in_len: in_data length.
 * @info_type:
 *
 * return value: zero, success; non-zero, failed.
 */
int set_info(pid_t pid, char *in_data, int in_len, int info_type);

/**
 * ui_gen_digest_inside() - get the digest of data_in.
 * @pid:
 * @data_in: source data address.
 * @data_in_data: source data len.
 * @data_out: the digest address.
 * @data_out_len: data out length.
 * @digest_type: 1 oneDeviceOneKey 2:securestorage
 *
 * return value: zero, success; non-zero, failed.
 */
int ui_gen_digest_inside(pid_t pid,
				char *data_in, int data_in_len, char *data_out,
				int *data_out_len,
				int digest_type);

#endif

