/*
** $Id: fontmakerRbf.c
** 
** fontmakerRbf.c: The Raw Bitmap Font operation set.
**
** Current maintainer: yangy.
**
** Create date: 2014/12/03
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "misc.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef _MGFONT_SXF

#include "devfont.h"
#include "charset.h"
#include "sunxi_font.h"
#include "fontname.h"

#define SXFONT_INFO_P(devfont) ((pSunxiFont)(devfont->data))
#define SXFONT_INFO_CHARINFO(devfont) (SXFONT_INFO_P(devfont)->dwCharInfo)
#define SXFONT_INFO_WIDTH(devfont) (GET_FONT_WIDTH(SXFONT_INFO_CHARINFO(devfont)))
#define SXFONT_INFO_HEIGHT(devfont) (SXFONT_INFO_P(devfont)->pHeader->YSize)

//#define db_msg() fprintf(stderr, "f:%s l:%d\n", __FUNCTION__, __LINE__)
extern int extract7Zip(char *fileName, unsigned char *fileBuffer, unsigned long *fileSize);

/********************** Load/Unload of raw bitmap font ***********************/
#if 0
static void* load_font_data (char* font_name, char* file_name, 
        char* real_font_name)
#endif

#define FONT_DATA_SIZE (1024*1024*3)	//pay attention to if the size is enough

void ReleaseSection(PFL_HEADER pHeader)
{
	if(pHeader->pSection != NULL)
	{
		free(pHeader->pSection);
		pHeader->pSection = NULL;
	}
}

void ExitFont(pSunxiFont sxFont)
{
	if('U' == sxFont->pHeader->magic[0])     //unicode 编码
		ReleaseSection(sxFont->pHeader);
}

void dump_para(PFL_HEADER pfl_header)
{
	
	fprintf(stderr, "Magic: %c %c %c %c\n", pfl_header->magic[0], pfl_header->magic[1], pfl_header->magic[2], pfl_header->magic[3]);
	fprintf(stderr, "nSection: %d\n", pfl_header->nSection);
	fprintf(stderr, "pSection: %p\n", pfl_header->pSection);
	fprintf(stderr, "Size: %ld\n", pfl_header->Size);
	fprintf(stderr, "YSize: %d\n", pfl_header->YSize);
}

int ReadFontSection(pSunxiFont sxFont)
{
	PFL_HEADER pfl_header = sxFont->pHeader;

	ReleaseSection(pfl_header);
	
	pfl_header->pSection = (FL_SECTION_INF *)calloc(1, pfl_header->nSection*sizeof(FL_SECTION_INF));
	if(pfl_header->pSection == NULL)
	{
		printf("Malloc fail!\n");
		return 0;
	}
	int size = pfl_header->nSection*sizeof(FL_SECTION_INF);
	if (sxFont->memFlag) {
		memcpy(pfl_header->pSection, sxFont->data+sizeof(FL_HEADER)-4, size);
	} else {
		fread(pfl_header->pSection, size, 1, sxFont->fp);
	}
	dump_para(pfl_header);
	return 1;
}

static int ReadFontHeader(pSunxiFont sxFont)
{
	int size = sizeof(FL_HEADER)-4;
	if (sxFont->memFlag) {
		memcpy(sxFont->pHeader, sxFont->data, size);
	} else {
		fread(sxFont->pHeader, size, 1, sxFont->fp);
	}
	//检测表示头
	if((sxFont->pHeader->magic[0] != 'U' && sxFont->pHeader->magic[0] != 'M')
		|| sxFont->pHeader->magic[1] != 'F' || sxFont->pHeader->magic[2] != 'L')
	{
		printf("Cann't support file format!\n");
		return 0;
	}

	if('U' == sxFont->pHeader->magic[0])     //unicode 编码
	{
		return ReadFontSection(sxFont);
	}

	return 1;	
}

