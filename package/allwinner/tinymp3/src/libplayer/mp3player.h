/*
 * mp3player.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __TINY_MP3_H__
#define __TINY_MP3_H__

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdint.h>
#include <mad.h>
#include <pthread.h>
typedef struct mad_decoder_s {
	struct mad_synth	synth;
	struct mad_stream	stream;
	struct mad_frame	frame;

	void		*dbuf;
	int		buf_size;
	int		in_buf_len;
	int		eof;
} mad_decoder_t;

typedef struct {
	int		fd;
	mad_decoder_t	decoder;

	int		channels;
	int		bps;
	int		samplerate;

	void    *ao_device;
} tiny_mp3_t;

typedef enum tinymp3_status
{
	TINYMP3_STATUS_BUSY = 0,
	TINYMP3_STATUS_IDLE = 1
}tinymp3_status_t;

typedef struct tinymp3_ops
{
	int (*read)(char *buf, int size, void *user);
	off_t (*lseek)(off_t offset, int whence, void *user);
}tiny_mp3_ops_t;

typedef struct tinymp3_ctx
{
	tiny_mp3_t *tiny;
	uint8_t *output;
	uint8_t quit;	// 0:go on   1:quit
	uint8_t volume; // software gain: 0 ~ 40

	pthread_mutex_t _mutex;
	pthread_cond_t _cond;
	tinymp3_status_t status; // 0:running 1:stop
	tiny_mp3_ops_t *ops;
	void *user;

	uint64_t position;

}tinymp3_ctx_t;

//extern int tinymp3_play(char *mp3_file);
//
//extern void tinymp3_reset(void);
//
//extern void tinymp3_stop(void);
//
//extern int cset(char* device,int argc, char *argv[], int roflag, int keep_handle);


extern tinymp3_ctx_t* tinymp3_ctx_create(void *user, tiny_mp3_ops_t *ops);
extern void tinymp3_ctx_destroy(tinymp3_ctx_t** ctx);

extern int tinymp3_ctx_prepare(tinymp3_ctx_t *ctx);
extern int tinymp3_ctx_play(tinymp3_ctx_t *ctx);

extern int tinymp3_ctx_prepare_file(tinymp3_ctx_t *ctx, const char *mp3_file);
extern int tinymp3_ctx_play_file(tinymp3_ctx_t *ctx);

extern int tinymp3_ctx_stop(tinymp3_ctx_t *ctx);
extern int tinymp3_ctx_setvolume(tinymp3_ctx_t *ctx, int volume);
extern int tinymp3_ctx_getvolume(tinymp3_ctx_t *ctx);

extern uint64_t tinymp3_ctx_get_position(tinymp3_ctx_t *ctx);
#ifdef __cplusplus
}
#endif

#endif /* __TINY_MP3_H__ */
