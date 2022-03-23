#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ms_common.h"

#define RIFF_SIGN_ID 0x46464952ul
#define WAVE_SIGN_ID 0x45564157ul
#define FMT__SIGN_ID 0x20746D66ul
#define FACT_SIGN_ID 0x74636166ul
#define DATA_SIGN_ID 0x61746164ul




 #define MAX_CAT_FILE_NUM 17



struct RIFF_HEADER
{
    int RiffID;
    int RiffSize;
    int RiffFormat;
};

struct WAVE_FORMAT
{
    unsigned short  FormatTag;
    unsigned short  Channels;
    unsigned int SamplesPerSec;
    unsigned int AvgBytesPerSec;
    unsigned short  BlockAlign;
    unsigned short  BitsPerSample;
    unsigned short  otherInfo;
};

struct FMT_BLOCK
{
    int       FmtID;
    int       FmtSize;
    struct WAVE_FORMAT wavFormat;
};

struct UNKNOW_BLOCK
{
    int ID;
    int Size;
};

struct FACT_BLOCK
{
    int FactID;
    int FactSize;
    unsigned char  Data[1];
};

struct DATA_BLOCK
{
    int DataID;
    int DataSize;
    unsigned char  Data[1];
};


unsigned char * openWaveFile(const char *name);
unsigned char * getWaveData(unsigned char * wav, int * dLen);
void   printWaveFormat(unsigned char * wav);
int    saveWaveFile(const char * name, unsigned char * wav);
unsigned char * catmultiWave(int cnt,char *wav1, char *wav2, ...);
unsigned char * catWave(unsigned char *wav1, unsigned char *wav2);
size_t getTotalLen(unsigned char * wav);
 #if 0
