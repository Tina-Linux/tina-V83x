#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include "crc16.h"
#include "log.h"
#include "fops.h"

#define BOOT_SCRIPT "/etc/init.d/rwcheck"
#define BOOT_SCRIPT_LINK (char *)"/etc/rc.d/S99rwcheck"

bool fops_is_existed(const char *path)
{
    return !access(path, F_OK | R_OK);
}

int fops_create_file(char *in_path, char *out_path,
        unsigned long long size, unsigned long long buf_size)
{
    char *buf;
    int fd_in, fd_out;
    int ret = -1;
    unsigned long long bytes = 0;

    buf = malloc(buf_size);
    if (!buf) {
        ERRNO("malloc failed\n");
        return -1;
    }

    fd_in = open(in_path, O_RDONLY);
    if (fd_in < 0) {
        ERRNO("open %s failed\n", in_path);
        goto free_buf;
    }
    fd_out = open(out_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd_out < 0) {
        ERRNO("open %s failed\n", out_path);
        goto close_in;
    }

    while (size > 0) {
        unsigned long long once = min(size, buf_size);
        unsigned long long done = 0;

        while (once > done) {
            int retval;
            retval = read(fd_in, buf + done, once - done);
            if (retval < 0) {
                ERRNO("read %s failed\n", in_path);
                goto close_out;
            }
            if (retval != once - done)
                lseek(fd_in, 0, SEEK_SET);
            done += retval;
        }
        if ((bytes = write(fd_out, buf, once)) != once) {
            ERRNO("write %s failed\n", out_path);
            goto close_out;
        }
        size -= once;
    }

    fsync(fd_out);
    ret = 0;
close_out:
    close(fd_out);
close_in:
    close(fd_in);
free_buf:
    free(buf);
    return ret;
}

int fops_fill_crc(char *path)
{
    int fd, ret, retval = -1;
    unsigned short crc;
    struct stat s;

    fd = open(path, O_RDWR);
    if (fd < 0) {
        ERRNO("open %s failed\n", path);
        goto err;
    }

    if (fstat(fd, &s) < 0) {
        ERRNO("fstat %s failed\n", path);
        goto err;
    }

    ret = get_crc16_fd(fd, s.st_size - 2, &crc);
    if (ret != 0) {
        errno = ret;
        goto err;
    }
    DEBUG("[crc:0x%x]", crc);

    if (lseek(fd, -2, SEEK_END) < 0) {
        ERRNO("lseek %s failed\n", path);
        goto err;
    }

    if ((ret = write(fd, &crc, 2)) != 2) {
        ERRNO("write %s failed\n", path);
        goto err;
    }

    fsync(fd);
    retval = 0;
err:
    close(fd);
    return retval;
}
