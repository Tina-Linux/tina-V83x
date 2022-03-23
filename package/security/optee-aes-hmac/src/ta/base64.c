/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.] */

#include <base64.h>

#include <assert.h>
#include <limits.h>
#include <string.h>

static const unsigned char data_bin2ascii[65] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define conv_bin2ascii(a) (data_bin2ascii[(a)&0x3f])

/* 64 char lines
 * pad input with 0
 * left over chars are set to =
 * 1 byte  => xx==
 * 2 bytes => xxx=
 * 3 bytes => xxxx
 */
#define BIN_PER_LINE (64 / 4 * 3)
#define CHUNKS_PER_LINE (64 / 4)
#define CHAR_PER_LINE (64 + 1)

/* 0xF0 is a EOLN
 * 0xF1 is ignore but next needs to be 0xF0 (for \r\n processing).
 * 0xF2 is EOF
 * 0xE0 is ignore at start of line.
 * 0xFF is error */

#define B64_EOLN 0xF0
#define B64_CR 0xF1
#define B64_EOF 0xF2
#define B64_WS 0xE0
#define B64_ERROR 0xFF
#define B64_NOT_BASE64(a) (((a) | 0x13) == 0xF3)

static const uint8_t data_ascii2bin[128] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0xF0, 0xFF,
	0xFF, 0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xF2, 0xFF, 0x3F,
	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
	0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12,
	0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24,
	0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
	0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

static uint8_t conv_ascii2bin(uint8_t a)
{
	if (a >= 128) {
		return 0xFF;
	}
	return data_ascii2bin[a];
}

size_t EVP_EncodeBlock(uint8_t *dst, const uint8_t *src, size_t src_len)
{
	uint32_t l;
	size_t remaining = src_len, ret = 0;

	while (remaining) {
		if (remaining >= 3) {
			l = (((uint32_t)src[0]) << 16L) |
			    (((uint32_t)src[1]) << 8L) | src[2];
			*(dst++) = conv_bin2ascii(l >> 18L);
			*(dst++) = conv_bin2ascii(l >> 12L);
			*(dst++) = conv_bin2ascii(l >> 6L);
			*(dst++) = conv_bin2ascii(l);
			remaining -= 3;
		} else {
			l = ((uint32_t)src[0]) << 16L;
			if (remaining == 2) {
				l |= ((uint32_t)src[1] << 8L);
			}

			*(dst++) = conv_bin2ascii(l >> 18L);
			*(dst++) = conv_bin2ascii(l >> 12L);
			*(dst++) = (remaining == 1) ? '=' :
						      conv_bin2ascii(l >> 6L);
			*(dst++)  = '=';
			remaining = 0;
		}
		ret += 4;
		src += 3;
	}

	*dst = '\0';
	return ret;
}

int EVP_DecodedLength(size_t *out_len, size_t len)
{
	if (len % 4 != 0) {
		return 0;
	}
	*out_len = (len / 4) * 3;
	return 1;
}

int EVP_DecodeBase64(uint8_t *out, size_t *out_len, size_t max_out,
		     const uint8_t *in, size_t in_len)
{
	uint8_t a, b, c, d;
	size_t pad_len = 0, len = 0, max_len, i;
	uint32_t l;

	if (!EVP_DecodedLength(&max_len, in_len) || max_out < max_len) {
		return 0;
	}

	for (i = 0; i < in_len; i += 4) {
		a = conv_ascii2bin(*(in++));
		b = conv_ascii2bin(*(in++));
		if (i + 4 == in_len && in[1] == '=') {
			if (in[0] == '=') {
				pad_len = 2;
			} else {
				pad_len = 1;
			}
		}
		if (pad_len < 2) {
			c = conv_ascii2bin(*(in++));
		} else {
			c = 0;
		}
		if (pad_len < 1) {
			d = conv_ascii2bin(*(in++));
		} else {
			d = 0;
		}
		if ((a & 0x80) || (b & 0x80) || (c & 0x80) || (d & 0x80)) {
			return 0;
		}
		l = ((((uint32_t)a) << 18L) | (((uint32_t)b) << 12L) |
		     (((uint32_t)c) << 6L) | (((uint32_t)d)));
		*(out++) = (uint8_t)(l >> 16L) & 0xff;
		if (pad_len < 2) {
			*(out++) = (uint8_t)(l >> 8L) & 0xff;
		}
		if (pad_len < 1) {
			*(out++) = (uint8_t)(l)&0xff;
		}
		len += 3 - pad_len;
	}
	*out_len = len;
	return 1;
}

int EVP_EncodedLength(size_t *out_len, size_t len)
{
	if (len + 2 < len) {
		return 0;
	}
	len += 2;
	len /= 3;
	if (((len << 2) >> 2) != len) {
		return 0;
	}
	len <<= 2;
	if (len + 1 < len) {
		return 0;
	}
	len++;
	*out_len = len;
	return 1;
}
