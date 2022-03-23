#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <math.h>
#include <tinyalsa/asoundlib.h>
#include "media_playback.h"
#include "adpcm.h"

#define SAVE_PCM_FLAG 0

int32_t close_flag = 0;

struct riff_wave_header {
	uint32_t riff_id;
	uint32_t riff_sz;
	uint32_t wave_id;
};

struct chunk_header {
	uint32_t id;
	uint32_t sz;
};

struct chunk_fmt {
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_per_sample;
};

struct ctx {
	struct pcm *pcm;

	struct riff_wave_header wave_header;
	struct chunk_header chunk_header;
	struct chunk_fmt chunk_fmt;

	FILE *file;
        FILE *savePcmFp;
};

struct cmd {
	const char *filename;
	const char *filetype;
	unsigned int card;
	unsigned int device;
	int flags;
	struct pcm_config config;
	unsigned int bits;
};

void cmd_init(struct cmd *cmd)
{
    cmd->filename = NULL;
    cmd->filetype = NULL;
    cmd->card = 0;
    cmd->device = 0;
    cmd->flags = PCM_OUT;
    cmd->config.period_size = 512;
    cmd->config.period_count = 2;
    cmd->config.channels = 2;
    cmd->config.rate = 16000;
    cmd->config.format = PCM_FORMAT_S16_LE;
    //cmd->config.silence_threshold = 1024 * 2;
    //cmd->config.stop_threshold = 1024 * 2;
    //cmd->config.start_threshold = 1024;

    cmd->config.silence_threshold = cmd->config.period_size * cmd->config.period_count;
    cmd->config.stop_threshold = cmd->config.period_size * cmd->config.period_count;
    cmd->config.start_threshold = cmd->config.period_size;
    cmd->bits = 16;
}

int ctx_init(struct ctx* ctx, const struct cmd *cmd)
{
    unsigned int bits = cmd->bits;
    struct pcm_config config = cmd->config;

    if (cmd->filename == NULL) {
        fprintf(stderr, "filename not specified\n");
        return -1;
    }

    ctx->file = fopen(cmd->filename, "rb");
    if (ctx->file == NULL) {
        fprintf(stderr, "failed to open '%s'\n", cmd->filename);
        return -1;
    }
    printf("ctx_init:  cmd->filetype = %p ,cmd->filetype = %s\n",cmd->filetype,cmd->filetype);
    if ((cmd->filetype != NULL) && (strcmp(cmd->filetype, "wav") == 0)) {
        if (fread(&ctx->wave_header, sizeof(ctx->wave_header), 1, ctx->file) != 1){
            fprintf(stderr, "error: '%s' does not contain a riff/wave header\n", cmd->filename);
            fclose(ctx->file);
            return -1;
        }
        if ((ctx->wave_header.riff_id != ID_RIFF) ||
            (ctx->wave_header.wave_id != ID_WAVE)) {
            fprintf(stderr, "error: '%s' is not a riff/wave file\n", cmd->filename);
            fclose(ctx->file);
            return -1;
        }
		unsigned int more_chunks = 1;
        do {
            if (fread(&ctx->chunk_header, sizeof(ctx->chunk_header), 1, ctx->file) != 1){
                fprintf(stderr, "error: '%s' does not contain a data chunk\n", cmd->filename);
                fclose(ctx->file);
                return -1;
            }
            switch (ctx->chunk_header.id) {
            case ID_FMT:
                if (fread(&ctx->chunk_fmt, sizeof(ctx->chunk_fmt), 1, ctx->file) != 1){
                    fprintf(stderr, "error: '%s' has incomplete format chunk\n", cmd->filename);
                    fclose(ctx->file);
                    return -1;
                }
                /* If the format header is larger, skip the rest */
                if (ctx->chunk_header.sz > sizeof(ctx->chunk_fmt))
                    fseek(ctx->file, ctx->chunk_header.sz - sizeof(ctx->chunk_fmt), SEEK_CUR);
                break;
            case ID_DATA:
                /* Stop looking for chunks */
                more_chunks = 0;
                break;
            default:
                /* Unknown chunk, skip bytes */
                fseek(ctx->file, ctx->chunk_header.sz, SEEK_CUR);
            }
        } while (more_chunks);
        config.channels = ctx->chunk_fmt.num_channels;
        config.rate = ctx->chunk_fmt.sample_rate;
        bits = ctx->chunk_fmt.bits_per_sample;
    }

	config.format = PCM_FORMAT_S16_LE;
    ctx->pcm = pcm_open(cmd->card,
                        cmd->device,
                        cmd->flags,
                        &config);
    if (ctx->pcm == NULL) {
        fprintf(stderr, "failed to allocate memory for pcm\n");
        fclose(ctx->file);
        return -1;
    } else if (!pcm_is_ready(ctx->pcm)) {
        fprintf(stderr, "failed to open for pcm %u,%u\n", cmd->card, cmd->device);
        fclose(ctx->file);
        pcm_close(ctx->pcm);
        return -1;
    }

    return 0;
}

void ctx_free(struct ctx *ctx)
{
    if (ctx == NULL) {
        return;
    }
    if (ctx->pcm != NULL) {
        //printf("do not close sound card\n");
        pcm_close(ctx->pcm);
    }
    if (ctx->file != NULL) {
        fclose(ctx->file);
    }
}

