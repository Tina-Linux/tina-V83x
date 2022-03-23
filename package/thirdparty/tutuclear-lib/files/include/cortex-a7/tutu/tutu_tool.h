/*
 * tutu_tool.h
 * yanchen.lu@gmems.com
 *
 */

#ifndef _TUTU_TOOL_H_
#define _TUTU_TOOL_H_

#ifdef _MSC_VER
#pragma warning( disable : 4996 )
#include <conio.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tutu_typedef.h"

typedef struct {
    char		PszIdent[256];
    char		PszValue[256];
} TIdentValue;

typedef struct {
    FILE		*fptr;
    int			iLineCount;
    TIdentValue *pIdentValueList;
} TIniFileInfo;

#ifdef __cplusplus
extern "C" {
#endif

TIniFileInfo *OpenIniFile(const char *pszFileName);
int CloseIniFile(TIniFileInfo *pIniFileInfo);
int ReadWord32(TIniFileInfo *pIniFileInfo, const char *pszIdent, W32 *pw32Value, W32 w32Default);
int ReadUWord32(TIniFileInfo *pIniFileInfo, const char *pszIdent, UW32 *puw32Value, UW32 uw32Default);
int ReadString(TIniFileInfo *pIniFileInfo, const char *pszIdent, char *pszValue, char *pszDefault);
int ReadUWord16(TIniFileInfo *pIniFileInfo,
                const char         *pszIdent,
                UW16      *puw16,
                UW16      uw16Default);
int ReadWord16(TIniFileInfo *pIniFileInfo,
               const char         *pszIdent,
               W16       *pw16,
               W16       w16Default);

#ifdef __cplusplus
}
#endif

#endif // _TUTU_TOOL_H_
