#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eq_mid.h"
#include "tsound_ctrl.h"
#define READ_MIN_SIZE   (1024)
void usage()
{
    eqlog("*****************************");
    eqlog("* ./testeq -i input -o output -r samplerate -n bin_num");
}
int main(int argvs, char** argv)
{
    char buffer[READ_MIN_SIZE] = {0};
    int samplerate_tmp = 44100, chan_tmp = 1;
    int items, idx=0, ret = 0;
    int temp = 0;
    void *sndHld = NULL;
	static FILE* in = NULL;
	static FILE* out = NULL;
	static FILE* out_i = NULL;

    eqlog("How many argvs : %d", argvs);
    if(argvs <= 1)
    {
        usage();
        return -1;
    }
    while(idx < argvs)
    {
        if(!strcmp(argv[idx], "-i"))
        {
            idx++;
            eqlog("in : %s", argv[idx]);
			in = fopen(argv[idx], "rb");
            if(!in)
            {
                eqlog("Invaild input file!");
                return -1;
            }
        }
        if(!strcmp(argv[idx], "-o"))
        {
            idx++;
            eqlog("out : %s", argv[idx]);
			out = fopen(argv[idx], "wb");
            if(!out)
            {
                eqlog("Invaild Output file!");
                return -1;
            }
        }
		if(!strcmp(argv[idx], "-s"))
        {
            idx++;
            eqlog("out_i : %s", argv[idx]);
			out_i = fopen(argv[idx], "wb");
            if(!out_i)
            {
                eqlog("Invaild Output file!");
                return -1;
            }
        }
        if(!strcmp(argv[idx], "-r"))
        {
            idx++;
            samplerate_tmp = atoi(argv[idx]);
            eqlog("samplerate_tmp : %d", samplerate_tmp);
        }
        if(!strcmp(argv[idx], "-c"))
        {
            idx++;
            chan_tmp = atoi(argv[idx]);
            eqlog("chan_tmp : %d", chan_tmp);
        }
        idx++;
    }
	eq_init(samplerate_tmp, chan_tmp);
    sndHld = TSoundDeviceCreate(NULL, NULL, samplerate_tmp, chan_tmp);
    if (NULL == sndHld)
    {
        eqlog("TSoundDeviceCreate fail...\n");
        return -1;
    }
    ret = TSoundDeviceStart(sndHld);
    if (ret < 0)
    {
        eqlog("TSoundDeviceStart fail!");
        return -1;
    }
    items = fread(buffer, chan_tmp*sizeof(short), READ_MIN_SIZE/(chan_tmp*sizeof(short)), in);
    while(items > 0)
    {
        if (out_i)
        {
        	fwrite(buffer, chan_tmp*sizeof(short), items, out_i);
        }
		eq_mid_proccess(buffer, items*sizeof(short)*chan_tmp);
        if (items > 0)
        {
            TSoundDeviceWrite(sndHld, buffer, items*sizeof(short)*chan_tmp);
        }
        if (out)
        {
        	items = fwrite(buffer, chan_tmp*sizeof(short), items, out);
        }
        items = fread(buffer, chan_tmp*sizeof(short), READ_MIN_SIZE/(chan_tmp*sizeof(short)), in);
    }
    TSoundDeviceDestroy(sndHld);
    eq_mid_destroy();
    fclose(in);
    if (out_i)
    {
        fclose(out_i);
    }
    if (out)
    {
        fclose(out);
    }
    return 0;
}
