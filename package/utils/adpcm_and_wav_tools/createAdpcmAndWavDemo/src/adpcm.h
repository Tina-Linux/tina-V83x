#ifndef __ADPCM_H
#define __ADPCM_H

typedef signed char     int8_t;
typedef short int         int16_t;
typedef int                int32_t;
typedef unsigned char      uint8_t;
typedef unsigned short int   uint16_t;

#ifdef  __cplusplus
extern "C" {
#endif
    uint8_t ADPCM_Encode(int32_t sample,int initFlag);
    int16_t ADPCM_Decode(uint8_t code,int initFlag);
#ifdef  __cplusplus
}
#endif

#endif /* __ADPCM_H*/