static int OpenFontFile(char *pFontFile, pSunxiFont sxFont)
{
	if((sxFont->fp= fopen(pFontFile, "rb")) == NULL)
	{
		printf("Cann't open : %s\n", pFontFile);
		return 0;
	}
	return 1;
}

static inline const char* 
get_extension (const char* filename)
{
    const char* ext;

    ext = strrchr (filename, '.');

    if (ext)
        return ext + 1;

    return NULL;
}

/*
 * read bitmap filedata to 'data'
 */
int InitFont(char *pFontFile, pSunxiFont sxFont)
{
	const char *ext = 0;
	int ret = 0;
	int fd;
	
	ext = get_extension(pFontFile);
	if (0 == strcmp(ext, "7z")) {
		sxFont->memFlag = TRUE;
	}
	
	if (sxFont->memFlag) {	//extract the 7zip file, then read bitmap font from mem.
	//fprintf(stderr, "%s %p %p", pFontFile, sxFont->data, &sxFont->dataSize);
		sxFont->data = (BYTE*) calloc(1, sxFont->dataSize);
	//	fprintf(stderr, "%s %p %p", pFontFile, sxFont->data, &sxFont->dataSize);
		
		extract7Zip(pFontFile, sxFont->data, &sxFont->dataSize);
		
		ret = ReadFontHeader(sxFont);
		sxFont->data = realloc(sxFont->data, sxFont->dataSize);
	} else {
		if(OpenFontFile(pFontFile, sxFont)) {	//read bitmap font form file directly
			ret = ReadFontHeader(sxFont);
			sxFont->dataSize = sxFont->pHeader->Size;
			sxFont->data = (BYTE*) calloc (1, sxFont->dataSize);
			fd = open(pFontFile, O_RDONLY);
			read(fd, sxFont->data, sxFont->pHeader->Size);
		    close (fd);
		}
	}
	
	return ret;
}

// 获取字符的像素宽度
DWORD ReadCharDistX_U(pSunxiFont sxFont, WORD wCode)
{	
	DWORD offset ;
	int   i;
	PFL_HEADER pHeader = sxFont->pHeader;
	sxFont->dwCharInfo = 0;
	//_MG_PRINTF("%s %d wCode````:%x %d [%x] [%x]\n", __FUNCTION__, __LINE__, wCode, pHeader->nSection,
	//	pHeader->pSection[0].First, pHeader->pSection[0].Last);
	
	for(i = 0;i<pHeader->nSection;i++)
	{
		if(wCode >= pHeader->pSection[i].First && wCode <= pHeader->pSection[i].Last)
			break;
	}
	if(i >= pHeader->nSection)
		return 0;
	
	offset = pHeader->pSection[i].OffAddr + FONT_INDEX_TAB_SIZE*(wCode - pHeader->pSection[i].First);
	if (sxFont->memFlag) {
		memcpy(&sxFont->dwCharInfo, sxFont->data+offset, sizeof(DWORD));
	} else {
		fseek(sxFont->fp, offset, SEEK_SET);
		fread(&sxFont->dwCharInfo, sizeof(DWORD), 1, sxFont->fp);
	}
	sxFont->lastGlyph = wCode;
	return GET_FONT_WIDTH(sxFont->dwCharInfo);
}


/********************************************************************
功能： 获取当前字符的像素宽度, 且将索引信息存入一个全局变量：g_dwCharInfo。
        根据索引信息，即同时能获取当前字符的点阵信息的起始地址。
参数： wCode -- 当字库为unicode编码格式时，则将wCode当unicode编码处理。
                否则反之（MBCS)。
********************************************************************/
int ReadCharDistX(pSunxiFont sxFont, WORD wCode)
{
	if (sxFont->lastGlyph != wCode) {
		return ReadCharDistX_U(sxFont, wCode);
	}
	return GET_FONT_WIDTH(sxFont->dwCharInfo);
}

