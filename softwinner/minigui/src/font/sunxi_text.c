#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

static int preone_cnt(unsigned char val)
{
	int i;
	int cnt = 0;

	for (i=7; i>=0; i--) {
		if (!(val & (1<<i))) {
			break;
		} else {
			cnt++;
		}
	}

	return cnt;
}

static int uc8_to_16(const char *buf_start, const char *buf_end, unsigned short *code)
{
	int i;
	int num;
	unsigned char val;
	unsigned int sum = 0;

	if (buf_start >= buf_end)
	{
		return 0;
	}

	val = buf_start[0];
	num = preone_cnt(buf_start[0]);

	if ((buf_start + num) > buf_end)
	{
		return 0;
	}

	if (0 == num)
	{
		*code = buf_start[0];
		return 1;
	}
	else
	{
		val  = val << num;
		val  = val >> num;
		sum += val;
		for (i=1; i<num; i++)
		{
			val  = buf_start[i] & 0x3f;
			sum  = sum << 6;
			sum += val;
		}
		*code = sum;
		return num;
	}
}

int ucs8_to_16(unsigned short *dest, const char *str)
{

	const char *str_utf8_end = str + strlen(str);
	int last_len = 0;
	int conved_mstr_len = 0;
	int cnt = 0;
	while (1)
	{
		unsigned short code = 0;
		last_len = uc8_to_16(str+conved_mstr_len, str_utf8_end, &code);
		if (0 == last_len)
		{
			return conved_mstr_len;
		}
		*(dest+cnt) = code;
		conved_mstr_len += last_len;
		cnt++;
	}
}


#ifndef SUNXI_MIN
#include "sunxi_text.h"

struct var_info
{
	unsigned int xres;
	unsigned int yres;
	unsigned int bits_per_pixel;
};

static struct var_info var;

static unsigned char *mem;
typedef unsigned short __u16; 

static unsigned int line_width;
static unsigned int byte_per_pixel;


static void lcd_put_pixel(unsigned int x, unsigned int y, unsigned int color)
{
	unsigned char *pen_8;
	unsigned short *pen_16;
	unsigned int *pen_32;
	unsigned int red, green, blue;
	if ((x >= var.xres) || (y >= var.yres))
	{
		return ;
	}
	pen_8 = mem + y*line_width+x*byte_per_pixel;
	pen_16 = (unsigned short *)pen_8;
	pen_32 = (unsigned int *)pen_8;

	switch(var.bits_per_pixel)
	{
	case 8:
		{
			*pen_8 = color;
			break;
		}
	case 16:
		{
			/* 565 */
			red   = (color >> 16) & 0xff;
			green = (color >> 8) & 0xff;
			blue  = (color >> 0) & 0xff;
			*pen_16 = ((red>>3)<< 11)|((green>>2) << 5)|(blue>>3); 
		}
	case 32:
		{
			*pen_32 = color;
			break;
		}
	default:
		{
			printf("can't support %dbpp\n", var.bits_per_pixel);
		}
	}
}

static void lcd_put_ascii(int x, int y, unsigned char c, int color)
{
	unsigned char *dots = (unsigned char *)&fontdata_8x16[c*16];
	int i, b;
	unsigned char byte;

	for(i=0; i<16; i++)
	{
		byte = dots[i];
		for(b=7; b>=0; b--)
		{
			if(byte & (1<<b))
			{
				/* show */
				lcd_put_pixel(x+7-b, y+i, color); /* white */
			}
			//else
			{
				/* hide */
				//	lcd_put_pixel(x+7-b, y+i, 0); /* black */
			}
		}
	}
}

static void lcd_put_chinese(int x, int y, unsigned int str, int color)
{
	unsigned int where  = (str&0xff) - 0xA1;
	unsigned int area = ((str&0xff00)>>8) - 0xA1;
	const unsigned char *dots = hzk16+(area * 94 + where)*32;
	int i, j, b;
	unsigned char byte;
	for(i=0; i<16; i++)
	{
		for(j=0; j<2; j++)
		{
			byte = dots[i*2 + j];
			for(b=7; b>=0; b--)
			{
				if(byte & (1<<b))
				{
					/* show */
					lcd_put_pixel(x+j*8+7-b, y+i, color); /* white */
				}
				//else
				{
					/* hide */
					//	lcd_put_pixel(x+j*8+7-b, y+i, 0); /* black */
				}
			}
		}
	}
}

static int utf8_to_u16le(char *buf_start, char *buf_end, unsigned int *code)
{
	int i;
	int num;
	unsigned char val;
	unsigned int sum = 0;

	if (buf_start >= buf_end)
	{
		return 0;
	}

	val = buf_start[0];
	num = preone_cnt(buf_start[0]);

	if ((buf_start + num) > buf_end)
	{
		return 0;
	}

	if (0 == num)
	{
		*code = buf_start[0];
		return 1;
	}
	else
	{
		val  = val << num;
		val  = val >> num;
		sum += val;
		for (i=1; i<num; i++)
		{
			val  = buf_start[i] & 0x3f;
			sum  = sum << 6;
			sum += val;
		}
		*code = sum;
		return num;
	}	
}

static void u16le_to_gb2312(unsigned int *code)
{
	int l = 0;
	int h = MAX_UNI_INDEX - 1;
	int mid;
	while (l <= h) 
	{
		mid = (l + h) / 2; 
		if (*code < UNI_to_GB[mid][0])
		{
			h = mid - 1;	
		}
		else if (*code > UNI_to_GB[mid][0])
		{
			l = mid + 1;
		}
		else
		{
			*code = UNI_to_GB[mid][1];
			break;
		}
	}
	if (l > h) //have not found.
	{
		*code = UNI_to_GB[0][1]; 
	}
}

