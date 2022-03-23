/*
	equalizer implemented in shelving filters and peak filters
	with this implementation, you can realize any desired frequency(amplitude) response equalizers
	First you should define the desired frequency and use the dedicated function to get the filter coeffs
	Then use the coeffs to create the equalizer object which should be used to do the equalizer function;
references:
	[1] Udo zolzer, Digital Audio Signal Processing, 2nd edition, A John Wiley & Sons, Ltd, Publication,2008

author: yhxie, 2014-09-12, yhxie@allwinnertech.com

*/

#define FIXED_POINT

#include "eq.h"
#ifdef FIXED_POINT
typedef int	eqdata_t;
typedef long long eqldata_t;
typedef short eqhdata_t;

#define QCONST32(x,bits) ((eqdata_t)(.5+(x)*(((eqdata_t)1)<<(bits))))
#else
//typedef double eqdata_t;
//typedef double eqldata_t;
//typedef double eqhdata_t;
//#define QCONST32(x,bits)	(x)

typedef float eqdata_t;
typedef float eqldata_t;
typedef float eqhdata_t;
#define QCONST32(x,bits)	(x)
#endif

#define EQ_SATURATE(x,a) (((x)>(a) ? (a) : (x)<-(a) ? -(a) : (x)))
#define PI	QCONST32(3.1415926,15)

/* biquad iir filter coefficients */
typedef struct
{
    /* numerator, Q15(FIXED_POINT) */
    eqdata_t b[3];
    /* denominator, Q15(FIXED_POINT) */
    eqdata_t a[3];
} eqcoeff_t;

/* biquad delay line */
typedef struct
{
    /* delay line */
    eqldata_t dline[2];
} eqdline_t;

typedef struct
{
    /* filter coeffs */
    eqcoeff_t *coeff;
    /* delay line holders */
    eqdline_t *dline;
    /* biquad num */
    int Chan;
    int N;
} equalizer_t;


#ifdef FIXED_POINT
#define EQSQRT(x)		eq_sqrt(x)
#define EQTAN(x)		eq_tan(x)
#define EQABS(x)		((x) >= 0 ? (x) : (-(x)))
#define EQPOW(x,y)		eq_pow10(y)
#define EQMULT(x,y)		((eqldata_t)(x)*(y))
#define EQMUL(x,y)		((eqdata_t)(((eqldata_t)(x)*(y))>>15))
#define EQMULL(x,y)		(((eqldata_t)(x)*(y))>>15)
#define EQRMULL(x,y)	((((eqldata_t)(x)*(y))+(1<<14))>>15)
#define EQRMUL(x,y)		((eqdata_t)((((eqldata_t)(x)*(y))+(1<<14))>>15))
#define EQDIV(x,y)		((x)/(y))

#define EQSHL(x, bits)	((x)<<(bits))
#define EQSHLL(x, bits) (((eqldata_t)(x))<<(bits))

#define EQSHRN(x, bits) ((eqdata_t)((x)>>(bits)))
#define EQRSHRN(x, bits) ((eqdata_t)((((x)+(1<<((bits)-1))))>>(bits)))

#define EQSHR(x, bits)  ((eqdata_t)((x)>>(bits)))
#define EQRSHR(x, bits) ((((x)+(1<<((bits)-1))))>>(bits))

#define EQLNDIV(x,y)	EQDIV(EQSHLL(x,15),y)

#define EQSATN(x)		((eqdata_t)((x)>32767 ? 32767 : ((x)<-32768 ? -32768 : (x))))
/*
	auxiliary functions
	these function maybe exists in the common modules for optimisation,
	in that case, you should remove these implementation to get the lower RO memory consumption
	we present them here just for the independent equalizer module
*/
/* sqrt function, Input Q15, Output Q15 */
/* count the leading zeros */
static int WebRtcSpl_NormU32(unsigned int a)
{
    int zeros;

    if (a == 0)
    {
        return 0;
    }

    if (!(0xFFFF0000 & a))
    {
        zeros = 16;
    }
    else
    {
        zeros = 0;
    }
    if (!(0xFF000000 & (a << zeros)))
    {
        zeros += 8;
    }
    if (!(0xF0000000 & (a << zeros)))
    {
        zeros += 4;
    }
    if (!(0xC0000000 & (a << zeros)))
    {
        zeros += 2;
    }
    if (!(0x80000000 & (a << zeros)))
    {
        zeros += 1;
    }

    return zeros;
}

