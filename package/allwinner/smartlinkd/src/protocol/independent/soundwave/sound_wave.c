#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <alsa/asoundlib.h>
#include <time.h>
#include "adt.h"
#include <uci.h>
#include <sm_link_manager.h>
#include <smg_log.h>
#include <pthread.h>
#define UCI_CONFIG_FILE "/etc/config/smartlinkd"

#define RECORDFILE "/tmp/smrecord.pcm"

#define ADT_STR 129

#ifdef __cplusplus
extern "C" {
#endif
const int gsample_rate = 16000;

static int f_send_finished = 0;
#ifdef RECORDFILE
int recordfd;
#endif
/*wav Audio head format */
typedef struct _wave_pcm_hdr
{
    char        riff[4];                // = "RIFF"
    int         size_8;                 // = FileSize - 8
    char        wave[4];                // = "WAVE"
    char        fmt[4];                 // = "fmt "
    int         fmt_size;               // = The size of the next structure : 16

    short int   format_tag;             // = PCM : 1
    short int   channels;               // = channels num : 1
    int         samples_per_sec;        // = Sampling Rate : 6000 | 8000 | 11025 | 16000
    int         avg_bytes_per_sec;      // = The number if bytes per second : samples_per_sec * bits_per_sample / 8
    short int   block_align;            // = Number if bytes per sample point : wBitsPerSample / 8
    short int   bits_per_sample;        // = Quantization bit number: 8 | 16

    char        data[4];                // = "data";
    int         data_size;              // = Pure data length : FileSize - 44
} wave_pcm_hdr;

/* default wav audio head data */
wave_pcm_hdr default_wav_hdr =
{
    { 'R', 'I', 'F', 'F' },
    0,
    {'W', 'A', 'V', 'E'},
    {'f', 'm', 't', ' '},
    16,
    1,
    1,
    16000,
    32000,
    2,
    16,
    {'d', 'a', 't', 'a'},
    0
};

int signal_exit=0;
//void localsigroutine(int dunno){
//	smg_printf("sig: %d coming!\n",dunno);
//	switch(dunno){
//		case SIGINT:
//		case SIGQUIT:
//		case SIGHUP:
//		{
//            signal_exit =1;
//			//exit(0);
//            break;
//		}
//		case SIGPIPE:
//		{
//			//When the client is closed after start scaning and parsing,
//			//this signal will come, ignore it!
//			smg_printf(SMG_DEBUG,"do nothings for PIPE signal\n");
//            break;
//		}
//	}
//}

snd_pcm_t* alsa_init(char *device, int sample_rate){
    int err;

    smg_printf(SMG_DEBUG,"device: %s sample_rate is %d\n", device, sample_rate);

    snd_pcm_t *capture_handle = NULL;
    snd_pcm_hw_params_t *hw_params = NULL;

    if((err = snd_pcm_open(&capture_handle, device, SND_PCM_STREAM_CAPTURE, 0)) < 0){
        smg_printf(SMG_DEBUG,"cannot open audio device (%s)\n",  snd_strerror(err));
        return NULL;
    }

    if((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
        smg_printf(SMG_DEBUG,"cannot allocate hardware parameter structure (%s)\n",snd_strerror(err));
        goto fail;
    }

    if((err = snd_pcm_hw_params_any(capture_handle, hw_params)) < 0) {
        smg_printf(SMG_DEBUG,"cannot initialize hardware parameter structure (%s)\n", snd_strerror(err));
        goto fail;
    }

    if((err = snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        smg_printf(SMG_DEBUG,"cannot allocate hardware parameter structure (%s)\n",snd_strerror(err));
        goto fail;
    }

    if((err = snd_pcm_hw_params_set_format(capture_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
        smg_printf(SMG_DEBUG,"cannot allocate hardware parameter structure (%s)\n",snd_strerror(err));
        goto fail;
    }

    if((err = snd_pcm_hw_params_set_rate(capture_handle, hw_params, sample_rate, 0)) < 0) {
        smg_printf(SMG_DEBUG,"cannot set sample rate (%s)\n", snd_strerror(err));
        goto fail;
    }

    if((err = snd_pcm_hw_params_set_channels(capture_handle, hw_params, 1)) <0) {
        smg_printf(SMG_DEBUG,"cannot set channel count (%s)\n", snd_strerror(err));
        goto fail;
    }

    if((err = snd_pcm_hw_params(capture_handle, hw_params)) < 0) {
        smg_printf(SMG_DEBUG,"cannot set parameters (%s)\n", snd_strerror(err));
        goto fail;
    }

    snd_pcm_hw_params_free(hw_params);

    return capture_handle;
fail:
    smg_printf(SMG_DEBUG,"close dev\n");
    snd_pcm_close(capture_handle);
    smg_printf(SMG_DEBUG,"Fail\n");
    return NULL;
}
void alsa_release(snd_pcm_t* capture_handle){
    snd_pcm_close(capture_handle);
}
int alsa_record(snd_pcm_t* capture_handle, char *buf, int frames)
{
	int bufbyte;
    int r;
    r = snd_pcm_readi(capture_handle, buf, frames);
    if (r == -EPIPE) {
        smg_printf(SMG_DEBUG,"overrun occurred\n");
        snd_pcm_prepare(capture_handle);
    } else if (r < 0) {
        smg_printf(SMG_DEBUG,"error from read: %s\n", snd_strerror(r));
    } else if (r > 0) {
        // 2 bytes/sample, 1 or 2 channels, r: frames
    }
#ifdef RECORDFILE
	bufbyte=(default_wav_hdr.fmt_size/8)*(default_wav_hdr.channels)*frames;
	if (write(recordfd,buf,bufbyte) != bufbyte) {
	    fprintf(stderr, "Error SNDWAV_Record[write]/n");
	    exit(-1);
	}
#endif
    return r;
}

static void decoder_info(config_decoder_t config_decoder)
{
	printf("---------------------------------------------------------------------------------\n");
	switch(config_decoder.freq_type)
	{
		case 0:printf("frequency type:    \tFREQ_TYPE_LOW\n");break;
		case 1:printf("frequency type:    \tFREQ_TYPE_MIDDLE\n");break;
		case 2:printf("frequency type:    \tFREQ_TYPE_HIGH\n");break;
		default:printf("frequency type:    \tERROR\n");break;
	}
	printf("sample rate:     \t%d\n",config_decoder.sample_rate);
	printf("max string length:\t%d\n",config_decoder.max_strlen);
	printf("grounp symbol num:\t%d\n",config_decoder.group_symbol_num);
	if(config_decoder.error_correct==1){
		printf("Error correction num:\t%d\n",config_decoder.error_correct_num);
	}
	printf("---------------------------------------------------------------------------------\n");
}

static int load_config(const char *section,const char *option,int value)
{
	struct uci_context *ctx =NULL;
	struct uci_package *pkg=NULL;
	struct uci_element *e=NULL ;

	char cmd[100];
	ctx = uci_alloc_context();	//Allocate a new uci context
	if(UCI_OK !=uci_load(ctx,UCI_CONFIG_FILE,&pkg)){
		printf("opps!! uci_load error");
		goto cleanup;
	}

	uci_foreach_element(&pkg->sections, e) // loop through a list of uci elements
	{
		struct uci_section *s = uci_to_section(e);
		if(!strcmp(section,s->e.name)){
			struct uci_option *o = uci_lookup_option(ctx, s, option);  //look up an option

			if ((NULL != o) && (UCI_TYPE_LIST == o->type)){
				struct uci_element *e=NULL;
				uci_foreach_element(&o->v.list, e)
				{
					if(value >= 0)
						sprintf(cmd,"%s %d",e->name,value);
					else
						sprintf(cmd,"%s",e->name);
					system(cmd);
				}

			}
		}
	}
	uci_unload(ctx, pkg);
cleanup:
	uci_free_context(ctx);
	ctx = NULL;
}

struct sound_wave_resource {
	snd_pcm_t *capture_handle;
	void* decoder;
};

static struct sound_wave_resource resource = {
	.capture_handle = NULL,
	.decoder =NULL,
};

int sound_wave_protocol_resource_free(void *arg)
{
	if(resource.capture_handle)
	    alsa_release(resource.capture_handle);

	load_config("record","record_off",-1);

    /* free handle */
	if(resource.decoder)
		decoder_destroy(resource.decoder);
}

int sound_wave_protocol(void *arg)
{
    int ret_dec;
    int items, bsize,i;
    void* decoder;
    short* buffer;
    char out[ADT_STR];
	char *_pcm_device = NULL;
	int _freq_type;
	int _sample_rate;
	int _timeouts;
	struct net_info netInfo;
	char *token = NULL;
	bool is_receive = false;

	struct pro_feedback info;

	struct pro_worker *_worker = (struct pro_worker *)arg;

	 _worker->enable = true;

	info.force_quit_sm = atoi((_worker->params->argv)[0]);
	info.protocol = _worker->type;

	 _pcm_device = (_worker->params->argv)[1];
	 _freq_type = atoi((_worker->params->argv)[2]);
	 _sample_rate = atoi((_worker->params->argv[3]));
	 _timeouts = atoi((_worker->params->argv[4]));

	 smg_printf(SMG_DEBUG,"pcm device:%s,freq type:%d,Sampling:%d,timeout:%d\n",
			_pcm_device,_freq_type,_sample_rate,_timeouts);

    /* ADT param */
    ret_dec = RET_DEC_NORMAL;
    config_decoder_t config_decoder;
    config_decoder.max_strlen = ADT_STR - 1;
    config_decoder.freq_type = _freq_type;
    config_decoder.sample_rate = _sample_rate;
    config_decoder.group_symbol_num = 10;
    config_decoder.error_correct = 1;
    config_decoder.error_correct_num = 4;
	decoder_info(config_decoder);
	load_config("record","record_on",-1);   //open record device
#ifdef RECORDFILE
    if ((recordfd = open(RECORDFILE, O_WRONLY | O_CREAT, 0644)) == -1) {
        fprintf(stderr, "Error open: [%s]/n", RECORDFILE);
		goto end;
    }
#endif
    /* create decoder handle */
    decoder = decoder_create(& config_decoder);
    if(decoder == NULL)
    {
        smg_printf(SMG_DEBUG,"allocate handle error !\n");
		goto end;
    }

	resource.decoder = decoder;


    /* get buffer size and allocate buffer */
    bsize = decoder_getbsize(decoder);
    buffer = (short*)malloc(sizeof(short)*bsize);
    if(buffer == NULL)
    {
        smg_printf(SMG_DEBUG,"allocate buffer error !\n");
        goto finish;
    }
    smg_printf(SMG_DEBUG,"bsize: %d\n",bsize);
    snd_pcm_t *capture_handle = alsa_init(_pcm_device, config_decoder.sample_rate);
    if(capture_handle == NULL)
        goto finish;

	resource.capture_handle = capture_handle;

    _timeouts = _timeouts * config_decoder.sample_rate/bsize;

	_worker->free_cb = sound_wave_protocol_resource_free;
    /* decoding loop */
    while(_worker->enable && ret_dec == RET_DEC_NORMAL && _timeouts--)
    {

        /*samples get from ADC */
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

        items = alsa_record(capture_handle,(char*)buffer,bsize);
        if(items < 0 && items != -EPIPE){

        }

        /*padding with zeros if items is less than bsize samples*/
        for(i = items; i< bsize; i++)
        {
            buffer[i] = 0;
        }
        /* input the pcm data to decoder */
        ret_dec = decoder_fedpcm(decoder, buffer);
    }
    /* check if we can get the output string */
    if(ret_dec != RET_DEC_ERROR)
    {
        /* get the decoder output */
        ret_dec = decoder_getstr(decoder, out);
        if(ret_dec == RET_DEC_NORMAL)
        {
            /* this is the final decoding output */
            smg_printf(SMG_DEBUG,"recv outchar: %s \n", out);
			token = strstr(out,"::div::");
			if(token == NULL) {
				is_receive = true;
				goto finish;
			}
			strcpy(netInfo.password,token+7);
			*token='\0';
			strcpy(netInfo.ssid,out);
			is_receive = true;
//            strncpy(info->adt_str,out,sizeof(info->adt_str));
        }else
        {
            /* decoding have not done, so nothing output */
            smg_printf(SMG_DEBUG,"decoder output nothing! \n");
        }
    }else
    {
        smg_printf(SMG_DEBUG,"decoder error \n");
    }

finish:
end:
	_worker->enable = false;
	_worker->cb(&info,is_receive,&netInfo);

    return 0;
}
