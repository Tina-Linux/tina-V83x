#ifndef __FILEOPS_H
#define __FILEOPS_H

#include <stdbool.h>

typedef struct{
    const char *pathname;
    char *base;
    char *dir;
}cutpath;

int cp(const char *from, const char *to);
int cp_fd(int from, int to);
int is_dir(const char *dir);
int is_existed(const char *path);
int mkdir_p(const char *dir);
int touch(const char *fpath);
int delete_lastline(const char *fpath);
unsigned long get_filesize(const char *fpath);
int get_tty(char *tty, int len);
int get_fstd(int fd, char *resp, int len);
cutpath cut_path(const char *path);

#endif