static int eq_sqrt(unsigned int x)
{
    static int sqrttable[3] = {QCONST32(0.5, 15), QCONST32(-1. / 8, 15), QCONST32(3. / 48, 15)};
    long long r;
    int zeros, x1, x2, x3, xt;
    if (x == 0)
    {
        return 0;
    }
    zeros = WebRtcSpl_NormU32(x);
    x <<= zeros;
    x1 = EQRSHR(x & 0x7fffffff, 16);	//Q15
    if (x1 & 0x4000)
    {
        /* force the x1 in [-.5 .5] to speed up the convergence */
        x1 = (x1 - 0x8000) >> 1;
        zeros -= 1;
    }
    x2 = EQRMUL(x1, x1);
    x3 = EQRMUL(x1, x2);
    r = EQSHLL(QCONST32(1, 15), 15) + EQMULT(sqrttable[0], x1) + EQMULT(sqrttable[1], x2) + EQMULT(sqrttable[2], x3);
    xt = EQRSHRN(r, 15);
    zeros = 16 - zeros;
    if (zeros & 1)
    {
        xt = EQRMUL(QCONST32(1.41421356, 15), xt);
    }
    zeros >>= 1;
    if (zeros > 0)
    {
        xt = EQSHL(xt, zeros);
    }
    else if (zeros < 0)
    {
        xt = EQRSHR(xt, -zeros);
    }
    return xt;


}
/* exp function, Input Q15, Output Q15 */
static int eq_exp(int value)
{
    static int table[6] = {QCONST32(1, 15), QCONST32(1 / 2., 15), QCONST32(1 / 6., 15), QCONST32(1 / 24., 15), QCONST32(1 / 120., 15), QCONST32(1 / 720., 15)}; //Q15
    int tmp;
    int sum = QCONST32(1, 15);
    int i, shift, sign;
    if (value > 363408)
    {
        return 0x7fffffff;
    }
    else if (value < -363408)
    {
        return 1;
    }

    shift = 0;
    sign = 1;
    if (value < 0)
    {
        value = -value;
        sign = -1;
    }
    if (value > 16384)
    {
        while (!(value & 0x20000000))
        {
            value <<= 1;
            shift += 1;
        }
        value >>= 16;
        shift = 16 - shift;

    }

    value = sign * value;

    tmp = QCONST32(1, 15);
    for (i = 0; i < 6; i++)
    {
        tmp = EQRMUL(tmp, value);
        sum = sum + EQRMUL(tmp, table[i]);
    }


    for (i = 0; i < shift; i++)
    {
        sum = EQRMUL(sum, sum);
    }

    return sum;
}
static int eq_pow10(int x)
{
    int ln10 = 75451; // ln(10) in Q15
    x = EQRMUL(x, ln10);
    x = eq_exp(x);
    return x;
}


static eqdata_t eq_cos(eqdata_t x)
{
    static int cosTable[5] = {QCONST32(1, 18), QCONST32(-1 / 2., 18), QCONST32(1 / 24., 18), QCONST32(-1 / 720., 18), QCONST32(1 / (720.*8 * 7), 18)};
    eqdata_t x2, x4, x6, x8;
    eqldata_t r;
    /* we are sure that x is in [0, pi/2]*/
    x2 = EQRMUL(x, x);
    x4 = EQRMUL(x2, x2);
    x6 = EQRMUL(x2, x4);
    x8 = EQRMUL(x4, x4);
    r = EQSHLL(cosTable[0], 15) + EQMULT(cosTable[1], x2) + EQMULT(cosTable[2], x4) + EQMULT(cosTable[3], x6) + EQMULT(cosTable[4], x8);
    r = EQRSHR(r, 18);
    x = EQSATN(r);
    return x;

}

