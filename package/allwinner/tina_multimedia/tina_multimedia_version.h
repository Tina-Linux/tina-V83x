/*
 * Copyright (c) 2008-2019 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : tina_multimedia_version.h
 * Description : tina multimedia version header file
 * History :
 *
 */

#ifndef TINA_MULTIMEDIA_VERSION_H
#define TINA_MULTIMEDIA_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#define REPO_TAG "tina3.5"
#define REPO_BRANCH "tina-dev"
#define REPO_DATE "Mon Jul 15 19:04:59 2019 +0800"
#define REPO_CHANGE_ID "I5f6c8a88d7b387a312b7744797a0d5f8ab07ee7a"

static inline void LogTinaVersionInfo(void)
{
    printf("\n"
         ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> tina_multimedia <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n"
         "tag   : %s\n"
         "branch: %s\n"
         "date  : %s\n"
         "Change-Id: %s\n"
         "-------------------------------------------------------------------------------\n",
         REPO_TAG, REPO_BRANCH, REPO_DATE,REPO_CHANGE_ID);
}

#ifdef __cplusplus
}
#endif

#endif