int main(int argc, char* argv[])
{
    int dLen;
MS_TRACE("int=%d,%d,%d,%d\r\n",sizeof(int),sizeof(unsigned int),sizeof(unsigned short),sizeof(long));
    unsigned char * data1 = openWaveFile("1.wav");
    printWaveFormat(data1);
    unsigned char * data2 = openWaveFile("2.wav");
    printWaveFormat(data2);
    unsigned char * data3 = openWaveFile("3.wav");
    printWaveFormat(data3);
    unsigned char * data4 = openWaveFile("4.wav");
    printWaveFormat(data4);
    unsigned char * data5 = openWaveFile("5.wav");
    printWaveFormat(data5);
    unsigned char * data6 = openWaveFile("6.wav");
    printWaveFormat(data6);
    unsigned char * data7 = openWaveFile("7.wav");
    printWaveFormat(data7);
    unsigned char * data8 = openWaveFile("8.wav");
    printWaveFormat(data8);
  ///  unsigned char * data9 = openWaveFile("9.wav");
  //  printWaveFormat(data9);
   // MS_TRACE("data=%x,%x,%x,%x,%x,%x,%x,%x\r\n",data1,data2,data3,data4,data5,data6,data7,data8);
    data1 = catmultiWave(8,data1,data2,data3,data4,data5,data6,data7,data8);
    printWaveFormat(data1);
    saveWaveFile("10.wav", data1);
    return 0;
}
 #endif
 int catmp3File(int argc,char *path1,char *path2,...)
{
		 va_list argp;
		 int argno = 0;
		 int i =0;
		 int result= -1;


		char cmd[512];

		 unsigned char *path[MAX_CAT_FILE_NUM]={NULL};
		 int len;



		 if(path1!=NULL){
				path[argno]= path1;
				argno++;
		 }
		 if(path2!=NULL){
			path[argno]= path2;
			argno++;
		 }

		 va_start(argp, path2);
			 i =2;
		 while (i < argc)
		 {
			 path[argno] = (char*)va_arg(argp,char*);

			 if(path[argno] ==NULL){
				// MS_TRACE("null=%d,%d\r\n",argno,argc);
				// break;
			 }
			 else{
				 argno++;
				 if(argno >= MAX_CAT_FILE_NUM){
					 break;
				 }
			 }
			 i++;
		 }
		 va_end(argp);
		 memset(cmd,0,512);

		 sprintf(cmd, "cat %s %s ", path[0],path[1]);
		 len = strlen(cmd);
		 i = 2;

		 while(i <argno)
		 {
		  sprintf(&cmd[len], "%s ", path[i]);

		  len = strlen(cmd);
		  i++;
		 }
		 sprintf(&cmd[len], " >/tmp/asr.mp3");

         system(cmd);
			 //save file


		return result;
}
int catWavFile(int argc,char *path1,char *path2,...)
{
		 va_list argp;
		 int argno = 0;
		 int i =0;
		 int result= -1;
		 size_t totallen =0;
		 unsigned char *wav2;
		 unsigned char *wav1;
		 unsigned char *data[MAX_CAT_FILE_NUM]={NULL};
		 size_t len[MAX_CAT_FILE_NUM]={0};
		 unsigned char *wave[MAX_CAT_FILE_NUM]={NULL};
		 unsigned char *path[MAX_CAT_FILE_NUM]={NULL};
		 size_t len1;



		 if(path1!=NULL){

			 wav1 = openWaveFile(path1);
			 if(wav1 !=NULL)
			 {
				path[argno]= path1;
				wave[argno]=wav1;
				data[argno]=getWaveData(wave[argno], &len[argno]);
				argno++;
			 }
		 }
		 if(path2!=NULL){
			 wav2 = openWaveFile(path2);
			 if(wav2 !=NULL)
			 {
				path[argno]= path2;
				wave[argno]=wav2;
				data[argno]=getWaveData(wave[argno], &len[argno]);

				argno++;

			 }
		 }
	//	 len1 = getTotalLen(wav1);
		// wave[argno] = wav2;
		// data[argno]=getWaveData(wave[argno], &len[argno]);

		// totallen = len[argno];
		// argno++;
		 va_start(argp, path2);
			 i =2;
		 while (i < argc)
		 {
			 path[argno] = (char*)va_arg(argp,char*);

			 if(path[argno] ==NULL){
				// MS_TRACE("null=%d,%d\r\n",argno,argc);
				// break;
			 }
			 else{
		//		 MS_TRACE("ff #%d is: %s\n", argno, path[argno]);
				 wave[argno] = openWaveFile(path[argno]);
				 if(wave[argno]==NULL){
					MS_TRACE("wav is empty=%d\r\n",argc);
					break;
				 }
				 data[argno]=getWaveData(wave[argno], &len[argno]);
					//	 totallen +=len[argno];
			//	 MS_TRACE("Parameter #%d is: %x\n", argno, wave[argno]);
				 argno++;
				 if(argno >= MAX_CAT_FILE_NUM){
					 break;
				 }
			 }
			 i++;
		 }
		 va_end(argp);

//		 if((argno+1) <argc)
//		 {
//		    MS_TRACE("param num err=%d\r\n",argc);
//			goto err;
//		 }
		 wav1 =wave[0];
		 len1 = getTotalLen(wav1);
		 for(i=1;i <argno;i++){
			totallen +=len[i];
		 }
		 unsigned char * nD = malloc(len1 + totallen + 10);
		 unsigned char * curData=NULL;
		 if(nD == NULL) {
			goto err;
		 }
		 memcpy(nD, wav1, len1);
		 free(wav1);
		 wav1 = nD;
		 wave[0]=wav1;
		 unsigned char * Data1 = getWaveData(wav1, &len1);
		 curData = Data1;

		 struct DATA_BLOCK * db1 = (struct DATA_BLOCK *)(Data1 - 8);
		 db1->DataSize += totallen;
		 curData += len[0];
		 for(i=1;i <argno;i++){
			memcpy(curData, data[i], len[i]);
			curData += len[i];
		 }

		//	 return wav1;

			 //save file
	FILE *fp = fopen("/tmp/asr.wav", "wb");
    if(fp == 0){
		MS_TRACE("asr.wav open err\r\n");
		goto err;
    }
    len1 = getTotalLen(wav1);
    struct RIFF_HEADER rh;
    rh.RiffFormat = WAVE_SIGN_ID;
    rh.RiffID     = RIFF_SIGN_ID;
    rh.RiffSize   = len1 + 4;
    fwrite(&rh, sizeof(rh), 1, fp);
    fwrite(wav1, 1, len1, fp);
    fclose(fp);
    result = 0;


    err:
		for(i=0;i <argno;i++){
			if(wave[i]){
				//MS_TRACE("i=%d,%x\r\n",i,wave[i]);
				free(wave[i]);
			}

	    }
       // free(wav1);
		return result;
}
unsigned char * openWaveFile(const char *name)
{
    size_t readByte;
    FILE * fp = fopen(name, "rb");
    if(fp==NULL) return NULL;

    struct RIFF_HEADER fh;
    if(fread(&fh, sizeof(fh), 1, fp) != 1)
    {
        fclose(fp);
        MS_TRACE("Riff Header¯¯\n");
        return NULL;
    }
    if(fh.RiffFormat != WAVE_SIGN_ID || fh.RiffID != RIFF_SIGN_ID)
    {
        fclose(fp);
        MS_TRACE("ID:%08X Format:%08X\n", fh.RiffID, fh.RiffFormat);
        return NULL;
    }
    unsigned char * r = malloc(fh.RiffSize + 10), *pr;
    if(r==NULL)
    {
        fclose(fp);
        MS_TRACE("ERR\n");
        return NULL;
    }
    readByte = fread(r, 1, fh.RiffSize-4, fp);
    if(readByte != fh.RiffSize-4)
    {
        free(r);
        fclose(fp);
        MS_TRACE("wave %d %d\n", readByte, fh.RiffSize);
        return NULL;
    }
    fclose(fp);

    struct FMT_BLOCK *fb = (struct FMT_BLOCK *)r;
    if(fb->FmtID != FMT__SIGN_ID)
    {
        MS_TRACE("ID:%08X\n", fb->FmtID);
        free(r);
        return NULL;
    }
    if(fb->wavFormat.FormatTag != 1)
    {
        free(r);
        MS_TRACE("Format:%d\n", fb->wavFormat.FormatTag);
        return NULL;
    }

    pr = r + 8 + fb->FmtSize;
    while(1)
    {
      struct  UNKNOW_BLOCK * ub = (struct UNKNOW_BLOCK *)pr;
        if(ub->ID == FACT_SIGN_ID)
        {
            MS_TRACE("Fact length: %d\n", ub->Size);
            pr += 8 + ub->Size ;
        }
        else break;
    }
   struct DATA_BLOCK * db = (struct DATA_BLOCK *)pr;
    if(db->DataID  != DATA_SIGN_ID)
    {
        free(r);
        MS_TRACE("\n");
        return NULL;
    }
    return r;
}

