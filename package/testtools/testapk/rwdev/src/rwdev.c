#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <libgen.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <errno.h>

#include "rwdev.h"
#define CLEANMEM "/proc/sys/vm/drop_caches"
#define MEMINFO "/proc/meminfo"
#define DEFAULT_BUFFER_SIZE "4M"
#define DEFAULT_LOOP_TIME 1
#define DEFAULT_ACTIONS (ACTION_WRITE | ACTION_READ | ACTION_VERIFY)
char DEFAULT_DATA[] = {
    0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
    0xFF, 0xFF, 0xCC, 0xCC, 0xCC, 0x33, 0xCC, 0xCC,
    0xCC, 0x33, 0x33, 0xCC, 0xCC, 0xCC, 0xFF, 0xFF,
    0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF,
    0xFF, 0xFF, 0xDD, 0xFF, 0xFF, 0xFF, 0xDD, 0xDD,
    0xFF, 0xFF, 0xFF, 0xBB, 0xFF, 0xFF, 0xFF, 0xBB,
    0xBB, 0xFF, 0xFF, 0xFF, 0x77, 0xFF, 0xFF, 0xFF,
    0x77, 0x77, 0xFF, 0x77, 0xBB, 0xDD, 0xEE, 0xFF,
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00,
    0x00, 0xFF, 0xFF, 0xCC, 0xCC, 0xCC, 0x33, 0xCC,
    0xCC, 0xCC, 0x33, 0x33, 0xCC, 0xCC, 0xCC, 0xFF,
    0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE,
    0xFF, 0xFF, 0xFF, 0xDD, 0xFF, 0xFF, 0xFF, 0xDD,
    0xDD, 0xFF, 0xFF, 0xFF, 0xBB, 0xFF, 0xFF, 0xFF,
    0xBB, 0xBB, 0xFF, 0xFF, 0xFF, 0x77, 0xFF, 0xFF,
    0xFF, 0x77, 0x77, 0xFF, 0x77, 0xBB, 0xDD, 0xEE,
};
struct task task = {0};

void help(void)
{
    INFO("  Usage: rwdev [-b bufsize] [-d hexdata] [-a actor] [-s testsize]\n");
    INFO("          [-l looptimes] [-h] <devfile> \n");
    INFO("\n");
    INFO("\t-a # : actions to do(0:write 1:read 2:verify), default all actors\n");
    INFO("\t-b # : buffer size, default 4M\n");
    INFO("\t-d # : the data in hex to write, default emmc5.1 spec tuning seq.\n");
    INFO("\t-h : show this massage and exit\n");
    INFO("\t-l # : loop times, default 1\n");
    INFO("\t-s # : test size, default same as ddr\n");
    INFO("\n");
    INFO("  buffer size trailing with b|k|m or not, and should be power of 2\n");
    INFO("\n");
    INFO("  rwdev support 3 actions:\n");
    INFO("  1. write: write in combination of buffer size and data.\n");
    INFO("  2. read: just do read in buffer size.\n");
    INFO("  3. verify: check data after reading.\n");
    INFO("\n");
    INFO("  support several -b|-f, like -b 512k -b 1m -d 0x55aa -d 0xff\n");
    INFO("  which means that the follow combinations will be test:\n");
    INFO("      1. 512K with data 0x55aa\n");
    INFO("      2. 512K with data 0xff\n");
    INFO("      3. 1M with data 0x55aa\n");
    INFO("      4. 1M with data 0xff\n");
    INFO("\n");
    INFO("  default data base on spec of emmc5.0, about sampling tuning\n");
    INFO("  sequence for HS200 and 8 bit mode\n");
}

void hexdump(char *buf, unsigned long bytes)
{
    unsigned long cnt = 0;
    DEBUG("\n");
    while (cnt < bytes) {
        DEBUG("%.2X", buf[cnt]);
        if (++cnt % 16 == 0) {
            DEBUG("\n");
        } else {
            DEBUG(" ");
        }
    }
    if (cnt % 16 != 0) {
        DEBUG("\n");
    }
}