static eqdata_t eq_tan(eqdata_t in)//0 - pi/4 , will be better. 0 - pi/2
{
    eqdata_t tan_table[16] = {0, 32768, 0, 10923,
                              0, 4369,  0, 1768,
                              0, 717,   0, 291,
                              0, 118,   0, 48
                             };
    eqldata_t in1 =  in;
    eqldata_t in3 = (in1 * in1 * in1) >> 30;
    eqldata_t in4 = (in3 * in1) >> 15;
    eqldata_t in5 = (in3 * in1 * in1) >> 30;
    eqldata_t in7 = (in5 * in1 * in1) >> 30;
    eqldata_t in9 = (in7 * in1 * in1) >> 30;
    eqldata_t in11 = (in9 * in1 * in1) >> 30;
    eqldata_t in13 = (in11 * in1 * in1) >> 30;
    eqldata_t in15 = (in13 * in1 * in1) >> 30;

    eqdata_t r = tan_table[1] * in1 +  tan_table[3] * in3 +  tan_table[5] * in5
                 +  tan_table[7] * in7 +  tan_table[9] * in9 +  tan_table[11] * in11
                 +  tan_table[13] * in13 +  tan_table[15] * in15;

    r >>= 15;
    //r = (((r)>32767 ? 32767 : ((r)<-32768 ? -32768 : (r))));
    return r;
}
#if 0
static int eq_tan(eqdata_t x)
{
    eqdata_t xcos, xsin;
    /* we are very confident that x is in [0,pi/2], so it is not necessary to make branchs */
    xcos = eq_cos(x);
    xsin = eq_cos(PI / 2 - x);
    return EQDIV(EQSHLL(xsin, 15), xcos);
}
#endif
#else
#define EQSQRT(x)	sqrt(x)
#define EQTAN(x)	tan(x)
#define EQABS(x)	fabs(x)
#define EQPOW(x,y)	pow(x,y)
#define EQMULT(x,y) ((x)*(y))
#define EQMUL(x,y) ((x)*(y))
#define EQDIV(x,y)	((x)/(y))
#define EQLNDIV(x,y)	((x)/(y))
#define EQSHLL(x, bits) (x)
#define EQRSHR(x, bits) (x)
#define EQSHRN(x, bits) (x)
#define EQRSHRN(x, bits) (x)
#define EQRMUL(x,y)	EQMUL(x,y)
#define EQMULL(x,y)		EQMUL(x,y)
#define EQRMULL(x,y)	EQMUL(x,y)
#define EQSHL(x,bits)	(x)
#endif
/*
	function to get the peak filter coeffs
	Algorithms: Table 5.4 && Table 5.3 in Udo Zolzer, "Digital Audio Signal Processing" 2nd Edition. Aug,2008, John Wiley & Sons, Ltd
*/
static void peakf(eq_core_prms_t *prms, int sampling_rate, eqcoeff_t *coeff)
{
    float V0, K, K2, den, recpQK, V0recpQK;
    float tmp = 0;

    V0 = pow(10, (((fabs(prms->G))) / (20)));
    K = tan((((((3.1415926)) * (prms->fc))) / (sampling_rate)));
    K2 = ((K) * (K));
    tmp = (float)K;



    recpQK = (float)(tmp / (prms->Q));
    V0recpQK = ((recpQK) * (V0));
    if (prms->fc >= sampling_rate / 2)
    {
        coeff->b[0] = 32768;
        coeff->a[0] = 32768;
        coeff->a[1] = coeff->a[2] = coeff->b[1] = coeff->b[2] = 0;
        return;
    }
    if (prms->G >= 0)
    {
        den = (1) + recpQK + K2;
        coeff->b[0] = QCONST32((((1) + V0recpQK + K2) / (den)), 15);
        coeff->b[1] = QCONST32(((((2) * (K2 - (1)))) / (den)), 15);
        coeff->b[2] = QCONST32((((1) - V0recpQK + K2) / (den)), 15);
        coeff->a[0] = QCONST32((1), 15);
        coeff->a[1] = QCONST32(((((2) * (K2 - (1)))) / (den)), 15);
        coeff->a[2] = QCONST32((((1) - recpQK + K2) / (den)), 15);
    }
    else
    {
        den = (1) + V0recpQK + K2;
        coeff->b[0] = QCONST32((((1) + recpQK + K2) / (den)), 15);
        coeff->b[1] = QCONST32(((((2) * (K2 - (1)))) / (den)), 15);
        coeff->b[2] = QCONST32((((1) - recpQK + K2) / (den)), 15);
        coeff->a[0] = QCONST32((1), 15);
        coeff->a[1] = QCONST32(((((2) * (K2 - (1)))) / (den)), 15);
        coeff->a[2] = QCONST32((((1) - V0recpQK + K2) / (den)), 15);
    }
}