unsigned char * getWaveData(unsigned char * wav, int * dLen)
{
   struct UNKNOW_BLOCK * ub = (struct UNKNOW_BLOCK *)wav;
    while(ub->ID != DATA_SIGN_ID)
    {
        switch(ub->ID)
        {
        case DATA_SIGN_ID:
            break;
        case FMT__SIGN_ID:
        case FACT_SIGN_ID:
            ub = (struct UNKNOW_BLOCK *)(((unsigned char *)ub) + ub->Size + 8);
            break;
        default:
            MS_TRACE("%08X\n", ub->ID );
            return NULL;
        }
    }
    struct DATA_BLOCK * db = (struct DATA_BLOCK *)ub;
    *dLen = db->DataSize;
    return db->Data;
}

size_t getTotalLen(unsigned char * wav)
{
    size_t r = 0;
    struct UNKNOW_BLOCK * ub = (struct UNKNOW_BLOCK *)wav;
    while(1)
    {
        switch(ub->ID)
        {
        case DATA_SIGN_ID:
            r += ub->Size + 8;
            return r;
        case FMT__SIGN_ID:
        case FACT_SIGN_ID:
            r += ub->Size + 8;
            ub = (struct UNKNOW_BLOCK *)(((unsigned char *)ub) + ub->Size + 8);
            break;
        default:
            MS_TRACE(" %08X\n", ub->ID );
            return 0;
        }
    }
    return -1;
}




