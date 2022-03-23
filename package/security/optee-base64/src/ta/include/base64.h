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

#ifndef OPENSSL_HEADER_BASE64_H
#define OPENSSL_HEADER_BASE64_H

#include <stdint.h>
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

/* base64 functions.
 *
 * For historical reasons, these functions have the EVP_ prefix but just do
 * base64 encoding and decoding. */


/* Encoding */

/* EVP_EncodeBlock encodes |src_len| bytes from |src| and writes the
 * result to |dst| with a trailing NUL. It returns the number of bytes
 * written, not including this trailing NUL. */
size_t EVP_EncodeBlock(uint8_t *dst, const uint8_t *src,
                                      size_t src_len);

/* EVP_EncodedLength sets |*out_len| to the number of bytes that will be needed
 * to call |EVP_EncodeBlock| on an input of length |len|. This includes the
 * final NUL that |EVP_EncodeBlock| writes. It returns one on success or zero
 * on error. */
int EVP_EncodedLength(size_t *out_len, size_t len);


/* Decoding */

/* EVP_DecodedLength sets |*out_len| to the maximum number of bytes
 * that will be needed to call |EVP_DecodeBase64| on an input of
 * length |len|. */
int EVP_DecodedLength(size_t *out_len, size_t len);

/* EVP_DecodeBase64 decodes |in_len| bytes from base64 and writes
 * |*out_len| bytes to |out|. |max_out| is the size of the output
 * buffer. If it is not enough for the maximum output size, the
 * operation fails. */
int EVP_DecodeBase64(uint8_t *out, size_t *out_len,
                                    size_t max_out, const uint8_t *in,
                                    size_t in_len);

#if defined(__cplusplus)
}  /* extern C */
#endif

#endif  /* OPENSSL_HEADER_BASE64_H */