/*
	function to get the shelving filter coeffs
*/
static void shelving(eq_core_prms_t *prms, int sampling_rate, eqcoeff_t *coeff)
{
    float V0, K, K2, den, sqrt2, V0K2, sqrtQV0, sqrtQV0K, sqrtQK;
    V0 = pow(10, (((fabs(prms->G))) / (20)));
    K = tan((((((3.1415926)) * (prms->fc))) / (sampling_rate)));
    K2 = ((K) * (K));
    V0K2 = ((V0) * (K2));
    sqrt2 = sqrt((2));
    sqrtQV0 = sqrt(((2) * (V0)));
    sqrtQV0K = ((sqrtQV0) * (K));
    sqrtQK = ((sqrt2) * (K));
    if (prms->type == LOWPASS_SHELVING)
    {
        if (prms->G >= 0)
        {
            den = (1) + sqrtQK + K2;
            coeff->b[0] = QCONST32((((1) + sqrtQV0K + V0K2) / (den)), 15);
            coeff->b[1] = QCONST32(((((2) * (V0K2 - (1)))) / (den)), 15);
            coeff->b[2] = QCONST32((((1) - sqrtQV0K + V0K2) / (den)), 15);
            coeff->a[0] = QCONST32((1), 15);
            coeff->a[1] = QCONST32(((((2) * (K2 - (1)))) / (den)), 15);
            coeff->a[2] = QCONST32((((1) - sqrtQK + K2) / (den)), 15);
        }
        else
        {
            den = (1) + sqrtQV0K + V0K2;
            coeff->b[0] = QCONST32((((1) + sqrtQK + K2) / (den)), 15);
            coeff->b[1] = QCONST32(((((2) * (K2 - (1)))) / (den)), 15);
            coeff->b[2] = QCONST32((((1) - sqrtQK + K2) / (den)), 15);
            coeff->a[0] = QCONST32((1), 15);
            coeff->a[1] = QCONST32(((((2) * (V0K2 - (1)))) / (den)), 15);
            coeff->a[2] = QCONST32((((1) - sqrtQV0K + V0K2) / (den)), 15);

        }
    }
    else
    {
        if (prms->G >= 0)
        {
            den = (1) + sqrtQK + K2;
            coeff->b[0] = QCONST32(((V0 + sqrtQV0K + K2) / (den)), 15);
            coeff->b[1] = QCONST32(((((2) * (K2 - V0))) / (den)), 15);
            coeff->b[2] = QCONST32(((V0 - sqrtQV0K + K2) / (den)), 15);
            coeff->a[0] = QCONST32((1), 15);
            coeff->a[1] = QCONST32(((((2) * (K2 - (1)))) / (den)), 15);
            coeff->a[2] = QCONST32((((1) - sqrtQK + K2) / (den)), 15);
        }
        else
        {
            den = V0 + sqrtQV0K + K2;
            coeff->b[0] = QCONST32((((1) + sqrtQK + K2) / (den)), 15);
            coeff->b[1] = QCONST32(((((2) * (K2 - (1)))) / (den)), 15);
            coeff->b[2] = QCONST32((((1) - sqrtQK + K2) / (den)), 15);

            sqrtQV0K = ((sqrt((((2)) / (V0)))) * (K));

            V0K2 = ((K2) / (V0));
            den = (1) + sqrtQV0K + V0K2;
            coeff->a[0] = QCONST32((1), 15);
            coeff->a[1] = QCONST32(((((2) * (V0K2 - (1)))) / (den)), 15);
            coeff->a[2] = QCONST32((((1) - sqrtQV0K + V0K2) / (den)), 15);

        }
    }
}

static void l_h_pass(eq_core_prms_t *prms, int sampling_rate, eqcoeff_t *coeff)
{
    float K, den, sqrt2, K2;
    K = tan(3.1415926 * prms->fc / sampling_rate);
    K2 = K * K;
    sqrt2 = sqrt(2);
    den = 1 + sqrt2 * K + K2;

    switch (prms->type)
    {
        case LOWPASS:
            coeff->b[0] = QCONST32((K * K / den), 15);
            coeff->b[1] = QCONST32((2 * K * K / den), 15);
            coeff->b[2] = QCONST32((K * K / den), 15);
            coeff->a[0] = QCONST32((1), 15);
            coeff->a[1] = QCONST32((2 * (K * K - 1) / den), 15);
            coeff->a[2] = QCONST32(((1 - sqrt(2) * K + K * K) / den), 15);
            break;
        case HIGHPASS:
            coeff->b[0] = QCONST32((1 / den), 15);
            coeff->b[1] = QCONST32((-2 / den), 15);
            coeff->b[2] = QCONST32((1 / den), 15);
            coeff->a[0] = QCONST32((1), 15);
            coeff->a[1] = QCONST32((2 * (K * K - 1) / den), 15);
            coeff->a[2] = QCONST32(((1 - sqrt(2) * K + K * K) / den), 15);
            break;
        default:
            printf("error case\n");
            break;
    }

}


