/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : iniparserapi.c
 * Description : iniparserapi
 * History :
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "cdcIniparser.h"
#include "CdcIniparserapi.h"

static dictionary* g_ini = NULL;

int  CdcIniParserInit()
{
    logd("CdcIniParserInit");
    char ini_name[265] = "/etc/cedarc.conf";

    g_ini = iniparser_load(ini_name);
    if (g_ini == NULL)
    {
        logd("cannot parse file: %s\n", ini_name);
        return -1 ;
    }
    else
    {
        logd("load conf file %s ok!\n", ini_name);
    }

    return 0;
}

void CdcIniParserDestory()
{
    if (g_ini == NULL)
    {
        iniparser_freedict(g_ini);
        g_ini = NULL;
    }
}

const char * CdcIniParserGetString(const char * key, const char * def)
{
    if (g_ini == NULL)
    {
        int ret = CdcIniParserInit();
        if (ret == -1)
            return NULL;
    }
    return iniparser_getstring(g_ini,key,def);
}

int CdcIniParserGetInt(const char * key, int notfound)
{
    if (g_ini == NULL)
    {
        int ret = CdcIniParserInit();
        if (ret == -1)
            return notfound;
    }
    return iniparser_getint(g_ini,key,notfound);;
}

double CdcIniparserGetDouble(const char * key, double notfound)
{
    if (g_ini == NULL)
    {
        int ret = CdcIniParserInit();
        if (ret == -1)
            return notfound;
    }
    return iniparser_getdouble(g_ini,key,notfound);
}

int CdcIniParserGetBoolean(const char * key, int notfound)
{
    if (g_ini == NULL)
    {
        int ret = CdcIniParserInit();
        if (ret == -1)
            return notfound;
    }
    return iniparser_getboolean(g_ini,key,notfound);
}

int CdcIniParserFindEntry(const char * entry)
{
    if (g_ini == NULL)
    {
        int ret = CdcIniParserInit();
        if (ret == -1)
            return 0;
    }
    return iniparser_find_entry(g_ini,entry);
}

int CdcGetConfigParamterInt(const char * key, int notfound)
{
    char paramterkey[128];
    sprintf(paramterkey,"paramter:%s",key);
    return CdcIniParserGetInt(paramterkey,notfound);
}

const char * CdcGetConfigParamterString(const char * key, const char * def)
{
    char paramterkey[128];
    sprintf(paramterkey,"paramter:%s",key);
    return CdcIniParserGetString(paramterkey,def);
}
double CdcGetConfigParamterDouble(const char * key, double notfound)
{
    char paramterkey[128];
    sprintf(paramterkey,"paramter:%s",key);
    return CdcIniparserGetDouble(paramterkey,notfound);
}
int CdcGetConfigParamterBoolean(const char * key, int notfound)
{
    char paramterkey[128];
    sprintf(paramterkey,"paramter:%s",key);
    return CdcIniParserGetBoolean(paramterkey,notfound);
}
