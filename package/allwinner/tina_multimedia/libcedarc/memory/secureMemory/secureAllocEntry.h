
/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : secureAllocEntry.h
* Description :
* History :
*   Author  : xyliu <xyliu@allwinnertech.com>
*   Date    : 2016/04/13
*   Comment :
*
*
*/
#if (PLATFORM_SURPPORT_SECURE_OS == 1)
#ifndef SECURE_ALLOC_ENTRY_H
#define SECURE_ALLOC_ENTRY_H


struct ScMemOpsS* __GetSecureMemOpsS();

#endif
#endif