/**********************************************************************
功能： 获取点阵信息
参数： wCode 在这里预留，主要是因为前面有保存一个全局的g_dwCharInfo，也就知道了该字符的相应信息(宽度+点阵信息的起始地址)。
       fontArray 存放点阵信息
	   bytesPerLine 每一行占多少个字节。
**********************************************************************/
int ReadCharDotArray(pSunxiFont sxFont, WORD wCode, BYTE *fontArray, WORD *bytesPerLine)
{	
	*bytesPerLine= (WORD)((GET_FONT_WIDTH(sxFont->dwCharInfo))+7)/PIXELS_PER_BYTE;	

	if(sxFont->dwCharInfo > 0)
	{		
		DWORD nDataLen = *bytesPerLine * sxFont->pHeader->YSize;			
		DWORD  dwOffset = GET_FONT_OFFADDR(sxFont->dwCharInfo);    //获取字符点阵的地址信息(低26位)
		if(sxFont->memFlag) {
			memcpy(fontArray, sxFont->data+dwOffset, nDataLen);
		} else {
			fseek(sxFont->fp, dwOffset, SEEK_SET);
			fread(fontArray, nDataLen, 1, sxFont->fp);
		}
		
		return 1;
	}
	
	return 0;
}


static void sunxi_font_exit(pSunxiFont sxFont)
{
	if (!sxFont) {
		return ;
	}
	if (sxFont->pHeader) {
		if (sxFont->pHeader->pSection) {
			free(sxFont->pHeader->pSection);
		}
		free(sxFont->pHeader);
	}
	if (sxFont->data) {
		free(sxFont->data);
	}
	free (sxFont);
}

static pSunxiFont sunxi_font_init()
{
	
	pSunxiFont sxFont = (pSunxiFont)calloc(1, sizeof(SunxiFont));
	sxFont->pHeader = (PFL_HEADER)calloc(1, sizeof(FL_HEADER));
	sxFont->memFlag = FALSE;
	sxFont->dataSize = FONT_DATA_SIZE; 
	//fprintf(stderr, "%p %ld\n", sxFont->data, sxFont->dataSize);
	return sxFont;
}

static void* load_font_data (const char* font_name, const char* file_name)
{
	
	pSunxiFont sxFont = sunxi_font_init();
	//fprintf(stderr, "%p %d\n", sxFont->data, sxFont->dataSize);
	InitFont(file_name, sxFont);
	
	return sxFont;
}

/********************** Init/Term of raw bitmap font ***********************/

static void unload_font_data (void* data)
{
	
	pSunxiFont sxFont = (pSunxiFont)data;
	ExitFont(sxFont);
	sunxi_font_exit(sxFont);
}

/*************** Raw bitmap font operations *********************************/

static DWORD get_glyph_type (LOGFONT* logfont, DEVFONT* devfont)
{
    return DEVFONTGLYPHTYPE_MONOBMP;
}

static int get_ave_width (LOGFONT* logfont, DEVFONT* devfont)
{
	return SXFONT_INFO_WIDTH(devfont) * GET_DEVFONT_SCALE(logfont, devfont);
}

static int get_font_height (LOGFONT* logfont, DEVFONT* devfont)
{
	return SXFONT_INFO_HEIGHT(devfont) * GET_DEVFONT_SCALE (logfont, devfont);
}

static int get_font_size (LOGFONT* logfont, DEVFONT* devfont, int expect)
{
	int height = SXFONT_INFO_HEIGHT(devfont);
    unsigned short scale = 1;

    if (logfont->style & FS_OTHER_AUTOSCALE)
        scale = font_GetBestScaleFactor (height, expect);

    SET_DEVFONT_SCALE (logfont, devfont, scale);

    return height * scale;
}

static int get_font_ascent (LOGFONT* logfont, DEVFONT* devfont)
{
	int height = SXFONT_INFO_HEIGHT(devfont);
	
	height *= GET_DEVFONT_SCALE (logfont, devfont);

	if (height >= 40)
		return height - 6;
	else if (height >= 20)
		return height - 3;
	else if (height >= 15)
		return height - 2;
	else if (height >= 10)
		return height - 1;

	return height;
}