void printWaveFormat(unsigned char * wav)
{
    int len;
    getWaveData(wav, &len);
   struct FMT_BLOCK *fb = (struct FMT_BLOCK *)wav;

}

unsigned char * catWave(unsigned char *wav1, unsigned char *wav2)
{
    struct FMT_BLOCK * fb1 = (struct FMT_BLOCK *)wav2;
    const struct FMT_BLOCK * fb2 = (const struct FMT_BLOCK *)wav2;
    if(
        fb1->wavFormat.AvgBytesPerSec == fb2->wavFormat.AvgBytesPerSec &&
        fb1->wavFormat.BitsPerSample  == fb2->wavFormat.BitsPerSample  &&
        fb1->wavFormat.BlockAlign     == fb2->wavFormat.BlockAlign     &&
        fb1->wavFormat.Channels       == fb2->wavFormat.Channels       &&
        fb1->wavFormat.FormatTag      == fb2->wavFormat.FormatTag      &&
        fb1->wavFormat.SamplesPerSec  == fb2->wavFormat.SamplesPerSec)
    {
        int len1 = getTotalLen(wav1), len2;
        unsigned char * Data2 = getWaveData(wav2, &len2);
        unsigned char * nD = malloc(len1 + len2 + 10);
        if(nD == NULL) return NULL;
        memcpy(nD, wav1, len1);
        free(wav1);
        wav1 = nD;
        unsigned char * Data1 = getWaveData(wav1, &len1);
        struct DATA_BLOCK * db1 = (struct DATA_BLOCK *)(Data1 - 8);
        db1->DataSize += len2;
        memcpy(Data1 + len1, Data2, len2);
        return wav1;
    }
    return NULL;
}


unsigned char * catmultiWave(int cnt,char *wav1, char *wav2, ...)
{

	va_list argp;
	int argno = 0;
        int i =0;
        int totallen =0;
        char *data[MAX_CAT_FILE_NUM]={NULL};
        int len[MAX_CAT_FILE_NUM]={0};
	char *wave[MAX_CAT_FILE_NUM]={NULL};
	int len1 = getTotalLen(wav1);

        wave[argno] = wav2;
        data[argno]=getWaveData(wave[argno], &len[argno]);

        totallen = len[argno];
	argno++;
	va_start(argp, wav2);
        i =2;
	while (i < cnt)
	{
		wave[argno] = (char*)va_arg(argp,char*);
MS_TRACE("ff #%d is: %x\n", argno, wave[argno]);
		if(wave[argno] ==NULL){
			MS_TRACE("null=%d\r\n",argno);
			break;
		}
		data[argno]=getWaveData(wave[argno], &len[argno]);
                totallen +=len[argno];
		MS_TRACE("Parameter #%d is: %x\n", argno, wave[argno]);
		argno++;
                if(argno >= MAX_CAT_FILE_NUM){
			break;
		}
		i++;
	}
	va_end(argp);






        unsigned char * nD = malloc(len1 + totallen + 10);
        unsigned char * curData=NULL;
        if(nD == NULL) return NULL;
        memcpy(nD, wav1, len1);
        free(wav1);
        wav1 = nD;
        unsigned char * Data1 = getWaveData(wav1, &len1);
        curData = Data1;
        struct DATA_BLOCK * db1 = (struct DATA_BLOCK *)(Data1 - 8);
        db1->DataSize += totallen;
        curData += len1;
        for(i=0;i <argno;i++){
	   memcpy(curData, data[i], len[i]);
           curData += len[i];
	}

        return wav1;

    return NULL;
}
int saveWaveFile(const char * name, unsigned char * wav)
{
    FILE *fp = fopen(name, "wb");
    if(fp == 0) return 0;
    int len = getTotalLen(wav);
    struct RIFF_HEADER rh;
    rh.RiffFormat = WAVE_SIGN_ID;
    rh.RiffID     = RIFF_SIGN_ID;
    rh.RiffSize   = len + 4;
    fwrite(&rh, sizeof(rh), 1, fp);
    fwrite(wav, 1, len, fp);
    fclose(fp);
    return 1;
}