void stream_close(int sig)
{
    /* allow the stream to be closed gracefully */
    signal(sig, SIG_IGN);
    close_flag = 1;
}

int play_sample(struct ctx *ctx)
{
    char *buffer;
    int size;
    int num_read;

    size = pcm_frames_to_bytes(ctx->pcm, pcm_get_buffer_size(ctx->pcm));
    buffer = malloc(size);
    if (!buffer) {
        fprintf(stderr, "unable to allocate %d bytes\n", size);
        return -1;
    }

    /* catch ctrl-c to shutdown cleanly */
    signal(SIGINT, stream_close);

    do {
        num_read = fread(buffer, 1, size, ctx->file);
        if (num_read > 0) {
		if (pcm_writei(ctx->pcm, buffer,
			pcm_bytes_to_frames(ctx->pcm, num_read)) < 0) {
                fprintf(stderr, "error playing sample\n");
                break;
            }
        }
    } while (!close_flag && num_read > 0);

    free(buffer);
    return 0;
}

int play_adpcm(struct ctx *ctx)
{
    int16_t *buffer;
    int each_frame_size;

    uint8_t originHighsample;
    uint8_t originLowsample;
    int16_t decodedsample;
    int needReadDataSize;
    int realReadSize; //the real size each read from file,the unit is byte
    int realWriteSize;//the real size which write to sound card ,the unit is byte

    each_frame_size = pcm_frames_to_bytes(ctx->pcm, pcm_get_buffer_size(ctx->pcm));
    printf("each frame size  = %d Byte\n",each_frame_size);
    needReadDataSize = each_frame_size/4;
    uint8_t readData[needReadDataSize];
    buffer = (int16_t *)malloc(each_frame_size);
    if (!buffer) {
        fprintf(stderr, "unable to allocate %d bytes\n", each_frame_size);
        return -1;
    }

    /* catch ctrl-c to shutdown cleanly */
    signal(SIGINT, stream_close);
    int initDecodeFlag = 0;
    do {
        /*decoded adpcm to pcm,and store the decoded pcm data in buffer begin*/
        int bufIndex = 0;
        realReadSize = fread(readData,sizeof(uint8_t),needReadDataSize,ctx->file);
        //printf("realReadSize = %d\n",realReadSize);
        for(int count = 0 ; count < realReadSize; count++){
            originHighsample = readData[count] >> 4;
            if(initDecodeFlag == 0){
                decodedsample = ADPCM_Decode(originHighsample,initDecodeFlag);
                initDecodeFlag = 1;
            }else{
                decodedsample = ADPCM_Decode(originHighsample,initDecodeFlag);
            }
            memcpy(buffer+bufIndex,&decodedsample,sizeof(int16_t));
            bufIndex++;
            originLowsample = readData[count] & 0x0F;
            decodedsample = ADPCM_Decode(originLowsample,initDecodeFlag);
            memcpy(buffer+bufIndex,&decodedsample,sizeof(int16_t));
            bufIndex++;
        }
        /*decoded adpcm to pcm,and store the decoded pcm data in buffer finish*/

        realWriteSize = realReadSize*4;
        if (realWriteSize > 0) {

#if SAVE_PCM_FLAG
    if(ctx->savePcmFp){
        fwrite(buffer,sizeof(int16_t),realWriteSize/sizeof(int16_t),ctx->savePcmFp);
    }
#endif
		if (pcm_writei(ctx->pcm, buffer,
			pcm_bytes_to_frames(ctx->pcm, realWriteSize)) < 0) {
                fprintf(stderr, "error playing adpcm\n");
                break;
            }

        }
    } while (!close_flag && realWriteSize > 0);
    free(buffer);
    buffer =NULL;
    return 0;
}


void playback(char *path, struct data_config *handle_config)
{
	struct cmd cmd;
	struct ctx ctx;
	int8_t flag;

	cmd_init(&cmd);
	cmd.filename = path;
        cmd.filetype = "wav";
	if (ctx_init(&ctx, &cmd) < 0) {
			printf("ctx_init error!!!\n");
		}

		printf("playing '%s': %u ch, %u hz, %u bit\n",
			cmd.filename,
			cmd.config.channels,
			cmd.config.rate,
			cmd.bits);
        if(ctx.chunk_fmt.audio_format == 1){//the pcm type is 1
            printf("the data is pcm\n");
            if (play_sample(&ctx) < 0) {
		ctx_free(&ctx);
		printf("play_sample error!!\n");
	    }
        }else if(ctx.chunk_fmt.audio_format == 2){//the adpcm type is 2
            printf("the data is adpcm\n");
#if SAVE_PCM_FLAG
            char save_pcm_path[256];
            strcpy(save_pcm_path,path);
            strcat(save_pcm_path,".decoded.pcm");
            ctx.savePcmFp = fopen(save_pcm_path,"wb");
            if(ctx.savePcmFp == NULL){
                printf("open %s fail\n",save_pcm_path);
            }
#endif
            if (play_adpcm(&ctx) < 0) {
		ctx_free(&ctx);
		printf("play_adpcm error!!\n");
	    }

#if SAVE_PCM_FLAG
            if(ctx.savePcmFp){
                fclose(ctx.savePcmFp);
                ctx.savePcmFp = NULL;
            }
#endif
        }else{
            printf("err:it is neither pcm nor adpcm,can not play\n");
        }
	ctx_free(&ctx);
	handle_config->play_flag = 0;
}