static void destroy_internal_memory(equalizer_t *equalizer)
{
    if (equalizer != NULL)
    {
        if (equalizer->coeff != NULL)
        {
            free(equalizer->coeff);
            equalizer->coeff = NULL;
        }
        if (equalizer->dline != NULL)
        {
            free(equalizer->dline);
            equalizer->dline = NULL;
        }
    }
}
/*
	function eq_create
description:
	use this function to create the equalizer object
prms:
	eq_prms_t: [in], desired frequency response array
returns:
	the equalizer handle
*/
void *eq_create(eq_prms_t *prms)
{
    equalizer_t *equalizer = (equalizer_t *)calloc(1, sizeof(equalizer_t));
    eq_core_prms_t *core_prms = NULL;
    if (equalizer != NULL && prms->biq_num > 0)
    {
        int i, ret = 0;
        equalizer->N = prms->biq_num;
        equalizer->Chan = prms->chan;
        equalizer->coeff = (eqcoeff_t *)calloc(equalizer->N, sizeof(eqcoeff_t));
        equalizer->dline = (eqdline_t *)calloc(equalizer->Chan, equalizer->N * sizeof(eqdline_t));
        if (equalizer->coeff == NULL || equalizer->dline == NULL)
        {
            destroy_internal_memory(equalizer);
            free(equalizer);
            equalizer = NULL;
        }
        if (equalizer != NULL)
        {
            if (prms->core_prms != NULL)
            {
                core_prms = (eq_core_prms_t *)(prms->core_prms);
            }
            for (i = 0; i < prms->biq_num; i++)
            {
                if (core_prms[i].type == BANDPASS_PEAK)
                {
                    peakf(&core_prms[i], prms->sampling_rate, &equalizer->coeff[i]);
                }
                else if (core_prms[i].type == LOWPASS_SHELVING || core_prms[i].type == HIHPASS_SHELVING)
                {
                    shelving(&core_prms[i], prms->sampling_rate, &equalizer->coeff[i]);
                }
                else if (core_prms[i].type == LOWPASS || core_prms[i].type == HIGHPASS)
                {
                    l_h_pass(&core_prms[i], prms->sampling_rate, &equalizer->coeff[i]);
                }
                else
                {
                    ret = - 1;
                }

            }
            if (ret == -1)
            {
                destroy_internal_memory(equalizer);
                equalizer = NULL;
            }
        }

    }
    return equalizer;
}
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
void eq_process(void *handle, short *x, int len)
{
    equalizer_t *equalizer = (equalizer_t *)handle;
    eqldata_t yj;
    eqldata_t wn;
    eqcoeff_t *pcoeff;
    eqdline_t *pdline;
    int i, j, c;
    int chan = equalizer->Chan;
    for (i = 0; i < len; i++)
    {
        for (c = 0; c < chan; c++)
        {
            pcoeff = equalizer->coeff;
            pdline = &equalizer->dline[c * equalizer->N];
            yj = EQSHLL(x[i * chan + c], 15);
            /* cascade biquad loop */
            for (j = 0; j < equalizer->N; j++)
            {
                /* we use the double word length in the biquad loop and the delay line memory, other wise the computation will suffer from the infinite word length
                   optimisation should be implementated in assembly,
                */
                wn = yj - EQRMULL(pcoeff->a[1], pdline->dline[0]) - EQRMULL(pcoeff->a[2], pdline->dline[1]);

                yj = EQRMULL(pcoeff->b[0], wn) + EQRMULL(pcoeff->b[1], pdline->dline[0]) + EQRMULL(pcoeff->b[2], pdline->dline[1]);
                pdline->dline[1] = pdline->dline[0];
                pdline->dline[0] = wn;
                pcoeff++;
                pdline++;
            }

            x[i * chan + c] = (short)EQ_SATURATE((EQRSHR(yj, 15)), 32767);
        }
    }

}
/*
	function eq_destroy
description:
	use this function to destroy the equalizer object
prms:
	handle:[in], equalizer handle
returns:
	none
*/
void eq_destroy(void *handle)
{
    equalizer_t *equalizer = (equalizer_t *)handle;
    destroy_internal_memory(equalizer);
    if (equalizer != NULL)
    {
        free(equalizer);
        equalizer = NULL;
    }

}
