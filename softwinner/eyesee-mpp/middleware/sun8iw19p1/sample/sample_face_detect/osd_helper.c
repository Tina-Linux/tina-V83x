
#include "osd_helper.h"

static int Not_In_Lib(char buff[FONTSIZE * FONTSIZE / 8])
{
	for (int i = 0; i < FONTSIZE * FONTSIZE / 8; i++)
	{
		if (buff[i])   //如果有一个不为0，表明buff已经被修改过，字库存在此字退出此函数
			return 0;
	}
	return 1;
}

static int Get_Asc_Code(unsigned char *Get_Input_Char, char buff[])
{
	unsigned long offset;
	FILE *ASC;
	/*打开字库文件asc16*/
	if ((ASC = fopen(ENGLISH_DOT_LIB, "rb+")) == NULL)
	{
		printf("Can't open asc, Please add it?");
		exit(0);
	}
	offset = *(Get_Input_Char) * 16 + 1;         /*通过ascii码算出偏移量*/
	fseek(ASC, offset, SEEK_SET);                /*将文件指针移动到偏移量的位置*/
	fread(buff, 16, 1, ASC);                     /*从偏移量的位置读取32个字节*/
	fclose(ASC);
	return 1;
}

static int Get_HzK_Code(unsigned char *Get_Input_Char, char buff[])
{
	int not_find = 0;
	unsigned char qh, wh;
	unsigned long offset;
	FILE *HZK;
	char file_name[] = CHINESE_DOT_LIB;
	if ((HZK = fopen(file_name, "rb+")) == NULL) /*打开字库文件hzk16*/
	{
		printf("Can't open %s,Please add it?\n", file_name);
		exit(0);
	}
	/*区码=内码(高字节)-160  位码=内码(低字节)-160*/
	qh     = *(Get_Input_Char) - 0xa0;           /*10进制的160等于16进制的A0*/
	wh     = *(Get_Input_Char + 1) - 0xa0;       /*获得区码与位码*/
	offset = (94 * (qh - 1) + (wh - 1)) * FONTSIZE * FONTSIZE / 8; /*计算该汉字在字库中偏移量*/
	not_find = fseek(HZK, offset, SEEK_SET); /*将文件指针移动到偏移量的位置*/
	if (not_find)
	{
		printf("未查到该区！error！！！\n");
		fclose(HZK);
		return 0;
	}
	fread(buff, FONTSIZE * FONTSIZE / 8, 1, HZK);    /*从偏移量的位置读取32个字节*/
	if (Not_In_Lib(buff))
	{
		fclose(HZK);
		printf("有未识别字符!\n");
		return 0;
	}
	//printf("qh:%d,wh:%d,offset:%ld\n\r", qh, wh, offset);
	fclose(HZK);
	return 1;
}

static void FillCode(unsigned char* pTextBuffer, int textlen, unsigned char* pBuffer_English, int pos, int font_size, int bEnglish)
{
	int offset = pos * font_size / 8 / 2;
	int stride = textlen * font_size / 8 / 2;
	int width  = (bEnglish == 1) ? font_size / 2 / 8 : font_size / 8;
	int sink   = (bEnglish == 1) ? 2 : 0;  //英文字符统一下移两行

	for (int row = 0; row < font_size - sink; row++)
	{
		for (int col = 0; col < width; col++)
		{
			*(pTextBuffer + offset + col + (row + sink) * stride) = *(pBuffer_English + col + row * width);
		}
	}
}