int clean_mem(void)
{
    int ret = -1;
    int fd = open(CLEANMEM, O_WRONLY);
    if (fd < 0)
        goto err;

    if (write(fd, "3", 1) != 1)
        goto err;

    ret = 0;
err:
    close(fd);
    return ret;
}

void print_head()
{
    time_t t = time(NULL);
    INFO("\n\trwdev: do write/read/verify for devices\n\n");
    INFO("\tversion: %s\n", VERSION);
    INFO("\tbuild: %s\n", COMPILE);
    INFO("\tdate: %s\n", ctime(&t));
    /* action */
    {
        INFO("\taction\t:");
        if (task.action & ACTION_WRITE)
            INFO(" write");
        if (task.action & ACTION_READ)
            INFO(" read");
        if (task.action & ACTION_VERIFY)
            INFO(" verify");
        INFO("\n");
    }
    /* data */
    {
        INFO("\tdata\t:");
        if (task.def_data == true) {
            INFO(" DEFAULT TUNING SEQUENCE");
        } else {
            struct data_list *data;
            list_for_each_entry(data, &task.data_head, lnode)
                INFO(" %s", data->data);
        }
        INFO("\n");
    }
    /* buffer */
    {
        INFO("\tbuffer\t:");
        struct size_list *size;
        list_for_each_entry(size, &task.size_head, lnode)
            INFO(" %lu%s", size->size.size, size->size.units);
        INFO("\n");
    }
    /* loop */
    INFO("\tloop\t: %d\n", task.times);
    /* device */
    INFO("\tdev\t: %s\n", task.dev);
    /* test size */
    INFO("\tsize\t: %ld%s\n", task.testsize.size, task.testsize.units);

    INFO("\n");
}

int do_write()
{
    int fd;
    unit *bufsize = &task.size->size;
    unsigned long bytes, sum = task.testsize.sumbytes;

    clean_mem();
    /* init speed */
    if (speed_start())
        goto err;
    /* open */
    fd = open(task.dev, O_WRONLY | O_SYNC | O_CREAT, 0777);
    if (fd < 0) {
        ERROR("FAILED: open - %s\n", strerror(errno));
        goto err;
    }
    /* write */
    do {
        bytes = min(sum, bufsize->sumbytes);
        if (write(fd, task.buf, bytes) != bytes) {
            ERROR("FAILED: write - %s\n", strerror(errno));
            goto err;
        }
        sum -= bytes;
        hexdump(task.buf, bytes);
    } while (sum > 0L);
    /* close */
    fsync(fd);
    close(fd);
    /* end speed */
    if ((task.speed = speed_end(task.testsize.sumbytes / KB)) < 0)
        goto err;

    return 0;
err:
    return -1;
}

int do_read()
{
    int fd;
    unit *bufsize = &task.size->size;
    unsigned long bytes, sum = task.testsize.sumbytes;

    memset(task.buf, 0, bufsize->sumbytes);
    clean_mem();
    /* init speed */
    if (speed_start())
        goto err;
    /* open */
    fd = open(task.dev, O_RDONLY | O_RSYNC | O_SYNC);
    if (fd < 0) {
        ERROR("FAILED: open - %s\n", strerror(errno));
        goto err;
    }
    /* read */
    do {
        bytes = min(sum, bufsize->sumbytes);
        if (read(fd, task.buf, bytes) != bytes) {
            ERROR("FAILED: read - %s\n", strerror(errno));
            goto err;
        }
        sum -= bytes;
        hexdump(task.buf, bytes);
    } while (sum > 0L);
    /* close */
    close(fd);
    /* end speed */
    if ((task.speed = speed_end(task.testsize.sumbytes / KB)) < 0)
        goto err;

    return 0;
err:
    return -1;
}

int get_write_crc(unsigned short *crc)
{
    unit *bufsize = &task.size->size;
    unsigned long bytes, sum = task.testsize.sumbytes;
    unsigned short wr_crc;

    /* crc start */
    wr_crc = get_crc16_start();

    /* get crc */
    do {
        bytes = min(sum, bufsize->sumbytes);
        sum -= bytes;
        wr_crc = get_crc16_continue(wr_crc, task.buf, bytes);
    } while (sum > 0L);

    /* crc end */
    *crc = get_crc16_end(wr_crc);
    return 0;
}

