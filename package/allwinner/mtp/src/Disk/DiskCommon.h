#ifndef _DISKCOMMON_H
#define _DISKCOMMON_H

#include "Disk.h"
#include <dirent.h>

#if defined(DEBUG) || defined(FORCE_DEBUG)

#if NO_DEBUG
#define DLOG(fmt, arg...)
#else
#include <stdio.h>
#define DLOG(fmt, arg...)               printf("[%s:%d] "fmt"\n", __FUNCTION__, __LINE__, ##arg)
#endif

#else

#define DLOG(fmt, arg...)

#endif

#ifndef PATH_MAX
#define PATH_MAX	(512)
#endif



#ifdef MEM_DEBUG

void *malloc_wrapper(size_t size);
void *calloc_wrapper(size_t nmemb, size_t size);
void *realloc_wrapper(void *ptr, size_t size);
char *strdup_wrapper(const char *s);
void free_wrapper(void *ptr);
void memleak_print(void);
void memleak_exit(void);
int scandir_wrapper(const char *dirp, struct dirent ***namelist,
              int (*filter)(const struct dirent *),
              int (*compar)(const struct dirent **, const struct dirent **));
#else

#define malloc_wrapper(size)				malloc(size)
#define calloc_wrapper(nmemb, size)			calloc(nmemb, size)
#define realloc_wrapper(ptr, size)			realloc(ptr, size)
#define strdup_wrapper(s)				strdup(s)
#define scandir_wrapper(dirp, namelist, filter, compar) scandir(dirp, namelist, filter, compar)
#define free_wrapper(ptr)				free(ptr)
#define memleak_print()
#define memleak_exit()

#endif

#endif