static void TransCode2RGB8888(unsigned char* pTextBuffer, unsigned char* pRGB8888, int width, int height)
{
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			for (int k = 0; k < 8; k++)
			{
				if (pTextBuffer[row * width + col] & (0x80 >> k))
				{
					*(pRGB8888 + row * width * 8 * 4 + col * 8 * 4 + k * 4 + 0) = 0;
					*(pRGB8888 + row * width * 8 * 4 + col * 8 * 4 + k * 4 + 1) = 0;
					*(pRGB8888 + row * width * 8 * 4 + col * 8 * 4 + k * 4 + 2) = 0;
					*(pRGB8888 + row * width * 8 * 4 + col * 8 * 4 + k * 4 + 3) = 255;
				}
				else
				{
					*(pRGB8888 + row * width * 8 * 4 + col * 8 * 4 + k * 4 + 0) = 255;
					*(pRGB8888 + row * width * 8 * 4 + col * 8 * 4 + k * 4 + 1) = 255;
					*(pRGB8888 + row * width * 8 * 4 + col * 8 * 4 + k * 4 + 2) = 255;
					*(pRGB8888 + row * width * 8 * 4 + col * 8 * 4 + k * 4 + 3) = 255;
				}
			}
		}
	}
}


void GenTextBuffer(char* szText, char* pTextBuffer, char* pRGB8888)
{
    char Buffer_English[FONTSIZE * FONTSIZE / 2 / 8] = {0};	//单个英文字符buffer
	char Buffer_Chinese[FONTSIZE * FONTSIZE / 8]     = {0};	//单个中文字符buffer
	unsigned char word[2];
    
	int count = strlen((char*)szText);
	memset(pTextBuffer, 0, FONTSIZE * FONTSIZE / 2 / 8 * count);
    
	int index = 0;
	while (index < count)
	{
		word[0] = szText[index];

		if (word[0] < 128)
		{// 英文字符
			Get_Asc_Code(&word[0], Buffer_English);
			FillCode((unsigned char*)pTextBuffer, count, (unsigned char*)Buffer_English, index, FONTSIZE, 1);
			index++;
			continue;
		}
		else
		{// 中文字符
			word[0] = szText[index + 0];
			word[1] = szText[index + 1];
			Get_HzK_Code(word, Buffer_Chinese);
			FillCode((unsigned char*)pTextBuffer, count, (unsigned char*)Buffer_Chinese, index, FONTSIZE, 0);
			index += 2;
			continue;
		}
	}
	TransCode2RGB8888((unsigned char*)pTextBuffer, (unsigned char*)pRGB8888, count, FONTSIZE);
}

void GenRectBuffer(char* pRGB8888, int region_w, int region_h, int x, int y, int w, int h, int len_w)
{
    // 左边框
    for (int row = 0; row < h; row++)
        for (int col = 0; col < len_w; col++)
        {
            *((unsigned char*)pRGB8888 + row * region_w * 4 + col * 4 + 0) = 0;     //B
            *((unsigned char*)pRGB8888 + row * region_w * 4 + col * 4 + 1) = 0;     //G
            *((unsigned char*)pRGB8888 + row * region_w * 4 + col * 4 + 2) = 255;   //R
            *((unsigned char*)pRGB8888 + row * region_w * 4 + col * 4 + 3) = 255;   //A
        }
        
    // 右边框
    for (int row = 0; row < h; row++)
        for (int col = w - 1 - len_w; col < w; col++)
        {
            *((unsigned char*)pRGB8888 + row * region_w * 4 + col * 4 + 0) = 0;     //B
            *((unsigned char*)pRGB8888 + row * region_w * 4 + col * 4 + 1) = 0;     //G
            *((unsigned char*)pRGB8888 + row * region_w * 4 + col * 4 + 2) = 255;   //R
            *((unsigned char*)pRGB8888 + row * region_w * 4 + col * 4 + 3) = 255;   //A
        }
        
    // 上边框
    for (int row = 0; row < len_w; row++)
        for (int col = 0; col < w; col++)
        {
            *((unsigned char*)pRGB8888 + row * region_w * 4 + col * 4 + 0) = 0;     //B
            *((unsigned char*)pRGB8888 + row * region_w * 4 + col * 4 + 1) = 0;     //G
            *((unsigned char*)pRGB8888 + row * region_w * 4 + col * 4 + 2) = 255;   //R
            *((unsigned char*)pRGB8888 + row * region_w * 4 + col * 4 + 3) = 255;   //A
        }
        
    // 下边框
    for (int row = h - 1 - len_w; row < h; row++)
        for (int col = 0; col < w; col++)
        {
            *((unsigned char*)pRGB8888 + row * region_w * 4 + col * 4 + 0) = 0;     //B
            *((unsigned char*)pRGB8888 + row * region_w * 4 + col * 4 + 1) = 0;     //G
            *((unsigned char*)pRGB8888 + row * region_w * 4 + col * 4 + 2) = 255;   //R
            *((unsigned char*)pRGB8888 + row * region_w * 4 + col * 4 + 3) = 255;   //A
        }
    return;
}

