#include<stdio.h>
#include<string.h>
#include<stdint.h>
#include<assert.h>

#include "do_audioresample.h"

int main(){
    AudioMix *AMX = NULL;
    init_audioresample(&AMX);

	FILE *fp = fopen("/tmp/in-16k.pcm","rwb+");
	FILE *fp_out = fopen("/tmp/out-48k.pcm","wb+");
	assert(fp != NULL);
	assert(fp_out != NULL);
	char buf[64*1024];
	char outbuf[64*1024];
	int reads = 0;
	int frame = 2;
	int samples = 4800;
	int offset = 0;
	while(1){
		bzero(buf, sizeof(buf));
		reads = fread(buf, frame, samples, fp);
		printf("get bytes: %d\n", reads);
		if(reads == 0) break;

		bzero(outbuf, sizeof(outbuf));
		int outbuflen;
		do_audioresample(AMX, 16000, 1, buf, reads*2, 48000, outbuf, &outbuflen);
		printf("do_audioresample outbuflen: %d\n", outbuflen);
		fwrite(outbuf, 1, outbuflen, fp_out);
		if(reads < samples) break;
	}
	fclose(fp);
	fclose(fp_out);
    destroy_audioresample(&AMX);
	printf("end!!!!!\n");
}
