#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fileops.h"
#include "log.h"

int cp_fd(int from, int to) {
    int ret = -1;
    int n, buf_len = 4096;
    char *buf = malloc(buf_len);
    if (buf == NULL) {
        ERROR("malloc failed - %s\n", strerror(errno));
        goto err;
    }

    while ((n = read(from, buf, buf_len)) > 0) {
        if (write(to, buf, n) != n) {
            ERROR("write failed - %s\n", strerror(errno));
            goto err;
        }
    }

    if (n < 0) {
        ERROR("read failed - %s\n", strerror(errno));
        goto err;
    }

    ret = 0;

err:
    free(buf);
    return ret;
}

int cp(const char *from, const char *to) {
    int ret = -1;
    int fd_to = -1, fd_from = -1;

    if (is_existed(from) != true) {
        ERROR("%s not existed\n", from);
        goto err;
    }
    fd_from = open(from, O_RDONLY);
    if (fd_from == -1) {
        ERROR("open %s failed - %s\n", from, strerror(errno));
        goto err;
    }

    fd_to = open(to, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if (fd_to == -1) {
        ERROR("open %s failed - %s\n", to, strerror(errno));
        goto err;
    }

    if (cp_fd(fd_from, fd_to) < 0) {
        ERROR("copy %s to %s failed\n", from, to);
        goto err;
    }

    ret = 0;

err:
    close(fd_from);
    close(fd_to);
    return ret;
}

int is_existed(const char *path)
{
    return !access(path, F_OK | R_OK);
}

int is_dir(const char *dir)
{
    if (access(dir, R_OK | X_OK) != 0)
        return -1;

    struct stat st;
    if (stat(dir, &st) == 0)
        return S_ISDIR(st.st_mode);
    return -1;
}

int mkdir_p(const char *dir)
{
    int ret = -1;
    // we must copy this dir-path
    // becasue once if dir is macro definition which can't modify
    // this function will be seg fault
    int len = strlen(dir) + 2;
    char *buf = malloc(len);
    if (buf == NULL) {
        ERROR("malloc failed - %s\n", strerror(errno));
        goto err;
    }
    memset(buf, 0, len);
    strncpy(buf, dir, len);

    char *l = strrchr(buf, '/');

    if (l) {
        *l = '\0';
        if (mkdir_p(buf) < 0)
            goto err;
        *l = '/';
        if (mkdir(buf, 0755) < 0 && errno != EEXIST) {
            ERROR("mkdir failed - %s\n", strerror(errno));
            goto err;
        }
    }

    ret = 0;
err:
    free(buf);
    return ret;
}

// file inexistent will be created
// file existent will be truncated
int touch(const char *fpath)
{
    int len, fd = -1, ret = -1;
    char *dir = NULL, *buf = NULL;

    if (fpath == NULL) {
        ERROR("no target, touch failed\n");
        goto err;
    }
    len = strlen(fpath) + 10;
    buf = malloc(len);
    if (buf == NULL) {
        ERROR("malloc failed - %s\n", strerror(errno));
        goto err;
    }
    memset(buf, 0, len);

    strncpy(buf, fpath, len);
    dir = dirname((char *)buf);

    //ensure dir is access
    if (is_dir(dir) != true) {
        ERROR("can't access to %s, make it\n", dir);
    } else {
        mkdir_p(dir);
    }
    //just touch a new file
    fd = open(fpath, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd == -1) {
        ERROR("create %s failed - %s\n", fpath, strerror(errno));
        goto err;
    }
    fsync(fd);
    close(fd);

    ret = 0;
err:
    free(buf);
    return ret;
}

unsigned long get_filesize(const char *fpath)
{
    unsigned long filesize = -1;
    struct stat statbuff;
    if (stat(fpath, &statbuff) < 0) {
        ERROR("stat %s failed - %s\n", fpath, strerror(errno));
        return filesize;
    } else
        filesize = statbuff.st_size;
    return filesize;

}

//delete one file last line
int delete_lastline(const char *fpath)
{
    FILE *fp;
    int filesize;
    fp = fopen(fpath, "rw");
    if(fp == NULL){
        printf("Open %s error!\n", fpath);
        return -1;
    }
    fseek(fp, -2, SEEK_END);
    while(fgetc(fp) != '\n'){
        fseek(fp, -2, SEEK_CUR);
    }
    filesize = ftell(fp)-1;
    truncate(fpath, filesize);
    if(fclose(fp) != 0){
        printf("close %s failed.\n", fpath);
        return -1;
    }
    return 0;
}

int get_tty(char *tty, int len)
{
    int ret = -1;

    FILE *fp = fopen("/proc/cmdline", "r");
    if (fp == NULL) {
        fprintf(stderr, "fopen failed - %s\n", strerror(errno));
        return -1;
    }
    char buf[2048] = {0};
    if (fread(buf, 1, 2048, fp) <= 0) {
        fprintf(stderr, "fread failed - %s\n", strerror(errno));
        goto out;
    }
    char *start = strstr(buf, "console=") - 1;
    if (start == NULL) {
        fprintf(stderr, "strstr failed - %s\n", strerror(errno));
        goto out;
    }
    start += sizeof("console=");
    char *p = strchr(start, ' ');
    if (p == NULL) {
        fprintf(stderr, "strchr failed - %s\n", strerror(errno));
        goto out;
    }
    *p = '\0';
    p = strchr(start, ',');
    if (p != NULL)
        *p = '\0';

    snprintf(tty, len, "/dev/%s", start);
    ret = 0;
out:
    fclose(fp);
    return ret;

}

/*
 * 功能: 从/proc中获取stdout/stderr的文件路径
 * 参数: STDOUT_FILENO or STDERR_FILENO
 */
int get_fstd(int fd, char *resp, int len)
{
    char buf[100];
    struct stat st;
    int length = -1;
    snprintf(buf, 100, "/proc/%d/fd/%d", getpid(), fd);
    if (lstat(buf, &st) == -1) {
        ERROR("lstat for %s failed\n", buf);
        return -1;
    }
    length = readlink(buf, resp, len);
    if (length == -1) {
        ERROR("readlink %s failed\n", buf);
        return -1;
    }
    resp[length] = '\0';
    if (length > st.st_size) {
        ERROR("%s symlink increased in size between lstat() and readlink()", buf);
        resp[0] = '\0';
        return -1;
    }

    return 0;
}

/*
 * truncate path and assignment to struct cut_path
 *
 * eg: path = "/base/production/pmutester"
 *     cut_path.dir = /base/production
 *     cut_path.base = pmutester
 *
 * NOTICE:
 * Please free cutpath space after use.
 * cutpath cp = cutpath(cpath);
 * free(cp.dir);
 */

cutpath cut_path(const char *path)
{
    cutpath cut_path;
    cut_path.pathname = path;
    cut_path.dir = strdup(path);
    cut_path.base = strrchr(cut_path.dir, '/');
    *(cut_path.base++) = '\0';
    return cut_path;
}