int get_read_crc(unsigned short *crc)
{
    int fd;
    unit *bufsize = &task.size->size;
    unsigned long bytes, sum = task.testsize.sumbytes;
    unsigned short rd_crc;

    memset(task.buf, 0, bufsize->sumbytes);
    clean_mem();
    /* crc start */
    rd_crc = get_crc16_start();
    /* open */
    fd = open(task.dev, O_RDONLY | O_RSYNC | O_SYNC);
    if (fd < 0) {
        ERROR("FAILED: open - %s\n", strerror(errno));
        goto err;
    }
    /* read */
    do {
        bytes = min(sum, bufsize->sumbytes);
        if (read(fd, task.buf, bytes) != bytes) {
            ERROR("FAILED: read - %s\n", strerror(errno));
            goto err;
        }
        sum -= bytes;
        /* get crc */
        rd_crc = get_crc16_continue(rd_crc, task.buf, bytes);

        hexdump(task.buf, bytes);

    } while (sum > 0L);
    /* close */
    close(fd);
    /* crc end */
    *crc = get_crc16_end(rd_crc);

    return 0;
err:
    return -1;
}

int do_verify()
{
    if (get_write_crc(&task.wr_crc)) {
        ERROR("FAILED: get write crc failed\n");
        return -1;
    }

    if (get_read_crc(&task.rd_crc)) {
        ERROR("FAILED: get read crc failed\n");
        return -1;
    }

    if (task.rd_crc != task.wr_crc) {
        ERROR("FAILED: diff crc - (%d != %d)\n", task.wr_crc, task.rd_crc);
        return -1;
    }
    return 0;
}


void fill_data_raw(unsigned long size, const char *data, unsigned long len)
{
    unsigned long s = size;
    char *buf = task.buf;
    do {
        memcpy(buf, data, min(s, len));
        s -= min(s, len);
        buf += min(s, len);
    } while (s > 0);
}

static inline int hex_index(char ch)
{
    if(ch >= '0' && ch <= '9')
    {
        return ch - '0';
    }
    if(ch >= 'A' && ch <='F')
    {
        return ch - 'A' + 10;
    }
    if(ch >= 'a' && ch <= 'f')
    {
        return ch - 'a' + 10;
    }
    return -1;
}

int fill_data(struct size_list *size, struct data_list *data)
{
    int ret = -1;
    unsigned long b, i, t, slen = strlen(data->data);
    /* 进一除法：e = (a + (b + 1)) / b */
    unsigned long bytes = ((slen - 2) + ( 2 - 1 )) / 2;
    char *cdata = calloc(sizeof(char), bytes);;
    if (!cdata) {
        ERROR("Calloc failed - %s\n", strerror(errno));
        return ret;
    }

    for (b = bytes, t = 1, i = slen - 1; i >= 2; i--) {
        int tmp = hex_index(*(data->data + i));
        if (tmp == -1) {
            ERROR("Invalid hex data: %c(%s)\n", *(data->data + i),  data->data);
            goto err;
        }
        cdata[b - 1] += (char)(t * tmp);

        if (t == 1) {
            t *= 16;
        } else {
            t = 1;
            b--;
            if (b < 0) {
                ERROR("data too long\n");
                goto err;
            }
        }
    }

    fill_data_raw(size->size.sumbytes, cdata, bytes);
    ret = 0;
err:
    free(cdata);
    return ret;
}

int begin_do()
{
    INFO("\n");

    if (task.def_data == true) {
        unit *size = &task.size->size;
        INFO("\t=== [%u] buf: %lu%s  data: DEFAULT ===\n",
                task.times, size->size, size->units);
    } else {
        char *data = task.data->data;
        unit *size = &task.size->size;
        INFO("\t=== [%u] buf: %lu%s  data: %s ===\n",
                task.times, size->size, size->units, data);
    }

    if (task.action & ACTION_WRITE) {
        INFO("\twrite\t... ");
        if (do_write())
            goto err;
        INFO("OK (%.2f KB/s)\n", task.speed);
    }

    if (task.action & ACTION_VERIFY || task.action & ACTION_READ) {
        INFO("\tread\t... ");
        if (do_read())
            goto err;
        INFO("OK (%.2f KB/s)\n", task.speed);
    }

    if (task.action & ACTION_VERIFY) {
        INFO("\tverify\t... ");
        if (do_verify())
            goto err;
        INFO("OK\n");
    }

    return 0;
err:
    return -1;
}

