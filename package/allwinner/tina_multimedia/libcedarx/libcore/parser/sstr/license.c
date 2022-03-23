#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libxml/tree.h"
#include "libxml/parser.h"
#include "libxml/xpath.h"
#include "libxml/xpathInternals.h"
#include <drmpath.h>
#include "awPlayReadyLicense.h"

#define ManifestPath  PLAYREADY_TMPFILE_DIR "Manifest.xml"
#define WRMHeaderPath PLAYREADY_TMPFILE_DIR "sstr.xml"

static const char *codes =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const unsigned char map[256] = {
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 253, 255,
255, 253, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 253, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
 52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,
255, 254, 255, 255, 255,   0,   1,   2,   3,   4,   5,   6,
  7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,
 19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,
 37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
 49,  50,  51, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255 };

static int base64_encode(const unsigned char *in,  unsigned long len,
                        unsigned char *out)
{
   unsigned long i, len2, leven;
   unsigned char *p;

   len2 = 4 * ((len + 2) / 3);
   p = out;
   leven = 3*(len / 3);
   for (i = 0; i < leven; i += 3) {
       *p++ = codes[in[0] >> 2];
       *p++ = codes[((in[0] & 3) << 4) + (in[1] >> 4)];
       *p++ = codes[((in[1] & 0xf) << 2) + (in[2] >> 6)];
       *p++ = codes[in[2] & 0x3f];
       in += 3;
   }

   if (i < len) {
       unsigned a = in[0];
       unsigned b = (i+1 < len) ? in[1] : 0;
       unsigned c = 0;

       *p++ = codes[a >> 2];
       *p++ = codes[((a & 3) << 4) + (b >> 4)];
       *p++ = (i+1 < len) ? codes[((b & 0xf) << 2) + (c >> 6)] : '=';
       *p++ = '=';
   }

   *p = '\0';

   return p - out;
}

static int base64_decode(const unsigned char *in, unsigned char *out)
{
    unsigned long t, x, y, z;
    unsigned char c;
    int g = 3;

    for (x = y = z = t = 0; in[x]!=0;) {
        c = map[in[x++]];
        if (c == 255) return -1;
        if (c == 253) continue;
        if (c == 254) { c = 0; g--; }
        t = (t<<6)|c;
        if (++y == 4) {
            out[z++] = (unsigned char)((t>>16)&255);
            if (g > 1) out[z++] = (unsigned char)((t>>8)&255);
            if (g > 2) out[z++] = (unsigned char)(t&255);
            y = t = 0;
        }
    }

    return z;
}

int sstrdoLicense(char* manifest)
{
    // 1. get base64-encoded wrmheader from network
    long i,j;
    int success = -1;
    FILE *fp = NULL;

    char *ProtectionHeader = manifest;

    // 2. base64decode wrmheader
    long len = strlen((char*)ProtectionHeader);
    long str_len;

    if(strstr((char*)ProtectionHeader,"=="))
        str_len = len/4*3-2;
    else if(strstr((char*)ProtectionHeader,"="))
        str_len = len/4*3-1;
    else
        str_len = len/4;

    unsigned char* Header = malloc(str_len+1);
    success = base64_decode(ProtectionHeader, Header);

    if(success <= 0){
        printf("base64 decode fail\n");
        free(Header);
        goto ErrorExit;
    }
    //drop all '\0',so we can get the real Header
    for(i=j=0;i< str_len;i++){
        if(Header[i] != '\0'){
            Header[j++] = Header[i];
        }
    }
    Header[j] = '\0';
    //printf("%s\n",Header);

    char * WRMHeader = strstr((char*)Header, "<WRMHEADER");
    //printf("WRMHeader=%s\n",WRMHeader);

    //save WRMHeader data in sstr.xml
    fp = fopen(WRMHeaderPath, "r" );
    if(fp){
        fclose(fp);
        fp = NULL;
        remove(WRMHeaderPath);
    }

    fp = fopen(WRMHeaderPath, "wb");
    if (fp){
        fwrite(WRMHeader, 1, strlen(WRMHeader), fp);
        fclose(fp);
    }

    // 3. ack and process license using wrmheader.
    if (WRMHeader != NULL) {
        //get license acquire URL
        char* LA_URL = strstr(WRMHeader, "<LA_URL>");
        LA_URL += 8;
        char* LA_URL_END = strstr(WRMHeader, "</LA_URL>");
        LA_URL_END[0] = '\0';
        //printf("LA_URL=%s\n",LA_URL);
        success = -1;
        success = doLicensing(LA_URL);
    }
    free(Header);
    // 4. show text in screen.
    if (!success){
        printf("license succeed.\n");
        return 0;
    }

ErrorExit:
    printf("license failed.\n");
    return -1;
}
