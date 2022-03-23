/*
 * tiny_mp3.c
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "auGaincom.h"
#include "libaudio/audio.h"
#include "mp3player.h"

//#include "utils_interface.h"

/*
 * This is perhaps the simplest example use of the MAD high-level API.
 * Standard input is mapped into memory via mmap(), then the high-level API
 * is invoked with three callbacks: input, output, and error. The output
 * callback converts MAD's high-resolution PCM samples to 16 bits, then
 * writes them to standard output in little-endian, stereo-interleaved
 * format.
 */
static int _tinymp3_ctx_read(tinymp3_ctx_t *ctx, char *buf, int size);
static off_t _tinymp3_ctx_lseek(tinymp3_ctx_t *ctx, off_t offset, int whence);
#if 1
#define DEBUG(x,y...)	(printf("DEBUG [ %s : %s : %d] "x"\n",__FILE__, __func__, __LINE__, ##y))
#else
#define DEBUG(x,y...)
#endif
#define ERROR(x,y...)	(printf("ERROR [ %s : %s : %d] "x"\n", __FILE__, __func__, __LINE__, ##y))

#define HDR_SIZE 4
//! how many valid frames in a row we need before accepting as valid MP3
#define MIN_MP3_HDRS 3//12
//! Used to describe a potential (chain of) MP3 headers we found
typedef struct mp3_hdr {
  off_t frame_pos; // start of first frame in this "chain" of headers
  off_t next_frame_pos; // here we expect the next header with same parameters
  int mp3_chans;
  int mp3_freq;
  int mpa_spf;
  int mpa_layer;
  int mpa_br;
  int cons_hdrs; // if this reaches MIN_MP3_HDRS we accept as MP3 file
  struct mp3_hdr *next;
} mp3_hdr_t;

//static tiny_mp3_t *tiny = NULL;
//static uint8_t Output[6912];
//static int quit_flag = 0;

enum decoder_return {
	DECODER_OK		= 0,
	DECODER_STOP		= 1,
	DECODER_EOF		= 2,
	DECODER_UNRECOVERY	= -1,
};

static int decode_init(mad_decoder_t *decode)
{
	mad_synth_init(&decode->synth);
	mad_stream_init(&decode->stream);
	mad_frame_init(&decode->frame);

	decode->buf_size = 1152;
	decode->dbuf = malloc(decode->buf_size);
	if (!decode->dbuf) {
		ERROR("Alloc decode buffer: %s", strerror(errno));
		return -1;
	}

	return 0;
}

static void decode_deinit(mad_decoder_t *decode)
{
	mad_synth_finish(&decode->synth);
	mad_frame_finish(&decode->frame);
	mad_stream_finish(&decode->stream);

	if (decode->dbuf) {
		free(decode->dbuf);
		decode->dbuf = NULL;
	}
}

static void decode_zero_in(mad_decoder_t *decode)
{
	decode->in_buf_len	= 0;
	decode->eof		= 0;
	//lseek(fd, 0, SEEK_SET);
}

static int decode_frame(tinymp3_ctx_t *ctx, mad_decoder_t *decode, uint8_t *quit)
{
	char *buf = decode->dbuf;
	ssize_t rSize = -1;
	int ret;

	while (!(*quit)) {
		/*
		rSize = read(fd,
			     buf + decode->in_buf_len,
			     decode->buf_size - decode->in_buf_len);
		*/

		rSize = _tinymp3_ctx_read(ctx, buf + decode->in_buf_len,
					 decode->buf_size - decode->in_buf_len);
		if (rSize == 0) {
			if (!decode->eof)
				decode->eof = 1;
		} else if (rSize < 0) {
			ERROR("read file: %s", strerror(errno));
			return DECODER_UNRECOVERY;
		}

		decode->in_buf_len += rSize;

		mad_stream_buffer(&decode->stream, (uint8_t *)buf, decode->in_buf_len);
		ret = mad_frame_decode(&decode->frame, &decode->stream);

		if (decode->stream.next_frame) {
			int num_bytes =
				(buf + decode->in_buf_len) - (char *)decode->stream.next_frame;
			memmove(buf, decode->stream.next_frame, num_bytes);
			decode->in_buf_len = num_bytes;
		}

		if (ret == 0)
			return DECODER_OK;

		if (ret < 0) {
			if (decode->eof) {
				return DECODER_EOF;
			} else if (MAD_RECOVERABLE(decode->stream.error) ||
			    decode->stream.error == MAD_ERROR_BUFLEN) {
				continue;
			} else {
				ERROR("mad decode:%s", mad_stream_errorstr(&decode->stream));
				return DECODER_UNRECOVERY;
			}
		}
	}

	return DECODER_STOP;
}