/*
 * begin to do write/read/verify test
 *
 * return 0 if successfully, others for failed.
 */
int begin(void)
{
    print_head();
    /* time loop */
    while (task.times-- > 0) {
        /* size loop */
        list_for_each_entry(task.size, &task.size_head, lnode) {
            task.buf = malloc(task.size->size.sumbytes);
            if (!task.buf) {
                ERROR("Malloc failed - %s\n", strerror(errno));
                goto err;
            }
            if (task.def_data == true) {
                fill_data_raw(task.size->size.sumbytes, DEFAULT_DATA,
                        (unsigned long)sizeof(DEFAULT_DATA));
                if (begin_do())
                    goto err;
            } else {
                /* data loop */
                list_for_each_entry(task.data, &task.data_head, lnode) {
                    if (fill_data(task.size, task.data))
                        goto err;
                    if (begin_do())
                        goto err;
                }
            }
            free(task.buf);
        }
    }

    return 0;
err:
    return -1;
}

int set_size(unit *unit, const char *str)
{
    unsigned long long size = 0;
    int len = strlen(str);

    size=atol(str);
    if(str[len-1] == 'k' || str[len-1] == 'K')
        unit->sumbytes = size * KB;
    else if (str[len-1] == 'm' || str[len-1] == 'M')
        unit->sumbytes = size * MB;
    else if (str[len-1] == 'g' || str[len-1] == 'G')
        unit->sumbytes = size * GB;
    else if (str[len-1] >= '0' && str[len-1] <= '9' && is_power_of_2(size))
        unit->sumbytes = size;
    else
        return -1;

    if (unit->sumbytes >= GB) {
        unit->size = unit->sumbytes / GB;
        strcpy(unit->units, "GB");
    } else if (unit->sumbytes >= MB) {
        unit->size = unit->sumbytes / MB;
        strcpy(unit->units, "MB");
    } else if (unit->sumbytes >= KB) {
        unit->size = unit->sumbytes / KB;
        strcpy(unit->units, "KB");
    } else {
        unit->size = unit->sumbytes;
        strcpy(unit->units, "B");
    }
    return 0;
}

char *get_abs_path(const char *file)
{
    char *dup, *dir, *base, *old, *new, *ret = NULL;

    /* old should free */
    old = getcwd(NULL, 0);
    if (!old)
        return NULL;
    /* dup should free */
    dup = strdup(file);
    if (!dup)
        goto free_old;
    /* base should NOT free */
    base = basename(dup);
    if (!base)
        goto free_dup;
    /* dir should NOT free */
    dir = dirname(dup);
    if (!dir)
        goto free_dup;

    if (chdir(dir) < 0)
        goto free_dup;
    new = getcwd(NULL, 0);
    if (!new)
        goto free_new;
    ret = malloc(strlen(new) + strlen(base) + 2);
    if (!ret)
        goto free_new;
    if (snprintf(ret, strlen(new) + strlen(base) + 2, "%s%s%s",
                new, new[strlen(new) - 1] == '/' ? "" : "/", base) < 0)
            goto err;

    goto free_new;
err:
    free(ret);
    ret = NULL;
free_new:
    chdir(new);
free_dup:
    free(dup);
free_old:
    free(old);
    return ret;
}

unsigned long long get_total_mem()
{
    unsigned long long size_kb;
    FILE *fp = fopen(MEMINFO, "r");
    if (fp == NULL)
        return 0;
    fscanf(fp, "%*s%llu%*s\n", &size_kb);
    fclose(fp);
    return size_kb;
}

