#ifndef SINE_DETECTOR_H
#define SINE_DETECTOR_H

#define	NN				1024
#define	CHANNL			1
#define	BYTE			2

#define SATURATE(x,a)	(((x)>(a) ? (a) : (x)<-(a) ? -(a) : (x)))

#define PI			3.1415926535897932384626434
#define MAX_16		32767
#define ALPHA		0.98
//#define THRESHOLD	10//25

typedef struct nf_ref{
	int		sampleRate;
	int		sin_flag;
	int		var_flag;
	int		startTime;
	int		eerNum;
	int		frameNum;
	float	freq;
	float	rr;
	float	aa;
	float	ybuf[2];
	float	xbuf[2];
	float	xPow;
	float	yPow;
	float	threshold;



}nf_ref_t;

int  NF_init(nf_ref_t *nf, int SampleRate, float freq, float threshold);
int  NF_end(nf_ref_t *nf);
int  notch_filter(nf_ref_t *nf, short *buffer, short *output, int DataNum);

#endif