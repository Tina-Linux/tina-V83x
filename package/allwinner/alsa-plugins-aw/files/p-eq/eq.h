#ifndef _EQ_H_
#define _EQ_H_
#include <stdlib.h>

#ifdef FIXED_POINT
#else
#include <math.h>
#endif
#include <math.h>
/* EQ filter type definition */
typedef enum
{
	/* low pass shelving filter */
	LOWPASS_SHELVING,
	/* band pass peak filter */
	BANDPASS_PEAK,
	/* high pass shelving filter */
	HIHPASS_SHELVING,
	LOWPASS,
	HIGHPASS
}eq_ftype_t;

/* equalizer parameters */
typedef struct
{
	/* boost/cut gain in dB */
	int G;
	/* cutoff/center frequency in Hz */
	int fc;
	/* quality factor */
	float Q;
	/* filter type */
	eq_ftype_t type;
}eq_core_prms_t;

typedef struct
{
    int enabled;
    int dump_flag;
	/*num of items(biquad)*/
	int biq_num;
	/* sampling rate */
	int sampling_rate;
	/* channel num */
	int chan;
	/* eq parameters for generate IIR coeff*/
	eq_core_prms_t* core_prms;
}eq_prms_t;


/*
	function eq_create
description:
	use this function to create the equalizer object
prms:
	eq_prms_t: [in], desired frequency response array
returns:
	the equalizer handle
*/
void* eq_create(eq_prms_t* prms);
/*
	function eq_process
description:
	equalizer processing function
prms:
	handle:[in,out] equalzier handle
	x:[in,out],	input signal
	len:[in], input length(in samples)
returns:
	none
*/
void eq_process(void* handle, short* x, int len);
/*
	function eq_destroy
description:
	use this function to destroy the equalizer object
prms:
	handle:[in], equalizer handle
returns:
	none
*/
void eq_destroy(void* handle);


#endif