void DrawRect_Nv21(char* pNV21_y, char* pNV21_vu, int region_w, int region_h, int x, int y, int w, int h, int len_w)
{
    
    int width = region_w;
    int height = region_h;
    int linewidth = len_w;
    int sx = x;
    int sy = y;
    int ex = x + w;
    int ey = y + h;
    
	int i, j;
	unsigned char *pY  = (unsigned char *)pNV21_y;
	unsigned char *pVU = (unsigned char *)pNV21_vu;
	unsigned char *pSrc, *pSrc1;
	int halfline = (linewidth >> 1);
	int sty, endy, stx, endx;
    
    int colorR = 0, colorG = 0, colorB = 0;
    int colory = 255;   //GETY(colorR, colorG, colorB);
    int coloru = 0;     //GETU(colorR, colorG, colorB);
    int colorv = 255;   //GETV(colorR, colorG, colorB);
	
	//top line
	sty  = MAX(sy - halfline, 0);
	endy = MIN(sy + halfline, height - 1);
	for(i = sty; i <= endy; i++)
	{
		pSrc = (unsigned char *)(pVU + (i / 2)  * width);
		pSrc1 = (unsigned char *)(pY + i * width);
		for(j = sx; j <= ex; j ++)
		{
			pSrc1[j] = colory;
			pSrc[2*(j >> 1) + 0] = colorv;
			pSrc[2*(j >> 1) + 1] = coloru;
		}
	}

	//bottom line
	sty  = MAX(ey - halfline, 0);
	endy = MIN(ey + halfline, height - 1);
	for(i = sty; i <= endy; i++)
	{
		pSrc = (unsigned char *)(pVU + (i / 2)  * width);
		pSrc1 = (unsigned char *)(pY + i * width);
		for(j = sx; j <= ex; j ++)
		{
			pSrc1[j] = colory;
			pSrc[2*(j >> 1) + 0] = colorv;
			pSrc[2*(j >> 1) + 1] = coloru;
		}
	}

	//left line
	stx  = MAX(sx - halfline, 0);
	endx = MIN(sx + halfline, width - 1);
	sty  = MAX(sy - halfline, 0);
	endy = MIN(ey + halfline, height - 1);
	for(i = sty; i <= endy; i++)
	{
		pSrc = (unsigned char *)(pVU + (i / 2)  * width);
		pSrc1 = (unsigned char *)(pY + i * width);
		for(j = stx; j <= endx; j ++)
		{
			pSrc1[j] = colory;
			pSrc[2*(j >> 1) + 0] = colorv;
			pSrc[2*(j >> 1) + 1] = coloru;
		}
	}

	//right line
	stx  = MAX(ex - halfline, 0);
	endx = MIN(ex + halfline, width - 1);

	for(i = sty; i <= endy; i++)
	{
		pSrc = (unsigned char *)(pVU + (i / 2)  * width);
		pSrc1 = (unsigned char *)(pY + i * width);
		for(j = stx; j <= endx; j ++)
		{
			pSrc1[j] = colory;
			pSrc[2*(j >> 1) + 0] = colorv;
			pSrc[2*(j >> 1) + 1] = coloru;
		}
	}
}
