#ifndef IEC61937_H
#define IEC61937_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * IEC61937 API
 */

typedef struct headbpcuv{
	unsigned other:3;
    unsigned V:1;
    unsigned U:1;
    unsigned C:1;
    unsigned P:1;
    unsigned B:1;
} headbpcuv;

typedef union word
{
	struct
	{
		unsigned int bit_0:1;
		unsigned int bit_1:1;
		unsigned int bit_2:1;
		unsigned int bit_3:1;
		unsigned int bit_4:1;
		unsigned int bit_5:1;
		unsigned int bit_6:1;
		unsigned int bit_7:1;
		unsigned int bit_8:1;
		unsigned int bit_9:1;
		unsigned int bit_10:1;
		unsigned int bit_11:1;
		unsigned int bit_12:1;
		unsigned int bit_13:1;
		unsigned int bit_14:1;
		unsigned int bit_15:1;
		unsigned int rsvd:16;
	}bits;
	unsigned int wval;
}word;

int add61937Head(void *out,void * temp, int samples);

#ifdef __cplusplus
}
#endif


#endif