int DrawInit(unsigned char *addr, unsigned int w, unsigned int h, unsigned int bpp)
{
	mem = addr;
	var.xres = w;
	var.yres = h;
	var.bits_per_pixel = bpp;
	byte_per_pixel	= var.bits_per_pixel / 8;
	line_width		= var.xres * byte_per_pixel;
	return 0;
}

int DrawString(char *str_utf8, unsigned int x, unsigned int y, int color)
{
	int cnt = 0;
	char *str_utf8_end = str_utf8 + strlen(str_utf8);
	int len = 0;
	while(1) 
	{
		unsigned int code = 0;
		len = utf8_to_u16le(str_utf8, str_utf8_end, &code);
		if (0 == len) {
			break;
		}
		if (len > 1) {
			u16le_to_gb2312(&code);
			lcd_put_chinese(x+(8*cnt), y, code, color);
			cnt++;
		} else {
			lcd_put_ascii(x+(8*cnt), y, code, color);
		}
		str_utf8 += len;
		cnt++;
	}
	return 0;
}

static int uc16_to_gb(unsigned short *code)
{
	int l = 0;
	int h = MAX_UNI_INDEX - 1;
	int mid;
	while (l <= h) 
	{
		mid = (l + h) / 2; 
		if (*code < UNI_to_GB[mid][0]) {
			h = mid - 1;	
		} else if (*code > UNI_to_GB[mid][0]) {
			l = mid + 1;
		} else {
			*code = UNI_to_GB[mid][1];
			break;
		}
	}

	if (l > h) //have not found.
	{
		*code = UNI_to_GB[0][1]; 
	}

	return 0;
}

static int uc16_to_jis(unsigned short *code)
{
	int l = 0;
	int h = MAX_JIS_UNI_INDEX - 1;
	int mid;

	while (l <= h) 
	{
		mid = (l + h) / 2; 
		if (*code < UNI_to_JIS[mid][1]) {
			h = mid - 1;	
		} else if (*code > UNI_to_JIS[mid][1]) {
			l = mid + 1;
		} else {
			*code = UNI_to_JIS[mid][0];
			break;
		}
	}

	if (l > h) //have not found.
	{
		*code = UNI_to_JIS[0][0]; 
	}

	return 0;
}
extern int uni2euckr(const wchar_t uni,unsigned char *out);
static int uc16_to_euckr(unsigned short *code)
{
	return uni2euckr(*code,code);
}

int Utf8_to_Jis(unsigned char *dest, const char *str_utf8)
{
	//char *str_u16 = FixStrAlloc(sizeof(char)*strlen(str_utf8));
	int conved_mstr_len = 0;
	int len = 0;
	int cnt = 0;
	unsigned short code = 0;
	const char *str_utf8_end = str_utf8 + strlen(str_utf8);

	while(1)
	{
		len = uc8_to_16(str_utf8+conved_mstr_len, str_utf8_end, &code);
		if (0 == len)
		{
			break;
		}
		else if (1 == len)
		{	
			dest[cnt] = (unsigned char)code;
		}
		else // (len > 1) 
		{
			uc16_to_jis(&code);
			dest[cnt]   = *((unsigned char *)&code + 1);
			dest[cnt+1] = *((unsigned char *)&code);
			cnt++;
		}
		conved_mstr_len += len;
		cnt++;
	}
	return conved_mstr_len;
}
int Utf8_to_Euckr(unsigned char *dest, const char *str_utf8)
{
	//char *str_u16 = FixStrAlloc(sizeof(char)*strlen(str_utf8));
	int conved_mstr_len = 0;
	int len = 0;
	int cnt = 0;
	unsigned short code = 0;
	const char *str_utf8_end = str_utf8 + strlen(str_utf8);

	while(1)
	{
		len = uc8_to_16(str_utf8+conved_mstr_len, str_utf8_end, &code);
		if (0 == len)
		{
			break;
		}
		else if (1 == len)
		{	
			dest[cnt] = (unsigned char)code;
		}
		else // (len > 1) 
		{
			uc16_to_euckr(&code);
			dest[cnt]   = *((unsigned char *)&code);
			dest[cnt+1] = *((unsigned char *)&code + 1);
			cnt++;
		}
		conved_mstr_len += len;
		cnt++;
	}
	return conved_mstr_len;
}

int Utf8_to_Gb(unsigned char *dest, const char *str_utf8)
{
	//char *str_u16 = FixStrAlloc(sizeof(char)*strlen(str_utf8));
	int conved_mstr_len = 0;
	int len = 0;
	int cnt = 0;
	unsigned short code = 0;
	const char *str_utf8_end = str_utf8 + strlen(str_utf8);
	while(1)
	{
		len = uc8_to_16(str_utf8+conved_mstr_len, str_utf8_end, &code);
		if (0 == len)
		{
			break;
		}
		else if (1 == len)
		{	
			dest[cnt] = (unsigned char)code;
		}
		else // (len > 1) 
		{
			uc16_to_gb(&code);
			dest[cnt]   = *((unsigned char *)&code + 1);
			dest[cnt+1] = *((unsigned char *)&code);
			cnt++;
		}
		conved_mstr_len += len;
		cnt++;
	}
	return conved_mstr_len;

}
#endif