/*
 * The following utility routine performs simple rounding, clipping, and
 * scaling of MAD's high-resolution samples down to 16 bits. It does not
 * perform any dithering or noise shaping, which would be recommended to
 * obtain any exceptional audio quality. It is therefore not recommended to
 * use this routine if high-quality output is desired.
 */

static inline
signed int scale(mad_fixed_t sample)
{
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static void play_frame(tinymp3_ctx_t *ctx, mad_decoder_t *decode, audio_output_t *ao)
{
	mad_fixed_t const *left_ch, *right_ch;
	struct mad_pcm *pcm = &decode->synth.pcm;
	uint32_t nchannels, nsamples, n;
	uint8_t *OutputPtr;

	uint8_t *output = ctx->output;
	ao_device_t *device = (ao_device_t*)(ctx->tiny->ao_device);

	/* pcm->samplerate contains the sampling frequency */
	mad_synth_frame(&decode->synth, &decode->frame);

	nchannels	= pcm->channels;
	nsamples	= pcm->length;
	left_ch		= pcm->samples[0];
	right_ch	= pcm->samples[1];

	n = nsamples;
	OutputPtr = output;

	while (nsamples--) {
		signed int sample;
		/* output sample(s) in 16-bit signed little-endian PCM */
		sample = scale(*left_ch++);
		*(OutputPtr++) = sample >> 0;
		*(OutputPtr++) = sample >> 8;
		if (nchannels == 2) {
			sample = scale(*right_ch++);
			*(OutputPtr++) = sample >> 0;
			*(OutputPtr++) = sample >> 8;
		}
	}

	if ((int)(OutputPtr - output) > 1152 *4) {
		ERROR("Output buffer over 1152 * 4");
	}

	if(n > 0){
		AudioGain ag;
		bzero(&ag, sizeof(AudioGain));
		ag.preamp = ctx->volume - 20;
		ag.InputChan = ag.OutputChan = nchannels;
		ag.InputPtr = ag.OutputPtr = (short*)output;
		ag.InputLen = n * nchannels * 2;
		ag.OutputChan = nchannels;
		//printf("gain: %d channel: %d samples: %d\n", gain, nchannels, n);
		tina_do_AudioGain(&ag);
	}

	ao->play(device, (short *)output, n);

	//ctx->position += (n*1000)/ctx->tiny->samplerate;
}

static int play_device_try(char *interface)
{
	audio_output_t *ao;

	ao = audio_get_output(interface);
	if (!ao) {
		ERROR("NOT found audio interface");
		return -1;
	}

	//if (ao->dev_try)
	//	return ao->dev_try() < 0 ? -1 : 0;

	return 0;
}

unsigned int id3v2_tag_size(tinymp3_ctx_t *ctx, uint8_t maj_ver) {
	unsigned int header_footer_size;
	unsigned int size;
	uint8_t data;
	int i;
	tiny_mp3_t *tiny = ctx->tiny;

	if(_tinymp3_ctx_read(ctx, &data, 1) <= 0)
		return -1;
	if(data == 0xff)
		return 0;
	if(_tinymp3_ctx_read(ctx, &data, 1) <= 0)
		return -1;
	header_footer_size = ((data & 0x10) && maj_ver >= 4) ? 20 : 10;

	size = 0;
	for(i = 0; i < 4; i++) {
		if(_tinymp3_ctx_read(ctx, &data, 1) <= 0)
			return -1;
		if (data & 0x80)
			return 0;
		size = size << 7 | data;
	}

	return header_footer_size + size;
}
//----------------------- mp3 audio frame header parser -----------------------

static const uint16_t tabsel_123[2][3][16] = {
   { {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,0},
     {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,0},
     {0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,0} },

   { {0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,0},
     {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0},
     {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0} }
};

static const int freqs[9] = { 44100, 48000, 32000,   // MPEG 1.0
                              22050, 24000, 16000,   // MPEG 2.0
                              11025, 12000,  8000};  // MPEG 2.5

/*
 * return frame size or -1 (bad frame)
 */
int mp_get_mp3_header(unsigned char* hbuf,int* chans, int* srate, int* spf, int* mpa_layer, int* br){
    int stereo,lsf,framesize,padding,bitrate_index,sampling_frequency, divisor;
    int bitrate;
    int layer;
    static const int mult[3] = { 12000, 144000, 144000 };
    uint32_t newhead =
      hbuf[0] << 24 |
      hbuf[1] << 16 |
      hbuf[2] <<  8 |
      hbuf[3];

    // head_check:
    if( (newhead & 0xffe00000) != 0xffe00000 ){
      //DEBUG("head_check failed\n");
      return -1;
    }

    layer = 4-((newhead>>17)&3);
    if(layer==4){
      DEBUG("not layer-1/2/3\n");
      return -1;
    }

    sampling_frequency = (newhead>>10)&0x3;  // valid: 0..2
    if(sampling_frequency==3){
      DEBUG("invalid sampling_frequency\n");
      return -1;
    }

    if( newhead & (1<<20) ) {
      // MPEG 1.0 (lsf==0) or MPEG 2.0 (lsf==1)
      lsf = !(newhead & (1<<19));
      sampling_frequency += lsf*3;
    } else {
      // MPEG 2.5
      lsf = 1;
      sampling_frequency += 6;
    }

    bitrate_index = (newhead>>12)&0xf;  // valid: 1..14
    padding   = (newhead>>9)&0x1;

    stereo    = ( ((newhead>>6)&0x3) == 3) ? 1 : 2;

    bitrate = tabsel_123[lsf][layer-1][bitrate_index];
    framesize = bitrate * mult[layer-1];

    if(!framesize){
      DEBUG("invalid framesize/bitrate_index\n");
      return -1;
    }

    divisor = layer == 3 ? (freqs[sampling_frequency] << lsf) : freqs[sampling_frequency];
    framesize /= divisor;
    framesize += padding;
    if(layer==1)
      framesize *= 4;

    if(srate)
      *srate = freqs[sampling_frequency];
    if(spf) {
      if(layer == 1)
        *spf = 384;
      else if(layer == 2)
        *spf = 1152;
      else if(sampling_frequency > 2) // not 1.0
        *spf = 576;
      else
        *spf = 1152;
    }
    if(mpa_layer) *mpa_layer = layer;
    if(chans) *chans = stereo;
    if(br) *br = bitrate;

    return framesize;
}

static mp3_hdr_t *add_mp3_hdr(mp3_hdr_t **list, off_t st_pos,
                               int mp3_chans, int mp3_freq, int mpa_spf,
                               int mpa_layer, int mpa_br, int mp3_flen) {
  mp3_hdr_t *tmp;
  int in_list = 0;
  while (*list && (*list)->next_frame_pos <= st_pos) {
    if (((*list)->next_frame_pos < st_pos) || ((*list)->mp3_chans != mp3_chans)
         || ((*list)->mp3_freq != mp3_freq) || ((*list)->mpa_layer != mpa_layer) ) {
      // wasn't valid!
      tmp = (*list)->next;
      free(*list);
      *list = tmp;
    } else {
      (*list)->cons_hdrs++;
      (*list)->next_frame_pos = st_pos + mp3_flen;
      (*list)->mpa_spf = mpa_spf;
      (*list)->mpa_br = mpa_br;
      if ((*list)->cons_hdrs >= MIN_MP3_HDRS) {
        // copy the valid entry, so that the list can be easily freed
        tmp = malloc(sizeof(mp3_hdr_t));
        memcpy(tmp, *list, sizeof(mp3_hdr_t));
        tmp->next = NULL;
        return tmp;
      }
      in_list = 1;
      list = &((*list)->next);
    }
  }
  if (!in_list) { // does not belong into an existing chain, insert
    // find right position to insert to keep sorting
    while (*list && (*list)->next_frame_pos <= st_pos + mp3_flen)
      list = &((*list)->next);
    tmp = malloc(sizeof(mp3_hdr_t));
    tmp->frame_pos = st_pos;
    tmp->next_frame_pos = st_pos + mp3_flen;
    tmp->mp3_chans = mp3_chans;
    tmp->mp3_freq = mp3_freq;
    tmp->mpa_spf = mpa_spf;
    tmp->mpa_layer = mpa_layer;
    tmp->mpa_br = mpa_br;
    tmp->cons_hdrs = 1;
    tmp->next = *list;
    *list = tmp;
  }
  return NULL;
}

/**
 * \brief free a list of MP3 header descriptions
 * \param list pointer to the head-of-list pointer
 */
static void free_mp3_hdrs(mp3_hdr_t **list) {
  mp3_hdr_t *tmp;
  while (*list) {
    tmp = (*list)->next;
    free(*list);
    *list = tmp;
  }
}

static int parse_header(tinymp3_ctx_t *ctx){
    int mp3_freq, mp3_chans, mp3_flen, mpa_layer, mpa_spf, mpa_br;
	mp3_hdr_t *mp3_hdrs = NULL, *mp3_found = NULL;
	uint8_t hdr[HDR_SIZE];
	off_t st_pos = 0;
	int step, rSize, n = 0;
	int ret = DECODER_STOP;

	rSize = _tinymp3_ctx_read(ctx, hdr, HDR_SIZE);
	while(n < 30000 && !(ctx->quit)){
		st_pos = _tinymp3_ctx_lseek(ctx, 0, SEEK_CUR) - HDR_SIZE;
		step = 1;
		if( hdr[0] == 'I' && hdr[1] == 'D' && hdr[2] == '3' && hdr[3] >= 2 && hdr[3] != 0xff) {
			unsigned int len = id3v2_tag_size(ctx, hdr[3]);
			if(len == -1){
				ERROR("id3v2_tag_size fail\n");
				ret = DECODER_EOF;
				goto parse_exit;
			}
			if(len > 0)
				_tinymp3_ctx_lseek(ctx, len-10, SEEK_CUR);
			step = 4;
		} else if((mp3_flen = mp_get_mp3_header(hdr, &mp3_chans, &mp3_freq,
						&mpa_spf, &mpa_layer, &mpa_br)) > 0) {
			mp3_found = add_mp3_hdr(&mp3_hdrs, st_pos, mp3_chans, mp3_freq,
					mpa_spf, mpa_layer, mpa_br, mp3_flen);
			if (mp3_found){
				ctx->tiny->channels = mp3_found->mp3_chans;
				ctx->tiny->bps	= mp3_found->mpa_br;
				ctx->tiny->samplerate = mp3_found->mp3_freq;
				_tinymp3_ctx_lseek(ctx, mp3_found->frame_pos, SEEK_SET);
				DEBUG("frame start pos: %ld\n",mp3_found->frame_pos);
				ret = DECODER_OK;
				goto parse_exit;
			}
		}

		if(step < HDR_SIZE)
			memmove(hdr,&hdr[step],HDR_SIZE-step);
		rSize = _tinymp3_ctx_read(ctx, &hdr[HDR_SIZE - step], step);
		if (rSize < step && rSize >= 0 ) {
			ERROR("_tinymp3_ctx_read fail: %d\n", rSize);
			ret = DECODER_EOF;
			goto parse_exit;
		} else if (rSize < 0) {
			ERROR("read file: %s", strerror(errno));
			ret = DECODER_UNRECOVERY;
			goto parse_exit;
		}
		n++;
	}

parse_exit:

	if(n == 30000)
		ret = DECODER_EOF;

	free_mp3_hdrs(&mp3_hdrs);
	if (mp3_found){
		free(mp3_found);
		mp3_found = NULL;
	}
	return ret;
}

static int prepare_audio(tinymp3_ctx_t *ctx)
{
	mad_decoder_t *decode = &ctx->tiny->decoder;
	ao_device_t *device = (ao_device_t*)(ctx->tiny->ao_device);

	int retval;

	retval = decode_init(decode);
	if (retval < 0) {
		return retval;
	}

	decode->in_buf_len	= 0;
	decode->eof = 0;
	retval = parse_header(ctx);

	switch (retval) {
		case DECODER_OK:
			device->fmt.channels = ctx->tiny->channels;
			device->fmt.bytes = 2; /* force 16bit */
			device->fmt.rate = ctx->tiny->samplerate;
			return 0;

		case DECODER_EOF:
			ERROR("NOT found mp3 header");
			ERROR("This is not a mp3 file");
		case DECODER_UNRECOVERY:
			return -1;

		case DECODER_STOP:
			DEBUG("Stop in action");
			return 0;
	}

}
static audio_output_t *set_audio(tinymp3_ctx_t *ctx, char *interface, uint8_t *quit)
{
	mad_decoder_t *decode = &ctx->tiny->decoder;
	audio_output_t *ao;
	//ao_format_t fmt;
	int retval;

	ao_device_t *device = (ao_device_t*)(ctx->tiny->ao_device);

	ao = audio_get_output(interface);
	if (!ao) {
		ERROR("NOT found audio interface");
		return NULL;
	}
	ao->init(device);

	retval = ao->start(device);
	if (retval < 0) {
		ao->deinit(device);
		return NULL;
	}

	return ao;
}

static uint8_t _tinymp3_ctx_get_status(tinymp3_ctx_t *ctx)
{
	uint8_t status;
	pthread_mutex_lock( &ctx->_mutex );
	status = ctx->status;
	pthread_mutex_unlock( &ctx->_mutex );
	return status;

}

static void _tinymp3_ctx_set_status(tinymp3_ctx_t *ctx, uint8_t status)
{
	pthread_mutex_lock( &ctx->_mutex );
	ctx->status = status;
	pthread_mutex_unlock( &ctx->_mutex );
}

/*
 * block util play finish, can stop by quit_flag
 */
static int _tinymp3_ctx_play(tinymp3_ctx_t *ctx)
{
	audio_output_t *ao;

	int retval;
	int err = 0;
	char *ao_type = "alsa";//default is oss

	tiny_mp3_t *tiny = ctx->tiny;;
	ao_device_t *device = (ao_device_t *)(ctx->tiny->ao_device);
	uint8_t *quit_flag = &ctx->quit;
	//if (AUDIO_ALSA == get_audio_type())
	//ao_type = "alsa";

	device = (ao_device_t*)(tiny->ao_device);

	retval = play_device_try(ao_type);
	if (retval < 0) {
		ERROR("Audio device occupied, Can not play\n");
		err = -1;
		goto err_device_try;
	}

	ao = set_audio(ctx, ao_type, quit_flag);
	if (!ao){
		DEBUG("set_pcm failed");
		err = -1;
		goto err_set_audio;
	}

	pthread_mutex_lock(&ctx->_mutex);
	ctx->status = TINYMP3_STATUS_BUSY;
	pthread_mutex_unlock(&ctx->_mutex);

    struct timeval tv;
    gettimeofday(&tv, NULL);
	ctx->position = tv.tv_sec * 1000 + tv.tv_usec/1000;

	while (!(*quit_flag)) {
		retval = decode_frame(ctx, &tiny->decoder, quit_flag);
		switch (retval) {
		case DECODER_OK:
			play_frame(ctx, &tiny->decoder, ao);
			break;

		case DECODER_UNRECOVERY:
			err = -1;
			DEBUG("Tinymp3 stop with unrecovery");
			goto err_decode_unrecovery;

		case DECODER_EOF:
			DEBUG("Tinymp3 stop with eof (quit_flag:%d)", *quit_flag);
			goto process_end;
			break;
		case DECODER_STOP:
			DEBUG("Tinymp3 stop with action %d", *quit_flag);
			*quit_flag = 1;
			goto process_end;
			break;
		}
	}
	ctx->position = 0;
process_end://mark
	if(*quit_flag)
		err = -255;

err_decode_unrecovery:
	ao->stop(device);
	ao->deinit(device);

err_set_audio:
	decode_deinit(&tiny->decoder);

err_decode_init:
err_device_try:

err_open_file:
	//free(tiny);
	//tiny= NULL;//mark

	pthread_mutex_lock(&ctx->_mutex);
	ctx->status = TINYMP3_STATUS_IDLE;
	if(ctx->quit == 1) {
		pthread_cond_signal(&ctx->_cond);
		err = 0;
    }
	pthread_mutex_unlock(&ctx->_mutex);

	return err;
}

static int _tinymp3_ctx_read(tinymp3_ctx_t *ctx, char *buf, int size)
{
	if(ctx->ops != NULL && ctx->ops->read != NULL)
		return ctx->ops->read(buf, size, ctx->user);
	return -1;
}

static off_t _tinymp3_ctx_lseek(tinymp3_ctx_t *ctx, off_t offset, int whence)
{
	if(ctx->ops != NULL && ctx->ops->lseek != NULL)
		return  ctx->ops->lseek(offset, whence, ctx->user);
	return -1;
}

static int _tinymp3_ctx_elem_alloc(tinymp3_ctx_t *ctx)
{
	ctx->tiny = (tiny_mp3_t*)malloc(sizeof(tiny_mp3_t));
	if (!ctx->tiny) {
		ERROR("Alloc mad_decoder: %s", strerror(errno));
		return -1;
	}
	memset(ctx->tiny, 0, sizeof(tiny_mp3_t));

	ctx->tiny->ao_device = malloc(sizeof(ao_device_t));
	if(!ctx->tiny->ao_device) {
		ERROR("Alloc ao_device: %s", strerror(errno));
		return -1;
	}
	memset(ctx->tiny->ao_device, 0, sizeof(ao_device_t));

	ctx->output = malloc(8192);
	if(!ctx->output){
		ERROR("Alloc output buffer: %s", strerror(errno));
		return -1;
	}
	memset(ctx->output, 0, 8192);
	return 0;
}

static void _tinymp3_ctx_elem_free(tinymp3_ctx_t *ctx)
{
	if(ctx == NULL) return;

	if(ctx->output){
		free(ctx->output);
		ctx->output = NULL;
	}
	if(ctx->tiny->ao_device) {
		free(ctx->tiny->ao_device);
		ctx->tiny->ao_device = NULL;
	}
	if(ctx->tiny) {
		free(ctx->tiny);
		ctx->tiny = NULL;
	}
}

static int _tinymp3_ctx_prepare(tinymp3_ctx_t *ctx)
{
	return prepare_audio(ctx);
}

tinymp3_ctx_t* tinymp3_ctx_create(void *user, tiny_mp3_ops_t *ops)
{
	tinymp3_ctx_t *ctx = (tinymp3_ctx_t*)malloc(sizeof(tinymp3_ctx_t));
	if(!ctx){
		ERROR("Alloc tinymp3_ctx: %s", strerror(errno));
		return NULL;
	}
	memset(ctx, 0 , sizeof(tinymp3_ctx_t));

	if(_tinymp3_ctx_elem_alloc(ctx) < 0) goto err_create;

	ctx->quit = 0;
	ctx->status = TINYMP3_STATUS_IDLE;
	ctx->volume = 20;
	ctx->ops = ops;
	ctx->user = user;
	ctx->position = 0;

	pthread_mutex_init(&(ctx->_mutex), NULL);
	pthread_cond_init(&(ctx->_cond), NULL);

	return ctx;

err_create:
	_tinymp3_ctx_elem_free(ctx);
	return NULL;
}

void tinymp3_ctx_destroy(tinymp3_ctx_t** ctx)
{
	if(ctx == NULL || *ctx == NULL) return;
	if(_tinymp3_ctx_get_status(*ctx) == TINYMP3_STATUS_BUSY){
		ERROR("tinymp3_ctx_destroy, tiny_mp3 ctx is busying, wait or stop");
		return;
	}
	_tinymp3_ctx_elem_free(*ctx);
	pthread_mutex_destroy(&(*ctx)->_mutex);
	pthread_cond_destroy(&(*ctx)->_cond);
	free(*ctx);
	*ctx = NULL;
	DEBUG("tinymp3_ctx_destroy!");
}

int tinymp3_ctx_play(tinymp3_ctx_t *ctx)
{
	if(_tinymp3_ctx_get_status(ctx) == TINYMP3_STATUS_BUSY){
		ERROR("tinymp3_ctx_play, tiny_mp3 ctx is busying, wait or stop");
		return -1;
	}
	return _tinymp3_ctx_play(ctx);
}

int tinymp3_ctx_prepare(tinymp3_ctx_t *ctx)
{
	return _tinymp3_ctx_prepare(ctx);
}

int tinymp3_ctx_stop(tinymp3_ctx_t *ctx)
{
	pthread_mutex_lock(&ctx->_mutex);
	if(ctx->status == TINYMP3_STATUS_BUSY){
		ctx->quit = 1;
		DEBUG("tinymp3_ctx_stop waiting!");
		pthread_cond_wait(&ctx->_cond, &ctx->_mutex);
	}
	ctx->quit= 0;
	pthread_mutex_unlock(&ctx->_mutex);
	DEBUG("tinymp3_ctx_stop end!");
}

int tinymp3_ctx_setvolume(tinymp3_ctx_t *ctx, int volume)
{
	if(volume >= 0 && volume <= 40){
		ctx->volume = volume;
		DEBUG("set volume : %d", volume);
	}
	return 0;
}

int tinymp3_ctx_getvolume(tinymp3_ctx_t *ctx)
{
	return ctx->volume;
}

uint64_t tinymp3_ctx_get_position(tinymp3_ctx_t *ctx)
{
	if(_tinymp3_ctx_get_status(ctx) == TINYMP3_STATUS_IDLE){
		ctx->position = 0;
		return -1;
	}
    struct timeval tv;
    gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec/1000 - ctx->position;
}

/**************************************************************/
static void get_file_type(struct stat *st, char *type, int len)
{
    switch (st->st_mode & S_IFMT) {
    case S_IFBLK:
        strncpy(type, "block device", len);
        break;
    case S_IFCHR:
        strncpy(type, "character device", len);
        break;
    case S_IFDIR:
        strncpy(type, "directory", len);
        break;
    case S_IFIFO:
        strncpy(type, "FIFO/pipe", len);
        break;
    case S_IFLNK:
        strncpy(type, "symlink", len);
        break;
    default:
        strncpy(type, "unknown file", len);
    }
}

static int filldata(char *buf, int size, void *user)
{
    return read((int)user, buf, size);
}
static off_t seekdata(off_t offset, int whence, void *user)
{
    return lseek((int)user, offset, whence);
}

tiny_mp3_ops_t tinymp3_ctx_file_ops = {
    .read = filldata,
    .lseek = seekdata,
};

int tinymp3_ctx_prepare_file(tinymp3_ctx_t *ctx, const char *mp3_file)
{
	struct stat stat;
	int fd = open(mp3_file, O_RDONLY);
    if (fd < 0) {
        ERROR("Open media file: %s\n", strerror(errno));
        exit(-1);
    }

    if (fstat(fd, &stat) == -1) {
        ERROR("Get file status: %s\n", strerror(errno));
        exit(-1);
    }

    if (!S_ISREG(stat.st_mode) && !S_ISLNK(stat.st_mode)) {
        char type[32] = {0};
        get_file_type(&stat, type, sizeof(type));
        ERROR("\'%s\' Is a %s\n", mp3_file, type);
        exit(-1);
    }

    if (!stat.st_size) {
        ERROR("\'%s\' Is a empty file\n", mp3_file);
        exit(-1);
    }
    ctx->ops = &tinymp3_ctx_file_ops;
    ctx->user = (void*)fd;

    return _tinymp3_ctx_prepare(ctx);
}

int tinymp3_ctx_play_file(tinymp3_ctx_t *ctx)
{
	if(_tinymp3_ctx_get_status(ctx) == TINYMP3_STATUS_BUSY){
		ERROR("tinymp3_ctx_play, tiny_mp3 ctx is busying, wait or stop");
		return -1;
	}
	int ret = _tinymp3_ctx_play(ctx);
	close((int)ctx->user);
	return ret;
}
