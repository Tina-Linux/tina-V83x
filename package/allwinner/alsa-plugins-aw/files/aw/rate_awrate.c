/* Allwinner Rate converter plugin using Public Parrot Hack
   Copyright (C) 2007 Jean-Marc Valin + 2018 Khan Chan

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

   1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include <stdio.h>
#include <stdint.h>
#include <alsa/asoundlib.h>
#include <alsa/pcm_rate.h>
#include "doResample.h"

#ifdef AWRATE_DUMP_PCM_DATA
#include <sys/stat.h>
static int dump_output_music_fd = -1;
static int dump_input_music_fd = -1;
#endif


struct rate_src {
    double ratio;
    unsigned int stored;
    unsigned int channels;
    int16_t *out;
    int16_t *in;
    int      point;
    ResCfg     rc;
    Resampler *r;
};

static snd_pcm_uframes_t input_frames(void *obj, snd_pcm_uframes_t frames)
{
	struct rate_src *rate = obj;
	if (frames == 0)
		return 0;
	return (snd_pcm_uframes_t)(frames / rate->ratio);
}

static snd_pcm_uframes_t output_frames(void *obj, snd_pcm_uframes_t frames)
{
	struct rate_src *rate = obj;
	if (frames == 0)
		return 0;
	return (snd_pcm_uframes_t)(frames * rate->ratio);
}

static void pcm_src_free(void *obj)
{
   struct rate_src *rate = obj;
   int i;

   if (rate->out) {
       free(rate->out);
   }
   if (rate->in) {
       free(rate->in);
   }
   rate->out = rate->in = NULL;

   if (rate->r)
   {
      printf("release resampler...\n");
      Destroy_Resampler(rate->r);
      rate->r = NULL;
   }
}

static int pcm_src_init(void *obj, snd_pcm_rate_info_t *info)
{
   struct rate_src *rate = obj;
   int err, i;

   if (!rate->r || rate->channels != info->channels) {
      pcm_src_free(rate);
      rate->rc.inch  = rate->channels = info->channels;
      rate->rc.insrt = info->in.period_size;
      rate->rc.outsrt = info->out.period_size;
      printf("Create resampler...\n");
      rate->r = Create_Resampler();
      if (!rate->r)
         return -EINVAL;
   }
   printf("out size : %d, in size : %d\n", info->out.period_size, info->in.period_size);
   rate->out = calloc(info->out.period_size * 2 * rate->channels, sizeof(int16_t));
   rate->in  = calloc(info->in.period_size * 2 * rate->channels, sizeof(int16_t));
   rate->ratio = ((double)info->out.period_size / (double)info->in.period_size);
   rate->point = info->in.period_size / 2;
   if (!rate->out || !rate->in) {
       pcm_src_free(rate);
       return -ENOMEM;
   }

   return 0;
}


static void pcm_src_reset(void *obj)
{
   struct rate_src *rate = obj;
   rate->stored = 0;
   //do nothing
}

static void inputdata(const int16_t *src, int16_t *dst, unsigned int frames,
    unsigned int chans, int overflow)
{
    memcpy(dst + overflow, src, frames*chans*sizeof(int16_t));
}

static void getoutdata(int16_t *src, int16_t *dst, unsigned int actual, unsigned int ideal,
    unsigned int chans)
{
#if 0
    //1st interplotion, if n samples needed to interplote, interploting one sample by 1st interplotion in every n block.
    // | -------------------- | ---------------------| --------------------- |
    //                      ^                       ^                       ^
    //                      |                       |                       |
    //                  sample 1                  sample 2               sample n
    int i = 0, j = 0;

    if(actual < ideal)
    {
        int delta  = ideal - actual;
        int stride = actual/delta;
        int16_t *ptr_src = src;
        int16_t *ptr_dts = dst;
        //printf("actual : %d, ideal : %d\n", actual, ideal);
        for(i = 0; i < delta; i++)
        {
            if(i == delta - 1)
            {
                stride = actual - i*stride;

            }

            memcpy(ptr_dts, ptr_src, stride*chans*sizeof(int16_t));
            for(j = 0; j < chans; j++)
            {
                ptr_dts[stride*chans + j] = ptr_dts[(stride - 1)*chans + j];
                ptr_dts[(stride - 1)*chans + j] = (ptr_dts[(stride - 2)*chans + j] + ptr_dts[(stride - 1)*chans + j])/2;
            }
            ptr_dts += (stride+1)*chans;
            ptr_src += stride*chans;
        }
    }
    else
    {
        memcpy(dst, src, ideal*chans*sizeof(int16_t));
    }
#endif

    unsigned int offset = actual < ideal ? ideal - actual : 0;
    memcpy(dst + offset*chans, src, actual*chans*sizeof(int16_t));
}

static void pcm_src_convert_s16(void *obj, int16_t *dst, unsigned int dst_frames,
                const int16_t *src, unsigned int src_frames)
{
   struct rate_src *rate = obj;

   int consumed = 0, chans=rate->channels, ret=0, i;
   int total_in = rate->stored + src_frames, new_stored;
#ifdef AWRATE_DUMP_PCM_DATA
	int dump_music_len = 0;
	if (dump_input_music_fd == -1) {
		if (access("/tmp/awrate_input.raw", F_OK|W_OK) != 0) {
			mode_t create_fd_mode = 0;
			create_fd_mode = S_IRUSR|S_IWUSR;
			dump_input_music_fd = open("/tmp/awrate_input.raw", O_WRONLY|O_CREAT|O_EXCL, create_fd_mode);
			if (dump_input_music_fd == -1) {
				if (errno == EEXIST)
					printf("dump mustic fd is already exists!");
				else
					printf("create dump mustic fd failed: %s!", strerror(errno));
			}
		} else {
			dump_input_music_fd = open("/tmp/awrate_input.raw", O_WRONLY|O_TRUNC);
			if (dump_input_music_fd == -1) {
				printf("open dump music fd failed: %s!", strerror(errno));
			}
		}
	}
	struct stat fd_info;
	if (fstat(dump_input_music_fd, &fd_info) == 0) {
		if (fd_info.st_size > 4194304) {
			printf("dump music file is larger than the limit, empty it");
			lseek(dump_input_music_fd, 0, SEEK_SET);
			ftruncate(dump_input_music_fd, 4194304);
		}
	}
	else {
		printf("get dump_input_music_fd info error");
	}
	dump_music_len = write(dump_input_music_fd, (char*)src, chans *src_frames * sizeof(int16_t));
#endif
   inputdata(src, rate->in, src_frames, chans, rate->point);
   {
       rate->rc.samples = src_frames;
       rate->rc.inbuf   = (char*)(rate->in + rate->point - rate->stored);
       rate->rc.outbuf  = (char*)rate->out;
       rate->r->prepare(rate->r, &rate->rc);
       ret = rate->r->process(rate->r);
       consumed = src_frames;
       new_stored = total_in-consumed;
       if(new_stored)
       {
           printf("????\n");
           memmove(rate->in + rate->point - new_stored,
                rate->in + rate->point - rate->stored + consumed,
                new_stored * chans * sizeof(int16_t));
       }
       rate->stored = new_stored;
   }
   getoutdata(rate->out, dst, ret, dst_frames, chans);
#ifdef AWRATE_DUMP_PCM_DATA
	if (dump_output_music_fd == -1) {
		if (access("/tmp/awrate_output.raw", F_OK|W_OK) != 0) {
			mode_t create_output_fd_mode = 0;
			create_output_fd_mode = S_IRUSR|S_IWUSR;
			dump_output_music_fd = open("/tmp/awrate_output.raw", O_WRONLY|O_CREAT|O_EXCL, create_output_fd_mode);
			if (dump_output_music_fd == -1) {
				if (errno == EEXIST)
					printf("dump mustic fd is already exists!");
				else
					printf("create dump mustic fd failed: %s!", strerror(errno));
			}
		} else {
			dump_output_music_fd = open("/tmp/awrate_output.raw", O_WRONLY|O_TRUNC);
			if (dump_output_music_fd == -1) {
				printf("open dump music fd failed: %s!", strerror(errno));
			}
		}
	}
	struct stat fd_out_info;
	if (fstat(dump_output_music_fd, &fd_out_info) == 0) {
		if (fd_out_info.st_size > 5242880) {
			printf("dump music file is larger than the limit, empty it");
			lseek(dump_output_music_fd, 0, SEEK_SET);
			ftruncate(dump_output_music_fd, 5242880);
		}
	}
	else {
		printf("get dump_output_music_fd info error");
	}
	dump_music_len = write(dump_output_music_fd, (char*)dst, chans * dst_frames * sizeof(int16_t));
	if (dump_music_len != chans * dst_frames * sizeof(int16_t)) {
		printf("dump output music data error: %s!", strerror(errno));
	}

#endif

}

static void pcm_src_close(void *obj)
{
   free(obj);
#ifdef AWRATE_DUMP_PCM_DATA
	if (dump_output_music_fd != -1) {
		close(dump_output_music_fd);
		dump_output_music_fd = -1;
	}

	if (dump_input_music_fd != -1) {
		close(dump_input_music_fd);
		dump_input_music_fd = -1;
	}

#endif

}

#if SND_PCM_RATE_PLUGIN_VERSION >= 0x010002
static int get_supported_rates(void *obj, unsigned int *rate_min,
                   unsigned int *rate_max)
{
    *rate_min = *rate_max = 0; /* both unlimited */
    return 0;
}

