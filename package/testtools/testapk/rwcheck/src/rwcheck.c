#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <sys/vfs.h>

#include "log.h"
#include "crc16.h"
#include "rwcheck.h"
#include "fops.h"

#define RANDOM_DATA "/dev/urandom"
#define DEFAULT_INPUT "rwcheck.org"
#define DEFAULT_INPUT_SIZE (13 * 1024)
#define MEMINFO "/proc/meminfo"
#define CLEANMEM "/proc/sys/vm/drop_caches"
#define DEFAULT_FILE_NAME "rwcheck.tmp"
#define DEFAULT_PERCENT 95
#define DEFAULT_TIMES 1
#define DEFAULT_SMALL_BUF_SIZE "512K"
#define DEFAULT_LARGE_BUF_SIZE "8M"
#define DEFAULT_UP_MODE_BEGIN_SIZE "4K"
#define DEFAULT_UP_MODE_END_SIZE "10G"
#define FAT_MAX_SIZE "4G"
#define FAT_MAX_SIZE_BYTES ((unsigned long long)4 * 1024 * 1024 * 1024)
#define is_power_of_2(val) !(val & (val - 1))
#define MAX_JOBS 5

#define FS_FAT_MAGIC 0x4d44
#define FS_EXT_MAGIC 0xEF53
#define FS_JFFS2_MAGIC 0x72b6
#define FS_NTFS_MAGIC 0x5346544e
#define FS_TMPFS_MAGIC 0x01021994

struct task task = {0};

struct fs_t fs_type[] = {
    {FS_FAT_MAGIC, "fat"},
    {FS_EXT_MAGIC, "ext"},
    {FS_JFFS2_MAGIC, "jffs2"},
    {FS_NTFS_MAGIC, "ntfs"},
    {FS_TMPFS_MAGIC, "tmpfs"},
    {0, "none"}
};

char str_mode[][20] = {
    "auto mode",
    "fixed mode",
    "increasing mode",
};

int set_size(struct size_unit *unit, const char *str)
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
        strncpy(unit->units, "GB", 2);
    } else if (unit->sumbytes >= MB) {
        unit->size = unit->sumbytes / MB;
        strncpy(unit->units, "MB", 2);
    } else if (unit->sumbytes >= KB) {
        unit->size = unit->sumbytes / KB;
        strncpy(unit->units, "KB", 2);
    } else {
        unit->size = unit->sumbytes;
        strncpy(unit->units, "B", 1);
    }
    return 0;
}