static int get_font_descent (LOGFONT* logfont, DEVFONT* devfont)
{
	int height = SXFONT_INFO_HEIGHT(devfont);
    
    height *= GET_DEVFONT_SCALE (logfont, devfont);

    if (height >= 40)
        return 6;
    else if (height >= 20)
        return 3;
    else if (height >= 15)
        return 2;
    else if (height >= 10)
        return 1;

    return 0;
}

static const void* get_glyph_monobitmap (LOGFONT* logfont, DEVFONT* devfont,
            const Glyph32 glyph_value, int* pitch, unsigned short* scale)
{
	//_MG_PRINTF("%s %d %x\n", __FUNCTION__, __LINE__, (unsigned short)glyph_value);
	int bitmap_size;
	pSunxiFont sxFont = (pSunxiFont)SXFONT_INFO_P(devfont);
	DWORD val = ReadCharDistX(sxFont, glyph_value);
	if (val == 0) {
		unsigned short char_question_mark = 0x3f;
		ReadCharDistX(sxFont, char_question_mark);
	}
	int width = SXFONT_INFO_WIDTH(devfont);
	int height = SXFONT_INFO_HEIGHT(devfont);
	bitmap_size = ((width + 7) >> 3) * height; 
	if(pitch)
		*pitch = (width + 7) >> 3;

	if (scale)
		*scale = GET_DEVFONT_SCALE (logfont, devfont);
	DWORD charInfo = SXFONT_INFO_CHARINFO(devfont);
	if(charInfo > 0)
	{		
		DWORD nDataLen = bitmap_size;			
		DWORD  dwOffset = GET_FONT_OFFADDR(charInfo);    //获取字符点阵的地址信息(低26位)

		if (!sxFont->memFlag) {
			fseek(sxFont->data, dwOffset, SEEK_SET);
		}
		return sxFont->data + dwOffset;
	}
}

static BOOL is_glyph_existed (LOGFONT* logfont, DEVFONT* devfont, 
        Glyph32 glyph_value)
{
	return 1;
}

static int get_glyph_advance (LOGFONT* logfont, DEVFONT* devfont, 
                Glyph32 glyph_value, int* px, int* py)
{
	DWORD val = ReadCharDistX(SXFONT_INFO_P(devfont), glyph_value);
	int advance = SXFONT_INFO_WIDTH(devfont) * GET_DEVFONT_SCALE (logfont, devfont);
	if (val == 0) {
		advance = 9;
	}
	*px += advance;
	return advance;
}

static int get_glyph_bbox (LOGFONT* logfont, DEVFONT* devfont,
            Glyph32 glyph_value, int* px, int* py, int* pwidth, int* pheight)
{
	DWORD val = ReadCharDistX(SXFONT_INFO_P(devfont), glyph_value);
	int scale = GET_DEVFONT_SCALE (logfont, devfont);
	int width = SXFONT_INFO_WIDTH(devfont) * scale;
	if (val == 0) {
		scale = 1;
		width = 9;
	}
	if (py)
		*py -= get_font_ascent (logfont, devfont);
	if (pwidth)
		*pwidth  = width;
	if (pheight)
		*pheight = SXFONT_INFO_HEIGHT(devfont) * scale;

	return width;
}

static int is_rotatable (LOGFONT* logfont, DEVFONT* devfont, int rot_desired)
{
    return 0;
}

/**************************** Global data ************************************/
FONTOPS __mg_sxf_ops = {
    get_glyph_type,
    get_ave_width,
    get_ave_width,  // max_width same as ave_width
    get_font_height,
    get_font_size,
    get_font_ascent,
    get_font_descent,

    is_glyph_existed,
    get_glyph_advance,
    get_glyph_bbox,

    get_glyph_monobitmap,
    NULL,
    NULL,

    NULL,
    NULL,
    NULL,
    is_rotatable,
    load_font_data,
    unload_font_data
};

#endif  /* _MGFONT_SXF */