static void dump(void *obj, snd_output_t *out)
{
    snd_output_printf(out, "Converter: libaw, xia, shang, zhou, qin, han, jin, north wei, south chen, sui, tang, song, liao, jin, western xia, yuan, ming, qing"
              "????"
              "\n");
}
#endif

static snd_pcm_rate_ops_t pcm_src_ops = {
    .close = pcm_src_close,
    .init = pcm_src_init,
    .free = pcm_src_free,
    .reset = pcm_src_reset,
    .adjust_pitch = 0,
    .convert_s16 = pcm_src_convert_s16,
    .input_frames = input_frames,
    .output_frames = output_frames,
#if SND_PCM_RATE_PLUGIN_VERSION >= 0x010002
    .version = SND_PCM_RATE_PLUGIN_VERSION,
    .get_supported_rates = get_supported_rates,
    .dump = dump,
#endif
};

int SND_PCM_RATE_PLUGIN_ENTRY(awrate)(unsigned int version, void **objp,
                       snd_pcm_rate_ops_t *ops)
{
    struct rate_src *rate;
    rate = calloc(1, sizeof(*rate));
    if (!rate)
        return -ENOMEM;
    *objp = rate;
#if SND_PCM_RATE_PLUGIN_VERSION >= 0x010002
    if (version == 0x010001)
    {
        memcpy(ops, &pcm_src_ops, sizeof(snd_pcm_rate_old_ops_t));
    }
    else
#endif
    {
        *ops = pcm_src_ops;
    }
    return 0;
}