void help(void)
{
    INFO("  Usage: rwcheck [-h] [-d dir] [-t times] [-b size] [-e size]\n");
    INFO("               [-s size] [-u size] [-p percent] [-i input] [-j jobs]\n");
    INFO("\n");
    INFO("\t-h : show this massage and exit\n");
    INFO("\t-d # : the diretory to check [default currect path]\n");
    INFO("\t-t # : check times\n");
    INFO("\t-b # : set begin size\n");
    INFO("\t-e # : set end size\n");
    INFO("\t-s # : set file size\n");
    INFO("\t-u # : set read/write buf size\n");
    INFO("\t-p # : set maximum ratio of total flash size to check. Eg. -p 95\n");
    INFO("\t-i # : input path of file [default <check_dir>/rwcheck.org]\n");
    INFO("\t       if file don't existed, copy 13K from /dev/urandom\n");
    INFO("\t-j # : multiple jobs\n");
    INFO("\n");
    INFO("  size trailing with k|m|g or not, and should be power of 2\n");
    INFO("\n");
    INFO("  rwcheck work in 3 mode:\n");
    INFO("  1. -s # : file in the same size\n");
    INFO("  2. -b # | -e # : file size increase by The multiplier of 2\n");
    INFO("  3. none : file size is 50%% of the free space, minimum 4k\n");
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

unsigned long long get_free_mem()
{
    unsigned long long free_kb = 0;
    char buf[1024]= {0}, *p;
    FILE *fp;

    /*
     * we must sync and clean cache before
     */
    sync();
    clean_mem();

    fp = fopen(MEMINFO, "r");
    if (fp == NULL)
        return 0;
    fread(buf, 1, 1024, fp);
    p = strstr(buf, "MemFree");
    if (p == NULL) {
        ERROR("mismatch MemFree in %s\n", MEMINFO);
        goto err;
    }
    if (sscanf(p, "%*s%llu%*s", &free_kb) != 1) {
        ERRNO("sscanf %s failed\n", MEMINFO);
        free_kb = errno;
    }

err:
    fclose(fp);
    return  free_kb;
}

unsigned long long get_free_flash()
{
    struct statfs s;
    if (statfs(task.checkdir, &s) < 0)
        goto err;
    return s.f_bsize * s.f_bavail;
err:
    return 0;
}

unsigned long long get_total_flash()
{
    struct statfs s;
    if (statfs(task.checkdir, &s) < 0)
        goto err;
    return s.f_bsize * s.f_blocks;
err:
    return 0;
}

unsigned long long get_test_size()
{
    /* We should get real size here */
    unsigned long long free_bytes = get_free_flash();
    unsigned long long test_bytes = 0;

    if (free_bytes == 0)
        goto end;
    if (free_bytes <= task.min_free_b)
        goto end;

    switch (task.mode) {
    case auto_mode:
        DEBUG("mode: auto mode\n");
        if (free_bytes < 64 * KB) {
            DEBUG("free (%llu B) < 64K, end!\n", free_bytes);
            goto end;
        }
        test_bytes = free_bytes / 2 / 1024 * 1024;
        break;
    case fixed_mode:
        DEBUG("mode: fix mode\n");
        if (free_bytes <= task.fixed_size.sumbytes) {
            DEBUG("free (%llu B) < set (%llu B), end!\n",
                    free_bytes, task.fixed_size.sumbytes);
            goto end;
        }
        test_bytes = task.fixed_size.sumbytes;
        break;
    case up_mode:
        DEBUG("mode: up mode\n");
        /* we use fixed_size as currect testing size in up mode */
        if (task.fixed_size.sumbytes == 0) {
            task.fixed_size.sumbytes = task.begin_size.sumbytes;
            DEBUG("initial : %llu B\n", task.fixed_size.sumbytes);
        } else {
            task.fixed_size.sumbytes *= 2;
            DEBUG("initial : %llu B\n", task.fixed_size.sumbytes);
        }
        /* larger than end size, reset */
        if (task.fixed_size.sumbytes > task.end_size.sumbytes) {
            DEBUG("size (%llu B) > end size (%llu B), reset!\n",
                    task.fixed_size.sumbytes, task.end_size.sumbytes);
            task.fixed_size.sumbytes = task.begin_size.sumbytes;
            DEBUG("reset : %llu B\n", task.fixed_size.sumbytes);
        }
        /* larger than free size, reset */
        if (task.fixed_size.sumbytes >= free_bytes) {
            DEBUG("size (%llu B) >= free size (%llu B), reset!\n",
                    task.fixed_size.sumbytes, free_bytes);
            task.fixed_size.sumbytes = task.begin_size.sumbytes;
            DEBUG("reset : %llu B\n", task.fixed_size.sumbytes);
        }
        /* larger than limit size, reset */
        if (task.min_free_b > free_bytes - task.fixed_size.sumbytes) {
            DEBUG("done size (%llu B) < limit size (%llu B), reset!\n",
                    free_bytes - task.fixed_size.sumbytes, task.min_free_b);
            task.fixed_size.sumbytes = task.begin_size.sumbytes;
            DEBUG("reset : %llu B\n", task.fixed_size.sumbytes);
        }
        test_bytes = task.fixed_size.sumbytes;
        break;
    }

    DEBUG("size : %llu B\n", test_bytes);
    if ((long long)task.min_free_b > (long long)(free_bytes - test_bytes)) {
        DEBUG("done free (%lld B) touch deadline (%lld B), end!\n",
                (long long)(free_bytes - test_bytes), (long long)task.min_free_b);
        goto end;
    }

    /* fat max file size is 4G */
    if (task.fs_type->magic == FS_FAT_MAGIC && test_bytes > FAT_MAX_SIZE_BYTES) {
        DEBUG("size (%llu B) touch fat32 max(%llu B), end!\n",
                free_bytes, FAT_MAX_SIZE_BYTES);
        /* vfat should not larger than 4G(4294967296)
         * but i don't know why that allway end in 4294967295.
         * so, for fat fs, we create (4294967296 - 2)
         */
        test_bytes = FAT_MAX_SIZE_BYTES - 2;
    }

    DEBUG("SIZE %llu B\n", test_bytes);
    return test_bytes;
end:
    return 0;
}

void print_head(void)
{
    time_t t = time(NULL);
    INFO("\n\trwcheck: do read and write check\n\n");
    INFO("\tversion: %s\n", VERSION);
    INFO("\tbuild: %s\n", COMPILE);
    INFO("\tdate: %s\n", ctime(&t));
    INFO("\tfree/total ddr %llu/%llu MB\n", task.free_mem_mb, task.total_mem_mb);
    INFO("\tfree/total flash %llu/%llu KB\n", task.free_flash_kb, task.total_flash_kb);
    if (task.fs_type->magic == FS_FAT_MAGIC) {
        INFO("\tflash filesystem %s (limit MAX file size to 4G)\n", task.fs_type->str);
    } else {
        INFO("\tflash filesystem %s\n", task.fs_type->str);
    }
    INFO("\tset mode to %s\n", str_mode[task.mode]);
    if (task.mode == fixed_mode) {
        INFO("\tset file size to %llu%s\n",
                task.fixed_size.size, task.fixed_size.units);
    } else if (task.mode == up_mode) {
        INFO("\tset file begin size to %llu%s\n",
                task.begin_size.size, task.begin_size.units);
        INFO("\tset file end size to %llu%s\n",
                task.end_size.size, task.end_size.units);
    }
    INFO("\tset times to %u\n", task.times);
    INFO("\tset max percent of free space to %u%%\n", task.percent);
    INFO("\tset buf size to %llu%s\n",
            task.buf_size.size, task.buf_size.units);
    INFO("\tset check diretory as %s\n", task.checkdir);
    INFO("\tset input file as %s\n", task.infile);
    INFO("\tset jobs as %d\n", task.jobs);
}

int do_check()
{
    int num = 0;
    unsigned short crc;
    char testfile[100];
    int ret = -1, jobs;

    /* do clean memory before check to ensure read data from flash */
    clean_mem();

    while(1) {
        for (jobs = 0; jobs < task.jobs; jobs++) {
            if (task.jobs == 1)
                snprintf(testfile, 100, "%s/%s.%d",
                    task.checkdir, DEFAULT_FILE_NAME, num);
            else
                snprintf(testfile, 100, "%s/%s.%d_%d",
                    task.checkdir, DEFAULT_FILE_NAME, num, jobs);

            if (fops_is_existed(testfile) == false)
                return 0;

            INFO("\tcheck\t: %s ... ", testfile);
            ret = get_crc16_path(testfile, &crc);
            if (ret)
                return -1;
            if (check_crc16_by_crc(crc) == false) {
                if (task.jobs == 1)
                    snprintf(testfile, 100, "%s/%s.%d",
                        task.checkdir, DEFAULT_FILE_NAME, num + 1);
                else
                    snprintf(testfile, 100, "%s/%s.%d_%d",
                        task.checkdir, DEFAULT_FILE_NAME, num + 1, jobs);
                if (fops_is_existed(testfile) == true) {
                    ERROR("FAILED: crc error\n");
                    return -1;
                }
                INFO("OK: ignore the last one\n");
            } else {
                INFO("OK\n");
            }
        }
        num++;
    }
}

int do_remove()
{
    int num = 0, jobs;
    char testfile[100];

    while (1) {
        for (jobs = 0; jobs < task.jobs; jobs++) {
            if (task.jobs == 1)
                snprintf(testfile, 100, "%s/%s.%d",
                    task.checkdir, DEFAULT_FILE_NAME, num);
            else
                snprintf(testfile, 100, "%s/%s.%d_%d",
                    task.checkdir, DEFAULT_FILE_NAME, num, jobs);

            if (fops_is_existed(testfile) == false)
                goto out;

            INFO("\tremove\t: %s ... ", testfile);
            if (remove((const char *)testfile) != 0) {
                ERRNO("\n");
                return -1;
            }
            INFO("OK\n");
        }
        num++;
    }

out:
    sync();
    /* temp file existed, we reset min free after removing temp file */
    if (task.min_free_b == 0) {
        task.min_free_b = get_total_flash() * (100 - task.percent) / 100;
        if (task.min_free_b == 0) {
            ERROR("get free flash failed or no enough free space\n");
            return -1;
        }
    }

    return 0;
}

struct pthread_arg {
    char path[100];
    unsigned long long bytes;
    int ret;
};

static void *_do_create(void *_arg)
{
    struct pthread_arg *arg = (struct pthread_arg *)_arg;
    unsigned long long bytes = arg->bytes;
    char *path = arg->path;
    int ret = -1;

    if (fops_is_existed(path) == true)
	    goto err;
    INFO("\r\tcreate\t: %s ... ", path);
    ret = fops_create_file(task.infile, path, bytes,
                    task.buf_size.sumbytes);
    if (ret)
        goto err;
    /* the last 2 btyes are reserved for saving crc */
    ret = fops_fill_crc(path);
    if (ret)
	    goto err;

    INFO("\r\tcreate\t: %s ... OK (%lluK)\n", path, bytes / KB);
err:
    arg->ret = ret;
    return arg;
}

int do_create()
{
    int num = 0;
    unsigned long long size;
    int ret, jobs;
    pthread_t pid[MAX_JOBS] = {0};
    struct pthread_arg parg[MAX_JOBS];

    /* reset for the next loop time */
    if (task.mode == up_mode)
        task.fixed_size.sumbytes = 0;

    if (task.min_free_b && get_free_flash() <= task.min_free_b)
        INFO("\tAlready used over %d%%\n", task.percent);

    while ((size = get_test_size())) {
        for (jobs = 0; jobs < task.jobs; jobs++) {
            parg[jobs].bytes = size / task.jobs;
            if (task.jobs == 1)
                snprintf(parg[jobs].path, 100, "%s/%s.%d",
                    task.checkdir, DEFAULT_FILE_NAME, num);
            else
                snprintf(parg[jobs].path, 100, "%s/%s.%d_%d",
                    task.checkdir, DEFAULT_FILE_NAME, num, jobs);

            if (fops_is_existed(parg[jobs].path) == true)
                return 0;

            ret = pthread_create(pid + jobs, NULL, _do_create,
                    (void *)(parg + jobs));
            if (ret) {
                ERRNO("pthread create failed\n");
                goto err;
            }
        }
        for (jobs = 0; jobs < task.jobs; jobs++) {
            if (!pid[jobs])
                break;
            ret = pthread_join(pid[jobs], NULL);
            pid[jobs] = 0;
            if (ret || parg[jobs].ret)
                goto err;
        }
        num++;
    }

    return 0;
err:
    for (jobs = 0; jobs < task.jobs; jobs++) {
        if (!pid[jobs])
            continue;
        pthread_cancel(pid[jobs]);
    }
    return -1;
}

int init_input_file(void)
{
    int ret;
    struct stat s;

    if (!task.infile)
        return -1;

    if (task.with_args_infile)
        return 0;

    ret = stat(task.infile, &s);
    if (ret && errno == ENOENT) {
        /* file not existed, create one */
        ret = fops_create_file(RANDOM_DATA, task.infile,
                DEFAULT_INPUT_SIZE, 4 * 1024);
        if (ret)
            return ret;
    } else if (ret) {
        ERRNO("stat %s failed\n", task.infile);
        return ret;
    } else {
        if (S_ISDIR(s.st_mode)) {
            ERROR("%s is a diretory\n", optarg);
            return -1;
        }
    }
    return 0;
}

/*
 * begin to do read/write test
 *
 * return 0 if successfully, others for failed.
 */
int begin(void)
{
    int ret = 0;

    print_head();
    while (task.times-- > 0) {
        INFO("\n");

        ret = init_input_file();
        if (ret)
            return ret;

        INFO("\t--- CREATE ---\n");
        ret = do_create();
        if (ret)
            return ret;

        INFO("\t--- CHECK ---\n");
        ret = do_check();
        if (ret)
            return ret;

        INFO("\t--- REMOVE ---\n");
        ret = do_remove();
        if (ret)
            return ret;

        /*
         * we remove old source to ensure data difference each time
         * do not remove before create to avoid destorying problem scene
         */
        if (!task.with_args_infile)
            remove(task.infile);
    }

    return 0;
}

char *get_absolute_path(const char *path)
{
    char *ret = NULL;
    char *pwd;

    pwd = getcwd(NULL, 0);
    if (!pwd)
        return NULL;
    if (chdir(path) < 0)
        goto out;
    ret = getcwd(NULL, 0);
    if (chdir(pwd) < 0) {
        free(ret);
        ret = NULL;
    }

out:
    free(pwd);
    return ret;
}

const struct fs_t *get_fs_type(void)
{
    struct statfs s;
    if (statfs(task.checkdir, &s) < 0)
        goto err;
    for (int cnt = 0; cnt < ARRAY_SIZE(fs_type); cnt++) {
        if (s.f_type == fs_type[cnt].magic)
            return fs_type + cnt;
    }
err:
    return fs_type + ARRAY_SIZE(fs_type) - 1;
}

int main(int argc, char **argv)
{
    int opts = 0;
    setvbuf(stdout, NULL, _IONBF, 0);
    while ((opts = getopt(argc, argv, ":d:t:b:e:s:u:p:hi:j:")) != EOF) {
        switch (opts) {
        case 'd':
            {
                struct stat s;
                if (stat(optarg, &s) < 0) {
                    ERRNO("not found %s\n", optarg);
                    goto err;
                }

                if (S_ISDIR(s.st_mode) == false) {
                    ERROR("%s is not diretory\n", optarg);
                    goto err;
                }
                task.checkdir = get_absolute_path(optarg);
                if (!task.checkdir) {
                    ERRNO("get absolute path from %s failed\n", optarg);
                    goto err;
                }
                break;
            }
        case 't':
            task.times = (unsigned int)atoi(optarg);
            break;
        case 'b':
            if (task.mode == fixed_mode) {
                ERROR("incompatible with -s and -b|-e\n");
                goto err;
            }
            if (set_size(&(task.begin_size), optarg) < 0) {
                ERROR("dentify size failed - %s\n", optarg);
                goto err;
            }
            task.mode = up_mode;
            break;
        case 'e':
            if (task.mode == fixed_mode) {
                ERROR("incompatible with -s and -b|-e\n");
                goto err;
            }
            if (set_size(&(task.end_size), optarg) < 0) {
                ERROR("dentify size failed - %s\n", optarg);
                goto err;
            }
            task.mode = up_mode;
            break;
        case 's':
            if (task.mode == up_mode) {
                ERROR("incompatible with -s and -b|-e\n");
                goto err;
            }
            if (set_size(&(task.fixed_size), optarg) < 0) {
                ERROR("dentify size failed - %s\n", optarg);
                goto err;
            }
            task.mode = fixed_mode;
            break;
        case 'u':
            if (set_size(&(task.buf_size), optarg) < 0) {
                ERROR("dentify size failed - %s\n", optarg);
                goto err;
            }
            break;
        case 'i':
            {
                struct stat s;
                if (stat(optarg, &s)) {
                    ERRNO("stat %s failed\n", optarg);
                    goto err;
                }
                if (S_ISDIR(s.st_mode)) {
                        ERROR("%s is a diretory\n", optarg);
                        goto err;
                }
                task.infile = optarg;
                task.with_args_infile = true;
            }
            break;
        case 'p':
            task.percent = atoi(optarg);
            break;
        case '?':
            ERROR("invalid option\n");
            help();
            goto err;
        case ':':
            ERROR("option requires an argument\n");
            help();
            goto err;
        case 'h':
            help();
            exit(0);
        case 'j':
            task.jobs = (int)atoi(optarg);
            if (task.jobs <= 0)
                ERROR("invalid argument for -j: %s\n", optarg);
            else if (task.jobs > MAX_JOBS)
                ERROR("-j should not lager than %d\n", MAX_JOBS);
            break;
        }
    }

    /* times */
    if (task.times == 0)
        task.times = DEFAULT_TIMES;
    /* checkdir */
    if (task.checkdir == NULL) {
        task.checkdir = get_absolute_path(".");
        if (!task.checkdir)
            goto err;
    }
    /* percent */
    if (task.percent == 0)
        task.percent = DEFAULT_PERCENT;
    /* input file */
    if (!task.infile) {
        int len = strlen(task.checkdir) + strlen(DEFAULT_INPUT) + 2;
        task.infile = malloc(len);
        if (!task.infile) {
            ERRNO("malloc fail\n");
            goto err;
        }
        snprintf(task.infile, len, "%s/%s", task.checkdir, DEFAULT_INPUT);
        task.with_args_infile = false;
    }
    /* fstype */
    task.fs_type = get_fs_type();
    /* total_flash_mb */
    task.total_flash_kb = get_total_flash() / KB;
    if (task.total_flash_kb == 0) {
        ERROR("get total flash failed\n");
        goto err;
    }
    /* free_flash_mb */
    task.free_flash_kb = get_free_flash() / KB;
    if (task.free_flash_kb == 0) {
        ERROR("get free flash failed\n");
        goto err;
    }
    /* min_free_b */
    {
        char testfile[100];
        snprintf(testfile, 100, "%s/%s.0", task.checkdir, DEFAULT_FILE_NAME);
        if (fops_is_existed(testfile) == true) {
            /**
             * If temp file here, we will get wrongly free size.
             * so, we set min_free_b as 0 and reset it after removing
             */
            task.min_free_b = 0;
        } else {
            task.min_free_b = get_total_flash() * (100 - task.percent) / 100;
            if (task.min_free_b == 0) {
                ERROR("get free flash failed or no enough free space\n");
                goto err;
            }
        }
    }
    /* total_mem_mb */
    task.total_mem_mb = get_total_mem() / 1024;
    if (task.total_mem_mb == 0) {
        ERROR("get total memory failed\n");
        goto err;
    }
    /* free_mem_mb */
    task.free_mem_mb = get_free_mem() / 1024;
    if (task.free_mem_mb == 0) {
        ERROR("get free memory failed\n");
        goto err;
    }
    /* jobs */
    if (!task.jobs || task.jobs < 0)
	    task.jobs = 1;
    if (task.jobs > MAX_JOBS)
	    task.jobs = MAX_JOBS;
    /* buf_size */
    if (task.buf_size.sumbytes == 0) {
        if (task.total_mem_mb > 64) {
            set_size(&(task.buf_size), DEFAULT_LARGE_BUF_SIZE);
        }
        else {
            set_size(&(task.buf_size), DEFAULT_SMALL_BUF_SIZE);
        }
    } else if (task.buf_size.sumbytes >= task.free_mem_mb * MB * 60 / 100) {
        ERROR("too large for buf\n");
        goto err;
    }
    /* begin_size & end_size */
    if (task.mode == up_mode) {
        if (task.begin_size.sumbytes == 0)
            set_size(&(task.begin_size), DEFAULT_UP_MODE_BEGIN_SIZE);
        if (task.end_size.sumbytes == 0)
            set_size(&(task.end_size), DEFAULT_UP_MODE_END_SIZE);
        if (task.begin_size.sumbytes > task.end_size.sumbytes) {
            ERROR("begin size %llu%s larger than end size %llu%s\n",
                    task.begin_size.size, task.begin_size.units,
                    task.end_size.size, task.end_size.units);
            goto err;
        }
    }
    /* fat max file size is 4G */
    if (task.fs_type->magic == FS_FAT_MAGIC) {
        if (task.mode == up_mode) {
            if (task.end_size.sumbytes > FAT_MAX_SIZE_BYTES)
                set_size(&(task.end_size), FAT_MAX_SIZE);
        } else if (task.mode == fixed_mode) {
            if (task.fixed_size.sumbytes > FAT_MAX_SIZE_BYTES)
                set_size(&(task.fixed_size), FAT_MAX_SIZE);
        }
    }

    return begin();
err:
    return -1;
}
