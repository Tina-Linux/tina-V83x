/*
 * Copyright (c) 2018 - 2019 M-SMART Research Institute of Midea Group.
 * All rights reserved.
 *
 * File Name            : ms2m_hal_chip.c
 * Introduction         :
 *
 * Current Version      : v1.0
 * Author               : Zachary Chau <zhouzh6@midea.com.cn>
 * Create Time          : 2018/07/10
 * Change Log           :
 *
 * All software, firmware and related documentation here in ("M-Smart Software") are
 * intellectual property of M-SMART Research Institute of Midea Group and protected
 * by law, including, but not limited to, copyright law and international treaties.
 *
 * Any use, modification, reproduction, retransmission, or republication of all
 * or part of M-Smart Software is expressly prohibited, unless prior written
 * permission has been granted by M-Smart.
 */

#include "stdint.h"
#include "string.h"
#include "unistd.h"
#include "time.h"
#include "pthread.h"

#include "elog.h"
#include "ms2m_hal_chip.h"
#include <curl/curl.h>
#include "ms_common.h"
#include <unistd.h>

#define CONFIG_PATH	"/data/cfg"

static pthread_mutex_t gs_flash_mutex;

//---------------------------------------------------------------------------//

unsigned long ms2m_hal_ticks2msec(unsigned long ticks)
{
	unsigned long HZ = sysconf( _SC_CLK_TCK );
	unsigned long msec = ((double)ticks)/((double)HZ)*1000.0;
	return msec;
}

unsigned long ms2m_hal_msec2ticks(unsigned long ms)
{
	unsigned long HZ = sysconf( _SC_CLK_TCK );
	unsigned long ticks = ((double)ms)/1000.0*((double)HZ);
	return ticks;
}

unsigned long ms2m_hal_get_ticks()
{
	unsigned long ticks = (unsigned long) times( NULL );
	return ticks;
}

MS2M_STATUS ms2m_hal_ticks_compare(unsigned long start_tick, unsigned long end_tick, unsigned long interval_ms)
{
	unsigned long interval_tick = ms2m_hal_msec2ticks(interval_ms);
	if(start_tick > end_tick)
	{
		//end_tick += 0xffffffff;
		return ((0XFFFFFFFF - start_tick + end_tick >= interval_tick) ? M_OK : M_ERROR);
	}
	return ((end_tick - start_tick >= interval_tick) ? M_OK : M_ERROR);
}

void ms2m_hal_srand(unsigned int seed)
{
	srand(seed);
}

unsigned int ms2m_hal_rand()
{
	return (unsigned int)rand();
}

void ms2m_hal_reboot()
{
	exit(0);
}

void ms2m_hal_msleep(unsigned int ms)
{
	struct timeval tv;
	memset(&tv,0,sizeof(struct timeval));
	tv.tv_sec = 0;
	tv.tv_usec = ms * 1000;
	select(0, NULL, NULL, NULL, &tv);
	return;
}

int MS2M_WRITE(const char *c, unsigned int l)
{
	int i = 0;
	for(i = 0; i < l; i++)
	{
		MS2M_PUTCHAR(c[i]);
	}
	return i;
}