int main(int argc, char **argv)
{
    int opts = 0;
    setvbuf(stdout, NULL, _IONBF, 0);
    INIT_LIST_HEAD(&task.data_head);
    INIT_LIST_HEAD(&task.size_head);

    while ((opts = getopt(argc, argv, ":b:s:d:a:l:h")) != EOF) {
        switch (opts) {
        case 'a':
            {
                int action = (unsigned int)atoi(optarg);
                switch (action) {
                    case 0:
                        task.action |= ACTION_WRITE;
                        break;
                    case 1:
                        task.action |= ACTION_READ;
                        break;
                    case 2:
                        task.action |= ACTION_VERIFY;
                        break;
                    default:
                        ERROR("Invalid action: %s\n", optarg);
                        goto err;
                }
                break;
            }
        case 'b':
            {
                struct size_list *new = malloc(sizeof(struct size_list));
                if (!new) {
                    ERROR("Malloc failed - %s\n", strerror(errno));
                    goto err;
                }
                if (set_size(&new->size, optarg)) {
                    ERROR("Invalid size %s\n", optarg);
                    free(new);
                    goto err;
                }
                list_add_tail(&new->lnode, &task.size_head);
                break;
            }
        case 'd':
            {
                struct data_list *new = malloc(sizeof(struct data_list));
                if (!new) {
                    ERROR("malloc failed - %s\n", strerror(errno));
                    goto err;
                }
                if (strncmp(optarg, "0x", 2)) {
                    ERROR("Invalid data %s, which should begin with '0x'\n", optarg);
                    free(new);
                    goto err;
                }
                new->data = optarg;
                list_add_tail(&new->lnode, &task.data_head);
                break;
            }
        case 's':
            if (set_size(&task.testsize, optarg)) {
                ERROR("Invalid size %s\n", optarg);
                goto err;
            }
            break;
        case 'l':
            task.times = (unsigned int)atoi(optarg);
            break;
        case '?':
            ERROR("Invalid option\n");
            goto err;
        case ':':
            ERROR("Option requires an argument\n");
            goto err;
        case 'h':
            help();
            exit(0);
        }
    }

    argc -= optind;
    argv += optind;

    /* device file */
    if (1 > argc) {
        ERROR("  Please point out device file\n\n");
        help();
        goto err;
    } else if (1 < argc) {
        ERROR("  Too much device file\n");
        goto err;
    } else {
        task.dev = get_abs_path(argv[0]);
        if (!task.dev) {
            ERROR("get %s failed\n", argv[0]);
            goto err;
        }
        if (!strncmp(task.dev, "/dev", 4)) {
            struct stat s;
            if (stat(task.dev, &s)) {
                ERROR("stat failed - %s\n", strerror(errno));
                goto err;
            }
            if (S_ISBLK(s.st_mode) == false) {
                ERROR("not block %s\n", task.dev);
                goto err;
            }
        } else {
            struct stat s;
            if (!stat(task.dev, &s) && !S_ISREG(s.st_mode)) {
                ERROR("not file %s\n", task.dev);
                goto err;
            }
        }
    }

    /* action */
    if (!task.action)
        task.action = ACTION_WRITE | ACTION_READ | ACTION_VERIFY;

    /* buffer size */
    if (list_empty(&task.size_head) == true) {
        struct size_list *new = malloc(sizeof(struct size_list));
        if (!new) {
            ERROR("Malloc failed - %s\n", strerror(errno));
            goto err;
        }
        if (set_size(&new->size, DEFAULT_BUFFER_SIZE)) {
            ERROR("Invalid size %s\n", optarg);
            free(new);
            goto err;
        }
        list_add_tail(&new->lnode, &task.size_head);
    }

    /* test size */
    if (!task.testsize.sumbytes) {
        unsigned long long ddr = get_total_mem();
        char buf[100] = {0};
        snprintf(buf, 100, "%lluK", ddr);
        set_size(&task.testsize, buf);
    }

    /* data */
    if (list_empty(&task.data_head) == true)
        task.def_data = true;

    /* loop times */
    if (!task.times)
        task.times = DEFAULT_LOOP_TIME;

    return begin();
err:
    return -1;
}
