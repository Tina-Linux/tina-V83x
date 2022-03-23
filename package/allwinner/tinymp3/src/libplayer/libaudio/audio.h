/*
	audio.h
 */

#ifndef _AUDIO_H__
#define _AUDIO_H__

/**
 * ao_format_t
 * @channels:
 * @bits:
 * @format:
 * @rate:
 */
typedef struct {
	int	channels;	/* number of audio channels */
	int	bytes;		/* bytes per sample */
	int	format;		/* pcm format */
	int	rate;		/* samples per second */
} ao_format_t;

/**
 * ao_device_t
 * @handler:
 * @hw_gain:
 * @fmt:
 */
typedef struct {
	void *handler;	/* number of audio channels */
	ao_format_t fmt;
	int hw_gain;
} ao_device_t;

/**
 * audio_output_t
 */
typedef struct {
	char	*name;

	int (*init)(ao_device_t *device);
	void (*deinit)(ao_device_t *device);

	int (*start)(ao_device_t *device);
	void (*play)(ao_device_t *device, short *buf, int samples);
	void (*stop)(ao_device_t *device);

	void (*help)(void);
	void (*volume)(ao_device_t *device, int vol);
	int (*dev_try)(ao_device_t *device);
	int (*get_space)(ao_device_t *device);
} audio_output_t;

/**
 * audio_get_output -
 */
extern audio_output_t *audio_get_output(char *name);

/**
 * audio_ls_outputs -
 */
extern void audio_ls_outputs(void);

#endif /* _AUDIO_H__ */