void ms2m_hal_mem_left()
{
	char *line = NULL;
	char *c = NULL;

	char file[64] = {0};
	char line_buff[256] = {0};
	char name[32] = {0};

	int vmpeak= -1;
	int vmsize  = -1;
	int vmhwm = -1;
	int vmrss = -1;

	const pid_t p = getpid();
	sprintf(file,"/proc/%d/status",p);
	FILE *fd = fopen (file, "r");

	while(1)
	{
		line = fgets (line_buff, sizeof(line_buff), fd);
		if(line == NULL)
		{
			break;
		}
		c = strstr(line,":");
		if(c != NULL)
		{
			*c = '\0';
		}
		if(strcmp(line,"VmPeak") == 0)
		{
			*c = ' ';
			sscanf (line_buff, "%31s %d", name, &vmpeak);
		}
		if(strcmp(line,"VmSize") == 0)
		{
			*c = ' ';
			sscanf (line_buff, "%31s %d", name, &vmsize);
		}
		else if(strcmp(line,"VmHWM") == 0)
		{
			*c = ' ';
			sscanf (line_buff, "%31s %d", name,&vmhwm);
		}
		else if(strcmp(line,"VmRSS") == 0)
		{
			*c = ' ';
			sscanf (line_buff, "%31s %d", name,&vmrss);
		}

		if(vmpeak < 0 || vmsize < 0 || vmhwm < 0 || vmrss < 0)
		{
			continue;
		}
		else
		{
			break;
		}
	}
	MS_TRACE ("\r\ncurrent pid:%d\r\n", p);
	MS_TRACE("vmsize:%d/%d\r\n",vmsize,vmpeak);
	MS_TRACE("rmsize:%d/%d\r\n\r\n",vmrss,vmhwm);
	fclose(fd);  //关闭文件fd
}

void ms2m_hal_flash_init()
{
	pthread_mutex_init(&gs_flash_mutex, NULL);
}

MS2M_STATUS ms2m_hal_flash_erase_sector(unsigned int addr)
{
	int ret = -1;
	char name[100] = {0};
	char num[20] = {0};

	sprintf(num,"%d",addr);
	strcpy(name,CONFIG_PATH"/block_");
	strcat(name,num);
	strcat(name,".bin");

	pthread_mutex_lock(&gs_flash_mutex);
	ret = remove(name);
	pthread_mutex_unlock(&gs_flash_mutex);

	if(0 == ret)
	{
		return M_OK;
	}
	return M_ERROR;
}

MS2M_STATUS ms2m_hal_flash_read(unsigned int addr, char *buf, unsigned int len)
{
	static unsigned int base_addr = 0xFFFFFFFF;
	static unsigned int forecast_addr = 0xFFFFFFFF;

	unsigned int real_len = 0;
	MS2M_STATUS ret = M_ERROR;
	FILE *fp = NULL;
	char name[100] = {0};
	char num[20] = {0};
	if(buf == NULL)
	{
		return M_ERROR;
	}

	if(forecast_addr == 0xFFFFFFFF || forecast_addr != addr)
	{
		sprintf(num,"%d",addr);
		strcpy(name,CONFIG_PATH"/block_");
		strcat(name,num);
		strcat(name,".bin");
		if(forecast_addr != 0xFFFFFFFF)
		{
			forecast_addr = 0xFFFFFFFF;
			base_addr = 0xFFFFFFFF;
		}
	}
	else
	{
		sprintf(num,"%d",base_addr);
		strcpy(name,CONFIG_PATH"/block_");
		strcat(name,num);
		strcat(name,".bin");
	}

	//xSemaphoreTake(gs_flash_mutex, portMAX_DELAY);
	pthread_mutex_lock(&gs_flash_mutex);
	fp = fopen(name, "r");
	if(fp == NULL)
	{
		ret = M_ERROR;
		goto error;
	}

	fseek(fp,0L,SEEK_END);
	real_len = ftell(fp);
	fseek(fp,0L,SEEK_SET);

	if(real_len <= len)
	{
		len = real_len;
	}
	else
	{
		if(base_addr == 0xFFFFFFFF)
		{
			base_addr = addr;
			forecast_addr = base_addr + len;
		}
		else
		{
			if(addr == forecast_addr)
			{
				if(addr - base_addr + len < real_len)
				{
					forecast_addr = forecast_addr + len;
					fseek(fp,addr - base_addr,SEEK_SET);
				}
				else
				{
					len = base_addr + real_len - addr;
					fseek(fp,addr - base_addr,SEEK_SET);
					forecast_addr = 0xFFFFFFFF;
					base_addr = 0xFFFFFFFF;
				}
			}

		}

	}
	if(1 == fread(buf, len, 1, fp))
	{
		ret = M_OK;
	}
	else
	{
		ret = M_ERROR;
	}
	fclose(fp);
	//xSemaphoreGive(gs_flash_mutex);
error:
	pthread_mutex_unlock(&gs_flash_mutex);

#ifdef __DEBUG_LOG__
	{
		int i;
		MS_TRACE("[%s]2\r\n", __FUNCTION__);
		for(i = 0; i < len; i++)
		{
			if(0 == i%10)
				MS_TRACE("\r\n");
			MS_TRACE("%02x ", ((unsigned char *)buf)[i]);
		}
		MS_TRACE("\r\n");
	}
#endif
	return ret;
}

MS2M_STATUS ms2m_hal_flash_write(unsigned int addr, char *buf, unsigned int len)
{
	MS2M_STATUS ret = M_ERROR;
	FILE *fp = NULL;
	char name[100] = {0};
	char num[20] = {0};

	if(buf == NULL)
		return M_ERROR;

	sprintf(num,"%d",addr);
	strcpy(name,CONFIG_PATH"/block_");
	strcat(name,num);
	strcat(name,".bin");

	//xSemaphoreTake(gs_flash_mutex, portMAX_DELAY);
	pthread_mutex_lock(&gs_flash_mutex);

	fp = fopen(name, "w");

	if(1 == fwrite(buf, len, 1, fp) )
		ret = M_OK;
	else
		ret = M_ERROR;

	fclose(fp);

	//xSemaphoreGive(gs_flash_mutex);
	pthread_mutex_unlock(&gs_flash_mutex);

#ifdef __DEBUG_LOG__
	int i;
	MS_TRACE("[%s]\r\n", __FUNCTION__);
	for(i = 0; i < len; i++)
	{
		if(0 == i%10)
			MS_TRACE("\r\n");
		MS_TRACE("%02x ", ((unsigned char *)buf)[i]);
	}
	MS_TRACE("\r\n");
#endif
	return ret;
}

MS2M_STATUS ms2m_hal_now_time(char *time_str)
{
	time_t timep;
	struct tm *p;

	time(&timep);
	p = localtime(&timep);
	if (p == NULL) {
		return M_ERROR;
	}
	snprintf(time_str, 18, "%02d-%02d %02d:%02d:%02d", p->tm_mon + 1, p->tm_mday,
			p->tm_hour, p->tm_min, p->tm_sec);
	return M_OK;
}

long get_download_file_lenth(const char *url){
	long downloadFileLenth = 0;
	CURL *handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_URL, url);
	curl_easy_setopt(handle, CURLOPT_HEADER, 1);
	curl_easy_setopt(handle, CURLOPT_NOBODY, 1);
	if (curl_easy_perform(handle) == CURLE_OK) {
		curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &downloadFileLenth);
	} else {
		downloadFileLenth = -1;
	}
	return downloadFileLenth;
}

MS2M_STATUS ms2m_hal_flash_http_get(unsigned int addr, const char *url, int *len)
{
	MS2M_STATUS ret = M_ERROR;
	FILE *fp = NULL;
	char name[100] = {0};
	char num[20] = {0};
	CURL *curl;
	CURLcode res;

	sprintf(num,"%d",addr);
	strcpy(name,CONFIG_PATH"/block_");
	strcat(name,num);
	strcat(name,".bin");

	//xSemaphoreTake(gs_flash_mutex, portMAX_DELAY);
	pthread_mutex_lock(&gs_flash_mutex);

	fp = fopen(name, "w");
	if(fp == NULL)
		goto func_exit;
	curl = curl_easy_init();
	if(curl){
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		res = curl_easy_perform(curl);
		if(res == CURLE_OK){
			ret = M_OK;
			if(len != NULL){
				*len = get_download_file_lenth(url);
			}
		}
		curl_easy_cleanup(curl);
	}
	fclose(fp);

	//xSemaphoreGive(gs_flash_mutex);
func_exit:
	pthread_mutex_unlock(&gs_flash_mutex);

	return ret;
}
